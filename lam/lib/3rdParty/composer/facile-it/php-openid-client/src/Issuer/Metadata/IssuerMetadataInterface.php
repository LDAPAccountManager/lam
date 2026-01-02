<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata;

use Facile\JoseVerifier\TokenVerifierInterface;
use JsonSerializable;
use Override;

/**
 * @psalm-api
 *
 * @psalm-import-type IssuerRemoteMetadataType from TokenVerifierInterface
 * @psalm-import-type OpenIdResponseMode from TokenVerifierInterface
 * @psalm-import-type OpenIdGrantType from TokenVerifierInterface
 * @psalm-import-type OpenIdClaimType from TokenVerifierInterface
 * @psalm-import-type OpenIdSubjectType from TokenVerifierInterface
 */
interface IssuerMetadataInterface extends JsonSerializable
{
    /**
     * @return mixed
     */
    public function get(string $name);

    public function has(string $name): bool;

    /**
     * @psalm-return non-empty-string
     */
    public function getIssuer(): string;

    /**
     * @psalm-return non-empty-string
     */
    public function getAuthorizationEndpoint(): string;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getTokenEndpoint(): ?string;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getUserinfoEndpoint(): ?string;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getRegistrationEndpoint(): ?string;

    /**
     * @psalm-return non-empty-string
     */
    public function getJwksUri(): string;

    /**
     * @return string[]|null
     *
     * @psalm-return list<non-empty-string>|null
     */
    public function getScopesSupported(): ?array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getResponseTypesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<OpenIdResponseMode>
     */
    public function getResponseModesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<OpenIdGrantType>
     */
    public function getGrantTypesSupported(): array;

    /**
     * @return string[]|null
     *
     * @psalm-return list<non-empty-string>|null
     */
    public function getAcrValuesSupported(): ?array;

    /**
     * @return string[]
     *
     * @psalm-return list<OpenIdSubjectType>
     */
    public function getSubjectTypesSupported(): array;

    /**
     * @return string[]|null
     *
     * @psalm-return list<non-empty-string>|null
     */
    public function getDisplayValuesSupported(): ?array;

    /**
     * @return string[]
     *
     * @psalm-return list<OpenIdClaimType>
     */
    public function getClaimTypesSupported(): array;

    /**
     * @return string[]|null
     *
     * @psalm-return list<non-empty-string>|null
     */
    public function getClaimsSupported(): ?array;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getServiceDocumentation(): ?string;

    /**
     * @return string[]|null
     *
     * @psalm-return list<non-empty-string>|null
     */
    public function getClaimsLocalesSupported(): ?array;

    /**
     * @return string[]|null
     *
     * @psalm-return list<non-empty-string>|null
     */
    public function getUiLocalesSupported(): ?array;

    public function isClaimsParameterSupported(): bool;

    public function isRequestParameterSupported(): bool;

    public function isRequestUriParameterSupported(): bool;

    public function isRequireRequestUriRegistration(): bool;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getOpPolicyUri(): ?string;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getOpTosUri(): ?string;

    /**
     * @return string[]|null
     *
     * @psalm-return list<non-empty-string>|null
     */
    public function getCodeChallengeMethodsSupported(): ?array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getTokenEndpointAuthMethodsSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getTokenEndpointAuthSigningAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIdTokenSigningAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIdTokenEncryptionAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIdTokenEncryptionEncValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getUserinfoSigningAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getUserinfoEncryptionAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getUserinfoEncryptionEncValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getAuthorizationSigningAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getAuthorizationEncryptionAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getAuthorizationEncryptionEncValuesSupported(): array;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getIntrospectionEndpoint(): ?string;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIntrospectionEndpointAuthMethodsSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIntrospectionEndpointAuthSigningAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIntrospectionSigningAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIntrospectionEncryptionAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getIntrospectionEncryptionEncValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getRequestObjectSigningAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getRequestObjectEncryptionAlgValuesSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getRequestObjectEncryptionEncValuesSupported(): array;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getRevocationEndpoint(): ?string;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getRevocationEndpointAuthMethodsSupported(): array;

    /**
     * @return string[]
     *
     * @psalm-return list<non-empty-string>
     */
    public function getRevocationEndpointAuthSigningAlgValuesSupported(): array;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getCheckSessionIframe(): ?string;

    /**
     * @psalm-return non-empty-string|null
     */
    public function getEndSessionIframe(): ?string;

    public function isFrontchannelLogoutSupported(): bool;

    public function isFrontchannelLogoutSessionSupported(): bool;

    public function isBackchannelLogoutSupported(): bool;

    public function isBackchannelLogoutSessionSupported(): bool;

    public function isTlsClientCertificateBoundAccessTokens(): bool;

    /**
     * @return array<string, string>
     */
    public function getMtlsEndpointAliases(): array;

    /**
     * @return array<string, mixed>
     *
     * @psalm-return IssuerRemoteMetadataType
     */
    #[Override]
    public function jsonSerialize(): array;

    /**
     * @return array<string, mixed>
     *
     * @psalm-return IssuerRemoteMetadataType
     */
    public function toArray(): array;
}
