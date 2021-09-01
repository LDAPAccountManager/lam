<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client\Metadata;

/**
 * @psalm-import-type ClientMetadataObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
interface MetadataFactoryInterface
{
    /**
     * @param array<string, mixed> $metadata
     *
     * @psalm-param ClientMetadataObject $metadata
     */
    public function fromArray(array $metadata): ClientMetadataInterface;
}
