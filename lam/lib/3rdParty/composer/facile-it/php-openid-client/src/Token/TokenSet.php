<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

use function array_key_exists;
use JsonSerializable;

/**
 * @psalm-import-type TokenSetType from TokenSetInterface
 * @psalm-import-type TokenSetClaimsType from TokenSetInterface
 * @psalm-import-type TokenSetMixedType from TokenSetInterface
 */
final class TokenSet implements TokenSetInterface, JsonSerializable
{
    /**
     * @var array<string, mixed>
     * @psalm-var TokenSetType
     */
    private $attributes = [];

    /**
     * @var array<string, mixed>
     * @psalm-var TokenSetClaimsType
     */
    private $claims = [];

    /**
     * @psalm-param TokenSetType $attributes
     * @psalm-param TokenSetClaimsType $claims
     */
    private function __construct(array $attributes, array $claims)
    {
        $this->attributes = $attributes;
        $this->claims = $claims;
    }

    /**
     * @param array<string, mixed> $data
     * @psalm-param TokenSetMixedType $data
     */
    public static function fromParams(array $data): TokenSetInterface
    {
        $claims = [];
        if (array_key_exists('claims', $data)) {
            $claims = $data['claims'];
            unset($data['claims']);
        }

        return new static($data, $claims);
    }

    public function getAttributes(): array
    {
        return $this->attributes;
    }

    public function getCode(): ?string
    {
        return $this->attributes['code'] ?? null;
    }

    public function getState(): ?string
    {
        return $this->attributes['state'] ?? null;
    }

    public function getTokenType(): ?string
    {
        return $this->attributes['token_type'] ?? null;
    }

    public function getAccessToken(): ?string
    {
        return $this->attributes['access_token'] ?? null;
    }

    public function getIdToken(): ?string
    {
        return $this->attributes['id_token'] ?? null;
    }

    public function getRefreshToken(): ?string
    {
        return $this->attributes['refresh_token'] ?? null;
    }

    public function getExpiresIn(): ?int
    {
        /** @var int|string|null $expiresIn */
        $expiresIn = $this->attributes['expires_in'] ?? null;

        return null !== $expiresIn ? (int) $expiresIn : null;
    }

    public function getCodeVerifier(): ?string
    {
        return $this->attributes['code_verifier'] ?? null;
    }

    public function withIdToken(string $idToken): TokenSetInterface
    {
        $clone = clone $this;
        $clone->attributes['id_token'] = $idToken;

        return $clone;
    }

    public function withClaims(array $claims): TokenSetInterface
    {
        $clone = clone $this;
        $clone->claims = $claims;

        return $clone;
    }

    /**
     * @return array<string, mixed>
     * @psalm-return TokenSetType
     */
    public function jsonSerialize(): array
    {
        return $this->attributes;
    }

    public function claims(): array
    {
        return $this->claims;
    }
}
