<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\ClaimChecker;

use Jose\Component\Checker\ClaimChecker;
use Jose\Component\Checker\InvalidClaimException;
use function sprintf;

final class AzpChecker implements ClaimChecker
{
    private const CLAIM_NAME = 'azp';

    /** @var string */
    private $azp;

    public function __construct(string $azp)
    {
        $this->azp = $azp;
    }

    /**
     * {@inheritdoc}
     */
    public function checkClaim($value): void
    {
        if ($value !== $this->azp) {
            throw new InvalidClaimException(sprintf('azp must be the client_id, expected %s, got: %s', $this->azp, (string) $value), self::CLAIM_NAME, $value);
        }
    }

    public function supportedClaim(): string
    {
        return self::CLAIM_NAME;
    }
}
