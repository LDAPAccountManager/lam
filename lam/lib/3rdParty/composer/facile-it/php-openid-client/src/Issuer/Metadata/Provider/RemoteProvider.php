<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use Facile\OpenIDClient\Exception\ExceptionInterface;
use Facile\OpenIDClient\Exception\RuntimeException;
use Override;

use function array_filter;

final class RemoteProvider implements RemoteProviderInterface
{
    /** @var RemoteProviderInterface[] */
    private array $providers;

    /**
     * @param RemoteProviderInterface[] $providers
     */
    public function __construct(array $providers)
    {
        $this->providers = $providers;
    }

    #[Override]
    public function isAllowedUri(string $uri): bool
    {
        return true;
    }

    #[Override]
    public function fetch(string $uri): array
    {
        $lastException = null;

        $providers = array_filter($this->providers, static fn(RemoteProviderInterface $provider): bool => $provider->isAllowedUri($uri));

        foreach ($providers as $provider) {
            try {
                return $provider->fetch($uri);
            } catch (ExceptionInterface $e) {
                $lastException = $e;
            }
        }

        throw new RuntimeException('Unable to fetch issuer metadata', 0, $lastException);
    }
}
