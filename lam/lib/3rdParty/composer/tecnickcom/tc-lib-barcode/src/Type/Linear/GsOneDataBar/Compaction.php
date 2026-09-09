<?php

declare(strict_types=1);

/**
 * Compaction.php
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

namespace Com\Tecnick\Barcode\Type\Linear\GsOneDataBar;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBar\Compaction
 *
 * General purpose data compaction of GS1 DataBar Expanded (ISO/IEC 24724)
 *
 * Turns a character sequence into a binary string with three encodation
 * schemes: numeric, which packs a pair of digits or a digit and the Function 1
 * Symbol Character into 7 bits and is current at the start of the field,
 * alphanumeric, which spends 5 or 6 bits per character, and ISO/IEC 646, which
 * spends 5, 7 or 8 bits per character. Latch characters switch between them.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Compaction
{
    /**
     * Function 1 Symbol Character, the element string separator
     *
     * @var string
     */
    public const FNC1 = "\xF1";

    /**
     * Number of bits of a symbol character
     */
    public const CHARACTER_BITS = 12;

    /**
     * Numeric encodation scheme
     */
    public const NUMERIC = 0;

    /**
     * Alphanumeric encodation scheme
     */
    public const ALPHANUMERIC = 1;

    /**
     * ISO/IEC 646 encodation scheme
     */
    public const ISO646 = 2;

    /**
     * Latch to the numeric encodation scheme
     *
     * @var string
     */
    protected const LATCH_NUMERIC = '000';

    /**
     * Latch to the alphanumeric encodation scheme, from the numeric scheme
     *
     * @var string
     */
    protected const LATCH_ALPHANUMERIC_FROM_NUMERIC = '0000';

    /**
     * Latch to the other of the alphanumeric and ISO/IEC 646 schemes.
     * It is also the padding unit, so the two schemes alternate without
     * encoding any data.
     *
     * @var string
     */
    protected const LATCH_SWITCH = '00100';

    /**
     * Value of the Function 1 Symbol Character in the numeric scheme
     */
    protected const FNC1_NUMERIC = 10;

    /**
     * Value of the Function 1 Symbol Character in the other two schemes
     */
    protected const FNC1_VALUE = 15;

    /**
     * Six bit values of the punctuation of the alphanumeric scheme
     *
     * @var array<string, int>
     */
    protected const ALPHANUMERIC_PUNCTUATION = [
        '*' => 58,
        ',' => 59,
        '-' => 60,
        '.' => 61,
        '/' => 62,
    ];

    /**
     * Eight bit values of the punctuation of the ISO/IEC 646 scheme
     *
     * @var array<string, int>
     */
    protected const ISO646_PUNCTUATION = [
        '!' => 232,
        '"' => 233,
        '%' => 234,
        '&' => 235,
        "'" => 236,
        '(' => 237,
        ')' => 238,
        '*' => 239,
        '+' => 240,
        ',' => 241,
        '-' => 242,
        '.' => 243,
        '/' => 244,
        ':' => 245,
        ';' => 246,
        '<' => 247,
        '=' => 248,
        '>' => 249,
        '?' => 250,
        '_' => 251,
        ' ' => 252,
    ];

    /**
     * Get the binary representation of a value.
     *
     * @param int $value  Value to represent
     * @param int $length Number of bits
     */
    protected function getBits(int $value, int $length): string
    {
        return \str_pad(\decbin($value), $length, '0', STR_PAD_LEFT);
    }

    /**
     * True when the character can be encoded by the numeric scheme.
     */
    protected function isNumeric(string $char): bool
    {
        return \ctype_digit($char) || $char === self::FNC1;
    }

    /**
     * True when the character can be encoded by the alphanumeric scheme.
     */
    protected function isAlphanumeric(string $char): bool
    {
        return $this->isNumeric($char) || $char >= 'A' && $char <= 'Z' || isset(self::ALPHANUMERIC_PUNCTUATION[$char]);
    }

    /**
     * Count how many of the characters that follow the given position can be
     * encoded by the given scheme, up to the given limit.
     *
     * @param string $data  Character sequence
     * @param int    $pos   Position of the first character to test
     * @param int    $limit Number of characters to test at most
     * @param bool   $alpha True to test the alphanumeric scheme, false the numeric one
     */
    protected function countEncodable(string $data, int $pos, int $limit, bool $alpha): int
    {
        $count = 0;
        $len = \strlen($data);
        while ($count < $limit && ($pos + $count) < $len) {
            $char = $data[$pos + $count];
            if (!($alpha ? $this->isAlphanumeric($char) : $this->isNumeric($char))) {
                break;
            }

            ++$count;
        }

        return $count;
    }

    /**
     * Get the bits of a character in the alphanumeric scheme.
     *
     * @throws BarcodeException if the character cannot be encoded
     */
    protected function getAlphanumericBits(string $char): string
    {
        if ($char === self::FNC1) {
            return $this->getBits(self::FNC1_VALUE, 5);
        }

        if (\ctype_digit($char)) {
            return $this->getBits(\ord($char) - 43, 5);
        }

        if ($char >= 'A' && $char <= 'Z') {
            return $this->getBits(\ord($char) - 33, 6);
        }

        $value = self::ALPHANUMERIC_PUNCTUATION[$char] ?? 0;
        if ($value === 0) {
            throw new BarcodeException('The alphanumeric encodation scheme cannot represent: ' . $char);
        }

        return $this->getBits($value, 6);
    }

    /**
     * Get the bits of a character in the ISO/IEC 646 scheme.
     *
     * @throws BarcodeException if the character cannot be encoded
     */
    protected function getIso646Bits(string $char): string
    {
        if ($char === self::FNC1) {
            return $this->getBits(self::FNC1_VALUE, 5);
        }

        if (\ctype_digit($char)) {
            return $this->getBits(\ord($char) - 43, 5);
        }

        if ($char >= 'A' && $char <= 'Z') {
            return $this->getBits(\ord($char) - 1, 7);
        }

        if ($char >= 'a' && $char <= 'z') {
            return $this->getBits(\ord($char) - 7, 7);
        }

        $value = self::ISO646_PUNCTUATION[$char] ?? 0;
        if ($value === 0) {
            throw new BarcodeException('The ISO/IEC 646 encodation scheme cannot represent: ' . $char);
        }

        return $this->getBits($value, 8);
    }

    /**
     * Encode the character at the given position in the numeric scheme.
     * The scheme packs a pair of characters into 7 bits. A single trailing digit
     * is packed either with the Function 1 Symbol Character, which the decoder
     * discards, or, when four to six bits are left in the symbol, on its own
     * into 4 bits.
     *
     * @param string $data    Character sequence
     * @param int    $pos     Position of the first character to encode, advanced by the encoded characters
     * @param int    $length  Number of bits emitted so far, the whole binary string included
     * @param int    $minimum Smallest number of symbol characters of the symbol
     * @param int    $mode    Current encodation scheme, changed by a latch
     */
    protected function encodeNumeric(string $data, int &$pos, int $length, int $minimum, int &$mode): string
    {
        $first = $data[$pos] ?? '';
        $second = $data[$pos + 1] ?? '';

        if (
            $second !== ''
            && $this->isNumeric($first)
            && $this->isNumeric($second)
            && ($first !== self::FNC1 || $second !== self::FNC1)
        ) {
            $left = $first === self::FNC1 ? self::FNC1_NUMERIC : (int) $first;
            $right = $second === self::FNC1 ? self::FNC1_NUMERIC : (int) $second;
            $pos += 2;
            return $this->getBits((11 * $left) + $right + 8, 7);
        }

        if ($second === '' && \ctype_digit($first)) {
            ++$pos;
            $left = (int) $first;
            $spare = $this->getCapacity($length, $minimum) - $length;
            if ($spare >= 4 && $spare <= 6) {
                return $this->getBits($left + 1, 4);
            }

            return $this->getBits((11 * $left) + self::FNC1_NUMERIC + 8, 7);
        }

        $mode = self::ALPHANUMERIC;
        return self::LATCH_ALPHANUMERIC_FROM_NUMERIC;
    }

    /**
     * Encode the character at the given position in the alphanumeric scheme.
     *
     * @param string $data Character sequence
     * @param int    $pos  Position of the character to encode, advanced by the encoded character
     * @param int    $mode Current encodation scheme, changed by a latch
     *
     * @throws BarcodeException if the character cannot be encoded
     */
    protected function encodeAlphanumeric(string $data, int &$pos, int &$mode): string
    {
        $char = $data[$pos] ?? '';
        if ($char !== self::FNC1) {
            if (!$this->isAlphanumeric($char)) {
                $mode = self::ISO646;
                return self::LATCH_SWITCH;
            }

            $numeric = $this->countEncodable($data, $pos, 6, false);
            if ($numeric >= 6 || $numeric >= 4 && ($pos + $numeric) === \strlen($data)) {
                $mode = self::NUMERIC;
                return self::LATCH_NUMERIC;
            }
        }

        ++$pos;
        return $this->getAlphanumericBits($char);
    }

    /**
     * Encode the character at the given position in the ISO/IEC 646 scheme.
     *
     * @param string $data Character sequence
     * @param int    $pos  Position of the character to encode, advanced by the encoded character
     * @param int    $mode Current encodation scheme, changed by a latch
     *
     * @throws BarcodeException if the character cannot be encoded
     */
    protected function encodeIso646(string $data, int &$pos, int &$mode): string
    {
        $char = $data[$pos] ?? '';
        if ($char !== self::FNC1) {
            $window = \min(10, \strlen($data) - $pos);
            if ($this->countEncodable($data, $pos, $window, true) === $window) {
                if ($this->countEncodable($data, $pos, 4, false) >= 4) {
                    $mode = self::NUMERIC;
                    return self::LATCH_NUMERIC;
                }

                if ($this->countEncodable($data, $pos, 5, true) >= 5) {
                    $mode = self::ALPHANUMERIC;
                    return self::LATCH_SWITCH;
                }
            }
        }

        ++$pos;
        return $this->getIso646Bits($char);
    }

    /**
     * Encode a character sequence into the general purpose data compaction field.
     *
     * @param string $data    Character sequence, with the Function 1 Symbol Character as separator
     * @param int    $prefix  Number of bits that precede the field in the binary string
     * @param int    $minimum Smallest number of symbol characters of the symbol
     * @param int    $mode    Encodation scheme in force at the end of the field
     *
     * @throws BarcodeException if the sequence cannot be encoded
     */
    public function encode(string $data, int $prefix, int $minimum, int &$mode): string
    {
        $bits = '';
        $mode = self::NUMERIC;
        $pos = 0;
        $len = \strlen($data);
        while ($pos < $len) {
            $bits .= match ($mode) {
                self::NUMERIC => $this->encodeNumeric($data, $pos, $prefix + \strlen($bits), $minimum, $mode),
                self::ALPHANUMERIC => $this->encodeAlphanumeric($data, $pos, $mode),
                default => $this->encodeIso646($data, $pos, $mode),
            };
        }

        return $bits;
    }

    /**
     * Number of symbol characters that holds the given number of bits, when the
     * symbology counts them differently from the smallest symbol that fits.
     *
     * @var (\Closure(int, int): int)|null
     */
    protected ?\Closure $charcount = null;

    /**
     * Set how the symbology counts the symbol characters.
     * The trailing digit of the numeric scheme takes the short form only when
     * it ends the symbol, so the count has to be the one of the symbol that is
     * actually drawn.
     *
     * @param \Closure(int, int): int $charcount Number of bits and smallest
     *                                          number of symbol characters to
     *                                          number of symbol characters
     */
    public function setCharacterCount(\Closure $charcount): void
    {
        $this->charcount = $charcount;
    }

    /**
     * Get the capacity in bits of the symbol that holds the given number of
     * bits.
     *
     * @param int $length  Number of bits to hold
     * @param int $minimum Smallest number of symbol characters of the symbol
     */
    public function getCapacity(int $length, int $minimum): int
    {
        $chars = $this->charcount === null
            ? \max($minimum, (int) \ceil($length / self::CHARACTER_BITS) + 1)
            : ($this->charcount)($length, $minimum);

        return self::CHARACTER_BITS * ($chars - 1);
    }

    /**
     * Get the padding of the binary string.
     * The numeric scheme is left first, because its latch is the only one that
     * a decoder does not read as data.
     *
     * @param int $missing Number of bits to fill
     * @param int $mode    Encodation scheme in force at the end of the data
     */
    public function getPadding(int $missing, int $mode): string
    {
        if ($missing <= 0) {
            return '';
        }

        $padding = $mode === self::NUMERIC ? self::LATCH_ALPHANUMERIC_FROM_NUMERIC : '';
        while (\strlen($padding) < $missing) {
            $padding .= self::LATCH_SWITCH;
        }

        return \substr($padding, 0, $missing);
    }
}
