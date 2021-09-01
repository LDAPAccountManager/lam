<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\JWK\JwksProviderInterface;

/**
 * @template TVerifier of TokenVerifierInterface
 * @psalm-import-type ClientMetadataObject from Psalm\PsalmTypes
 * @psalm-import-type IssuerMetadataObject from Psalm\PsalmTypes
 */
interface TokenVerifierBuilderInterface
{
    /**
     * @param array<string, mixed> $clientMetadata
     * @psalm-param ClientMetadataObject $clientMetadata
     */
    public function setClientMetadata(array $clientMetadata): void;

    /**
     * @param array<string, mixed> $issuerMetadata
     * @psalm-param IssuerMetadataObject $issuerMetadata
     */
    public function setIssuerMetadata(array $issuerMetadata): void;

    public function setClockTolerance(int $clockTolerance): void;

    public function setAadIssValidation(bool $aadIssValidation): void;

    public function setJwksProvider(?JwksProviderInterface $jwksProvider): void;

    public function setClientJwksProvider(?JwksProviderInterface $clientJwksProvider): void;

    /**
     * @psalm-return TVerifier
     */
    public function build(): TokenVerifierInterface;
}
