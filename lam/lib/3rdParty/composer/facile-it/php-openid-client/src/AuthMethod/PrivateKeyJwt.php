<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use function array_merge;
use Facile\OpenIDClient\AlgorithmManagerBuilder;
use function Facile\OpenIDClient\base64url_encode;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\RuntimeException;
use Jose\Component\Core\JWK;
use Jose\Component\Core\JWKSet;
use Jose\Component\Signature\JWSBuilder;
use Jose\Component\Signature\Serializer\CompactSerializer;
use Jose\Component\Signature\Serializer\JWSSerializer;
use function json_encode;
use function random_bytes;
use function time;

final class PrivateKeyJwt extends AbstractJwtAuth
{
    /** @var JWSBuilder */
    private $jwsBuilder;

    /** @var JWSSerializer */
    private $jwsSerializer;

    /** @var null|JWK */
    private $jwk;

    /** @var int */
    private $tokenTTL;

    /**
     * PrivateKeyJwt constructor.
     */
    public function __construct(
        ?JWSBuilder $jwsBuilder = null,
        ?JWSSerializer $serializer = null,
        ?JWK $jwk = null,
        int $tokenTTL = 60
    ) {
        $this->jwsBuilder = $jwsBuilder ?? new JWSBuilder((new AlgorithmManagerBuilder())->build());
        $this->jwsSerializer = $serializer ?? new CompactSerializer();
        $this->jwk = $jwk;
        $this->tokenTTL = $tokenTTL;
    }

    public function getSupportedMethod(): string
    {
        return 'private_key_jwt';
    }

    /**
     * @param array<string, mixed> $claims
     */
    protected function createAuthJwt(OpenIDClient $client, array $claims = []): string
    {
        $issuer = $client->getIssuer();
        $issuerMetadata = $issuer->getMetadata();

        $clientId = $client->getMetadata()->getClientId();

        $jwk = $this->jwk ?? JWKSet::createFromKeyData($client->getJwksProvider()->getJwks())->selectKey('sig');

        if (null === $jwk) {
            throw new RuntimeException('Unable to get a client signature jwk');
        }

        $time = time();
        $jti = base64url_encode(random_bytes(32));

        /** @var string $payload */
        $payload = json_encode(array_merge(
            $claims,
            [
                'iss' => $clientId,
                'sub' => $clientId,
                'aud' => $issuerMetadata->getIssuer(),
                'iat' => $time,
                'exp' => $time + $this->tokenTTL,
                'jti' => $jti,
            ]
        ));

        $jws = $this->jwsBuilder->create()
            ->withPayload($payload)
            ->addSignature($jwk, ['alg' => $jwk->get('alg'), 'jti' => $jti])
            ->build();

        return $this->jwsSerializer->serialize($jws, 0);
    }
}
