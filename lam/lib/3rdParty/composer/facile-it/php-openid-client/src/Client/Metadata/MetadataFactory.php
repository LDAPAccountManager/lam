<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client\Metadata;

use Override;

/**
 * @psalm-api
 */
final class MetadataFactory implements MetadataFactoryInterface
{
    #[Override]
    public function fromArray(array $metadata): ClientMetadataInterface
    {
        return ClientMetadata::fromArray($metadata);
    }
}
