<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Builder;

use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Facile\JoseVerifier\TokenVerifierInterface;
use Facile\JoseVerifier\UserInfoVerifier;

/**
 * @psalm-api
 *
 * @psalm-import-type IssuerMetadataType from TokenVerifierInterface
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 *
 * @template-extends AbstractTokenVerifierBuilder<UserInfoVerifier>
 */
final class UserInfoVerifierBuilder extends AbstractTokenVerifierBuilder
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
    public function build(): UserInfoVerifier
    {
        return new UserInfoVerifier(
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
        return $this->clientMetadata['userinfo_signed_response_alg'] ?? null;
    }

    protected function getExpectedEncAlg(): ?string
    {
        return $this->clientMetadata['userinfo_encrypted_response_alg'] ?? null;
    }

    protected function getExpectedEnc(): ?string
    {
        return $this->clientMetadata['userinfo_encrypted_response_enc'] ?? null;
    }
}
