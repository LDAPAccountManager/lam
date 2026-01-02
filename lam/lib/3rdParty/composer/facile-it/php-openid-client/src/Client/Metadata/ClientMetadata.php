<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client\Metadata;

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
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 */
final class ClientMetadata implements ClientMetadataInterface
{
    /**
     * @var array<string, mixed>
     *
     * @psalm-var ClientMetadataType
     */
    private array $metadata;

    /** @var string[] */
    private static array $requiredKeys = [
        'client_id',
    ];

    /** @var array<string, mixed> */
    private static array $defaults = [];

    /**
     * IssuerMetadata constructor.
     *
     * @param array<string, mixed> $claims
     *
     * @psalm-param ClientMetadataType|array<empty, empty> $claims
     */
    public function __construct(string $clientId, array $claims = [])
    {
        $requiredClaims = [
            'client_id' => $clientId,
        ];

        $defaults = self::$defaults;

        /** @var ClientMetadataType $merged */
        $merged = array_merge($defaults, $claims, $requiredClaims);
        $this->metadata = $merged;
    }

    /**
     * @param array<string, mixed> $claims
     *
     * @psalm-param ClientMetadataType $claims
     *
     * @return static
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

    #[Override]
    public function getClientId(): string
    {
        return $this->metadata['client_id'];
    }

    #[Override]
    public function getClientSecret(): ?string
    {
        return $this->metadata['client_secret'] ?? null;
    }

    #[Override]
    public function getRedirectUris(): array
    {
        return $this->metadata['redirect_uris'] ?? [];
    }

    #[Override]
    public function getResponseTypes(): array
    {
        return $this->metadata['response_types'] ?? ['code'];
    }

    #[Override]
    public function getTokenEndpointAuthMethod(): string
    {
        return $this->metadata['token_endpoint_auth_method'] ?? 'client_secret_basic';
    }

    #[Override]
    public function getAuthorizationSignedResponseAlg(): ?string
    {
        return $this->metadata['authorization_signed_response_alg'] ?? null;
    }

    #[Override]
    public function getAuthorizationEncryptedResponseAlg(): ?string
    {
        return $this->metadata['authorization_encrypted_response_alg'] ?? null;
    }

    #[Override]
    public function getAuthorizationEncryptedResponseEnc(): ?string
    {
        return $this->metadata['authorization_encrypted_response_enc'] ?? null;
    }

    #[Override]
    public function getIdTokenSignedResponseAlg(): string
    {
        return $this->metadata['id_token_signed_response_alg'] ?? 'RS256';
    }

    #[Override]
    public function getIdTokenEncryptedResponseAlg(): ?string
    {
        return $this->metadata['id_token_encrypted_response_alg'] ?? null;
    }

    #[Override]
    public function getIdTokenEncryptedResponseEnc(): ?string
    {
        return $this->metadata['id_token_encrypted_response_enc'] ?? null;
    }

    #[Override]
    public function getUserinfoSignedResponseAlg(): ?string
    {
        return $this->metadata['userinfo_signed_response_alg'] ?? null;
    }

    #[Override]
    public function getUserinfoEncryptedResponseAlg(): ?string
    {
        return $this->metadata['userinfo_encrypted_response_alg'] ?? null;
    }

    #[Override]
    public function getUserinfoEncryptedResponseEnc(): ?string
    {
        return $this->metadata['userinfo_encrypted_response_enc'] ?? null;
    }

    #[Override]
    public function getRequestObjectSigningAlg(): ?string
    {
        return $this->metadata['request_object_signing_alg'] ?? null;
    }

    #[Override]
    public function getRequestObjectEncryptionAlg(): ?string
    {
        return $this->metadata['request_object_encryption_alg'] ?? null;
    }

    #[Override]
    public function getRequestObjectEncryptionEnc(): ?string
    {
        return $this->metadata['request_object_encryption_enc'] ?? null;
    }

    #[Override]
    public function getIntrospectionEndpointAuthMethod(): string
    {
        return $this->metadata['introspection_endpoint_auth_method'] ?? $this->getTokenEndpointAuthMethod();
    }

    #[Override]
    public function getRevocationEndpointAuthMethod(): string
    {
        return $this->metadata['revocation_endpoint_auth_method'] ?? $this->getTokenEndpointAuthMethod();
    }

    #[Override]
    public function getJwks(): ?array
    {
        return $this->metadata['jwks'] ?? null;
    }

    #[Override]
    public function jsonSerialize(): array
    {
        return $this->toArray();
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

    /**
     * @return mixed|null
     */
    #[Override]
    public function get(string $name)
    {
        return $this->metadata[$name] ?? null;
    }
}
