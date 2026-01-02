<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Exception;

use Throwable;

/**
 * @psalm-api
 */
class InvalidTokenClaimException extends RuntimeException implements InvalidTokenExceptionInterface
{
    private string $claim;

    /** @var mixed */
    private $value;

    /**
     * @param mixed $value
     */
    public function __construct(string $message, string $claim, $value, ?Throwable $previous = null)
    {
        parent::__construct($message, 0, $previous);
        $this->claim = $claim;
        $this->value = $value;
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
