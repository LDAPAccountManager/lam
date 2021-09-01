<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client\Metadata;

use function array_diff;
use function array_key_exists;
use function array_keys;
use function array_merge;
use function count;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use function implode;

/**
 * @psalm-import-type ClientMetadataObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
final class ClientMetadata implements ClientMetadataInterface
{
    /**
     * @var array<string, mixed>
     * @psalm-var ClientMetadataObject
     */
    private $metadata;

    /** @var string[] */
    private static $requiredKeys = [
        'client_id',
    ];

    /** @var array<string, mixed> */
    private static $defaults = [];

    /**
     * IssuerMetadata constructor.
     *
     * @param array<string, mixed> $claims
     * @psalm-param ClientMetadataObject|array<empty, empty> $claims
     */
    public function __construct(string $clientId, array $claims = [])
    {
        $requiredClaims = [
            'client_id' => $clientId,
        ];

        $defaults = self::$defaults;

        /** @var ClientMetadataObject $merged */
        $merged = array_merge($defaults, $claims, $requiredClaims);
        $this->metadata = $merged;
    }

    /**
     * @param array<string, mixed> $claims
     *
     * @return static
     *
     * @psalm-param ClientMetadataObject $claims
     */
    public static function fromArray(array $claims): self
    {
        $missingKeys = array_diff(self::$requiredKeys, array_keys($claims));
        if (0 !== count($missingKeys)) {
            throw new InvalidArgumentException(
                'Invalid client metadata. Missing keys: ' . implode(', ', $missingKeys)
            );
        }

        return new static($claims['client_id'], $claims);
    }

    public function getClientId(): string
    {
        return $this->metadata['client_id'];
    }

    public function getClientSecret(): ?string
    {
        return $this->metadata['client_secret'] ?? null;
    }

    public function getRedirectUris(): array
    {
        return $this->metadata['redirect_uris'] ?? [];
    }

    public function getResponseTypes(): array
    {
        return $this->metadata['response_types'] ?? ['code'];
    }

    public function getTokenEndpointAuthMethod(): string
    {
        return $this->metadata['token_endpoint_auth_method'] ?? 'client_secret_basic';
    }

    public function getAuthorizationSignedResponseAlg(): ?string
    {
        return $this->metadata['authorization_signed_response_alg'] ?? null;
    }

    public function getAuthorizationEncryptedResponseAlg(): ?string
    {
        return $this->metadata['authorization_encrypted_response_alg'] ?? null;
    }

    public function getAuthorizationEncryptedResponseEnc(): ?string
    {
        return $this->metadata['authorization_encrypted_response_enc'] ?? null;
    }

    public function getIdTokenSignedResponseAlg(): string
    {
        return $this->metadata['id_token_signed_response_alg'] ?? 'RS256';
    }

    public function getIdTokenEncryptedResponseAlg(): ?string
    {
        return $this->metadata['id_token_encrypted_response_alg'] ?? null;
    }

    public function getIdTokenEncryptedResponseEnc(): ?string
    {
        return $this->metadata['id_token_encrypted_response_enc'] ?? null;
    }

    public function getUserinfoSignedResponseAlg(): ?string
    {
        return $this->metadata['userinfo_signed_response_alg'] ?? null;
    }

    public function getUserinfoEncryptedResponseAlg(): ?string
    {
        return $this->metadata['userinfo_encrypted_response_alg'] ?? null;
    }

    public function getUserinfoEncryptedResponseEnc(): ?string
    {
        return $this->metadata['userinfo_encrypted_response_enc'] ?? null;
    }

    public function getRequestObjectSigningAlg(): ?string
    {
        return $this->metadata['request_object_signing_alg'] ?? null;
    }

    public function getRequestObjectEncryptionAlg(): ?string
    {
        return $this->metadata['request_object_encryption_alg'] ?? null;
    }

    public function getRequestObjectEncryptionEnc(): ?string
    {
        return $this->metadata['request_object_encryption_enc'] ?? null;
    }

    public function getIntrospectionEndpointAuthMethod(): string
    {
        return $this->metadata['introspection_endpoint_auth_method'] ?? $this->getTokenEndpointAuthMethod();
    }

    public function getRevocationEndpointAuthMethod(): string
    {
        return $this->metadata['revocation_endpoint_auth_method'] ?? $this->getTokenEndpointAuthMethod();
    }

    public function getJwks(): ?array
    {
        return $this->metadata['jwks'] ?? null;
    }

    public function jsonSerialize(): array
    {
        return $this->toArray();
    }

    public function toArray(): array
    {
        return $this->metadata;
    }

    public function has(string $name): bool
    {
        return array_key_exists($name, $this->metadata);
    }

    /**
     * @return mixed|null
     */
    public function get(string $name)
    {
        return $this->metadata[$name] ?? null;
    }
}
