<?php

declare(strict_types=1);

/**
 * Telepen.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\Telepen;
 *
 * Telepen Barcode type class
 * Telepen (full ASCII)
 *
 * Telepen encodes a stream of bits rather than one pattern per character. Each
 * character is its ASCII value in eight bits with even parity, least
 * significant bit first, so the stream always holds an even number of zero
 * bits. The stream is split into single one bits and blocks of zero, any number
 * of one bits, zero, and each of those is one bar and space pair.
 *
 * Telepen is a registered trademark of S.B. Electronic Systems Ltd.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Telepen extends \Com\Tecnick\Barcode\Type\Linear
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'TELEPEN';

    /**
     * Start character, ASCII underscore
     */
    protected const START = 95;

    /**
     * Stop character, ASCII lower case z
     */
    protected const STOP = 122;

    /**
     * Modulus of the check character
     */
    protected const MODULUS = 127;

    /**
     * Narrow bar followed by a narrow space
     *
     * @var string
     */
    protected const NARROW_NARROW = '10';

    /**
     * Wide bar followed by a narrow space
     *
     * @var string
     */
    protected const WIDE_NARROW = '1110';

    /**
     * Wide bar followed by a wide space
     *
     * @var string
     */
    protected const WIDE_WIDE = '111000';

    /**
     * Narrow bar followed by a wide space
     *
     * @var string
     */
    protected const NARROW_WIDE = '1000';

    /**
     * Get the eight bits of a character, even parity and least significant bit first
     */
    protected function getCharBits(int $char): string
    {
        $bits = \strrev(\str_pad(\decbin($char), 7, '0', STR_PAD_LEFT));
        return $bits . (string) (\substr_count($bits, '1') % 2);
    }

    /**
     * Calculate the check character
     *
     * @param string $code Code to represent.
     */
    protected function getChecksum(string $code): int
    {
        $sum = 0;
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $sum += \ord($code[$pos]);
        }

        return ($this::MODULUS - ($sum % $this::MODULUS)) % $this::MODULUS;
    }

    /**
     * Check that every character of the code is in the encodable ASCII range
     *
     * @throws BarcodeException in case of an unsupported character
     */
    protected function validateCode(): void
    {
        if ($this->code === '') {
            throw new BarcodeException('Empty input string');
        }

        $clen = \strlen($this->code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            if (\ord($this->code[$pos]) > 127) {
                throw new BarcodeException('Invalid character: ' . (\ord($this->code[$pos]) & 0xFF));
            }
        }
    }

    /**
     * Format code
     */
    protected function formatCode(): void
    {
        $this->extcode = $this->code . \chr($this->getChecksum($this->code));
    }

    /**
     * Get the bit stream of the symbol, from the start character to the stop character
     */
    protected function getBitStream(): string
    {
        $bits = $this->getCharBits($this::START);
        $clen = \strlen($this->extcode);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $bits .= $this->getCharBits(\ord($this->extcode[$pos]));
        }

        return $bits . $this->getCharBits($this::STOP);
    }

    /**
     * Get the bar and space pair of the block of one bits that starts at the
     * given position, and advance the position past the block
     *
     * @param int $pos Position of the zero bit that opens the block
     *
     * @throws BarcodeException if the block is not closed by a zero bit
     */
    protected function getBlockSequence(string $bits, int &$pos): string
    {
        $end = \strpos($bits, '0', $pos + 1);
        if (!\is_int($end)) {
            throw new BarcodeException('Unterminated block of one bits');
        }

        $ones = $end - $pos - 1;
        $pos = $end + 1;

        return match ($ones) {
            0 => $this::WIDE_NARROW,
            1 => $this::WIDE_WIDE,
            default => $this::NARROW_WIDE . \str_repeat($this::NARROW_NARROW, \max(0, $ones - 2)) . $this::NARROW_WIDE,
        };
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->validateCode();
        $this->formatCode();

        $bits = $this->getBitStream();
        $len = \strlen($bits);
        $seq = '';
        $pos = 0;
        while ($pos < $len) {
            if ($bits[$pos] === '1') {
                $seq .= $this::NARROW_NARROW;
                ++$pos;
                continue;
            }

            $seq .= $this->getBlockSequence($bits, $pos);
        }

        // the trailing space of the stop character is part of the quiet zone
        $this->processBinarySequence($this->getRawCodeRows(\rtrim($seq, '0')));
    }
}
