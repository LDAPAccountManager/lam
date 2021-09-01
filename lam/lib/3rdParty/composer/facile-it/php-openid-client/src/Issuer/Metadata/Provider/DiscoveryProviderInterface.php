<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

/**
 * @psalm-import-type IssuerMetadataObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
interface DiscoveryProviderInterface extends RemoteProviderInterface
{
    /**
     * @return array<string, mixed>
     * @psalm-return IssuerMetadataObject
     */
    public function discovery(string $url): array;
}
