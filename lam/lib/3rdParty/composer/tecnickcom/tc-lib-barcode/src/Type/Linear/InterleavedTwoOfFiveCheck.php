<?php

declare(strict_types=1);

/**
 * InterleavedTwoOfFiveCheck.php
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
 * Com\Tecnick\Barcode\Type\Linear\InterleavedTwoOfFiveCheck;
 *
 * InterleavedTwoOfFiveCheck Barcode type class
 * Interleaved 2 of 5 + CHECKSUM
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class InterleavedTwoOfFiveCheck extends \Com\Tecnick\Barcode\Type\Linear\StandardTwoOfFiveCheck
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'I25+';

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '11221',
        '1' => '21112',
        '2' => '12112',
        '3' => '22111',
        '4' => '11212',
        '5' => '21211',
        '6' => '12211',
        '7' => '11122',
        '8' => '21121',
        '9' => '12121',
        'A' => '11',
        'Z' => '21',
    ];

    protected function getPattern(string $digit): string
    {
        return $this::CHBAR[$digit] ?? '';
    }

    /**
     * Format code
     */
    protected function formatCode(): void
    {
        $this->extcode = $this->code . $this->getChecksum($this->code);
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->formatCode();
        if ((\strlen($this->extcode) % 2) !== 0) {
            // add leading zero if code-length is odd
            $this->extcode = '0' . $this->extcode;
        }

        // add start and stop codes
        $this->extcode = 'AA' . \strtolower($this->extcode) . 'ZA';
        $this->ncols = 0;
        $this->nrows = 1;
        $this->bars = [];
        $clen = \strlen($this->extcode);
        for ($idx = 0; $idx < $clen; $idx += 2) {
            $char_bar = $this->extcode[$idx];
            $char_space = $this->extcode[$idx + 1];
            if (!\array_key_exists($char_bar, $this::CHBAR) || !\array_key_exists($char_space, $this::CHBAR)) {
                throw new BarcodeException('Invalid character sequence: ' . $char_bar . $char_space);
            }

            // create a bar-space sequence
            $seq = '';
            $bar_pattern = $this->getPattern($char_bar);
            $space_pattern = $this->getPattern($char_space);
            $chrlen = \strlen($bar_pattern);
            for ($pos = 0; $pos < $chrlen; ++$pos) {
                $seq .= ($bar_pattern[$pos] ?? '0') . ($space_pattern[$pos] ?? '0');
            }

            $seqlen = \strlen($seq);
            for ($pos = 0; $pos < $seqlen; ++$pos) {
                $bar_width = (int) $seq[$pos];
                if (($pos % 2) === 0 && $bar_width > 0) {
                    $this->bars[] = [$this->ncols, 0, $bar_width, 1];
                }

                $this->ncols += $bar_width;
            }
        }

        --$this->ncols;
    }
}
