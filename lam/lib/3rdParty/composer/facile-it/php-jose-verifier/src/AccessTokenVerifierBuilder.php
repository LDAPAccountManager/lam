<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

/**
 * @template-extends AbstractTokenVerifierBuilder<JWTVerifier>
 */
final class AccessTokenVerifierBuilder extends AbstractTokenVerifierBuilder
{
    protected function getVerifier(string $issuer, string $clientId): AbstractTokenVerifier
    {
        return new JWTVerifier($issuer, $clientId, $this->buildDecrypter());
    }

    protected function getExpectedAlg(): ?string
    {
        return null;
    }

    protected function getExpectedEncAlg(): ?string
    {
        return null;
    }

    protected function getExpectedEnc(): ?string
    {
        return null;
    }
}
