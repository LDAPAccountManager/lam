<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use function class_exists;
use function Facile\OpenIDClient\base64url_encode;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Facile\OpenIDClient\Exception\LogicException;
use function Facile\OpenIDClient\jose_secret_key;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Signature\Algorithm\HS256;
use Jose\Component\Signature\JWSBuilder;
use Jose\Component\Signature\Serializer\CompactSerializer;
use Jose\Component\Signature\Serializer\JWSSerializer;
use function json_encode;
use function random_bytes;
use function time;

final class ClientSecretJwt extends AbstractJwtAuth
{
    /** @var null|JWSBuilder */
    private $jwsBuilder;

    /** @var JWSSerializer */
    private $jwsSerializer;

    /**
     * ClientSecretJwt constructor.
     */
    public function __construct(
        ?JWSBuilder $jwsBuilder = null,
        ?JWSSerializer $jwsSerializer = null
    ) {
        $this->jwsBuilder = $jwsBuilder;
        $this->jwsSerializer = $jwsSerializer ?? new CompactSerializer();
    }

    public function getSupportedMethod(): string
    {
        return 'client_secret_jwt';
    }

    private function getJwsBuilder(): JWSBuilder
    {
        if (null !== $this->jwsBuilder) {
            return $this->jwsBuilder;
        }

        if (! class_exists(HS256::class)) {
            throw new LogicException('To use the client_secret_jwt auth method you should install web-token/jwt-signature-algorithm-hmac package');
        }

        return $this->jwsBuilder = new JWSBuilder(new AlgorithmManager([new HS256()]));
    }

    protected function createAuthJwt(OpenIDClient $client, array $claims = []): string
    {
        $clientSecret = $client->getMetadata()->getClientSecret();

        if (null === $clientSecret) {
            throw new InvalidArgumentException($this->getSupportedMethod() . ' cannot be used without client_secret metadata');
        }

        $clientId = $client->getMetadata()->getClientId();
        $issuer = $client->getIssuer();
        $issuerMetadata = $issuer->getMetadata();

        $jwk = jose_secret_key($clientSecret);

        $time = time();
        $jti = base64url_encode(random_bytes(32));

        /** @var string $payload */
        $payload = json_encode(
            $claims +
            [
                'iss' => $clientId,
                'sub' => $clientId,
                'aud' => $issuerMetadata->getIssuer(),
                'iat' => $time,
                'exp' => $time + 60,
                'jti' => $jti,
            ]
        );

        $jws = $this->getJwsBuilder()->create()
            ->withPayload($payload)
            ->addSignature($jwk, ['alg' => 'HS256', 'jti' => $jti])
            ->build();

        return $this->jwsSerializer->serialize($jws, 0);
    }
}
