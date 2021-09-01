<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use function is_array;
use function json_decode;
use function json_encode;
use Psr\SimpleCache\CacheInterface;
use function sha1;
use function substr;

/**
 * @psalm-import-type IssuerMetadataObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
final class CachedProviderDecorator implements RemoteProviderInterface
{
    /** @var RemoteProviderInterface */
    private $provider;

    /** @var CacheInterface */
    private $cache;

    /** @var null|int */
    private $cacheTtl;

    /**
     * @var callable
     * @psalm-var callable(string): string
     */
    private $cacheIdGenerator;

    /**
     * @psalm-param null|callable(string): string $cacheIdGenerator
     */
    public function __construct(
        RemoteProviderInterface $provider,
        CacheInterface $cache,
        ?int $cacheTtl = null,
        ?callable $cacheIdGenerator = null
    ) {
        $this->provider = $provider;
        $this->cache = $cache;
        $this->cacheTtl = $cacheTtl;
        $this->cacheIdGenerator = $cacheIdGenerator ?? static function (string $uri): string {
            return substr(sha1($uri), 0, 65);
        };
    }

    public function fetch(string $uri): array
    {
        $cacheId = ($this->cacheIdGenerator)($uri);

        /** @var string $cached */
        $cached = $this->cache->get($cacheId) ?? '';
        /** @var null|string|IssuerMetadataObject $data */
        $data = json_decode($cached, true);

        if (is_array($data)) {
            return $data;
        }

        $data = $this->provider->fetch($uri);

        $this->cache->set($cacheId, json_encode($data), $this->cacheTtl);

        return $data;
    }

    public function isAllowedUri(string $uri): bool
    {
        return $this->provider->isAllowedUri($uri);
    }
}
