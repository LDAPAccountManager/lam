<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client\Metadata;

use Facile\JoseVerifier\TokenVerifierInterface;

/**
 * @psalm-api
 *
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 */
interface MetadataFactoryInterface
{
    /**
     * @param array<string, mixed> $metadata
     *
     * @psalm-param ClientMetadataType $metadata
     */
    public function fromArray(array $metadata): ClientMetadataInterface;
}
