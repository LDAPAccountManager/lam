<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Client\Metadata;

use Facile\JoseVerifier\TokenVerifierInterface;
use JsonSerializable;
use Override;

/**
 * @psalm-api
 *
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 * @psalm-import-type JWTPayloadType from TokenVerifierInterface
 * @psalm-import-type JWKSetType from TokenVerifierInterface
 */
interface ClientMetadataInterface extends JsonSerializable
{
    /**
     * @return mixed|null
     */
    public function get(string $name);

    public function has(string $name): bool;

    public function getClientId(): string;

    public function getClientSecret(): ?string;

    /**
     * @return string[]
     */
    public function getRedirectUris(): array;

    /**
     * @return string[]
     */
    public function getResponseTypes(): array;

    public function getTokenEndpointAuthMethod(): string;

    public function getAuthorizationSignedResponseAlg(): ?string;

    public function getAuthorizationEncryptedResponseAlg(): ?string;

    public function getAuthorizationEncryptedResponseEnc(): ?string;

    public function getIdTokenSignedResponseAlg(): string;

    public function getIdTokenEncryptedResponseAlg(): ?string;

    public function getIdTokenEncryptedResponseEnc(): ?string;

    public function getUserinfoSignedResponseAlg(): ?string;

    public function getUserinfoEncryptedResponseAlg(): ?string;

    public function getUserinfoEncryptedResponseEnc(): ?string;

    public function getRequestObjectSigningAlg(): ?string;

    public function getRequestObjectEncryptionAlg(): ?string;

    public function getRequestObjectEncryptionEnc(): ?string;

    public function getIntrospectionEndpointAuthMethod(): string;

    public function getRevocationEndpointAuthMethod(): string;

    /**
     * @psalm-return JWKSetType|null
     */
    public function getJwks(): ?array;

    /**
     * @return array<string, mixed>
     *
     * @psalm-return ClientMetadataType
     */
    public function toArray(): array;

    /**
     * @return ClientMetadataType
     */
    #[Override]
    public function jsonSerialize(): array;
}
