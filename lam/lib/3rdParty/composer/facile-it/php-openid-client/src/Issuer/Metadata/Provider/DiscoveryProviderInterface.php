<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use Facile\JoseVerifier\TokenVerifierInterface;

/**
 * @psalm-import-type IssuerMetadataType from TokenVerifierInterface
 */
interface DiscoveryProviderInterface extends RemoteProviderInterface
{
    /**
     * @return array<string, mixed>
     *
     * @psalm-return IssuerMetadataType
     */
    public function discovery(string $url): array;
}
