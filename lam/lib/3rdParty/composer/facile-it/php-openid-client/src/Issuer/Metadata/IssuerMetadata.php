<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata;

use function array_diff;
use function array_key_exists;
use function array_keys;
use function array_merge;
use function count;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use function implode;

/**
 * @psalm-import-type IssuerMetadataObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
final class IssuerMetadata implements IssuerMetadataInterface
{
    /**
     * @var array<string, mixed>
     * @psalm-var IssuerMetadataObject
     */
    private $metadata;

    /** @var string[] */
    private static $requiredKeys = [
        'issuer',
        'authorization_endpoint',
        'jwks_uri',
    ];

    /**
     * IssuerMetadata constructor.
     *
     * @param array<string, mixed> $claims
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

        /** @var IssuerMetadataObject $merged */
        $merged = array_merge($claims, $requiredClaims);
        $this->metadata = $merged;
    }

    /**
     * @param array<string, mixed> $claims
     *
     * @return static
     *
     * @psalm-param IssuerMetadataObject $claims
     */
    public static function fromArray(array $claims): self
    {
        $missingKeys = array_diff(self::$requiredKeys, array_keys($claims));
        if (0 !== count($missingKeys)) {
            throw new InvalidArgumentException('Invalid issuer metadata. Missing keys: ' . implode(', ', $missingKeys));
        }

        return new static(
            $claims['issuer'],
            $claims['authorization_endpoint'],
            $claims['jwks_uri'],
            $claims
        );
    }

    public function getIssuer(): string
    {
        return $this->metadata['issuer'];
    }

    public function getAuthorizationEndpoint(): string
    {
        return $this->metadata['authorization_endpoint'];
    }

    public function getTokenEndpoint(): ?string
    {
        return $this->metadata['token_endpoint'] ?? null;
    }

    public function getUserinfoEndpoint(): ?string
    {
        return $this->metadata['userinfo_endpoint'] ?? null;
    }

    public function getRegistrationEndpoint(): ?string
    {
        return $this->metadata['registration_endpoint'] ?? null;
    }

    public function getJwksUri(): string
    {
        return $this->metadata['jwks_uri'];
    }

    public function getScopesSupported(): ?array
    {
        return $this->metadata['scopes_supported'] ?? null;
    }

    public function getResponseTypesSupported(): array
    {
        return $this->metadata['response_types_supported'];
    }

    public function getResponseModesSupported(): array
    {
        return $this->metadata['response_modes_supported'] ?? ['query', 'fragment'];
    }

    public function getGrantTypesSupported(): array
    {
        return $this->metadata['grant_types_supported'] ?? ['authorization_code', 'implicit'];
    }

    public function getAcrValuesSupported(): ?array
    {
        return $this->metadata['acr_values_supported'] ?? null;
    }

    public function getSubjectTypesSupported(): array
    {
        return $this->metadata['subject_types_supported'] ?? ['public'];
    }

    public function getDisplayValuesSupported(): ?array
    {
        return $this->metadata['display_values_supported'] ?? null;
    }

    public function getClaimTypesSupported(): array
    {
        return $this->metadata['claim_types_supported'] ?? ['normal'];
    }

    public function getClaimsSupported(): ?array
    {
        return $this->metadata['claims_supported'] ?? null;
    }

    public function getServiceDocumentation(): ?string
    {
        return $this->metadata['service_documentation'] ?? null;
    }

    public function getClaimsLocalesSupported(): ?array
    {
        return $this->metadata['claims_locales_supported'] ?? null;
    }

    public function getUiLocalesSupported(): ?array
    {
        return $this->metadata['ui_locales_supported'] ?? null;
    }

    public function isClaimsParameterSupported(): bool
    {
        return $this->metadata['claims_parameter_supported'] ?? false;
    }

    public function isRequestParameterSupported(): bool
    {
        return $this->metadata['request_parameter_supported'] ?? false;
    }

    public function isRequestUriParameterSupported(): bool
    {
        return $this->metadata['request_uri_parameter_supported'] ?? false;
    }

    public function isRequireRequestUriRegistration(): bool
    {
        return $this->metadata['require_request_uri_registration'] ?? true;
    }

    public function getOpPolicyUri(): ?string
    {
        return $this->metadata['op_policy_uri'] ?? null;
    }

    public function getOpTosUri(): ?string
    {
        return $this->metadata['op_tos_uri'] ?? null;
    }

    public function getCodeChallengeMethodsSupported(): ?array
    {
        return $this->metadata['code_challenge_methods_supported'] ?? null;
    }

    public function getTokenEndpointAuthMethodsSupported(): array
    {
        return $this->metadata['token_endpoint_auth_methods_supported'] ?? ['client_secret_basic'];
    }

    public function getTokenEndpointAuthSigningAlgValuesSupported(): array
    {
        /** @var list<non-empty-string> $default */
        $default = ['RS256'];

        return $this->metadata['token_endpoint_auth_signing_alg_values_supported'] ?? $default;
    }

    public function getIdTokenSigningAlgValuesSupported(): array
    {
        /** @var list<non-empty-string> $default */
        $default = ['RS256'];

        return $this->metadata['id_token_signing_alg_values_supported'] ?? $default;
    }

    public function getIdTokenEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['id_token_encryption_alg_values_supported'] ?? [];
    }

    public function getIdTokenEncryptionEncValuesSupported(): array
    {
        return $this->metadata['id_token_encryption_enc_values_supported'] ?? [];
    }

    public function getUserinfoSigningAlgValuesSupported(): array
    {
        return $this->metadata['userinfo_signing_alg_values_supported'] ?? [];
    }

    public function getUserinfoEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['userinfo_encryption_alg_values_supported'] ?? [];
    }

    public function getUserinfoEncryptionEncValuesSupported(): array
    {
        return $this->metadata['userinfo_encryption_enc_values_supported'] ?? [];
    }

    public function getAuthorizationSigningAlgValuesSupported(): array
    {
        return $this->metadata['authorization_signing_alg_values_supported'] ?? [];
    }

    public function getAuthorizationEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['authorization_encryption_alg_values_supported'] ?? [];
    }

    public function getAuthorizationEncryptionEncValuesSupported(): array
    {
        return $this->metadata['authorization_encryption_enc_values_supported'] ?? [];
    }

    public function getIntrospectionEndpoint(): ?string
    {
        return $this->metadata['introspection_endpoint'] ?? null;
    }

    public function getIntrospectionEndpointAuthMethodsSupported(): array
    {
        return $this->metadata['introspection_endpoint_auth_methods_supported'] ?? [];
    }

    public function getIntrospectionEndpointAuthSigningAlgValuesSupported(): array
    {
        return $this->metadata['introspection_endpoint_auth_signing_alg_values_supported'] ?? [];
    }

    public function getIntrospectionSigningAlgValuesSupported(): array
    {
        return $this->metadata['introspection_signing_alg_values_supported'] ?? [];
    }

    public function getIntrospectionEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['introspection_encryption_alg_values_supported'] ?? [];
    }

    public function getIntrospectionEncryptionEncValuesSupported(): array
    {
        return $this->metadata['introspection_encryption_enc_values_supported'] ?? [];
    }

    public function getRequestObjectSigningAlgValuesSupported(): array
    {
        /** @var list<non-empty-string> $default */
        $default = ['none', 'RS256'];

        return $this->metadata['request_object_signing_alg_values_supported'] ?? $default;
    }

    public function getRequestObjectEncryptionAlgValuesSupported(): array
    {
        return $this->metadata['request_object_encryption_alg_values_supported'] ?? [];
    }

    public function getRequestObjectEncryptionEncValuesSupported(): array
    {
        return $this->metadata['request_object_encryption_enc_values_supported'] ?? [];
    }

    public function getRevocationEndpoint(): ?string
    {
        return $this->metadata['revocation_endpoint'] ?? null;
    }

    public function getRevocationEndpointAuthMethodsSupported(): array
    {
        return $this->metadata['revocation_endpoint_auth_methods_supported'] ?? [];
    }

    public function getRevocationEndpointAuthSigningAlgValuesSupported(): array
    {
        return $this->metadata['revocation_endpoint_auth_signing_alg_values_supported'] ?? [];
    }

    public function getCheckSessionIframe(): ?string
    {
        return $this->metadata['check_session_iframe'] ?? null;
    }

    public function getEndSessionIframe(): ?string
    {
        return $this->metadata['end_session_iframe'] ?? null;
    }

    public function isFrontchannelLogoutSupported(): bool
    {
        return $this->metadata['frontchannel_logout_supported'] ?? false;
    }

    public function isFrontchannelLogoutSessionSupported(): bool
    {
        return $this->metadata['frontchannel_logout_session_supported'] ?? false;
    }

    public function isBackchannelLogoutSupported(): bool
    {
        return $this->metadata['backchannel_logout_supported'] ?? false;
    }

    public function isBackchannelLogoutSessionSupported(): bool
    {
        return $this->metadata['backchannel_logout_session_supported'] ?? false;
    }

    public function isTlsClientCertificateBoundAccessTokens(): bool
    {
        return $this->metadata['tls_client_certificate_bound_access_tokens'] ?? false;
    }

    public function getMtlsEndpointAliases(): array
    {
        return $this->metadata['mtls_endpoint_aliases'] ?? [];
    }

    public function jsonSerialize(): array
    {
        return $this->metadata;
    }

    public function toArray(): array
    {
        return $this->metadata;
    }

    public function has(string $name): bool
    {
        return array_key_exists($name, $this->metadata);
    }

    public function get(string $name)
    {
        return $this->metadata[$name] ?? null;
    }
}
