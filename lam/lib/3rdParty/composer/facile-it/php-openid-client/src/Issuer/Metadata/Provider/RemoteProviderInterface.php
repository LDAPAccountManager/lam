<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

/**
 * @psalm-import-type IssuerMetadataObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
interface RemoteProviderInterface
{
    public function isAllowedUri(string $uri): bool;

    /**
     * @return array<string, mixed>
     * @psalm-return IssuerMetadataObject
     */
    public function fetch(string $uri): array;
}
