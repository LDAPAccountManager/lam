<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Internal;

use DateTimeImmutable;
use Psr\Clock\ClockInterface;

/**
 * @internal
 */
final class InternalClock implements ClockInterface
{
    private ?DateTimeImmutable $now;

    public function __construct(?DateTimeImmutable $dateTime = null)
    {
        $this->now = $dateTime;
    }

    public function now(): DateTimeImmutable
    {
        return $this->now ?? new DateTimeImmutable();
    }
}
