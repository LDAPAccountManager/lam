<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use Facile\JoseVerifier\TokenVerifierInterface;
use JsonException;
use Psr\SimpleCache\CacheInterface;
use Override;

use function is_array;
use function json_decode;
use function json_encode;
use function sha1;
use function substr;

/**
 * @psalm-import-type IssuerRemoteMetadataType from TokenVerifierInterface
 */
final class CachedProviderDecorator implements RemoteProviderInterface
{
    private RemoteProviderInterface $provider;

    private CacheInterface $cache;

    private ?int $cacheTtl = null;

    /**
     * @var callable
     *
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
        $this->cacheIdGenerator = $cacheIdGenerator ?? static fn(string $uri): string => substr(sha1($uri), 0, 65);
    }

    /**
     * @return array<string, mixed>
     *
     * @psalm-suppress MixedReturnTypeCoercion
     *
     * @psalm-return IssuerRemoteMetadataType
     */
    #[Override]
    public function fetch(string $uri): array
    {
        $cacheId = ($this->cacheIdGenerator)($uri);

        /** @var string $cached */
        $cached = $this->cache->get($cacheId) ?? '';

        try {
            /** @psalm-var null|string|IssuerRemoteMetadataType $data */
            $data = json_decode($cached, true, 512, JSON_THROW_ON_ERROR);
        } catch (JsonException $e) {
            $data = null;
        }

        if (is_array($data)) {
            /** @psalm-var IssuerRemoteMetadataType $data */
            return $data;
        }

        $data = $this->provider->fetch($uri);

        $this->cache->set($cacheId, json_encode($data, JSON_THROW_ON_ERROR), $this->cacheTtl);

        return $data;
    }

    #[Override]
    public function isAllowedUri(string $uri): bool
    {
        return $this->provider->isAllowedUri($uri);
    }
}
