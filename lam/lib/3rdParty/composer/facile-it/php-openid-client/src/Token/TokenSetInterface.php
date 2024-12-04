<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

/**
 * @psalm-type ClaimSourceAggregateType = array{}&array{JWT: string}
 * @psalm-type ClaimSourceDistributedType = array{}&array{endpoint: string, access_token?: string}
 * @psalm-type ClaimSourceType = ClaimSourceAggregateType|ClaimSourceDistributedType
 * @psalm-type AddressClaimType = array{}&array{
 *     formatted?: string,
 *     street_address?: string,
 *     locality?: string,
 *     region?: string,
 *     postal_code?: string,
 *     country?: string,
 * }
 * @psalm-type TokenSetClaimsType = array{}&array{
 *     sub?: string,
 *     name?: string,
 *     given_name?: string,
 *     family_name?: string,
 *     middle_name?: string,
 *     nickname?: string,
 *     preferred_username?: string,
 *     profile?: string,
 *     picture?: string,
 *     website?: string,
 *     email?: string,
 *     email_verified?: bool,
 *     gender?: string,
 *     birthdate?: string,
 *     zoneinfo?: string,
 *     locale?: string,
 *     phone_number?: string,
 *     phone_number_verified?: bool,
 *     address?: AddressClaimType,
 *     updated_at?: int,
 *     _claim_names?: array<string, string>,
 *     _claim_sources?: array<string, ClaimSourceType>,
 * }
 * @psalm-type TokenSetAttributesType = array{}&array{
 *     code?: string,
 *     access_token?: string,
 *     id_token?: string,
 *     token_type?: string,
 *     refresh_token?: string,
 *     expires_in?: int,
 *     state?: string,
 *     code_verifier?: string,
 * }
 * @psalm-type TokenSetType = array{}&array{code?: string, state?: string, token_type?: string, access_token?: string, id_token?: string, refresh_token?: string, expires_in?: int, code_verifier?: string}
 * @psalm-type TokenSetMixedType = TokenSetAttributesType&array{claims?: TokenSetClaimsType}
 */
interface TokenSetInterface
{
    /**
     * Get all attributes
     *
     * @return array<string, mixed>
     *
     * @psalm-return TokenSetAttributesType
     */
    public function getAttributes(): array;

    public function getTokenType(): ?string;

    public function getAccessToken(): ?string;

    public function getIdToken(): ?string;

    public function getRefreshToken(): ?string;

    public function getExpiresIn(): ?int;

    public function getCodeVerifier(): ?string;

    public function getCode(): ?string;

    public function getState(): ?string;

    /**
     * @return array<string, mixed>
     *
     * @psalm-return TokenSetClaimsType
     */
    public function claims(): array;

    public function withIdToken(string $idToken): self;

    /**
     * @param array<string, mixed> $claims
     *
     * @return $this
     *
     * @psalm-param TokenSetClaimsType $claims
     */
    public function withClaims(array $claims): self;
}
