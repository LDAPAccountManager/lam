<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

final class MemoryJwksProvider implements JwksProviderInterface
{
    /** @psalm-var array{keys: list<array<string, mixed>>} */
    private array $jwks;

    /**
     * @psalm-param array{keys: list<array<string, mixed>>} $jwks
     */
    public function __construct(array $jwks = ['keys' => []])
    {
        $this->jwks = $jwks;
    }

    public function getJwks(): array
    {
        return $this->jwks;
    }

    public function reload(): static
    {
        return $this;
    }
}
