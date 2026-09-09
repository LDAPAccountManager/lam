<?php

declare(strict_types=1);

/**
 * Mailmark.php
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
use Com\Tecnick\Barcode\Math;
use Com\Tecnick\Barcode\Type\Linear\Mailmark\PostCode;
use Com\Tecnick\Barcode\Type\ReedSolomon;

/**
 * Com\Tecnick\Barcode\Type\Linear\Mailmark;
 *
 * Mailmark Barcode type class
 * Royal Mail Mailmark 4-state barcode, types C and L
 *
 * The Application String is split into six fields, each field is converted to an
 * integer, the integers are consolidated into a single value, the value is split
 * into data numbers of thirty or thirty two values, Reed Solomon check numbers
 * over GF(32) are appended, every number becomes a six bit symbol of a fixed
 * parity, the symbols are reordered into extender groups and each group drives
 * the ascenders and descenders of three bars.
 *
 * The type of the barcode follows from the length of the Application String:
 * twenty two characters for the type C, twenty six for the type L.
 *
 * Mailmark and Royal Mail are registered trademarks of Royal Mail Group Ltd.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Mailmark extends \Com\Tecnick\Barcode\Type\Linear\FourState
{
    /**
     * Bar identifier of the full bar
     *
     * @var string
     */
    protected const FULL = 'F';

    /**
     * Bar identifier of the ascender
     *
     * @var string
     */
    protected const ASCENDER = 'A';

    /**
     * Bar identifier of the descender
     *
     * @var string
     */
    protected const DESCENDER = 'D';

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'MAILMARK';

    /**
     * Number of characters of the Supply Chain ID field, by Application String length
     *
     * @var array<int, int>
     */
    protected const SUPPLY_CHAIN_LENGTH = [
        22 => 2,
        26 => 6,
    ];

    /**
     * Number of data numbers with thirty values and with thirty two values,
     * by Application String length
     *
     * @var array<int, array{int, int}>
     */
    protected const DATA_NUMBERS = [
        22 => [9, 7],
        26 => [11, 8],
    ];

    /**
     * Number of Reed Solomon check numbers, by Application String length
     *
     * @var array<int, int>
     */
    protected const CHECK_NUMBERS = [
        22 => 6,
        26 => 7,
    ];

    /**
     * Extender group of each data and check symbol, in the logical order of the
     * symbols, by Application String length
     *
     * @var array<int, array<int, int>>
     */
    protected const EXTENDER_GROUP = [
        22 => [3, 5, 7, 11, 13, 14, 16, 17, 19, 0, 1, 2, 4, 6, 8, 9, 10, 12, 15, 18, 20, 21],
        26 => [2, 5, 7, 8, 13, 14, 15, 16, 21, 22, 23, 0, 1, 3, 4, 6, 9, 10, 11, 12, 17, 18, 19, 20, 24, 25],
    ];

    /**
     * Allowed character values of the Format field
     *
     * @var string
     */
    protected const FORMAT_CHARS = '01234';

    /**
     * Allowed character values of the Version ID field
     *
     * @var string
     */
    protected const VERSION_CHARS = '1234';

    /**
     * Allowed character values of the Class field
     *
     * @var string
     */
    protected const CLASS_CHARS = '0123456789ABCDE';

    /**
     * Allowed character values of the Supply Chain ID and Item ID fields
     *
     * @var string
     */
    protected const DIGITS = '0123456789';

    /**
     * Number of characters of the Item ID field
     */
    protected const ITEM_ID_LENGTH = 8;

    /**
     * Number of bars of an extender group
     */
    protected const GROUP_BARS = 3;

    /**
     * Word size in bits of the Reed Solomon code over GF(32)
     */
    protected const WORD_SIZE = 5;

    /**
     * Number of bits of a data or check symbol
     */
    protected const SYMBOL_BITS = 6;

    /**
     * Get the six bit symbols of the given parity, in ascending order.
     * The symbols of the data numbers with thirty values have a non-zero even
     * number of one bits, all the others an odd number.
     *
     * @param int $parity 0 for an odd number of one bits, 1 for an even one
     *
     * @return array<int, int>
     */
    protected function getSymbolTable(int $parity): array
    {
        $table = [];
        for ($value = 1; $value < (1 << $this::SYMBOL_BITS); ++$value) {
            if ((\substr_count(\decbin($value), '1') % 2) !== $parity) {
                continue;
            }

            $table[] = $value;
        }

        return $table;
    }

    /**
     * Get the integer value of a field, in the base of its allowed characters
     *
     * @throws BarcodeException in case of an unsupported character
     */
    protected function getFieldValue(string $field, string $chars): int
    {
        $value = 0;
        $flen = \strlen($field);
        for ($pos = 0; $pos < $flen; ++$pos) {
            $index = \strpos($chars, $field[$pos]);
            if (!\is_int($index)) {
                throw new BarcodeException('Invalid character: ' . (\ord($field[$pos]) & 0xFF));
            }

            $value = ($value * \strlen($chars)) + $index;
        }

        return $value;
    }

    /**
     * Get the Consolidated Data Value of the Application String
     *
     * @return numeric-string
     *
     * @throws BarcodeException in case of an invalid field
     */
    protected function getConsolidatedValue(): string
    {
        $schain = $this::SUPPLY_CHAIN_LENGTH[\strlen($this->code)] ?? 0;
        $format = $this->getFieldValue(\substr($this->code, 0, 1), $this::FORMAT_CHARS);
        $version = $this->getFieldValue(\substr($this->code, 1, 1), $this::VERSION_CHARS);
        $class = $this->getFieldValue(\substr($this->code, 2, 1), $this::CLASS_CHARS);
        $chain = $this->getFieldValue(\substr($this->code, 3, $schain), $this::DIGITS);
        $offset = 3 + $schain;
        $item = $this->getFieldValue(\substr($this->code, $offset, $this::ITEM_ID_LENGTH), $this::DIGITS);
        $postcode = new PostCode();
        $value = (string) $postcode->getValue(\substr($this->code, $offset + $this::ITEM_ID_LENGTH));

        $value = Math::add(Math::mul($value, (string) 10 ** $this::ITEM_ID_LENGTH), (string) $item);
        $value = Math::add(Math::mul($value, (string) 10 ** $schain), (string) $chain);
        $value = Math::add(Math::mul($value, '15'), (string) $class);
        $value = Math::add(Math::mul($value, '5'), (string) $format);

        return Math::add(Math::mul($value, '4'), (string) $version);
    }

    /**
     * Get the data numbers of the Consolidated Data Value.
     * The least significant numbers have thirty two values and the rest thirty.
     *
     * @return array<int, int>
     *
     * @throws BarcodeException in case of an invalid field
     */
    protected function getDataNumbers(): array
    {
        [$thirty, $thirtytwo] = $this::DATA_NUMBERS[\strlen($this->code)] ?? [0, 0];
        $value = $this->getConsolidatedValue();
        /** @var array<int, int> $numbers */
        $numbers = [];
        for ($idx = 0; $idx < $thirtytwo; ++$idx) {
            \array_unshift($numbers, (int) Math::mod($value, '32'));
            $value = Math::div($value, '32');
        }

        for ($idx = 1; $idx < $thirty; ++$idx) {
            \array_unshift($numbers, (int) Math::mod($value, '30'));
            $value = Math::div($value, '30');
        }

        \array_unshift($numbers, (int) $value);

        return $numbers;
    }

    /**
     * Get the six bit symbols of the data numbers and of the check numbers, in
     * their logical order
     *
     * @return array<int, int>
     *
     * @throws BarcodeException in case of an invalid field
     */
    protected function getSymbols(): array
    {
        $data = $this->getDataNumbers();
        $reedSolomon = new ReedSolomon($this::WORD_SIZE);
        $check = $reedSolomon->checkwords($data, $this::CHECK_NUMBERS[\strlen($this->code)] ?? 0);

        [$thirty] = $this::DATA_NUMBERS[\strlen($this->code)] ?? [0, 0];
        $odd = $this->getSymbolTable(1);
        $even = $this->getSymbolTable(0);

        $symbols = [];
        foreach (\array_merge($data, $check) as $key => $number) {
            $symbols[] = $key < $thirty ? $even[$number] ?? 0 : $odd[$number] ?? 0;
        }

        return $symbols;
    }

    /**
     * Get the extender groups, in the physical order of the bars
     *
     * @return array<int, int>
     *
     * @throws BarcodeException in case of an invalid field
     */
    protected function getExtenderGroups(): array
    {
        $order = $this::EXTENDER_GROUP[\strlen($this->code)] ?? [];
        $groups = \array_fill(0, \count($order), 0);
        foreach ($this->getSymbols() as $key => $symbol) {
            $groups[$order[$key] ?? 0] = $symbol;
        }

        return $groups;
    }

    /**
     * Get the bar identifiers of the symbol.
     * Every extender group drives three bars: for an even group the high three
     * bits are the ascenders and the low three the descenders, for an odd group
     * the other way round, most significant bit leftmost.
     *
     * @throws BarcodeException in case of error
     */
    protected function getBarIdentifiers(): string
    {
        $bars = '';
        foreach ($this->getExtenderGroups() as $index => $group) {
            $high = $group >> $this::GROUP_BARS;
            $low = $group & ((1 << $this::GROUP_BARS) - 1);
            $ascenders = ($index % 2) === 0 ? $high : $low;
            $descenders = ($index % 2) === 0 ? $low : $high;
            for ($bit = $this::GROUP_BARS - 1; $bit >= 0; --$bit) {
                $mask = 1 << $bit;
                $bars .= match (true) {
                    ($ascenders & $mask) !== 0 && ($descenders & $mask) !== 0 => 'F',
                    ($ascenders & $mask) !== 0 => 'A',
                    ($descenders & $mask) !== 0 => 'D',
                    default => 'T',
                };
            }
        }

        return $bars;
    }

    /**
     * Check that the Application String has the length of one of the two types
     *
     * @throws BarcodeException if the length is not supported
     */
    protected function validateCode(): void
    {
        if (!\array_key_exists(\strlen($this->code), $this::SUPPLY_CHAIN_LENGTH)) {
            throw new BarcodeException(
                'The application string must be 22 characters for the barcode C or 26 for the barcode L, '
                . \strlen($this->code)
                . ' given',
            );
        }

        if ($this->code[1] !== '1') {
            throw new BarcodeException('Only the version ID 1 is supported: ' . $this->code[1]);
        }
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->validateCode();
        $this->extcode = $this->getBarIdentifiers();

        $this->setStateBars($this->extcode);
    }
}
