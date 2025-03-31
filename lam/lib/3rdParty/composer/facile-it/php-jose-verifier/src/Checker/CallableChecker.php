<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Checker;

use Jose\Component\Checker\ClaimChecker;
use Jose\Component\Checker\HeaderChecker;
use Jose\Component\Checker\InvalidClaimException;
use Jose\Component\Checker\InvalidHeaderException;

/**
 * @internal
 */
final class CallableChecker implements ClaimChecker, HeaderChecker
{
    /** @var string */
    private $key;

    /**
     * @var callable
     *
     * @psalm-var callable(mixed): bool
     */
    private $callable;

    /**
     * @psalm-param callable(mixed): bool $callable
     */
    public function __construct(string $key, callable $callable)
    {
        $this->key = $key;
        $this->callable = $callable;
    }

    /**
     * @param mixed $value
     *
     * @throws InvalidClaimException if the claim is invalid
     */
    public function checkClaim($value): void
    {
        $callable = $this->callable;
        $isValid = $callable($value);
        if (! $isValid) {
            throw new InvalidClaimException(sprintf('Invalid claim "%s"', $this->key), $this->key, $value);
        }
    }

    public function supportedClaim(): string
    {
        return $this->key;
    }

    /**
     * @param mixed $value
     */
    public function checkHeader($value): void
    {
        $callable = $this->callable;
        $isValid = $callable($value);
        if (! $isValid) {
            throw new InvalidHeaderException(sprintf('Invalid header "%s"', $this->key), $this->key, $value);
        }
    }

    public function supportedHeader(): string
    {
        return $this->key;
    }

    public function protectedHeaderOnly(): bool
    {
        return true;
    }
}
