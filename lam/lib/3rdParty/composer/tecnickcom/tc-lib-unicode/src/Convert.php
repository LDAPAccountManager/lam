<?php

declare(strict_types=1);

/**
 * Convert.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Com\Tecnick\Unicode;

use Com\Tecnick\Unicode\Exception as UniException;

/**
 * Com\Tecnick\Unicode\Convert
 *
 * Malformed input policy: a string or char array that is not valid UTF-8 raises an
 * exception, while a code point that cannot be encoded (negative, surrogate or greater
 * than U+10FFFF) is replaced with '?'.
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class Convert extends \Com\Tecnick\Unicode\Convert\Encoding
{
    /**
     * Highest Unicode code point.
     */
    protected const MAX_CODEPOINT = 0x10_FFFF;

    /**
     * First code point of the surrogate range.
     */
    protected const FIRST_SURROGATE = 0xD800;

    /**
     * Last code point of the surrogate range.
     */
    protected const LAST_SURROGATE = 0xDFFF;

    /**
     * Returns the code point unchanged, or the one of '?' when it cannot be encoded.
     * Surrogates and out-of-range values are substituted here because
     * mb_convert_encoding() passes surrogates through and pack('N') keeps only the low
     * 32 bits.
     *
     * @param int $ord Unicode code point
     */
    protected static function encodableOrd(int $ord): int
    {
        if ($ord < 0 || $ord > self::MAX_CODEPOINT) {
            return 0x3F; // '?' character
        }

        if ($ord >= self::FIRST_SURROGATE && $ord <= self::LAST_SURROGATE) {
            return 0x3F; // '?' character
        }

        return $ord;
    }

    /**
     * Returns the unicode string containing the character specified by value
     *
     * @param int $ord Unicode character value to convert
     *
     * @return string Returns the unicode string
     *
     * @throws UniException
     */
    public function chr(int $ord): string
    {
        $result = \mb_convert_encoding(\pack('N', self::encodableOrd($ord)), 'UTF-8', 'UCS-4BE');
        if ($result === false) {
            throw new UniException('Error converting character');
        }

        return $result;
    }

    /**
     * Returns the unicode value of the specified character.
     * If more than one character is given, only the first codepoint is returned.
     *
     * @param string $chr Unicode character
     *
     * @return int Returns the unicode value
     *
     * @throws UniException
     */
    public function ord(string $chr): int
    {
        if (!\mb_check_encoding($chr, 'UTF-8')) {
            throw new UniException('Invalid UTF-8 string');
        }

        $ucs = \mb_convert_encoding($chr, 'UCS-4BE', 'UTF-8');
        if ($ucs === false || \strlen($ucs) < 4) {
            throw new UniException('Error converting string');
        }

        $uni = \unpack('N', $ucs);
        if ($uni === false) {
            throw new UniException('Error converting string');
        }

        return $uni[1];
    }

    /**
     * Converts an UTF-8 string to an array of UTF-8 characters
     *
     * @param string $str String to convert
     *
     * @return array<int, string>
     *
     * @throws UniException
     */
    public function strToChrArr(string $str): array
    {
        $ret = \preg_split('//u', $str, -1, PREG_SPLIT_NO_EMPTY);
        if ($ret === false) {
            throw new UniException('Error splitting string');
        }

        return $ret;
    }

    /**
     * Converts an array of UTF-8 chars to an array of codepoints (integer values)
     *
     * @param array<string> $chars Array of UTF-8 chars
     *
     * @return array<int>
     *
     * @throws UniException
     */
    public function chrArrToOrdArr(array $chars): array
    {
        if ($chars === []) {
            return [];
        }

        return $this->strToOrdArr(\implode('', $chars));
    }

    /**
     * Converts an array of UTF-8 code points to an array of chars
     *
     * @param array<int> $ords Array of UTF-8 code points
     *
     * @return array<string>
     *
     * @throws UniException
     */
    public function ordArrToChrArr(array $ords): array
    {
        if ($ords === []) {
            return [];
        }

        $valid = \array_map(self::encodableOrd(...), $ords);

        $str = \mb_convert_encoding(\pack('N*', ...$valid), 'UTF-8', 'UCS-4BE');
        if ($str === false) {
            throw new UniException('Error converting code points');
        }

        return \mb_str_split($str, 1, 'UTF-8');
    }

    /**
     * Converts an UTF-8 string to an array of UTF-8 codepoints (integer values)
     *
     * @param string $str String to convert
     *
     * @return array<int>
     *
     * @throws UniException
     */
    public function strToOrdArr(string $str): array
    {
        if ($str === '') {
            return [];
        }

        if (!\mb_check_encoding($str, 'UTF-8')) {
            throw new UniException('Invalid UTF-8 string');
        }

        $ucs = \mb_convert_encoding($str, 'UCS-4BE', 'UTF-8');
        if ($ucs === false) {
            throw new UniException('Error converting string');
        }

        $ords = \unpack('N*', $ucs);
        if ($ords === false) {
            throw new UniException('Error unpacking string');
        }

        return \array_values($ords);
    }

    /**
     * Extract a slice of the $uniarr array and return it as string
     *
     * @param array<string> $uniarr The input array of characters
     * @param int   $start  The position of the starting element
     * @param int|null   $end    The position of the first element that will not be returned.
     *                           An $end that is not after $start returns an empty string.
     *
     * @return string
     */
    public function getSubUniArrStr(array $uniarr, int $start = 0, ?int $end = null): string
    {
        if ($end === null) {
            $end = \count($uniarr);
        }

        // A negative length makes array_slice() stop that many elements before the end of
        // the array, so an empty range is clamped to zero.
        return \implode('', \array_slice($uniarr, $start, \max(0, $end - $start)));
    }
}
