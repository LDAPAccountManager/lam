<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Internal\Checker;

use Facile\JoseVerifier\Internal\InternalClock;
use Jose\Component\Checker\ClaimChecker;
use Jose\Component\Checker\InvalidClaimException;
use Psr\Clock\ClockInterface;

use function is_int;

/**
 * @internal
 */
final class AuthTimeChecker implements ClaimChecker
{
    private const CLAIM_NAME = 'auth_time';

    private int $maxAge;

    private int $allowedTimeDrift;

    private ClockInterface $clock;

    public function __construct(int $maxAge, int $allowedTimeDrift = 0, ?ClockInterface $clock = null)
    {
        $this->maxAge = $maxAge;
        $this->allowedTimeDrift = $allowedTimeDrift;
        $this->clock = $clock ?? new InternalClock();
    }

    /**
     * @throws InvalidClaimException
     */
    public function checkClaim(mixed $value): void
    {
        if (! is_int($value)) {
            throw new InvalidClaimException('"auth_time" must be an integer.', self::CLAIM_NAME, $value);
        }

        if ($value + $this->maxAge < $this->clock->now()->getTimestamp() - $this->allowedTimeDrift) {
            throw new InvalidClaimException('Too much time has elapsed since the last End-User authentication.', self::CLAIM_NAME, $value);
        }
    }

    public function supportedClaim(): string
    {
        return self::CLAIM_NAME;
    }
}
