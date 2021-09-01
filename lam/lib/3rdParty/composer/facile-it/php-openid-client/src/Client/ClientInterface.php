<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client;

use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\OpenIDClient\AuthMethod\AuthMethodFactoryInterface;
use Facile\OpenIDClient\Client\Metadata\ClientMetadataInterface;
use Facile\OpenIDClient\Issuer\IssuerInterface;
use Psr\Http\Client\ClientInterface as HttpClient;

interface ClientInterface
{
    public function getIssuer(): IssuerInterface;

    public function getMetadata(): ClientMetadataInterface;

    public function getJwksProvider(): JwksProviderInterface;

    public function getAuthMethodFactory(): AuthMethodFactoryInterface;

    public function getHttpClient(): ?HttpClient;
}
