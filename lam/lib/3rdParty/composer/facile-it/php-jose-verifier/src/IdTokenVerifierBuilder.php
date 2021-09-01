<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

/**
 * @template-extends AbstractTokenVerifierBuilder<IdTokenVerifier>
 */
final class IdTokenVerifierBuilder extends AbstractTokenVerifierBuilder
{
    protected function getVerifier(string $issuer, string $clientId): AbstractTokenVerifier
    {
        return new IdTokenVerifier($issuer, $clientId, $this->buildDecrypter());
    }

    protected function getExpectedAlg(): ?string
    {
        return $this->getClientMetadata()['id_token_signed_response_alg'] ?? null;
    }

    protected function getExpectedEncAlg(): ?string
    {
        return $this->getClientMetadata()['id_token_encrypted_response_alg'] ?? null;
    }

    protected function getExpectedEnc(): ?string
    {
        return $this->getClientMetadata()['id_token_encrypted_response_enc'] ?? null;
    }
}
