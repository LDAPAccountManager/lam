<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Internal\Checker;

use Jose\Component\Checker\ClaimChecker;
use Jose\Component\Checker\InvalidClaimException;

use function sprintf;

/**
 * @internal
 */
final class NonceChecker implements ClaimChecker
{
    private const CLAIM_NAME = 'nonce';

    private string $nonce;

    public function __construct(string $nonce)
    {
        $this->nonce = $nonce;
    }

    /**
     * @param mixed $value
     *
     * @throws InvalidClaimException
     */
    public function checkClaim($value): void
    {
        if ($value !== $this->nonce) {
            throw new InvalidClaimException(sprintf('Nonce mismatch, expected %s, got: %s', $this->nonce, (string) $value), self::CLAIM_NAME, $value);
        }
    }

    public function supportedClaim(): string
    {
        return self::CLAIM_NAME;
    }
}
