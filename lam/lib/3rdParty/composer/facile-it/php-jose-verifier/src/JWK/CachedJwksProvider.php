<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

use Facile\JoseVerifier\Exception\RuntimeException;
use JsonException;
use Psr\SimpleCache\CacheInterface;

use function is_string;
use function json_decode;
use function json_encode;

/**
 * Wrapper to provide cache feature for a {@see JwksProviderInterface}.
 *
 * @psalm-api
 */
final class CachedJwksProvider extends AbstractJwksProvider
{
    private JwksProviderInterface $provider;

    private CacheInterface $cache;

    private string $cacheKey;

    private ?int $ttl;

    public function __construct(JwksProviderInterface $provider, CacheInterface $cache, string $cacheKey, ?int $ttl)
    {
        $this->provider = $provider;
        $this->cache = $cache;
        $this->cacheKey = $cacheKey;
        $this->ttl = $ttl;
    }

    /**
     * @throws RuntimeException Whenever a runtime error occurred
     */
    public function getJwks(): array
    {
        try {
            /** @var null|string $cached */
            $cached = $this->cache->get($this->cacheKey);
        } catch (\Psr\SimpleCache\InvalidArgumentException $e) {
            throw new RuntimeException('An error occurred fetching cached JWKSet', 0, $e);
        }

        if (is_string($cached)) {
            try {
                /** @var mixed $jwks */
                $jwks = json_decode($cached, true, 512, JSON_THROW_ON_ERROR);
            } catch (JsonException $e) {
                throw new RuntimeException('Unable to decode cached JWKSet', 0, $e);
            }

            if ($this->isJWKSet($jwks)) {
                return $jwks;
            }
        }

        $jwks = $this->provider->getJwks();

        try {
            $this->cache->set($this->cacheKey, json_encode($jwks, JSON_THROW_ON_ERROR), $this->ttl);
        } catch (JsonException $e) {
            throw new RuntimeException('Unable to encode JWKSet before cache', 0, $e);
        } catch (\Psr\SimpleCache\InvalidArgumentException $e) {
            throw new RuntimeException('An error occurred saving JWKSet in cache', 0, $e);
        }

        return $jwks;
    }

    /**
     * @throws RuntimeException
     */
    public function reload(): static
    {
        try {
            $this->cache->delete($this->cacheKey);
        } catch (\Psr\SimpleCache\InvalidArgumentException $e) {
            throw new RuntimeException('An error occurred deleting JWKSet from cache', 0, $e);
        }

        return $this;
    }
}
