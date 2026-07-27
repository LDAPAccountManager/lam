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
        $result = \mb_convert_encoding(\pack('N', $ord), 'UTF-8', 'UCS-4BE');
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

        $str = \mb_convert_encoding(\pack('N*', ...$ords), 'UTF-8', 'UCS-4BE');
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
     *
     * @return string
     */
    public function getSubUniArrStr(array $uniarr, int $start = 0, ?int $end = null): string
    {
        if ($end === null) {
            $end = \count($uniarr);
        }

        return \implode('', \array_slice($uniarr, $start, $end - $start));
    }
}
