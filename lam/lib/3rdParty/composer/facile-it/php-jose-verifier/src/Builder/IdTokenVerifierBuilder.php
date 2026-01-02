<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Builder;

use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Facile\JoseVerifier\IdTokenVerifier;
use Facile\JoseVerifier\TokenVerifierInterface;

/**
 * @psalm-api
 *
 * @psalm-import-type IssuerMetadataType from TokenVerifierInterface
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 *
 * @template-extends AbstractTokenVerifierBuilder<IdTokenVerifier>
 */
final class IdTokenVerifierBuilder extends AbstractTokenVerifierBuilder
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
    public function build(): IdTokenVerifier
    {
        return new IdTokenVerifier(
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
        return $this->clientMetadata['id_token_signed_response_alg'] ?? null;
    }

    protected function getExpectedEncAlg(): ?string
    {
        return $this->clientMetadata['id_token_encrypted_response_alg'] ?? null;
    }

    protected function getExpectedEnc(): ?string
    {
        return $this->clientMetadata['id_token_encrypted_response_enc'] ?? null;
    }
}
