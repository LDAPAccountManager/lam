<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Exception;

use Throwable;

/**
 * @psalm-api
 */
class InvalidTokenClaimException extends RuntimeException implements InvalidTokenExceptionInterface
{
    /**
     * @param mixed $value
     */
    public function __construct(
        string $message,
        private readonly string $claim,
        private $value,
        ?Throwable $previous = null,
    ) {
        parent::__construct($message, 0, $previous);
    }

    public function getClaim(): string
    {
        return $this->claim;
    }

    /**
     * Returns the claim value that caused the exception.
     *
     * @return mixed
     */
    public function getValue()
    {
        return $this->value;
    }
}
