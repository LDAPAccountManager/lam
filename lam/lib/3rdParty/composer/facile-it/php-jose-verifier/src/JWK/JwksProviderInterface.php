<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

/**
 * @psalm-import-type JWKSetObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
interface JwksProviderInterface
{
    /**
     * Get keys
     *
     * @psalm-return JWKSetObject
     */
    public function getJwks(): array;

    /**
     * Require reload keys from source
     *
     * @return $this
     */
    public function reload(): self;
}
