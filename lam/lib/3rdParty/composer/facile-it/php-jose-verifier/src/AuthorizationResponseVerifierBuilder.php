<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

/**
 * @template-extends AbstractTokenVerifierBuilder<JWTVerifier>
 */
class AuthorizationResponseVerifierBuilder extends AbstractTokenVerifierBuilder
{
    /**
     * @inheritDoc
     */
    protected function getVerifier(string $issuer, string $clientId): AbstractTokenVerifier
    {
        return new JWTVerifier($issuer, $clientId, $this->buildDecrypter());
    }

    protected function getExpectedAlg(): ?string
    {
        return $this->getClientMetadata()['authorization_signed_response_alg'] ?? null;
    }

    protected function getExpectedEncAlg(): ?string
    {
        return $this->getClientMetadata()['authorization_encrypted_response_alg'] ?? null;
    }

    protected function getExpectedEnc(): ?string
    {
        return $this->getClientMetadata()['authorization_encrypted_response_enc'] ?? null;
    }
}
