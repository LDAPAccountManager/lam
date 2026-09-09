<?php

declare(strict_types=1);

/**
 * Hibc.php
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
 * Com\Tecnick\Barcode\Type\Hibc
 *
 * Hibc data structure class
 * HIBC (Health Industry Bar Code) data structures
 *
 * Validates a data structure of the Supplier Labeling Standard (ANSI/HIBC 2.6)
 * or of the Provider Applications Standard (ANSI/HIBC 1.3) and appends the
 * modulo 43 check character. The data structure is independent of the symbology
 * that carries it, so the same string is handed unchanged to CODE 39, CODE 128,
 * Data Matrix, QR Code or Aztec Code.
 *
 * HIBC and HIBCC are trademarks of the Health Industry Business
 * Communications Council.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Hibc
{
    /**
     * Encodable characters, in the order of the numerical values used by the
     * modulo 43 check character. The value of a character is its position here.
     *
     * @var string
     */
    public const CHARSET = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-. $/+%';

    /**
     * Characters of a data field
     *
     * @var string
     */
    protected const ALPHANUM = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ';

    /**
     * Characters of a flag field
     *
     * @var string
     */
    protected const LETTERS = 'ABCDEFGHIJKLMNOPQRSTUVWXYZ';

    /**
     * Characters of a numeric field
     *
     * @var string
     */
    protected const DIGITS = '0123456789';

    /**
     * Characters of the Lot/Batch or Serial Number field. The minus sign and
     * the period are the only special characters allowed in the secondary data.
     *
     * @var string
     */
    protected const LOTCHARS = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ-.';

    /**
     * Flag character of a HIBC data structure
     *
     * @var string
     */
    protected const FLAG = '+';

    /**
     * Length of the Labeler Identification Code
     */
    protected const LIC_LENGTH = 4;

    /**
     * Maximum length of the Product or Catalog Number
     */
    protected const MAX_PCN = 18;

    /**
     * Maximum length of the Lot/Batch or Serial Number field
     */
    protected const MAX_LOT = 18;

    /**
     * Maximum length of the data field of a Provider Applications field
     */
    protected const MAX_PAS_DATA = 15;

    /**
     * Length of the five digit Julian date of the superseded secondary format
     */
    protected const JULIAN_LENGTH = 5;

    /**
     * Number of characters of the expiry date field, keyed by the date format
     * indicator that opens it. The indicator is counted because for the formats
     * 0 and 1 it doubles as the first digit of the month.
     *
     * @var array<int|string, int>
     */
    protected const DATE_LENGTH = [
        '0' => 4, // MMYY
        '1' => 4, // MMYY
        '2' => 7, // MMDDYY
        '3' => 7, // YYMMDD
        '4' => 9, // YYMMDDHH
        '5' => 6, // YYJJJ
        '6' => 8, // YYJJJHH
        '7' => 1, // no date field
    ];

    /**
     * Supplemental data identifiers, each with the minimum length, the maximum
     * length and whether the data that follows it is numeric.
     *
     * @var array<string, array{int, int, bool}>
     */
    protected const SUPPLEMENTAL = [
        '14D' => [8, 8, true], // expiry date, YYYYMMDD
        '16D' => [8, 8, true], // production date, YYYYMMDD
        'S' => [1, 18, false], // serial number
        'Q' => [1, 5, true], // quantity
    ];

    /**
     * Validate a HIBC data structure and append its modulo 43 check character.
     *
     * @param string $code Data structure, including the leading flag character
     *                     and without the check character.
     *
     * @throws BarcodeException if the data structure is malformed
     */
    public function format(string $code): string
    {
        $this->validate($code);
        return $code . $this->checkCharacter($code);
    }

    /**
     * Calculate the modulo 43 check character of a data structure.
     * It is the character whose value is the remainder of the division by 43 of
     * the sum of the values of all the characters of the message.
     *
     * @param string $code Data structure without the check character.
     */
    public function checkCharacter(string $code): string
    {
        $sum = 0;
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $value = \strpos($this::CHARSET, $code[$pos]);
            $sum += $value === false ? 0 : $value;
        }

        return $this::CHARSET[$sum % \strlen($this::CHARSET)];
    }

    /**
     * Check the data structure and dispatch it to the format it declares.
     * The character that follows the flag character tells the formats apart: a
     * slash opens a Provider Applications data structure, an alphabetic
     * character a Supplier Labeling primary data structure, and a digit or a
     * dollar sign a Supplier Labeling secondary data structure.
     *
     * @param string $code Data structure without the check character.
     *
     * @throws BarcodeException if the data structure is malformed
     */
    protected function validate(string $code): void
    {
        if ($code === '') {
            throw new BarcodeException('Empty input');
        }

        if (\strspn($code, $this::CHARSET) !== \strlen($code)) {
            throw new BarcodeException('The code contains characters outside the HIBC character set: ' . $code);
        }

        if ($code[0] !== $this::FLAG) {
            throw new BarcodeException('A HIBC data structure starts with the flag character "+": ' . $code);
        }

        $data = \substr($code, 1);
        $first = $data[0] ?? '';
        if ($first === '/') {
            $this->validateProviderData(\substr($data, 1));
            return;
        }

        if (\strspn($first, $this::LETTERS) === 1) {
            $this->validatePrimaryData($data);
            return;
        }

        if ($first === '$' || \strspn($first, $this::DIGITS) === 1) {
            $this->validateSecondaryData($data);
            return;
        }

        throw new BarcodeException('The flag character must be followed by "/", a letter, a digit or "$": ' . $code);
    }

    /**
     * Check a Supplier Labeling primary data structure, made of the Labeler
     * Identification Code, the Product or Catalog Number and the Unit of
     * Measure Identifier. A slash concatenates the secondary data structure to
     * it, in which case the two are covered by a single check character.
     *
     * @param string $data Primary data structure without the flag character.
     *
     * @throws BarcodeException if the data structure is malformed
     */
    protected function validatePrimaryData(string $data): void
    {
        $slash = \strpos($data, '/');
        $primary = $slash === false ? $data : \substr($data, 0, $slash);
        $plen = \strlen($primary);
        $minlen = $this::LIC_LENGTH + 2;
        $maxlen = $this::LIC_LENGTH + $this::MAX_PCN + 1;
        if ($plen < $minlen || $plen > $maxlen) {
            throw new BarcodeException(
                'The primary data structure is '
                . $plen
                . ' characters long, expecting from '
                . $minlen
                . ' to '
                . $maxlen,
            );
        }

        if (\strspn($primary, $this::ALPHANUM) !== $plen) {
            throw new BarcodeException('The primary data structure must be alphanumeric: ' . $primary);
        }

        if (\strspn($primary, $this::LETTERS) === 0) {
            throw new BarcodeException('The first character of the Labeler Identification Code must be alphabetic: '
            . $primary);
        }

        if (\strspn($primary[$plen - 1], $this::DIGITS) === 0) {
            throw new BarcodeException('The Unit of Measure Identifier must be a digit: ' . $primary);
        }

        if ($slash !== false) {
            // the concatenated structure is covered by a single check
            // character, so the secondary part carries no Link Character
            $this->validateSecondaryData(\substr($data, $slash + 1), false);
        }
    }

    /**
     * Check a Supplier Labeling secondary data structure and the supplemental
     * data fields that may follow it.
     *
     * @param string $data   Secondary data structure without the flag character.
     * @param bool   $linked True when the structure stands in a symbol of its
     *                       own and carries the Link Character.
     *
     * @throws BarcodeException if the data structure is malformed
     */
    protected function validateSecondaryData(string $data, bool $linked = true): void
    {
        if ($data === '') {
            throw new BarcodeException('Empty secondary data structure');
        }

        $slash = \strpos($data, '/');
        $field = $slash === false ? $data : \substr($data, 0, $slash);
        $this->validateLotField(\substr($field, $this->getDateOffset($field)), $linked);
        if ($slash !== false) {
            $this->validateSupplementalData(\substr($data, $slash + 1));
        }
    }

    /**
     * Get the offset of the Lot/Batch or Serial Number field, that is the
     * number of characters taken by the reference identifier and by the expiry
     * date field that follows it.
     *
     * @param string $field Secondary data structure up to the supplemental data.
     *
     * @throws BarcodeException if the reference identifier is malformed
     */
    protected function getDateOffset(string $field): int
    {
        if (\str_starts_with($field, '$$+')) {
            return 3 + $this->getDateLength($field, 3);
        }

        if (\str_starts_with($field, '$$')) {
            return 2 + $this->getDateLength($field, 2);
        }

        // the serial number and the lot number references without a date field
        if (\str_starts_with($field, '$+')) {
            return 2;
        }

        if (\str_starts_with($field, '$')) {
            return 1;
        }

        // a leading digit opens the five digit Julian date of the superseded format
        if (\strspn($field, $this::DIGITS) < $this::JULIAN_LENGTH) {
            throw new BarcodeException('A secondary data structure that starts with a digit carries a five digit Julian date: '
            . $field);
        }

        return $this::JULIAN_LENGTH;
    }

    /**
     * Get the number of characters of the expiry date field, the date format
     * indicator included.
     *
     * @param string $field  Secondary data structure up to the supplemental data.
     * @param int    $offset Position of the date format indicator.
     *
     * @throws BarcodeException if the date field is malformed
     */
    protected function getDateLength(string $field, int $offset): int
    {
        $indicator = $field[$offset] ?? '';
        $length = $this::DATE_LENGTH[$indicator] ?? 0;
        if ($length === 0) {
            throw new BarcodeException('The date format indicator must be a digit from 0 to 7: ' . $field);
        }

        if (\strspn($field, $this::DIGITS, $offset, $length) !== $length) {
            throw new BarcodeException('The expiry date field must be a number of ' . $length . ' digits: ' . $field);
        }

        return $length;
    }

    /**
     * Check the Lot/Batch or Serial Number field.
     * The field is followed by the Link Character when the secondary data
     * structure is encoded in a symbol of its own. The Link Character is the
     * check character of the primary symbol, so it may be any character of the
     * HIBC character set and it cannot be told apart from the last character of
     * the field, which is why only the characters before it are checked.
     *
     * @param string $field  Lot/Batch or Serial Number field, and the Link
     *                       Character when there is one.
     * @param bool   $linked True when the field is followed by the Link
     *                       Character. A concatenated primary and secondary
     *                       data structure has none, so the whole field is
     *                       known and every character of it is checked.
     *
     * @throws BarcodeException if the field is malformed
     */
    protected function validateLotField(string $field, bool $linked = true): void
    {
        $link = $linked ? 1 : 0;
        $flen = \strlen($field);
        if ($flen > ($this::MAX_LOT + $link)) {
            throw new BarcodeException(
                'The Lot/Batch or Serial Number field is longer than ' . $this::MAX_LOT . ' characters: ' . $field,
            );
        }

        $lot = \substr($field, 0, \max(0, $flen - $link));
        if (\strspn($lot, $this::LOTCHARS) !== \strlen($lot)) {
            throw new BarcodeException('The Lot/Batch or Serial Number field must be alphanumeric: ' . $field);
        }
    }

    /**
     * Check the supplemental data fields, each opened by a data identifier.
     * The quantity is the last field when it is used together with others.
     *
     * @param string $data Supplemental data fields, separated by a slash.
     *
     * @throws BarcodeException if a supplemental data field is malformed
     */
    protected function validateSupplementalData(string $data): void
    {
        $fields = \explode('/', $data);
        $last = \count($fields) - 1;
        foreach ($fields as $idx => $field) {
            $dii = $this->validateSupplementalField($field);
            if ($dii === 'Q' && $idx !== $last) {
                throw new BarcodeException('The quantity must be the last supplemental data field: ' . $data);
            }
        }
    }

    /**
     * Check one supplemental data field and return its data identifier.
     *
     * @param string $field Supplemental data field, data identifier included.
     *
     * @throws BarcodeException if the field is malformed
     */
    protected function validateSupplementalField(string $field): string
    {
        foreach ($this::SUPPLEMENTAL as $dii => $format) {
            if (!\str_starts_with($field, $dii)) {
                continue;
            }

            $value = \substr($field, \strlen($dii));
            $vlen = \strlen($value);
            $charset = $format[2] ? $this::DIGITS : $this::ALPHANUM;
            if ($vlen < $format[0] || $vlen > $format[1] || \strspn($value, $charset) !== $vlen) {
                throw new BarcodeException(
                    'The supplemental data field ' . $dii . ' does not match its format: ' . $field,
                );
            }

            return $dii;
        }

        throw new BarcodeException('Unknown supplemental data identifier: ' . $field);
    }

    /**
     * Check a Provider Applications data structure, made of one or more fields
     * separated by a slash. The first field carries the flag characters that
     * tell where the data structure is located and what the data is, the
     * following ones only the flag characters that tell what the data is.
     *
     * @param string $data Data structure without the "+/" identifier.
     *
     * @throws BarcodeException if the data structure is malformed
     */
    protected function validateProviderData(string $data): void
    {
        $fields = \explode('/', $data);
        foreach ($fields as $idx => $field) {
            $offset = $idx === 0 ? $this->getProviderFlagLength($field, 0) : 0;
            $offset += $this->getProviderFlagLength($field, $offset);
            $value = \substr($field, $offset);
            $vlen = \strlen($value);
            if ($vlen < 1 || $vlen > $this::MAX_PAS_DATA) {
                throw new BarcodeException(
                    'A Provider Applications data field holds from 1 to '
                    . $this::MAX_PAS_DATA
                    . ' characters: '
                    . $field,
                );
            }

            if (\strspn($value, $this::ALPHANUM) !== $vlen) {
                throw new BarcodeException('A Provider Applications data field must be alphanumeric: ' . $field);
            }
        }
    }

    /**
     * Get the number of characters of a Provider Applications flag field.
     * A flag character is a single letter, or three letters when it begins with
     * the letter "Y", which is reserved for the flag characters that HIBCC
     * assigns as new needs are defined.
     *
     * @param string $field  Provider Applications field.
     * @param int    $offset Position of the flag field.
     *
     * @throws BarcodeException if the flag field is malformed
     */
    protected function getProviderFlagLength(string $field, int $offset): int
    {
        $length = ($field[$offset] ?? '') === 'Y' ? 3 : 1;
        if (\strspn($field, $this::LETTERS, $offset, $length) !== $length) {
            throw new BarcodeException('A Provider Applications flag character must be alphabetic: ' . $field);
        }

        return $length;
    }
}
