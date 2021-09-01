<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\ClaimChecker;

use Base64Url\Base64Url;
use function hash;
use Jose\Component\Checker\ClaimChecker;
use Jose\Component\Checker\InvalidClaimException;
use function round;
use function sprintf;
use function strlen;
use function substr;

abstract class AbstractHashChecker implements ClaimChecker
{
    /** @var string */
    private $valueToCheck;

    /** @var string */
    private $alg;

    /**
     * SHashChecker constructor.
     */
    public function __construct(string $valueToCheck, string $alg)
    {
        $this->valueToCheck = $valueToCheck;
        $this->alg = $alg;
    }

    private function getShaSize(string $alg): string
    {
        $size = substr($alg, -3);

        switch ($size) {
            case '512':
                return 'sha512';
            case '384':
                return 'sha384';
            default:
                return 'sha256';
        }
    }

    /**
     * {@inheritdoc}
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
