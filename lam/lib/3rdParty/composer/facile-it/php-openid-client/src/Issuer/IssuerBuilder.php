<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer;

use Facile\JoseVerifier\JWK\JwksProviderBuilder;
use Facile\OpenIDClient\Issuer\Metadata\IssuerMetadata;
use Facile\OpenIDClient\Issuer\Metadata\Provider\MetadataProviderBuilder;

final class IssuerBuilder implements IssuerBuilderInterface
{
    /** @var MetadataProviderBuilder|null */
    private $metadataProviderBuilder;

    /** @var JwksProviderBuilder|null */
    private $jwksProviderBuilder;

    public function setMetadataProviderBuilder(?MetadataProviderBuilder $metadataProviderBuilder): self
    {
        $this->metadataProviderBuilder = $metadataProviderBuilder;

        return $this;
    }

    public function setJwksProviderBuilder(?JwksProviderBuilder $jwksProviderBuilder): self
    {
        $this->jwksProviderBuilder = $jwksProviderBuilder;

        return $this;
    }

    private function buildMetadataProviderBuilder(): MetadataProviderBuilder
    {
        return $this->metadataProviderBuilder ?? new MetadataProviderBuilder();
    }

    private function buildJwksProviderBuilder(): JwksProviderBuilder
    {
        return $this->jwksProviderBuilder ?? new JwksProviderBuilder();
    }

    public function build(string $resource): IssuerInterface
    {
        $metadataBuilder = $this->buildMetadataProviderBuilder();
        $metadata = IssuerMetadata::fromArray($metadataBuilder->build()->fetch($resource));

        $jwksProviderBuilder = $this->buildJwksProviderBuilder();
        $jwksProviderBuilder->setJwksUri($metadata->getJwksUri());
        $jwksProvider = $jwksProviderBuilder->build();

        return new Issuer(
            $metadata,
            $jwksProvider
        );
    }
}
