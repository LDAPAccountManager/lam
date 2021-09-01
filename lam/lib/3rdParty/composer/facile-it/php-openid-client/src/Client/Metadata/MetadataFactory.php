<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client\Metadata;

final class MetadataFactory implements MetadataFactoryInterface
{
    public function fromArray(array $metadata): ClientMetadataInterface
    {
        return ClientMetadata::fromArray($metadata);
    }
}
