<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use Facile\JoseVerifier\TokenVerifierInterface;

/**
 * @psalm-import-type IssuerRemoteMetadataType from TokenVerifierInterface
 */
interface RemoteProviderInterface
{
    public function isAllowedUri(string $uri): bool;

    /**
     * @return array<string, mixed>
     *
     * @psalm-return IssuerRemoteMetadataType
     */
    public function fetch(string $uri): array;
}
