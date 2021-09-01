<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Http\Discovery\Psr17FactoryDiscovery;
use Http\Discovery\Psr18ClientDiscovery;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use Psr\SimpleCache\CacheInterface;
use function sha1;
use function substr;

/**
 * @psalm-import-type JWKSetObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
class JwksProviderBuilder
{
    /**
     * @var array|null
     * @psalm-var null|JWKSetObject
     */
    private $jwks;

    /** @var string|null */
    private $jwksUri;

    /** @var ClientInterface|null */
    private $httpClient;

    /** @var RequestFactoryInterface|null */
    private $requestFactory;

    /** @var CacheInterface|null */
    private $cache;

    /** @var int|null */
    private $cacheTtl = 86400;

    /**
     * @psalm-param JWKSetObject $jwks
     */
    public function setJwks(array $jwks): self
    {
        $this->jwks = $jwks;

        return $this;
    }

    public function setJwksUri(?string $jwksUri): self
    {
        $this->jwksUri = $jwksUri;

        return $this;
    }

    public function setHttpClient(?ClientInterface $httpClient): self
    {
        $this->httpClient = $httpClient;

        return $this;
    }

    public function setRequestFactory(?RequestFactoryInterface $requestFactory): self
    {
        $this->requestFactory = $requestFactory;

        return $this;
    }

    public function setCache(?CacheInterface $cache): self
    {
        $this->cache = $cache;

        return $this;
    }

    public function setCacheTtl(?int $cacheTtl): self
    {
        $this->cacheTtl = $cacheTtl;

        return $this;
    }

    protected function buildRequestFactory(): RequestFactoryInterface
    {
        return $this->requestFactory ?? Psr17FactoryDiscovery::findRequestFactory();
    }

    protected function buildHttpClient(): ClientInterface
    {
        return $this->httpClient ?? Psr18ClientDiscovery::find();
    }

    public function build(): JwksProviderInterface
    {
        if (null !== $this->jwks && null !== $this->jwksUri) {
            throw new InvalidArgumentException('You should provide only one between remote or static jwks');
        }

        if (null === $this->jwksUri) {
            $jwks = $this->jwks ?? ['keys' => []];

            return new MemoryJwksProvider($jwks);
        }

        $provider = new RemoteJwksProvider(
            $this->buildHttpClient(),
            $this->buildRequestFactory(),
            $this->jwksUri
        );

        if (null !== $this->cache) {
            $provider = new CachedJwksProvider(
                $provider,
                $this->cache,
                substr(sha1(__CLASS__ . $this->jwksUri), 0, 65),
                $this->cacheTtl
            );
        }

        return $provider;
    }
}
