<?php

declare(strict_types=1);

/**
 * AustraliaPost.php
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
use Com\Tecnick\Barcode\Type\ReedSolomon;

/**
 * Com\Tecnick\Barcode\Type\Linear\AustraliaPost;
 *
 * AustraliaPost Barcode type class
 * Australia Post 4-State Customer Barcode
 *
 * The symbol is the start bars, the Format Control Code, the sorting code, the
 * Customer Information field of the wider formats, a filler bar, twelve Reed
 * Solomon error correction bars and the stop bars. Every group of three bars is
 * one symbol of a Reed Solomon code over GF(64).
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class AustraliaPost extends \Com\Tecnick\Barcode\Type\Linear\FourState
{
    /**
     * Bar identifier of the full bar
     *
     * @var string
     */
    protected const FULL = '0';

    /**
     * Bar identifier of the ascender
     *
     * @var string
     */
    protected const ASCENDER = '1';

    /**
     * Bar identifier of the descender
     *
     * @var string
     */
    protected const DESCENDER = '2';

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'AUSPOST';

    /**
     * Start bars
     *
     * @var string
     */
    protected const START = '13';

    /**
     * Stop bars
     *
     * @var string
     */
    protected const STOP = '13';

    /**
     * Filler bar, a tracker
     *
     * @var string
     */
    protected const FILLER = '3';

    /**
     * Number of bars of the Customer Information field, by Format Control Code
     *
     * @var array<int|string, int>
     */
    protected const FORMAT_CONTROL_CODE = [
        '00' => 1,
        '11' => 1,
        '59' => 16,
        '62' => 31,
    ];

    /**
     * Number of digits of the sorting code, the Delivery Point Identifier
     */
    protected const SORTING_CODE_LENGTH = 8;

    /**
     * Number of Reed Solomon check symbols
     */
    protected const CHECK_SYMBOLS = 4;

    /**
     * Number of bars of a Reed Solomon symbol
     */
    protected const SYMBOL_BARS = 3;

    /**
     * Word size in bits of the Reed Solomon code over GF(64)
     */
    protected const WORD_SIZE = 6;

    /**
     * N Encoding Table, two bars per digit
     *
     * @var array<int|string, string>
     */
    protected const N_TABLE = [
        '0' => '00',
        '1' => '01',
        '2' => '02',
        '3' => '10',
        '4' => '11',
        '5' => '12',
        '6' => '20',
        '7' => '21',
        '8' => '22',
        '9' => '30',
    ];

    /**
     * C Encoding Table, three bars per character
     *
     * @var array<int|string, string>
     */
    protected const C_TABLE = [
        'A' => '000',
        'B' => '001',
        'C' => '002',
        'D' => '010',
        'E' => '011',
        'F' => '012',
        'G' => '020',
        'H' => '021',
        'I' => '022',
        'J' => '100',
        'K' => '101',
        'L' => '102',
        'M' => '110',
        'N' => '111',
        'O' => '112',
        'P' => '120',
        'Q' => '121',
        'R' => '122',
        'S' => '200',
        'T' => '201',
        'U' => '202',
        'V' => '210',
        'W' => '211',
        'X' => '212',
        'Y' => '220',
        'Z' => '221',
        'a' => '023',
        'b' => '030',
        'c' => '031',
        'd' => '032',
        'e' => '033',
        'f' => '103',
        'g' => '113',
        'h' => '123',
        'i' => '130',
        'j' => '131',
        'k' => '132',
        'l' => '133',
        'm' => '203',
        'n' => '213',
        'o' => '223',
        'p' => '230',
        'q' => '231',
        'r' => '232',
        's' => '233',
        't' => '303',
        'u' => '313',
        'v' => '323',
        'w' => '330',
        'x' => '331',
        'y' => '332',
        'z' => '333',
        '0' => '222',
        '1' => '300',
        '2' => '301',
        '3' => '302',
        '4' => '310',
        '5' => '311',
        '6' => '312',
        '7' => '320',
        '8' => '321',
        '9' => '322',
        ' ' => '003',
        '#' => '013',
    ];

    /**
     * Encoding table of the Customer Information field, C or N
     */
    protected string $table = 'C';

    /**
     * Set the encoding table of the Customer Information field
     */
    protected function setParameters(): void
    {
        $table = \strtoupper((string) ($this->params[0] ?? 'C'));
        $this->table = $table === 'N' ? 'N' : 'C';
    }

    /**
     * Get the bar values of a string, by the requested encoding table
     *
     * @param array<int|string, string> $table Encoding table
     *
     * @throws BarcodeException in case of an unsupported character
     */
    protected function getEncodedField(string $code, array $table): string
    {
        $bars = '';
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $char = $code[$pos];
            if (!\array_key_exists($char, $table)) {
                throw new BarcodeException('Invalid character: ' . (\ord($char) & 0xFF));
            }

            $bars .= $table[$char];
        }

        return $bars;
    }

    /**
     * Get the Format Control Code, the sorting code and the Customer Information
     * of the input code
     *
     * @return array{string, string, string}
     *
     * @throws BarcodeException if the code is too short or the format is unknown
     */
    protected function getFields(): array
    {
        $prefix = 2 + $this::SORTING_CODE_LENGTH;
        if (\strlen($this->code) < $prefix) {
            throw new BarcodeException(
                'The code must start with the 2 digit Format Control Code and the '
                . $this::SORTING_CODE_LENGTH
                . ' digit sorting code',
            );
        }

        $fcc = \substr($this->code, 0, 2);
        $sorting = \substr($this->code, 2, $this::SORTING_CODE_LENGTH);
        if (!\array_key_exists($fcc, $this::FORMAT_CONTROL_CODE)) {
            throw new BarcodeException('Unsupported Format Control Code: ' . $fcc);
        }

        if (!\ctype_digit($sorting)) {
            throw new BarcodeException('The sorting code must be a number: ' . $sorting);
        }

        if ($fcc === '00' && $sorting !== \str_repeat('0', $this::SORTING_CODE_LENGTH)) {
            throw new BarcodeException('The Format Control Code 00 is only valid with a zero sorting code');
        }

        return [$fcc, $sorting, \substr($this->code, $prefix)];
    }

    /**
     * Get the bars of the Customer Information field, padded with filler bars
     *
     * @param int $width Number of bars of the field
     *
     * @throws BarcodeException if the information does not fit the field
     */
    protected function getCustomerBars(string $info, int $width): string
    {
        $bars = $this->table === 'N'
            ? $this->getEncodedField($info, $this::N_TABLE)
            : $this->getEncodedField($info, $this::C_TABLE);

        if (\strlen($bars) > $width) {
            throw new BarcodeException(
                'The customer information is too long: ' . \strlen($bars) . ' bars (maximum ' . $width . ')',
            );
        }

        return \str_pad($bars, $width, $this::FILLER);
    }

    /**
     * Get the Reed Solomon check bars of the information bars
     */
    protected function getCheckBars(string $bars): string
    {
        $data = [];
        foreach (\str_split($bars, $this::SYMBOL_BARS) as $symbol) {
            $data[] = (int) \base_convert($symbol, 4, 10);
        }

        $reedSolomon = new ReedSolomon($this::WORD_SIZE);
        $check = '';
        foreach ($reedSolomon->checkwords($data, $this::CHECK_SYMBOLS) as $value) {
            $check .= \str_pad(\base_convert((string) $value, 10, 4), $this::SYMBOL_BARS, '0', STR_PAD_LEFT);
        }

        return $check;
    }

    /**
     * Get the bar values of the whole symbol, from the start bars to the stop bars
     *
     * @throws BarcodeException in case of error
     */
    protected function getBarValues(): string
    {
        [$fcc, $sorting, $info] = $this->getFields();
        $this->extcode = $fcc . $sorting . $info;
        $width = $this::FORMAT_CONTROL_CODE[$fcc] ?? 1;
        $bars = $this->getEncodedField($fcc . $sorting, $this::N_TABLE) . $this->getCustomerBars($info, $width);

        return $this::START . $bars . $this->getCheckBars($bars) . $this::STOP;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->setStateBars($this->getBarValues());
    }
}
