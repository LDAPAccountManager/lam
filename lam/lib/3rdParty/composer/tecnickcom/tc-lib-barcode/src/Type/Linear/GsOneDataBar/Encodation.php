<?php

declare(strict_types=1);

/**
 * Encodation.php
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
use Com\Tecnick\Barcode\Type\GsOneElementString;

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBar\Encodation
 *
 * Encodation methods of GS1 DataBar Expanded (ISO/IEC 24724)
 *
 * Chooses the encodation method of a sequence of GS1 Application Identifier
 * element strings and builds its compressed data field. The general methods
 * carry every element string in the general purpose data compaction field; the
 * selected sequences of element strings listed in section 5.5.2.3.3 of the GS1
 * General Specifications are packed into shorter fixed fields instead.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Encodation
{
    /**
     * Application Identifier of the Global Trade Item Number
     *
     * @var string
     */
    protected const APPID = '01';

    /**
     * Indicator digit that the variable measure methods require
     *
     * @var string
     */
    protected const INDICATOR = '9';

    /**
     * Number of digits of a Global Trade Item Number
     */
    protected const ITEM_LENGTH = 14;

    /**
     * Value of the date subfield when no date is encoded
     */
    protected const NO_DATE = 38_400;

    /**
     * Application Identifiers of the date element string, in the order of the
     * two bits that select them
     *
     * @var array<int, string>
     */
    protected const DATE_APPID = ['11', '13', '15', '17'];

    /**
     * Element string parser
     */
    protected GsOneElementString $parser;

    public function __construct()
    {
        $this->parser = new GsOneElementString();
    }

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
     * Get the 40 bit compression of a Global Trade Item Number of a variable
     * measure trade item: the twelve digits that are left once the leading 9 and
     * the check digit are dropped, in four groups of three digits.
     *
     * @param string $value Data field of the Application Identifier (01)
     */
    protected function getMeasureItemBits(string $value): string
    {
        $bits = '';
        for ($pos = 1; $pos < ($this::ITEM_LENGTH - 1); $pos += 3) {
            $bits .= $this->getBits((int) \substr($value, $pos, 3), 10);
        }

        return $bits;
    }

    /**
     * Check the Global Trade Item Number of the first element string.
     *
     * @param string $value Data field of the Application Identifier (01)
     *
     * @throws BarcodeException if the check digit is wrong
     */
    protected function checkItem(string $value): void
    {
        $check = $this->parser->getCheckDigit(\substr($value, 0, $this::ITEM_LENGTH - 1));
        if ($check !== (int) ($value[$this::ITEM_LENGTH - 1] ?? '')) {
            throw new BarcodeException('Invalid check digit: ' . $check);
        }
    }

    /**
     * Get the encodation of the item identification and weight sequences of
     * fixed length, or null when the element strings are not one of them.
     *
     * @param array<int, array{string, string}> $elements Application Identifier and value pairs
     *
     * @return array{string, string, string, int, int}|null
     */
    protected function getWeightEncodation(array $elements): ?array
    {
        if (\count($elements) !== 2) {
            return null;
        }

        $item = $this->getMeasureItemBits($elements[0][1] ?? '');
        $appid = $elements[1][0] ?? '';
        $value = (int) ($elements[1][1] ?? '0');

        // weight in kilogrammes with three decimal places, up to 32,767 kg
        if ($appid === '3103' && $value <= 32_767) {
            return ['0100', $item . $this->getBits($value, 15), '', 6, 6];
        }

        // weight in pounds with two decimal places, up to 99.99 lb
        if ($appid === '3202' && $value <= 9_999) {
            return ['0101', $item . $this->getBits($value, 15), '', 6, 6];
        }

        // weight in pounds with three decimal places, up to 22.767 lb
        if ($appid === '3203' && $value <= 22_767) {
            return ['0101', $item . $this->getBits($value + 10_000, 15), '', 6, 6];
        }

        return null;
    }

    /**
     * Get the encodation of the item identification, weight and date sequences
     * of fixed length, or null when the element strings are not one of them.
     *
     * @param array<int, array{string, string}> $elements Application Identifier and value pairs
     *
     * @return array{string, string, string, int, int}|null
     *
     * @throws BarcodeException if the date is not a valid calendar date
     */
    protected function getDateEncodation(array $elements): ?array
    {
        $count = \count($elements);
        if ($count < 2 || $count > 3) {
            return null;
        }

        $appid = $elements[1][0] ?? '';
        $weight = $elements[1][1] ?? '';
        // the weight is carried by its last five digits, so the first one must be zero
        if (\preg_match('/^3[12]0[0-9]\z/', $appid) !== 1 || $weight[0] !== '0') {
            return null;
        }

        $date = $this::NO_DATE;
        // the second digit of the Application Identifier tells the metric
        // element string 310x from the non metric one 320x
        $selector = $appid[1] === '2' ? 1 : 0;
        if ($count === 3) {
            $index = \array_search($elements[2][0] ?? '', $this::DATE_APPID, true);
            if ($index === false) {
                return null;
            }

            $date = $this->getDateValue($elements[2][1] ?? '');
            $selector += 2 * (int) $index;
        }

        $bits =
            $this->getMeasureItemBits($elements[0][1] ?? '')
            . $this->getBits((int) ($appid[3] . \substr($weight, 1)), 20)
            . $this->getBits($date, 16);

        return ['0111' . $this->getBits($selector, 3), $bits, '', 8, 8];
    }

    /**
     * Get the 16 bit compression of a date in the YYMMDD format.
     *
     * @param string $date Six digit date
     *
     * @throws BarcodeException if the date is not a valid calendar date
     */
    protected function getDateValue(string $date): int
    {
        $year = (int) \substr($date, 0, 2);
        $month = (int) \substr($date, 2, 2);
        $day = (int) \substr($date, 4, 2);
        if ($month < 1 || $month > 12 || $day > 31) {
            throw new BarcodeException('Invalid date: ' . $date);
        }

        return ($year * 384) + (($month - 1) * 32) + $day;
    }

    /**
     * Get the encodation of the item identification and price sequences, or null
     * when the element strings are not one of them.
     *
     * @param array<int, array{string, string}> $elements Application Identifier and value pairs
     *
     * @return array{string, string, string, int, int}|null
     */
    protected function getPriceEncodation(array $elements): ?array
    {
        $appid = $elements[1][0] ?? '';
        $value = $elements[1][1] ?? '';
        if (\preg_match('/^39[23][0-3]\z/', $appid) !== 1) {
            return null;
        }

        $item = $this->getMeasureItemBits($elements[0][1] ?? '');
        $decimals = $this->getBits((int) $appid[3], 2);
        $rest = \array_slice($elements, 2);

        // the price with the ISO 4217 currency code, which takes ten more bits
        if ($appid[2] === '3') {
            $bits = $item . $decimals . $this->getBits((int) \substr($value, 0, 3), 10);
            return ['01101', $bits, $this->getData(\substr($value, 3), $rest), 0, 7];
        }

        return ['01100', $item . $decimals, $this->getData($value, $rest), 0, 6];
    }

    /**
     * Join a leading data field to the element strings that follow it.
     *
     * @param string                            $value    Data field left to the general purpose compaction
     * @param array<int, array{string, string}> $elements Element strings that follow it
     */
    protected function getData(string $value, array $elements): string
    {
        if ($elements === []) {
            return $value;
        }

        return $value . Compaction::FNC1 . $this->parser->getData($elements, Compaction::FNC1);
    }

    /**
     * Get the encodation method of a sequence of element strings.
     *
     * @param array<int, array{string, string}> $elements Application Identifier and value pairs
     *
     * @return array{string, string, string, int, int} Encodation method bits, compressed data
     *                                                 field bits, characters left to the general
     *                                                 purpose data compaction, fixed number of
     *                                                 symbol characters or zero, and smallest
     *                                                 number of symbol characters
     *
     * @throws BarcodeException if the element strings cannot be encoded
     */
    public function getEncodation(array $elements): array
    {
        $first = $elements[0] ?? ['', ''];
        if ($first[0] !== $this::APPID) {
            return ['00', '', $this->parser->getData($elements, Compaction::FNC1), 0, 4];
        }

        $this->checkItem($first[1]);
        $rest = \array_slice($elements, 1);

        if ($rest !== [] && $first[1][0] === $this::INDICATOR) {
            $encodation =
                $this->getWeightEncodation($elements) ?? $this->getDateEncodation(
                    $elements,
                ) ?? $this->getPriceEncodation($elements);
            if ($encodation !== null) {
                return $encodation;
            }
        }

        // the leading digit of the item identification in 4 bits, then four
        // groups of three digits in 10 bits each
        $bits = $this->getBits((int) $first[1][0], 4) . $this->getMeasureItemBits($first[1]);
        return ['1', $bits, $this->parser->getData($rest, Compaction::FNC1), 0, 5];
    }
}
