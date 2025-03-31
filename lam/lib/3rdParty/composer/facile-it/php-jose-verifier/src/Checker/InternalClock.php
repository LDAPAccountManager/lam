<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Checker;

use DateTimeImmutable;
use Psr\Clock\ClockInterface;

/**
 * @internal
 */
final class InternalClock implements ClockInterface
{
    public function now(): DateTimeImmutable
    {
        return new DateTimeImmutable();
    }
}
