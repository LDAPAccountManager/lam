<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\Exception\InvalidTokenExceptionInterface;

/**
 * @psalm-api
 *
 * @psalm-type JWKSetType = array{keys: list<array<string, mixed>>}
 * @psalm-type JWTPayloadType = array<string, mixed>
 * @psalm-type OpenIdDisplayType = non-empty-string
 * @psalm-type OpenIdClaimType = 'normal'|'aggregated'|'distribuited'
 * @psalm-type OpenIdResponseType = non-empty-string
 * @psalm-type OpenIdResponseMode = 'query'|'fragment'|non-empty-string
 * @psalm-type OpenIdGrantType = 'authorization_code'|'implicit'|'password'|'client_credentials'|'refresh_token'|'urn:ietf:params:oauth:grant-type:jwt-bearer'|'urn:ietf:params:oauth:grant-type:saml2-bearer'|'urn:ietf:params:oauth:grant-type:token-exchange'|non-empty-string
 * @psalm-type OpenIdApplicationType = 'web'|'native'
 * @psalm-type OpenIdSubjectType = 'pairwise'|'public'
 * @psalm-type OpenIdAuthMethodType = 'client_secret_post'|'client_secret_basic'|'client_secret_jwt'|'private_key_jwt'|'none'|'tls_client_auth'|'self_signed_tls_client_auth'
 * @psalm-type ClientMetadataType = array{
 *     client_id: non-empty-string,
 *     client_secret?: string,
 *     redirect_uris?: list<non-empty-string>,
 *     jwks?: JWKSetType,
 *     jwks_uri?: non-empty-string,
 *     response_types?: list<OpenIdResponseType>,
 *     grant_types?: list<OpenIdGrantType>,
 *     application_type?: OpenIdApplicationType,
 *     contacts?: list<non-empty-string>,
 *     client_name?: string,
 *     logo_uri?: non-empty-string,
 *     client_uri?: non-empty-string,
 *     policy_uri?: non-empty-string,
 *     tos_uri?: non-empty-string,
 *     sector_identifier_uri?: non-empty-string,
 *     subject_type?: OpenIdSubjectType,
 *     authorization_signed_response_alg?: non-empty-string,
 *     authorization_encrypted_response_alg?: non-empty-string,
 *     authorization_encrypted_response_enc?: non-empty-string,
 *     id_token_signed_response_alg?: non-empty-string,
 *     id_token_encrypted_response_alg?: non-empty-string,
 *     id_token_encrypted_response_enc?: non-empty-string,
 *     userinfo_signed_response_alg?: non-empty-string,
 *     userinfo_encrypted_response_alg?: non-empty-string,
 *     userinfo_encrypted_response_enc?: non-empty-string,
 *     request_object_signing_alg?: non-empty-string,
 *     request_object_encryption_alg?: non-empty-string,
 *     request_object_encryption_enc?: non-empty-string,
 *     token_endpoint_auth_method?: OpenIdAuthMethodType,
 *     token_endpoint_auth_signing_alg?: non-empty-string,
 *     default_max_age?: int,
 *     require_auth_time?: bool,
 *     default_acr_values?: list<string>,
 *     initiate_login_uri?: non-empty-string,
 *     request_uris?: list<non-empty-string>,
 *     scope?: string,
 *     software_id?: non-empty-string,
 *     software_version?: non-empty-string,
 *     introspection_endpoint_auth_method?: non-empty-string,
 *     revocation_endpoint_auth_method?: non-empty-string,
 *     ...
 * }
 * @psalm-type IssuerMetadataType = array{issuer: string, jwks_uri: string, ...}
 * @psalm-type IssuerRemoteMetadataType = array{
 *     issuer: non-empty-string,
 *     authorization_endpoint: non-empty-string,
 *     token_endpoint?: non-empty-string,
 *     userinfo_endpoint?: non-empty-string,
 *     jwks_uri: non-empty-string,
 *     registration_endpoint?: non-empty-string,
 *     scopes_supported?: list<non-empty-string>,
 *     response_types_supported: non-empty-list<OpenIdResponseType>,
 *     response_modes_supported?: list<OpenIdResponseMode>,
 *     grant_types_supported?: list<OpenIdGrantType>,
 *     acr_values_supported?: list<non-empty-string>,
 *     subject_types_supported?: list<OpenIdSubjectType>,
 *     id_token_signing_alg_values_supported: non-empty-list<non-empty-string>,
 *     id_token_encryption_alg_values_supported?: list<non-empty-string>,
 *     id_token_encryption_enc_values_supported?: list<non-empty-string>,
 *     userinfo_signing_alg_values_supported?: list<non-empty-string>,
 *     userinfo_encryption_alg_values_supported?: list<non-empty-string>,
 *     userinfo_encryption_enc_values_supported?: list<non-empty-string>,
 *     request_object_signing_alg_values_supported?: list<non-empty-string>,
 *     request_object_encryption_alg_values_supported?: list<non-empty-string>,
 *     request_object_encryption_enc_values_supported?: list<non-empty-string>,
 *     token_endpoint_auth_methods_supported?: list<OpenIdAuthMethodType>,
 *     token_endpoint_auth_signing_alg_values_supported?: list<non-empty-string>,
 *     display_values_supported?: list<OpenIdDisplayType>,
 *     claim_types_supported?: list<OpenIdClaimType>,
 *     claims_supported?: list<non-empty-string>,
 *     service_documentation?: non-empty-string,
 *     claims_locales_supported?: list<non-empty-string>,
 *     ui_locales_supported?: list<non-empty-string>,
 *     claims_parameter_supported?: bool,
 *     request_parameter_supported?: bool,
 *     request_uri_parameter_supported?: bool,
 *     require_request_uri_registration?: bool,
 *     op_policy_uri?: non-empty-string,
 *     op_tos_uri?: non-empty-string,
 *     check_session_iframe?: non-empty-string,
 *     code_challenge_methods_supported?: list<non-empty-string>,
 *     authorization_signing_alg_values_supported?: list<non-empty-string>,
 *     authorization_encryption_alg_values_supported?: list<non-empty-string>,
 *     authorization_encryption_enc_values_supported?: list<non-empty-string>,
 *     introspection_endpoint?: non-empty-string,
 *     introspection_endpoint_auth_methods_supported?: list<non-empty-string>,
 *     introspection_endpoint_auth_signing_alg_values_supported?: list<non-empty-string>,
 *     introspection_signing_alg_values_supported?: list<non-empty-string>,
 *     introspection_encryption_alg_values_supported?: list<non-empty-string>,
 *     introspection_encryption_enc_values_supported?: list<non-empty-string>,
 *     revocation_endpoint?: non-empty-string,
 *     revocation_endpoint_auth_methods_supported?: list<non-empty-string>,
 *     revocation_endpoint_auth_signing_alg_values_supported?: list<non-empty-string>,
 *     end_session_iframe?: non-empty-string,
 *     frontchannel_logout_supported?: bool,
 *     frontchannel_logout_session_supported?: bool,
 *     backchannel_logout_supported?: bool,
 *     backchannel_logout_session_supported?: bool,
 *     mtls_endpoint_aliases?: array<string, string>,
 *     tls_client_certificate_bound_access_tokens?: bool,
 *     ...
 * }
 */
interface TokenVerifierInterface
{
    public function withNonce(?string $nonce): TokenVerifierInterface;

    public function withMaxAge(?int $maxAge): TokenVerifierInterface;

    /**
     * Verify OpenID token.
     *
     * @throws InvalidTokenExceptionInterface
     *
     * @return array The JWT Payload
     *
     * @psalm-return JWTPayloadType
     */
    public function verify(string $jwt): array;
}
