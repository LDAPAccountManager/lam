<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use Http\Discovery\Psr17FactoryDiscovery;
use Http\Discovery\Psr18ClientDiscovery;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use Psr\Http\Message\UriFactoryInterface;
use Psr\SimpleCache\CacheInterface;

class MetadataProviderBuilder
{
    /** @var null|DiscoveryProviderInterface */
    private $discoveryProvider;

    /** @var null|WebFingerProviderInterface */
    private $webFingerProvider;

    /** @var ClientInterface|null */
    private $client;

    /** @var RequestFactoryInterface|null */
    private $requestFactory;

    /** @var UriFactoryInterface|null */
    private $uriFactory;

    /** @var CacheInterface|null */
    private $cache;

    /** @var int|null */
    private $cacheTtl;

    public function setDiscoveryProvider(DiscoveryProviderInterface $discoveryProvider): self
    {
        $this->discoveryProvider = $discoveryProvider;

        return $this;
    }

    public function setHttpClient(?ClientInterface $client): self
    {
        $this->client = $client;

        return $this;
    }

    public function setWebFingerProvider(WebFingerProviderInterface $webFingerProvider): self
    {
        $this->webFingerProvider = $webFingerProvider;

        return $this;
    }

    /**
     * @deprecated use MetadataProviderBuilder::setHttpClient() instead
     */
    public function setClient(?ClientInterface $client): self
    {
        $this->client = $client;

        return $this;
    }

    public function setRequestFactory(?RequestFactoryInterface $requestFactory): self
    {
        $this->requestFactory = $requestFactory;

        return $this;
    }

    public function setUriFactory(?UriFactoryInterface $uriFactory): self
    {
        $this->uriFactory = $uriFactory;

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

    private function buildClient(): ClientInterface
    {
        return $this->client ?? Psr18ClientDiscovery::find();
    }

    public function buildRequestFactory(): RequestFactoryInterface
    {
        return $this->requestFactory ?? Psr17FactoryDiscovery::findRequestFactory();
    }

    public function buildUriFactory(): UriFactoryInterface
    {
        return $this->uriFactory ?? Psr17FactoryDiscovery::findUriFactory();
    }

    private function buildDiscoveryProvider(): DiscoveryProviderInterface
    {
        return $this->discoveryProvider ?? new DiscoveryProvider(
            $this->buildClient(),
            $this->buildRequestFactory(),
            $this->buildUriFactory()
        );
    }

    public function buildWebFingerProvider(): WebFingerProviderInterface
    {
        return $this->webFingerProvider ?? new WebFingerProvider(
            $this->buildClient(),
            $this->buildRequestFactory(),
            $this->buildUriFactory(),
            $this->buildDiscoveryProvider()
        );
    }

    public function build(): RemoteProviderInterface
    {
        $provider = new RemoteProvider([
            $this->buildDiscoveryProvider(),
            $this->buildWebFingerProvider(),
        ]);

        if (null !== $this->cache) {
            $provider = new CachedProviderDecorator($provider, $this->cache, $this->cacheTtl);
        }

        return $provider;
    }
}
