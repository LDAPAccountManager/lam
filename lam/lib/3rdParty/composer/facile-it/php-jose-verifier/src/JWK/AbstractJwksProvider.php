<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

abstract class AbstractJwksProvider implements JwksProviderInterface
{
    /**
     * @param mixed $data
     *
     * @psalm-assert-if-true array{keys: list<array<string, mixed>>} $data
     */
    protected function isJWKSet($data): bool
    {
        return is_array($data) && array_key_exists('keys', $data) && is_array($data['keys']);
    }
}
