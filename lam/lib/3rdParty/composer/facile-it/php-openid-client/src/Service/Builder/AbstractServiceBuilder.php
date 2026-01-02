<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service\Builder;

use Http\Discovery\Psr17FactoryDiscovery;
use Http\Discovery\Psr18ClientDiscovery;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;

/**
 * @psalm-api
 */
abstract class AbstractServiceBuilder
{
    private ?ClientInterface $httpClient = null;

    private ?RequestFactoryInterface $requestFactory = null;

    public function setHttpClient(ClientInterface $httpClient): self
    {
        $this->httpClient = $httpClient;

        return $this;
    }

    public function setRequestFactory(RequestFactoryInterface $requestFactory): self
    {
        $this->requestFactory = $requestFactory;

        return $this;
    }

    protected function getHttpClient(): ClientInterface
    {
        return $this->httpClient ??= Psr18ClientDiscovery::find();
    }

    protected function getRequestFactory(): RequestFactoryInterface
    {
        return $this->requestFactory ??= Psr17FactoryDiscovery::findRequestFactory();
    }
}
