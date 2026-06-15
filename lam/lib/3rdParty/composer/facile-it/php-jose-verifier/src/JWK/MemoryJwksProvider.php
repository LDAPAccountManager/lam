<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

final readonly class MemoryJwksProvider implements JwksProviderInterface
{
    /**
     * @psalm-param array{keys: list<array<string, mixed>>} $jwks
     */
    public function __construct(
        /** @psalm-var array{keys: list<array<string, mixed>>} */
        private array $jwks = ['keys' => []],
    ) {}

    public function getJwks(): array
    {
        return $this->jwks;
    }

    public function reload(): static
    {
        return $this;
    }
}
