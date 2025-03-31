<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Checker;

use Jose\Component\Checker\HeaderChecker;
use Jose\Component\Checker\InvalidHeaderException;

/**
 * This class is a header parameter checker.
 * When the "enc" header parameter is present, it will check if the value is within the allowed ones.
 *
 * @internal
 */
final class ContentEncryptionAlgorithmChecker implements HeaderChecker
{
    private const HEADER_NAME = 'enc';

    /** @var bool */
    private $protectedHeader;

    /** @var string[] */
    private $supportedAlgorithms;

    /**
     * @param string[] $supportedAlgorithms
     */
    public function __construct(array $supportedAlgorithms, bool $protectedHeader = false)
    {
        $this->supportedAlgorithms = $supportedAlgorithms;
        $this->protectedHeader = $protectedHeader;
    }

    /**
     * @param mixed $value
     *
     * @throws InvalidHeaderException if the header is invalid
     */
    public function checkHeader($value): void
    {
        if (! is_string($value)) {
            throw new InvalidHeaderException('"enc" must be a string.', self::HEADER_NAME, $value);
        }
        if (! in_array($value, $this->supportedAlgorithms, true)) {
            throw new InvalidHeaderException('Unsupported algorithm.', self::HEADER_NAME, $value);
        }
    }

    public function supportedHeader(): string
    {
        return self::HEADER_NAME;
    }

    public function protectedHeaderOnly(): bool
    {
        return $this->protectedHeader;
    }
}
