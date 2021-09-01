<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer;

use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\OpenIDClient\Issuer\Metadata\IssuerMetadataInterface;

interface IssuerInterface
{
    public function getMetadata(): IssuerMetadataInterface;

    public function getJwksProvider(): JwksProviderInterface;
}
