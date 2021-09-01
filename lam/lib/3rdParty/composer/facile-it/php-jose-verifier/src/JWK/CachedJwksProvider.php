<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

use function is_array;
use function is_string;
use function json_decode;
use function json_encode;
use Psr\SimpleCache\CacheInterface;

/**
 * @psalm-import-type JWKSetObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
class CachedJwksProvider implements JwksProviderInterface
{
    /** @var JwksProviderInterface */
    private $provider;

    /** @var CacheInterface */
    private $cache;

    /** @var string */
    private $cacheKey;

    /** @var null|int */
    private $ttl;

    public function __construct(JwksProviderInterface $provider, CacheInterface $cache, string $cacheKey, ?int $ttl)
    {
        $this->provider = $provider;
        $this->cache = $cache;
        $this->cacheKey = $cacheKey;
        $this->ttl = $ttl;
    }

    /**
     * @inheritDoc
     */
    public function getJwks(): array
    {
        /** @var null|string $cached */
        $cached = $this->cache->get($this->cacheKey);

        if (is_string($cached)) {
            /** @var null|JWKSetObject $jwks */
            $jwks = json_decode($cached, true);

            if (is_array($jwks)) {
                return $jwks;
            }
        }

        $jwks = $this->provider->getJwks();
        $this->cache->set($this->cacheKey, json_encode($jwks), $this->ttl);

        return $jwks;
    }

    /**
     * @inheritDoc
     */
    public function reload(): JwksProviderInterface
    {
        $this->cache->delete($this->cacheKey);

        return $this;
    }
}
