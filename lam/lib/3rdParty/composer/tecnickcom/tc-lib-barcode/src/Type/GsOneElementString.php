<?php

declare(strict_types=1);

/**
 * GsOneElementString.php
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

namespace Com\Tecnick\Barcode\Type;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\GsOneElementString
 *
 * GS1 Application Identifier element strings (GS1 General Specifications)
 *
 * Reads the bracketed form "(ai)value(ai)value...", which is also the human
 * readable interpretation, and checks the Application Identifier syntax, the
 * encodable character set 82 and the predefined element string lengths.
 * Parentheses are reserved as Application Identifier delimiters and cannot
 * appear in a value. Element strings whose Application Identifier is not listed
 * in PREDEFINED are of variable length and need an FNC1 separator unless they
 * are last.
 *
 * GS1 is a registered trademark of GS1 AISBL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class GsOneElementString
{
    /**
     * GS1 Application Identifier encodable character set 82
     * (the GS1 subset of ISO/IEC 646)
     *
     * @var string
     */
    public const CSET82 = '!"%&\'()*+,-./0123456789:;<=>?ABCDEFGHIJKLMNOPQRSTUVWXYZ_abcdefghijklmnopqrstuvwxyz';

    /**
     * Total length of the element strings with a predefined length, keyed by the
     * first two digits of the Application Identifier. Every other Application
     * Identifier is of variable length and needs an FNC1 separator.
     *
     * @var array<int|string, int>
     */
    public const PREDEFINED = [
        '00' => 20,
        '01' => 16,
        '02' => 16,
        '03' => 16,
        '04' => 18,
        '11' => 8,
        '12' => 8,
        '13' => 8,
        '14' => 8,
        '15' => 8,
        '16' => 8,
        '17' => 8,
        '18' => 8,
        '19' => 8,
        '20' => 4,
        '31' => 10,
        '32' => 10,
        '33' => 10,
        '34' => 10,
        '35' => 10,
        '36' => 10,
        '41' => 16,
    ];

    /**
     * Calculate the GS1 modulo 10 check digit of a numeric string.
     * The rightmost data digit has weight 3 and the weights alternate to the left.
     *
     * @param string $code Data digits without the check digit
     */
    public function getCheckDigit(string $code): int
    {
        $sum = 0;
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $sum += (int) $code[$pos] * ((($clen - $pos) % 2) === 1 ? 3 : 1);
        }

        return (10 - ($sum % 10)) % 10;
    }

    /**
     * Split the bracketed code into Application Identifier and value pairs.
     *
     * @param string $code Bracketed element strings
     *
     * @return array<int, array{string, string}>
     *
     * @throws BarcodeException if the bracketed form is malformed or cannot be encoded
     */
    public function parse(string $code): array
    {
        $clen = \strlen($code);
        if ($clen === 0) {
            throw new BarcodeException('Empty input');
        }

        $elements = [];
        $offset = 0;
        while ($offset < $clen) {
            if ($code[$offset] !== '(') {
                throw new BarcodeException(
                    'An Application Identifier in brackets is expected at position ' . $offset . ': ' . $code,
                );
            }

            $close = \strpos($code, ')', $offset);
            if ($close === false) {
                throw new BarcodeException('Unterminated Application Identifier: ' . $code);
            }

            $appid = \substr($code, $offset + 1, $close - $offset - 1);
            $offset = $close + 1;
            $next = \strpos($code, '(', $offset);
            $end = $next === false ? $clen : $next;
            $value = \substr($code, $offset, $end - $offset);
            $this->validate($appid, $value);
            $elements[] = [$appid, $value];
            $offset = $end;
        }

        return $elements;
    }

    /**
     * Check an Application Identifier and its value.
     *
     * @param string $appid Application Identifier digits, without brackets
     * @param string $value Data field of the element string
     *
     * @throws BarcodeException if the element string cannot be encoded
     */
    public function validate(string $appid, string $value): void
    {
        $ailen = \strlen($appid);
        if ($ailen < 2 || $ailen > 4 || !\ctype_digit($appid)) {
            throw new BarcodeException('Invalid Application Identifier: (' . $appid . ')');
        }

        if ($value === '') {
            throw new BarcodeException('Empty data field for the Application Identifier (' . $appid . ')');
        }

        if (\strcspn($value, '()') !== \strlen($value)) {
            throw new BarcodeException(
                'The parentheses are reserved as Application Identifier delimiters: (' . $appid . ')' . $value,
            );
        }

        if (\strspn($value, self::CSET82) !== \strlen($value)) {
            throw new BarcodeException(
                'The data field of the Application Identifier ('
                . $appid
                . ') contains characters outside the GS1 encodable character set 82',
            );
        }

        $length = self::PREDEFINED[\substr($appid, 0, 2)] ?? 0;
        if ($length === 0) {
            return;
        }

        if (($ailen + \strlen($value)) !== $length) {
            throw new BarcodeException(
                'The element string ('
                . $appid
                . ')'
                . $value
                . ' has a predefined length of '
                . $length
                . ' characters',
            );
        }

        if (!\ctype_digit($value)) {
            throw new BarcodeException(
                'The data field of the Application Identifier (' . $appid . ') must be a number',
            );
        }
    }

    /**
     * True when the element string of the given Application Identifier has a
     * predefined length and needs no FNC1 separator.
     *
     * @param string $appid Application Identifier digits, without brackets
     */
    public function hasPredefinedLength(string $appid): bool
    {
        return \array_key_exists(\substr($appid, 0, 2), self::PREDEFINED);
    }

    /**
     * Get the human readable interpretation of the element strings.
     *
     * @param array<int, array{string, string}> $elements Application Identifier and value pairs
     */
    public function getHumanReadable(array $elements): string
    {
        $hri = '';
        foreach ($elements as [$appid, $value]) {
            $hri .= '(' . $appid . ')' . $value;
        }

        return $hri;
    }

    /**
     * Get the character sequence of the element strings, with a separator after
     * every variable length element string that is not the last one.
     *
     * @param array<int, array{string, string}> $elements  Application Identifier and value pairs
     * @param string                            $separator Function 1 Symbol Character
     */
    public function getData(array $elements, string $separator): string
    {
        $data = '';
        $prefix = '';
        foreach ($elements as [$appid, $value]) {
            $data .= $prefix . $appid . $value;
            $prefix = $this->hasPredefinedLength($appid) ? '' : $separator;
        }

        return $data;
    }
}
