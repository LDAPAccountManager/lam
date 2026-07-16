<?php

declare(strict_types=1);

/**
 * CodeOneOne.php
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
 * Com\Tecnick\Barcode\Type\Linear\CodeOneOne;
 *
 * CodeOneOne Barcode type class
 * CODE 11
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class CodeOneOne extends \Com\Tecnick\Barcode\Type\Linear
{
    protected function getBarWidth(string $char, int $pos): int
    {
        $pattern = $this::CHBAR[$char] ?? '000000';
        return (int) ($pattern[$pos] ?? '0');
    }

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'CODE11';

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '111121',
        '1' => '211121',
        '2' => '121121',
        '3' => '221111',
        '4' => '112121',
        '5' => '212111',
        '6' => '122111',
        '7' => '111221',
        '8' => '211211',
        '9' => '211111',
        '-' => '112111',
        'S' => '112211',
    ];

    /**
     * Calculate the checksum.
     *
     * @param string $code Code to represent.
     *
     * @return string char checksum.
     */
    protected function getChecksum(string $code): string
    {
        $len = \strlen($code);
        // calculate check digit C
        $ptr = 1;
        $cval = 0;
        for ($pos = $len - 1; $pos >= 0; --$pos) {
            $digit = $code[$pos];
            $dval = $digit === '-' ? 10 : (int) $digit;

            $cval += $dval * $ptr;
            ++$ptr;
            if ($ptr > 10) {
                $ptr = 1;
            }
        }

        $cval %= 11;
        $ccheck = $cval === 10 ? '-' : (string) $cval;

        if ($len <= 10) {
            return $ccheck;
        }

        // calculate check digit K (computed over the code with the C check digit appended)
        $code .= $ccheck;
        $klen = \strlen($code);
        $ptr = 1;
        $kval = 0;
        for ($pos = $klen - 1; $pos >= 0; --$pos) {
            $digit = $code[$pos];
            $dval = $digit === '-' ? 10 : (int) $digit;

            $kval += $dval * $ptr;
            ++$ptr;
            if ($ptr > 9) {
                $ptr = 1;
            }
        }

        $kval %= 11;
        $kcheck = $kval === 10 ? '-' : (string) $kval;

        return $ccheck . $kcheck;
    }

    /**
     * Format code
     */
    protected function formatCode(): void
    {
        $this->extcode = 'S' . $this->code . $this->getChecksum($this->code) . 'S';
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->ncols = 0;
        $this->nrows = 1;
        $this->bars = [];
        $this->formatCode();
        $clen = \strlen($this->extcode);
        for ($chr = 0; $chr < $clen; ++$chr) {
            $char = $this->extcode[$chr];
            if (!\array_key_exists($char, $this::CHBAR)) {
                throw new BarcodeException('Invalid character: ' . (\ord($char) & 0xFF));
            }

            for ($pos = 0; $pos < 6; ++$pos) {
                $bar_width = $this->getBarWidth($char, $pos);
                if (($pos % 2) === 0 && $bar_width > 0) {
                    $this->bars[] = [$this->ncols, 0, $bar_width, 1];
                }

                $this->ncols += $bar_width;
            }
        }

        --$this->ncols;
    }
}
