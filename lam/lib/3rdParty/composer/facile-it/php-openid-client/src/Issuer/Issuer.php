<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer;

use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\OpenIDClient\Issuer\Metadata\IssuerMetadataInterface;
use Override;

final class Issuer implements IssuerInterface
{
    private IssuerMetadataInterface $metadata;

    private JwksProviderInterface $jwksProvider;

    public function __construct(IssuerMetadataInterface $metadata, JwksProviderInterface $jwksProvider)
    {
        $this->metadata = $metadata;
        $this->jwksProvider = $jwksProvider;
    }

    #[Override]
    public function getMetadata(): IssuerMetadataInterface
    {
        return $this->metadata;
    }

    #[Override]
    public function getJwksProvider(): JwksProviderInterface
    {
        return $this->jwksProvider;
    }
}
