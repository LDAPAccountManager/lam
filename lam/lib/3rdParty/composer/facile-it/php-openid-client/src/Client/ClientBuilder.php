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
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Facile\OpenIDClient\Issuer\IssuerInterface;
use Http\Discovery\Psr18ClientDiscovery;
use Psr\Http\Client\ClientInterface as HttpClient;

class ClientBuilder
{
    /** @var ClientMetadataInterface|null */
    private $clientMetadata;

    /** @var IssuerInterface|null */
    private $issuer;

    /** @var JwksProviderInterface|null */
    private $jwksProvider;

    /** @var AuthMethodFactoryInterface|null */
    private $authMethodFactory;

    /** @var HttpClient|null */
    private $httpClient;

    public function setClientMetadata(?ClientMetadataInterface $clientMetadata): self
    {
        $this->clientMetadata = $clientMetadata;

        return $this;
    }

    public function setIssuer(?IssuerInterface $issuer): self
    {
        $this->issuer = $issuer;

        return $this;
    }

    public function setJwksProvider(?JwksProviderInterface $jwksProvider): self
    {
        $this->jwksProvider = $jwksProvider;

        return $this;
    }

    public function setAuthMethodFactory(?AuthMethodFactoryInterface $authMethodFactory): self
    {
        $this->authMethodFactory = $authMethodFactory;

        return $this;
    }

    public function setHttpClient(?HttpClient $httpClient): self
    {
        $this->httpClient = $httpClient;

        return $this;
    }

    private function buildJwksProvider(): JwksProviderInterface
    {
        if (null !== $this->jwksProvider) {
            return $this->jwksProvider;
        }

        if (null === $this->clientMetadata) {
            return new MemoryJwksProvider(['keys' => []]);
        }

        $jwks = $this->clientMetadata->getJwks();

        if (null !== $jwks) {
            new MemoryJwksProvider($jwks);
        }

        return new MemoryJwksProvider(['keys' => []]);
    }

    private function buildAuthMethodFactory(): AuthMethodFactoryInterface
    {
        return $this->authMethodFactory ?? new AuthMethodFactory([
            new ClientSecretBasic(),
            new ClientSecretJwt(),
            new ClientSecretPost(),
            new None(),
            new PrivateKeyJwt(),
            new TLSClientAuth(),
            new SelfSignedTLSClientAuth(),
        ]);
    }

    private function buildHttpClient(): HttpClient
    {
        return $this->httpClient ?? Psr18ClientDiscovery::find();
    }

    public function build(): ClientInterface
    {
        if (null === $this->issuer) {
            throw new InvalidArgumentException('Issuer must be provided');
        }

        if (null === $this->clientMetadata) {
            throw new InvalidArgumentException('Client metadata must be provided');
        }

        return new Client(
            $this->issuer,
            $this->clientMetadata,
            $this->buildJwksProvider(),
            $this->buildAuthMethodFactory(),
            $this->buildHttpClient()
        );
    }
}
