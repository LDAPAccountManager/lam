<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\RequestObject;

use function array_filter;
use function array_merge;
use Facile\OpenIDClient\AlgorithmManagerBuilder;
use function Facile\OpenIDClient\base64url_encode;
use Facile\OpenIDClient\Client\ClientInterface;
use Facile\OpenIDClient\Exception\RuntimeException;
use function Facile\OpenIDClient\jose_secret_key;
use function implode;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Core\JWKSet;
use Jose\Component\Encryption\Compression\CompressionMethodManager;
use Jose\Component\Encryption\Compression\Deflate;
use Jose\Component\Encryption\JWEBuilder;
use Jose\Component\Encryption\Serializer\CompactSerializer as EncryptionCompactSerializer;
use Jose\Component\Encryption\Serializer\JWESerializer;
use Jose\Component\Signature\JWSBuilder;
use Jose\Component\Signature\Serializer\CompactSerializer as SignatureCompactSerializer;
use Jose\Component\Signature\Serializer\JWSSerializer;
use function json_encode;
use function preg_match;
use function random_bytes;
use function strpos;
use function time;

class RequestObjectFactory
{
    /** @var AlgorithmManager */
    private $algorithmManager;

    /** @var JWSBuilder */
    private $jwsBuilder;

    /** @var JWEBuilder */
    private $jweBuilder;

    /** @var JWSSerializer */
    private $signatureSerializer;

    /** @var JWESerializer */
    private $encryptionSerializer;

    public function __construct(
        ?AlgorithmManager $algorithmManager = null,
        ?JWSBuilder $jwsBuilder = null,
        ?JWEBuilder $jweBuilder = null,
        ?JWSSerializer $signatureSerializer = null,
        ?JWESerializer $encryptionSerializer = null
    ) {
        $this->algorithmManager = $algorithmManager ?? (new AlgorithmManagerBuilder())->build();
        $this->jwsBuilder = $jwsBuilder ?? new JWSBuilder($this->algorithmManager);
        $this->jweBuilder = $jweBuilder ?? new JWEBuilder(
            $this->algorithmManager,
            $this->algorithmManager,
            new CompressionMethodManager([new Deflate()])
        );
        $this->signatureSerializer = $signatureSerializer ?? new SignatureCompactSerializer();
        $this->encryptionSerializer = $encryptionSerializer ?? new EncryptionCompactSerializer();
    }

    /**
     * @param array<string, mixed> $params
     */
    public function create(ClientInterface $client, array $params = []): string
    {
        $payload = $this->createPayload($client, $params);
        $signedToken = $this->createSignedToken($client, $payload);

        return $this->createEncryptedToken($client, $signedToken);
    }

    /**
     * @param array<string, mixed> $params
     */
    private function createPayload(ClientInterface $client, array $params = []): string
    {
        $metadata = $client->getMetadata();
        $issuer = $client->getIssuer();

        $payload = json_encode(array_merge($params, [
            'iss' => $metadata->getClientId(),
            'aud' => $issuer->getMetadata()->getIssuer(),
            'client_id' => $metadata->getClientId(),
            'jti' => base64url_encode(random_bytes(32)),
            'iat' => time(),
            'exp' => time() + 300,
        ]));

        if (false === $payload) {
            throw new RuntimeException('Unable to encode payload');
        }

        return $payload;
    }

    private function createSignedToken(ClientInterface $client, string $payload): string
    {
        $metadata = $client->getMetadata();

        /** @var string $alg */
        $alg = $metadata->get('request_object_signing_alg') ?? 'none';

        if ('none' === $alg) {
            return implode('.', [
                base64url_encode((string) json_encode(['alg' => $alg])),
                base64url_encode($payload),
                '',
            ]);
        }

        if (0 === strpos($alg, 'HS')) {
            $jwk = jose_secret_key($metadata->getClientSecret() ?? '');
        } else {
            $jwk = JWKSet::createFromKeyData($client->getJwksProvider()->getJwks())
                ->selectKey('sig', $this->algorithmManager->get($alg));
        }

        if (null === $jwk) {
            throw new RuntimeException('No key to sign with alg ' . $alg);
        }

        $ktyIsOct = $jwk->has('kty') && $jwk->get('kty') === 'oct';

        $header = array_filter([
            'alg' => $alg,
            'typ' => 'JWT',
            'kid' => ! $ktyIsOct && $jwk->has('kid') ? $jwk->get('kid') : null,
        ]);

        $jws = $this->jwsBuilder->create()
            ->withPayload($payload)
            ->addSignature($jwk, $header)
            ->build();

        return $this->signatureSerializer->serialize($jws, 0);
    }

    private function createEncryptedToken(ClientInterface $client, string $payload): string
    {
        $metadata = $client->getMetadata();

        /** @var null|string $alg */
        $alg = $metadata->get('request_object_encryption_alg');

        if (null === $alg) {
            return $payload;
        }

        /** @var null|string $enc */
        $enc = $metadata->get('request_object_encryption_enc');

        if ((bool) preg_match('/^(RSA|ECDH)/', $alg)) {
            $jwks = JWKSet::createFromKeyData($client->getIssuer()->getJwksProvider()->getJwks());
            $jwk = $jwks->selectKey('enc', $this->algorithmManager->get($alg));
        } else {
            $jwk = jose_secret_key(
                $metadata->getClientSecret() ?? '',
                'dir' === $alg ? $enc : $alg
            );
        }

        if (null === $jwk) {
            throw new RuntimeException('No key to encrypt with alg ' . $alg);
        }

        $ktyIsOct = $jwk->has('kty') && $jwk->get('kty') === 'oct';

        $header = array_filter([
            'alg' => $alg,
            'enc' => $enc,
            'cty' => 'JWT',
            'kid' => ! $ktyIsOct && $jwk->has('kid') ? $jwk->get('kid') : null,
        ]);

        $jwe = $this->jweBuilder->create()
            ->withPayload($payload)
            ->withSharedProtectedHeader($header)
            ->addRecipient($jwk)
            ->build();

        return $this->encryptionSerializer->serialize($jwe, 0);
    }
}
