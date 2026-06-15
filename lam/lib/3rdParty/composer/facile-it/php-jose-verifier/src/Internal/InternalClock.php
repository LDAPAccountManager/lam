<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Internal;

use DateTimeImmutable;
use Psr\Clock\ClockInterface;

/**
 * @internal
 */
final readonly class InternalClock implements ClockInterface
{
    public function __construct(
        private ?DateTimeImmutable $now = null,
    ) {}

    public function now(): DateTimeImmutable
    {
        return $this->now ?? new DateTimeImmutable();
    }
}
