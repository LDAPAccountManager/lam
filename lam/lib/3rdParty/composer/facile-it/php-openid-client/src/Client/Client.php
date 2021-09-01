<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client;

use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\JoseVerifier\JWK\MemoryJwksProvider;
use Facile\OpenIDClient\AuthMethod\AuthMethodFactory;
use Facile\OpenIDClient\AuthMethod\AuthMethodFactoryInterface;
use Facile\OpenIDClient\AuthMethod\ClientSecretBasic;
use Facile\OpenIDClient\AuthMethod\ClientSecretJwt;
use Facile\OpenIDClient\AuthMethod\ClientSecretPost;
use Facile\OpenIDClient\AuthMethod\None;
use Facile\OpenIDClient\AuthMethod\PrivateKeyJwt;
use Facile\OpenIDClient\AuthMethod\SelfSignedTLSClientAuth;
use Facile\OpenIDClient\AuthMethod\TLSClientAuth;
use Facile\OpenIDClient\Client\Metadata\ClientMetadataInterface;
use Facile\OpenIDClient\Issuer\IssuerInterface;
use Psr\Http\Client\ClientInterface as HttpClient;

final class Client implements ClientInterface
{
    /** @var IssuerInterface */
    private $issuer;

    /** @var ClientMetadataInterface */
    private $metadata;

    /** @var JwksProviderInterface */
    private $jwksProvider;

    /** @var AuthMethodFactoryInterface */
    private $authMethodFactory;

    /** @var null|HttpClient */
    private $httpClient;

    /**
     * Client constructor.
     */
    public function __construct(
        IssuerInterface $issuer,
        ClientMetadataInterface $metadata,
        ?JwksProviderInterface $jwksProvider = null,
        ?AuthMethodFactoryInterface $authMethodFactory = null,
        ?HttpClient $httpClient = null
    ) {
        $this->issuer = $issuer;
        $this->metadata = $metadata;
        $this->jwksProvider = $jwksProvider ?? new MemoryJwksProvider();
        $this->authMethodFactory = $authMethodFactory ?? new AuthMethodFactory([
            new ClientSecretBasic(),
            new ClientSecretJwt(),
            new ClientSecretPost(),
            new None(),
            new PrivateKeyJwt(),
            new TLSClientAuth(),
            new SelfSignedTLSClientAuth(),
        ]);
        $this->httpClient = $httpClient;
    }

    public function getIssuer(): IssuerInterface
    {
        return $this->issuer;
    }

    public function getMetadata(): ClientMetadataInterface
    {
        return $this->metadata;
    }

    public function getJwksProvider(): JwksProviderInterface
    {
        return $this->jwksProvider;
    }

    public function getAuthMethodFactory(): AuthMethodFactoryInterface
    {
        return $this->authMethodFactory;
    }

    public function getHttpClient(): ?HttpClient
    {
        return $this->httpClient;
    }
}
