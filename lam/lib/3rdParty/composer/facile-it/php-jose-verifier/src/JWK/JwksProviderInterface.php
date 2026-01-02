<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

interface JwksProviderInterface
{
    /**
     * Get keys.
     *
     * @psalm-return array{keys: list<array<string, mixed>>}
     */
    public function getJwks(): array;

    /**
     * Require reload keys from source.
     */
    public function reload(): JwksProviderInterface;
}
