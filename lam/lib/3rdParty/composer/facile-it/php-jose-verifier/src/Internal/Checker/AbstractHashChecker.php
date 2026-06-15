<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Internal\Checker;

use Base64Url\Base64Url;
use Jose\Component\Checker\ClaimChecker;
use Jose\Component\Checker\InvalidClaimException;

use function hash;
use function round;
use function sprintf;
use function strlen;
use function substr;

/**
 * @internal
 */
abstract class AbstractHashChecker implements ClaimChecker
{
    public function __construct(
        private readonly string $valueToCheck,
        private readonly string $alg,
    ) {}

    private function getShaSize(string $alg): string
    {
        $size = substr($alg, -3);

        return match ($size) {
            '512' => 'sha512',
            '384' => 'sha384',
            default => 'sha256',
        };
    }

    /**
     * @param mixed $value
     *
     * @throws InvalidClaimException
     */
    public function checkClaim($value): void
    {
        $hash = hash($this->getShaSize($this->alg), $this->valueToCheck, true);
        $generated = Base64Url::encode(substr($hash, 0, (int) round(strlen($hash) / 2)));

        if ($value !== $generated) {
            throw new InvalidClaimException(sprintf($this->supportedClaim() . ' mismatch, expected %s, got: %s', $generated, (string) $value), $this->supportedClaim(), $value);
        }
    }
}
