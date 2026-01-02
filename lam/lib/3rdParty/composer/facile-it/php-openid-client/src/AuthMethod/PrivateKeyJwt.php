<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use Facile\OpenIDClient\AlgorithmManagerBuilder;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\RuntimeException;
use Jose\Component\Core\JWK;
use Jose\Component\Core\JWKSet;
use Jose\Component\Signature\JWSBuilder;
use Jose\Component\Signature\Serializer\CompactSerializer;
use Jose\Component\Signature\Serializer\JWSSerializer;
use Override;

use function array_merge;
use function Facile\OpenIDClient\base64url_encode;
use function Facile\OpenIDClient\get_endpoint_uri;
use function json_encode;
use function random_bytes;
use function time;

final class PrivateKeyJwt extends AbstractJwtAuth
{
    private JWSBuilder $jwsBuilder;

    private JWSSerializer $jwsSerializer;

    private ?JWK $jwk;

    private int $tokenTTL;

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

    #[Override]
    public function getSupportedMethod(): string
    {
        return 'private_key_jwt';
    }

    /**
     * @param array<string, mixed> $claims
     */
    #[Override]
    protected function createAuthJwt(OpenIDClient $client, array $claims = []): string
    {
        $clientId = $client->getMetadata()->getClientId();

        $jwk = $this->jwk ?? JWKSet::createFromKeyData($client->getJwksProvider()->getJwks())->selectKey('sig');

        if (null === $jwk) {
            throw new RuntimeException('Unable to get a client signature jwk');
        }

        $time = time();
        $jti = base64url_encode(random_bytes(32));

        $payload = json_encode(array_merge(
            $claims,
            [
                'iss' => $clientId,
                'sub' => $clientId,
                'aud' => get_endpoint_uri($client, 'token_endpoint'),
                'iat' => $time,
                'exp' => $time + $this->tokenTTL,
                'jti' => $jti,
            ]
        ), JSON_THROW_ON_ERROR);

        $jwkAlg = $jwk->get('alg');

        if (! is_string($jwkAlg)) {
            throw new RuntimeException('Invalid JWK `alg` value');
        }

        $jws = $this->jwsBuilder->create()
            ->withPayload($payload)
            ->addSignature($jwk, ['alg' => $jwkAlg, 'jti' => $jti])
            ->build();

        return $this->jwsSerializer->serialize($jws, 0);
    }
}
