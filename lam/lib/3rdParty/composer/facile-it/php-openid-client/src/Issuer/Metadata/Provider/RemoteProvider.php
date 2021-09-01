<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use function array_filter;
use Facile\OpenIDClient\Exception\ExceptionInterface;
use Facile\OpenIDClient\Exception\RuntimeException;

final class RemoteProvider implements RemoteProviderInterface
{
    /** @var RemoteProviderInterface[] */
    private $providers;

    /**
     * @param RemoteProviderInterface[] $providers
     */
    public function __construct(array $providers)
    {
        $this->providers = $providers;
    }

    public function isAllowedUri(string $uri): bool
    {
        return true;
    }

    public function fetch(string $uri): array
    {
        $lastException = null;

        $providers = array_filter($this->providers, static function (RemoteProviderInterface $provider) use ($uri): bool {
            return $provider->isAllowedUri($uri);
        });

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
