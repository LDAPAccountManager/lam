<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata;

use Facile\JoseVerifier\TokenVerifierInterface;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Override;

use function array_diff;
use function array_key_exists;
use function array_keys;
use function array_merge;
use function count;
use function implode;

/**
 * @psalm-import-type IssuerRemoteMetadataType from TokenVerifierInterface
 */
final class IssuerMetadata implements IssuerMetadataInterface
{
    /**
     * @var array<string, mixed>
     *
     * @psalm-var IssuerRemoteMetadataType
     */
    private array $metadata;

    /** @var string[] */
    private static array $requiredKeys = [
        'issuer',
        'authorization_endpoint',
        'jwks_uri',
    ];

    /**
     * @param array<string, mixed> $claims
     *
     * @psalm-param IssuerRemoteMetadataType|array $claims
     */
    public function __construct(
        string $issuer,
        string $authorizationEndpoint,
        string $jwksUri,
        array $claims = []
    ) {
        $requiredClaims = [
            'issuer' => $issuer,
            'authorization_endpoint' => $authorizationEndpoint,
            'jwks_uri' => $jwksUri,
        ];

        /** @psalm-var IssuerRemoteMetadataType $merged */
        $merged = array_merge($claims, $requiredClaims);
        $this->metadata = $merged;
    }

    /**
     * @param array<string, mixed> $claims
     *
     * @psalm-param IssuerRemoteMetadataType $claims
     *
     * @return static
     */
    public static function fromArray(array $claims): self
    {
        $missingKeys = array_diff(self::$requiredKeys, array_keys($claims));
        if (0 !== count($missingKeys)) {
            throw new InvalidArgumentException('Invalid issuer metadata. Missing keys: ' . implode(', ', $missingKeys));
        }

        return new IssuerMetadata(
            $claims['issuer'],
            $claims['authorization_endpoint'],
            $claims['jwks_uri'],
            $claims
        );
    }

    #[Override]
    public function getIssuer(): string
    {
        return $this->metadata['issuer'];
    }

    #[Override]
    public function getAuthorizationEndpoint(): string
    {
        return $this->metadata['authorization_endpoint'];
    }

    #[Override]
    public function getTokenEndpoint(): ?string
    {
        return $this->metadata['token_endpoint'] ?? null;
    }

    #[Override]
    public function getUserinfoEndpoint(): ?string
    {
        return $this->metadata['userinfo_endpoint'] ?? null;
    }

    #[Override]
    public function getRegistrationEndpoint(): ?string
    {
        return $this->metadata['registration_endpoint'] ?? null;
    }

    #[Override]
    public function getJwksUri(): string
    {
        return $this->metadata['jwks_uri'];
    }

    #[Override]
    public function getScopesSupported(): ?array
    {
        return $this->metadata['scopes_supported'] ?? null;
    }

    #[Override]
    public function getResponseTypesSupported(): array
    {
        return $this->metadata['response_types_supported'];
    }

    #[Override]
    public function getResponseModesSupported(): array
    {
        return $this->metadata['response_modes_supported'] ?? ['query', 'fragment'];
    }

    #[Override]
    public function getGrantTypesSupported(): array
    {
        return $this->metadata['grant_types_supported'] ?? ['authorization_code', 'implicit'];
    }

    #[Override]
    public function getAcrValuesSupported(): ?array
    {
        return $this->metadata['acr_values_supported'] ?? null;
    }

    #[Override]
    public function getSubjectTypesSupported(): array
    {
        return $this->metadata['subject_types_supported'] ?? ['public'];
    }

    #[Override]
    public function getDisplayValuesSupported(): ?array
    {
        return $this->metadata['display_values_supported'] ?? null;
    }

    #[Override]
    public function getClaimTypesSupported(): array
    {
        return $this->metadata['claim_types_supported'] ?? ['normal'];
    }

    #[Override]
    public function getClaimsSupported(): ?array
    {
        return $this->metadata['claims_supported'] ?? null;
    }

    #[Override]
    public function getServiceDocumentation(): ?string
    {
        return $this->metadata['service_documentation'] ?? null;
    }

    #[Override]
    public function getClaimsLocalesSupported(): ?array
    {
        return $this->metadata['claims_locales_supported'] ?? null;
    }

    #[Override]
    public function getUiLocalesSupported(): ?array
    {
        return $this->metadata['ui_locales_supported'] ?? null;
    }

    #[Override]
    public function isClaimsParameterSupported(): bool
    {
        return $this->metadata['claims_parameter_supported'] ?? false;
    }

    #[Override]
    public function isRequestParameterSupported(): bool
    {
        return $this->metadata['request_parameter_supported'] ?? false;
    }

    #[Override]
    public function isRequestUriParameterSupported(): bool
    {
        return $this->metadata['request_uri_parameter_supported'] ?? false;
    }

    #[Override]
    public function isRequireRequestUriRegistration(): bool
    {
        return $this->metadata['require_request_uri_registration'] ?? true;
    }

    #[Override]
    public function getOpPolicyUri(): ?string
    {
        return $this->metadata['op_policy_uri'] ?? null;
    }

    #[Override]
    public function getOpTosUri(): ?string
    {
        return $this->metadata['op_tos_uri'] ?? null;
    }

    #[Override]
    public function getCodeChallengeMethodsSupported(): ?array
    {
        return $this->metadata['code_challenge_methods_supported'] ?? null;
    }

    #[Override]
    public function getTokenEndpointAuthMethodsSupported(): array
    {
        return $this->metadata['token_endpoint_auth_methods_supported'] ?? ['client_secret_basic'];
    }

    #[Override]
    public function getTokenEndpointAuthSigningAlgValuesSupported(): array
    {
        /** @var list<non-empty-string> $default */
        $default = ['RS256'];

        return $this->metadata['token_endpoint_auth_signing_alg_values_supported'] ?? $default;
    }

    #[Override]
    public function getIdTokenSigningAlgValuesSupported(): array
    {
        /** @var list<non-empty-string> $default */
        $default = ['RS256'];

        return $this->metadata['id_token_signing_alg_values_supported'] ?? $default;
    }

    #[Override]
    public function getIdTokenEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['id_token_encryption_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getIdTokenEncryptionEncValuesSupported(): array
    {
        return $this->metadata['id_token_encryption_enc_values_supported'] ?? [];
    }

    #[Override]
    public function getUserinfoSigningAlgValuesSupported(): array
    {
        return $this->metadata['userinfo_signing_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getUserinfoEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['userinfo_encryption_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getUserinfoEncryptionEncValuesSupported(): array
    {
        return $this->metadata['userinfo_encryption_enc_values_supported'] ?? [];
    }

    #[Override]
    public function getAuthorizationSigningAlgValuesSupported(): array
    {
        return $this->metadata['authorization_signing_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getAuthorizationEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['authorization_encryption_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getAuthorizationEncryptionEncValuesSupported(): array
    {
        return $this->metadata['authorization_encryption_enc_values_supported'] ?? [];
    }

    #[Override]
    public function getIntrospectionEndpoint(): ?string
    {
        return $this->metadata['introspection_endpoint'] ?? null;
    }

    #[Override]
    public function getIntrospectionEndpointAuthMethodsSupported(): array
    {
        return $this->metadata['introspection_endpoint_auth_methods_supported'] ?? [];
    }

    #[Override]
    public function getIntrospectionEndpointAuthSigningAlgValuesSupported(): array
    {
        return $this->metadata['introspection_endpoint_auth_signing_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getIntrospectionSigningAlgValuesSupported(): array
    {
        return $this->metadata['introspection_signing_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getIntrospectionEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['introspection_encryption_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getIntrospectionEncryptionEncValuesSupported(): array
    {
        return $this->metadata['introspection_encryption_enc_values_supported'] ?? [];
    }

    #[Override]
    public function getRequestObjectSigningAlgValuesSupported(): array
    {
        /** @var list<non-empty-string> $default */
        $default = ['none', 'RS256'];

        return $this->metadata['request_object_signing_alg_values_supported'] ?? $default;
    }

    #[Override]
    public function getRequestObjectEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['request_object_encryption_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getRequestObjectEncryptionEncValuesSupported(): array
    {
        return $this->metadata['request_object_encryption_enc_values_supported'] ?? [];
    }

    #[Override]
    public function getRevocationEndpoint(): ?string
    {
        return $this->metadata['revocation_endpoint'] ?? null;
    }

    #[Override]
    public function getRevocationEndpointAuthMethodsSupported(): array
    {
        return $this->metadata['revocation_endpoint_auth_methods_supported'] ?? [];
    }

    #[Override]
    public function getRevocationEndpointAuthSigningAlgValuesSupported(): array
    {
        return $this->metadata['revocation_endpoint_auth_signing_alg_values_supported'] ?? [];
    }

    #[Override]
    public function getCheckSessionIframe(): ?string
    {
        return $this->metadata['check_session_iframe'] ?? null;
    }

    #[Override]
    public function getEndSessionIframe(): ?string
    {
        return $this->metadata['end_session_iframe'] ?? null;
    }

    #[Override]
    public function isFrontchannelLogoutSupported(): bool
    {
        return $this->metadata['frontchannel_logout_supported'] ?? false;
    }

    #[Override]
    public function isFrontchannelLogoutSessionSupported(): bool
    {
        return $this->metadata['frontchannel_logout_session_supported'] ?? false;
    }

    #[Override]
    public function isBackchannelLogoutSupported(): bool
    {
        return $this->metadata['backchannel_logout_supported'] ?? false;
    }

    #[Override]
    public function isBackchannelLogoutSessionSupported(): bool
    {
        return $this->metadata['backchannel_logout_session_supported'] ?? false;
    }

    #[Override]
    public function isTlsClientCertificateBoundAccessTokens(): bool
    {
        return $this->metadata['tls_client_certificate_bound_access_tokens'] ?? false;
    }

    #[Override]
    public function getMtlsEndpointAliases(): array
    {
        return $this->metadata['mtls_endpoint_aliases'] ?? [];
    }

    #[Override]
    public function jsonSerialize(): array
    {
        return $this->metadata;
    }

    #[Override]
    public function toArray(): array
    {
        return $this->metadata;
    }

    #[Override]
    public function has(string $name): bool
    {
        return array_key_exists($name, $this->metadata);
    }

    #[Override]
    public function get(string $name)
    {
        return $this->metadata[$name] ?? null;
    }
}
