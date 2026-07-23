<?php

declare(strict_types=1);

/**
 * Codabar.php
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
 * Com\Tecnick\Barcode\Type\Linear\Codabar;
 *
 * Codabar Barcode type class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Codabar extends \Com\Tecnick\Barcode\Type\Linear
{
    protected function getBarWidth(string $char, int $pos): int
    {
        $pattern = $this::CHBAR[$char] ?? '11111111';
        return (int) ($pattern[$pos] ?? '1');
    }

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'CODABAR';

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '11111221',
        '1' => '11112211',
        '2' => '11121121',
        '3' => '22111111',
        '4' => '11211211',
        '5' => '21111211',
        '6' => '12111121',
        '7' => '12112111',
        '8' => '12211111',
        '9' => '21121111',
        '-' => '11122111',
        '$' => '11221111',
        ':' => '21112121',
        '/' => '21211121',
        '.' => '21212111',
        '+' => '11222221',
        'A' => '11221211',
        'B' => '12121121',
        'C' => '11121221',
        'D' => '11122211',
    ];

    /**
     * Format code
     */
    protected function formatCode(): void
    {
        $this->extcode = 'A' . \strtoupper($this->code) . 'A';
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

            for ($pos = 0; $pos < 8; ++$pos) {
                $bar_width = $this->getBarWidth($char, $pos);
                if (($pos % 2) === 0) {
                    $this->bars[] = [$this->ncols, 0, $bar_width, 1];
                }

                $this->ncols += $bar_width;
            }
        }

        --$this->ncols;
    }
}
