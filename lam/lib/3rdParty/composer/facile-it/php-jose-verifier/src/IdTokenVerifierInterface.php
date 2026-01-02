<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

/**
 * @psalm-api
 */
interface IdTokenVerifierInterface extends TokenVerifierInterface
{
    public function withAccessToken(?string $accessToken): IdTokenVerifierInterface;

    public function withCode(?string $code): IdTokenVerifierInterface;

    public function withState(?string $state): IdTokenVerifierInterface;
}
