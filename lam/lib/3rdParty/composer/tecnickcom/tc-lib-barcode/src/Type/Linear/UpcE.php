<?php

declare(strict_types=1);

/**
 * UpcE.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\UpcE;
 *
 * UpcE Barcode type class
 * UPC-E
 *
 * UPC-E is a variation of UPC-A which allows for a more compact barcode by eliminating "extra" zeros.
 * Since the resulting UPC-E barcode is about half the size as an UPC-A barcode, UPC-E is generally used on products
 * with very small packaging where a full UPC-A barcode couldn't reasonably fit.
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class UpcE extends \Com\Tecnick\Barcode\Type\Linear\UpcA
{
    protected function getCharAt(string $value, int $index): string
    {
        return $value[$index] ?? '0';
    }

    /**
     * @return array<int, string>
     */
    protected function getUpceParityPattern(string $digit, int $check): array
    {
        $pattern = $this::PARITIES_UPCE[$digit][$check] ?? ['A', 'A', 'A', 'A', 'A', 'A'];
        return \array_values($pattern);
    }

    protected function getBarPattern(string $parity, string $digit): string
    {
        return $this::CHBAR[$parity][$digit] ?? '';
    }

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'UPCE';

    /**
     * Fixed code length
     */
    protected int $code_length = 12;

    /**
     * Map parities
     *
     * @var array<int|string, array<int|string, array<string>>>
     */
    protected const PARITIES_UPCE = [
        0 => [
            '0' => ['B', 'B', 'B', 'A', 'A', 'A'],
            '1' => ['B', 'B', 'A', 'B', 'A', 'A'],
            '2' => ['B', 'B', 'A', 'A', 'B', 'A'],
            '3' => ['B', 'B', 'A', 'A', 'A', 'B'],
            '4' => ['B', 'A', 'B', 'B', 'A', 'A'],
            '5' => ['B', 'A', 'A', 'B', 'B', 'A'],
            '6' => ['B', 'A', 'A', 'A', 'B', 'B'],
            '7' => ['B', 'A', 'B', 'A', 'B', 'A'],
            '8' => ['B', 'A', 'B', 'A', 'A', 'B'],
            '9' => ['B', 'A', 'A', 'B', 'A', 'B'],
        ],
        1 => [
            '0' => ['A', 'A', 'A', 'B', 'B', 'B'],
            '1' => ['A', 'A', 'B', 'A', 'B', 'B'],
            '2' => ['A', 'A', 'B', 'B', 'A', 'B'],
            '3' => ['A', 'A', 'B', 'B', 'B', 'A'],
            '4' => ['A', 'B', 'A', 'A', 'B', 'B'],
            '5' => ['A', 'B', 'B', 'A', 'A', 'B'],
            '6' => ['A', 'B', 'B', 'B', 'A', 'A'],
            '7' => ['A', 'B', 'A', 'B', 'A', 'B'],
            '8' => ['A', 'B', 'A', 'B', 'B', 'A'],
            '9' => ['A', 'B', 'B', 'A', 'B', 'A'],
        ],
    ];

    /**
     * Convert UPC-E code to UPC-A
     *
     * @param string $code Code to convert.
     */
    protected function convertUpceToUpca(string $code): string
    {
        if ($code[5] < '3') {
            return '0' . $code[0] . $code[1] . $code[5] . '0000' . $code[2] . $code[3] . $code[4];
        }

        if ($code[5] === '3') {
            return '0' . $code[0] . $code[1] . $code[2] . '00000' . $code[3] . $code[4];
        }

        if ($code[5] === '4') {
            return '0' . $code[0] . $code[1] . $code[2] . $code[3] . '00000' . $code[4];
        }

        return '0' . $code[0] . $code[1] . $code[2] . $code[3] . $code[4] . '0000' . $code[5];
    }

    /**
     * Convert UPC-A code to UPC-E
     *
     * @param string $code Code to convert.
     */
    protected function convertUpcaToUpce(string $code): string
    {
        $tmp = \substr($code, 4, 3);
        if ($tmp === '000' || $tmp === '100' || $tmp === '200') {
            // manufacturer code ends in 000, 100, or 200
            return \substr($code, 2, 2) . \substr($code, 9, 3) . \substr($code, 4, 1);
        }

        $tmp = \substr($code, 5, 2);
        if ($tmp === '00') {
            // manufacturer code ends in 00
            return \substr($code, 2, 3) . \substr($code, 10, 2) . '3';
        }

        $tmp = \substr($code, 6, 1);
        if ($tmp === '0') {
            // manufacturer code ends in 0
            return \substr($code, 2, 4) . \substr($code, 11, 1) . '4';
        }

        // manufacturer code does not end in zero
        return \substr($code, 2, 5) . \substr($code, 11, 1);
    }

    /**
     * Format the code
     *
     * @throws BarcodeException in case of error
     */
    protected function formatCode(): void
    {
        $code = $this->code;
        if (\strlen($code) === 6) {
            $code = $this->convertUpceToUpca($code);
        }

        $code = \str_pad($code, $this->code_length - 1, '0', STR_PAD_LEFT);
        // append the computed check digit only when the input does not already carry it,
        // otherwise getChecksum() just validates it and returns 0 (avoid a spurious digit)
        $check = $this->getChecksum($code);
        if (\strlen($code) < $this->code_length) {
            $code .= $check;
        }

        ++$this->code_length;
        $this->extcode = '0' . $code;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (!\is_numeric($this->code)) {
            throw new BarcodeException('Input code must be a number');
        }

        $this->formatCode();
        $upce_code = $this->convertUpcaToUpce($this->extcode);
        $seq = '101'; // left guard bar
        $parity = $this->getUpceParityPattern($this->getCharAt($this->extcode, 1), $this->check);
        for ($pos = 0; $pos < 6; ++$pos) {
            $seq .= $this->getBarPattern($this->getCharAt($parity[$pos] ?? 'A', 0), $this->getCharAt($upce_code, $pos));
        }

        $seq .= '010101'; // right guard bar
        $this->processBinarySequence($this->getRawCodeRows($seq));
    }
}
