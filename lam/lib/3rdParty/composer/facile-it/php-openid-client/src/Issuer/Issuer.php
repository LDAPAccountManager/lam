<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer;

use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\OpenIDClient\Issuer\Metadata\IssuerMetadataInterface;

final class Issuer implements IssuerInterface
{
    /** @var IssuerMetadataInterface */
    private $metadata;

    /** @var JwksProviderInterface */
    private $jwksProvider;

    public function __construct(IssuerMetadataInterface $metadata, JwksProviderInterface $jwksProvider)
    {
        $this->metadata = $metadata;
        $this->jwksProvider = $jwksProvider;
    }

    public function getMetadata(): IssuerMetadataInterface
    {
        return $this->metadata;
    }

    public function getJwksProvider(): JwksProviderInterface
    {
        return $this->jwksProvider;
    }
}
