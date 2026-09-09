<?php

declare(strict_types=1);

/**
 * Compaction.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\MicroQrCode;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\Square\QrCode\Data as QrData;

/**
 * Com\Tecnick\Barcode\Type\Square\MicroQrCode\Compaction
 *
 * Encodation modes of the MicroQrCode Barcode type class
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Compaction
{
    /**
     * Returns the encodation mode able to represent the given code, choosing
     * the one that yields the shortest bit stream.
     *
     * @param string $code Code to encode.
     */
    protected function detectMode(string $code): int
    {
        if (\preg_match('/^[0-9]*\z/', $code) === 1) {
            return Data::MODE_NUMERIC;
        }

        if ($this->isAlphanumeric($code)) {
            return Data::MODE_ALPHANUM;
        }

        return Data::MODE_BYTE;
    }

    /**
     * Returns whether every character of the code belongs to the alphanumeric
     * character set of Table 5 of ISO/IEC 18004.
     *
     * @param string $code Code to encode.
     */
    protected function isAlphanumeric(string $code): bool
    {
        $len = \strlen($code);
        for ($idx = 0; $idx < $len; ++$idx) {
            if (!\str_contains(QrData::AN_CHARS, $code[$idx])) {
                return false;
            }
        }

        return true;
    }

    /**
     * Check that the code can be represented in the given encodation mode.
     *
     * @param string $code Code to encode.
     * @param int    $mode Encodation mode.
     *
     * @throws BarcodeException if the code contains a character the mode cannot represent
     */
    protected function checkMode(string $code, int $mode): void
    {
        if ($mode === Data::MODE_NUMERIC && \preg_match('/^[0-9]*\z/', $code) !== 1) {
            throw new BarcodeException('The numeric mode can only encode digits: ' . $code);
        }

        if ($mode === Data::MODE_ALPHANUM && !$this->isAlphanumeric($code)) {
            throw new BarcodeException('The alphanumeric mode cannot encode this code: ' . $code);
        }
    }

    /**
     * Returns the number of bits taken by the data of the given code in the
     * given encodation mode, excluding the mode and character count indicators.
     *
     * @param string $code Code to encode.
     * @param int    $mode Encodation mode.
     */
    protected function getDataBits(string $code, int $mode): int
    {
        $len = \strlen($code);

        return match ($mode) {
            Data::MODE_NUMERIC => (10 * \intdiv($len, 3)) + (Data::NUMERIC_REMAINDER_BITS[$len % 3] ?? 0),
            Data::MODE_ALPHANUM => (11 * \intdiv($len, 2)) + (6 * ($len % 2)),
            default => 8 * $len,
        };
    }

    /**
     * Returns the data bit stream of the given code in the given encodation mode.
     *
     * @param string $code Code to encode.
     * @param int    $mode Encodation mode.
     */
    protected function getDataStream(string $code, int $mode): string
    {
        return match ($mode) {
            Data::MODE_NUMERIC => $this->getNumericStream($code),
            Data::MODE_ALPHANUM => $this->getAlphanumStream($code),
            default => $this->getByteStream($code),
        };
    }

    /**
     * Returns the bit stream of the numeric mode: groups of three digits in ten
     * bits, and one or two trailing digits in four or seven bits.
     *
     * @param string $code Code to encode.
     */
    protected function getNumericStream(string $code): string
    {
        $bits = '';
        $len = \strlen($code);
        for ($idx = 0; $idx < $len; $idx += 3) {
            $group = \substr($code, $idx, 3);
            $size = Data::NUMERIC_REMAINDER_BITS[\strlen($group) % 3] ?? 0;
            $bits .= $this->getBits((int) $group, $size === 0 ? 10 : $size);
        }

        return $bits;
    }

    /**
     * Returns the bit stream of the alphanumeric mode: pairs of characters in
     * eleven bits, and one trailing character in six bits.
     *
     * @param string $code Code to encode.
     */
    protected function getAlphanumStream(string $code): string
    {
        $bits = '';
        $len = \strlen($code);
        for ($idx = 0; $idx < $len; $idx += 2) {
            $first = \max(0, (int) \strpos(QrData::AN_CHARS, $code[$idx]));
            if (($idx + 1) >= $len) {
                $bits .= $this->getBits($first, 6);
                break;
            }

            $second = \max(0, (int) \strpos(QrData::AN_CHARS, $code[$idx + 1]));
            $bits .= $this->getBits((45 * $first) + $second, 11);
        }

        return $bits;
    }

    /**
     * Returns the bit stream of the byte mode: eight bits per character.
     *
     * @param string $code Code to encode.
     */
    protected function getByteStream(string $code): string
    {
        $bits = '';
        $len = \strlen($code);
        for ($idx = 0; $idx < $len; ++$idx) {
            $bits .= $this->getBits(\ord($code[$idx]), 8);
        }

        return $bits;
    }

    /**
     * Returns the binary representation of a value over a fixed number of bits.
     *
     * @param int $value Value to represent.
     * @param int $size  Number of bits.
     */
    protected function getBits(int $value, int $size): string
    {
        if ($size <= 0) {
            return '';
        }

        return \substr(\str_pad(\decbin($value), $size, '0', \STR_PAD_LEFT), -$size);
    }
}
