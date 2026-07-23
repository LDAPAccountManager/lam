<?php

declare(strict_types=1);

/**
 * Postnet.php
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
 * Com\Tecnick\Barcode\Type\Linear\Postnet;
 *
 * Postnet Barcode type class
 * POSTNET
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Postnet extends \Com\Tecnick\Barcode\Type\Linear
{
    protected function getBarHeight(string $char, int $pos): int
    {
        $pattern = $this::CHBAR[$char] ?? '11111';
        return (int) ($pattern[$pos] ?? '1');
    }

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'POSTNET';

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '22111',
        '1' => '11122',
        '2' => '11212',
        '3' => '11221',
        '4' => '12112',
        '5' => '12121',
        '6' => '12211',
        '7' => '21112',
        '8' => '21121',
        '9' => '21211',
    ];

    /**
     * Calculate the checksum.
     *
     * @param string $code Code to represent.
     *
     * @return int char checksum.
     */
    protected function getChecksum(string $code): int
    {
        $sum = 0;
        $len = \strlen($code);
        for ($pos = 0; $pos < $len; ++$pos) {
            $sum += (int) $code[$pos];
        }

        $check = $sum % 10;
        if ($check > 0) {
            return 10 - $check;
        }

        return $check;
    }

    /**
     * Format code
     *
     * @throws BarcodeException in case of error
     */
    protected function formatCode(): void
    {
        $code = \preg_replace('/[-\s]+/', '', $this->code);
        if ($code === null) {
            throw new BarcodeException('Code not valid');
        }
        $this->extcode = $code . $this->getChecksum($code);
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->ncols = 0;
        $this->nrows = 2;
        $this->bars = [];
        $this->formatCode();
        $clen = \strlen($this->extcode);
        // start bar
        $this->bars[] = [$this->ncols, 0, 1, 2];
        $this->ncols += 2;
        for ($chr = 0; $chr < $clen; ++$chr) {
            $char = $this->extcode[$chr];
            if (!\array_key_exists($char, $this::CHBAR)) {
                throw new BarcodeException('Invalid character: ' . (\ord($char) & 0xFF));
            }

            for ($pos = 0; $pos < 5; ++$pos) {
                $bar_height = $this->getBarHeight($char, $pos);
                $this->bars[] = [$this->ncols, (int) \floor(1 / $bar_height), 1, $bar_height];
                $this->ncols += 2;
            }
        }

        // end bar
        $this->bars[] = [$this->ncols, 0, 1, 2];
        ++$this->ncols;
    }
}
