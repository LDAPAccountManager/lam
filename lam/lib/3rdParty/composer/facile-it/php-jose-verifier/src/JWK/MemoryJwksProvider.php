<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\JWK;

/**
 * @psalm-import-type JWKSetObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
class MemoryJwksProvider implements JwksProviderInterface
{
    /**
     * @var array
     * @psalm-var JWKSetObject
     */
    private $jwks;

    /**
     * @psalm-param JWKSetObject $jwks
     */
    public function __construct(array $jwks = ['keys' => []])
    {
        $this->jwks = $jwks;
    }

    /**
     * @inheritDoc
     */
    public function getJwks(): array
    {
        return $this->jwks;
    }

    /**
     * @inheritDoc
     */
    public function reload(): JwksProviderInterface
    {
        return $this;
    }
}
