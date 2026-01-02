<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Nyholm\Psr7\Factory\Psr17Factory;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use Psr\SimpleCache\CacheInterface;
use Symfony\Component\HttpClient\Psr18Client;

use function sha1;
use function substr;

/**
 * @psalm-api
 *
 * @psalm-type JWKSetType = array{keys: list<array<string, mixed>>}
 */
final class JwksProviderBuilder
{
    /** @psalm-var null|JWKSetType */
    private ?array $jwks = null;

    private ?string $jwksUri = null;

    private ?ClientInterface $httpClient = null;

    private ?RequestFactoryInterface $requestFactory = null;

    private ?CacheInterface $cache = null;

    private ?int $cacheTtl = 86_400;

    /**
     * @psalm-param JWKSetType $jwks
     */
    public function withJwks(array $jwks): static
    {
        $new = clone $this;
        $new->jwks = $jwks;

        return $new;
    }

    public function withJwksUri(string $jwksUri): static
    {
        $new = clone $this;
        $new->jwksUri = $jwksUri;

        return $new;
    }

    public function withHttpClient(ClientInterface $httpClient): static
    {
        $new = clone $this;
        $new->httpClient = $httpClient;

        return $new;
    }

    public function withRequestFactory(RequestFactoryInterface $requestFactory): static
    {
        $new = clone $this;
        $new->requestFactory = $requestFactory;

        return $new;
    }

    public function withCache(CacheInterface $cache): static
    {
        $new = clone $this;
        $new->cache = $cache;

        return $new;
    }

    public function withCacheTtl(int $cacheTtl): static
    {
        $new = clone $this;
        $new->cacheTtl = $cacheTtl;

        return $new;
    }

    protected function buildRequestFactory(): RequestFactoryInterface
    {
        return $this->requestFactory ?? new Psr17Factory();
    }

    protected function buildHttpClient(): ClientInterface
    {
        return $this->httpClient ?? new Psr18Client();
    }

    /**
     * @throws InvalidArgumentException
     */
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
