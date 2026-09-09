<?php

declare(strict_types=1);

/**
 * ErrorCorrection.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Datamatrix;

use Com\Tecnick\Barcode\Type\ReedSolomon;

/**
 * Com\Tecnick\Barcode\Type\Square\Datamatrix\ErrorCorrection
 *
 * Error correction methods and other utilities for Datamatrix Barcode type class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class ErrorCorrection
{
    /**
     * @param array<int, int> $values
     */
    protected function getArrayInt(array $values, int $idx): int
    {
        return $values[$idx] ?? 0;
    }

    /**
     * Add error correction codewords to data codewords array (ANNEX E).
     *
     * The data codewords are split over $nbk interleaved blocks and each block
     * is followed by its own error correction codewords, interleaved the same way.
     *
     * @param array<int, int> $wdc Array of datacodewords.
     * @param int   $nbk Number of blocks.
     * @param int   $ncw Number of data codewords per block.
     * @param int   $ncc Number of correction codewords per block.
     * @param int   $ngf Number of fields on log/antilog table (power of 2).
     * @param int   $vpp The value of its prime modulus polynomial (301 for ECC200).
     *
     * @return array<int, int> data codewords + error codewords
     */
    public function getErrorCorrection(array $wdc, int $nbk, int $ncw, int $ncc, int $ngf = 256, int $vpp = 301): array
    {
        $reedSolomon = new ReedSolomon((int) \log($ngf, 2), $vpp);
        // total number of data codewords: the last blocks are one codeword shorter when
        // the data codewords do not divide evenly between the interleaved blocks
        $num_wd = \min(\count($wdc), $nbk * $ncw);
        // total number of error codewords
        $num_we = $nbk * $ncc;
        // for each block
        for ($b = 0; $b < $nbk; ++$b) {
            // create interleaved data block
            $block = [];
            for ($n = $b; $n < $num_wd; $n += $nbk) {
                $block[] = $this->getArrayInt($wdc, $n);
            }

            // calculate error correction codewords for this block
            $wec = $reedSolomon->checkwords($block, $ncc);
            // add error codewords at the end of data codewords
            $j = 0;
            for ($i = $b; $i < $num_we; $i += $nbk) {
                $wdc[$num_wd + $i] = $this->getArrayInt($wec, $j);
                ++$j;
            }
        }

        // reorder codewords
        \ksort($wdc);
        return $wdc;
    }
}
