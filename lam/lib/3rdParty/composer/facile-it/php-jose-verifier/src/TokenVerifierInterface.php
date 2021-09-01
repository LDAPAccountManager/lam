<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\Exception\InvalidTokenException;

/**
 * @psalm-import-type JWTPayloadObject from Psalm\PsalmTypes
 */
interface TokenVerifierInterface
{
    /**
     * @return $this
     */
    public function withNonce(?string $nonce);

    /**
     * @return $this
     */
    public function withMaxAge(?int $maxAge);

    /**
     * Verify OpenID token
     *
     * @throws InvalidTokenException
     *
     * @return array The JWT Payload
     * @psalm-return JWTPayloadObject
     */
    public function verify(string $jwt): array;
}
