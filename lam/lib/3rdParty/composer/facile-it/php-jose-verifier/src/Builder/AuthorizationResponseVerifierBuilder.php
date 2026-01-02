<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Builder;

use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Facile\JoseVerifier\JWTVerifier;
use Facile\JoseVerifier\TokenVerifierInterface;

/**
 * @psalm-api
 *
 * @psalm-import-type IssuerMetadataType from TokenVerifierInterface
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 *
 * @template-extends AbstractTokenVerifierBuilder<JWTVerifier>
 */
final class AuthorizationResponseVerifierBuilder extends AbstractTokenVerifierBuilder
{
    /**
     * @psalm-param IssuerMetadataType $issuerMetadata
     * @psalm-param ClientMetadataType $clientMetadata
     */
    public static function create(array $issuerMetadata, array $clientMetadata): static
    {
        return new self($issuerMetadata, $clientMetadata);
    }

    /**
     * @throws InvalidArgumentException
     */
    public function build(): JWTVerifier
    {
        return new JWTVerifier(
            $this->getIssuer(),
            $this->getClientId(),
            $this->getClientSecret(),
            $this->getAuthTimeRequired(),
            $this->clockTolerance,
            $this->aadIssValidation,
            $this->getExpectedAzp(),
            $this->getExpectedAlg(),
            $this->getJwksProvider(),
            $this->buildDecrypter()
        );
    }

    protected function getExpectedAlg(): ?string
    {
        return $this->clientMetadata['authorization_signed_response_alg'] ?? null;
    }

    protected function getExpectedEncAlg(): ?string
    {
        return $this->clientMetadata['authorization_encrypted_response_alg'] ?? null;
    }

    protected function getExpectedEnc(): ?string
    {
        return $this->clientMetadata['authorization_encrypted_response_enc'] ?? null;
    }
}
