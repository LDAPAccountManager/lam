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
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Datamatrix;

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
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
     * Product of two numbers in a Power-of-Two Galois Field
     *
     * @param int   $numa First number to multiply.
     * @param int   $numb Second number to multiply.
     * @param array<int, int> $log  Log table.
     * @param array<int, int> $alog Anti-Log table.
     * @param int   $ngf  Number of Factors of the Reed-Solomon polynomial.
     *
     * @return int product
     */
    protected function getGFProduct(int $numa, int $numb, array $log, array $alog, int $ngf): int
    {
        if ($numa === 0 || $numb === 0) {
            return 0;
        }

        $a = $this->getArrayInt($log, $numa);
        $b = $this->getArrayInt($log, $numb);
        $idx = ($a + $b) % ($ngf - 1);

        return $this->getArrayInt($alog, $idx);
    }

    /**
     * Add error correction codewords to data codewords array (ANNEX E).
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
        // generate the log ($log) and antilog ($alog) tables
        $log = [0];
        $alog = [1];
        $this->genLogs($log, $alog, $ngf, $vpp);

        // generate the polynomial coefficients (c)
        $plc = \array_fill(0, \max(0, $ncc + 1), 0);
        $plc[0] = 1;
        for ($i = 1; $i <= $ncc; ++$i) {
            $plc[$i] = $this->getArrayInt($plc, $i - 1);
            for ($j = $i - 1; $j >= 1; --$j) {
                $plc[$j] =
                    $this->getArrayInt($plc, $j - 1)
                    ^ $this->getGFProduct(
                        $this->getArrayInt($plc, $j),
                        $this->getArrayInt($alog, $i),
                        $log,
                        $alog,
                        $ngf,
                    );
            }

            $plc[0] = $this->getGFProduct(
                $this->getArrayInt($plc, 0),
                $this->getArrayInt($alog, $i),
                $log,
                $alog,
                $ngf,
            );
        }

        \ksort($plc);

        // total number of data codewords
        $num_wd = $nbk * $ncw;
        // total number of error codewords
        $num_we = $nbk * $ncc;
        // for each block
        for ($b = 0; $b < $nbk; ++$b) {
            // create interleaved data block
            $block = [];
            for ($n = $b; $n < $num_wd; $n += $nbk) {
                $block[] = $this->getArrayInt($wdc, $n);
            }

            // initialize error codewords
            $wec = \array_fill(0, \max(0, $ncc + 1), 0);
            // calculate error correction codewords for this block
            for ($i = 0; $i < $ncw; ++$i) {
                $ker = $this->getArrayInt($wec, 0) ^ $this->getArrayInt($block, $i);
                for ($j = 0; $j < $ncc; ++$j) {
                    $wec[$j] =
                        $this->getArrayInt($wec, $j + 1)
                        ^ $this->getGFProduct($ker, $this->getArrayInt($plc, $ncc - $j - 1), $log, $alog, $ngf);
                }
            }

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

    /**
     * Generate the log ($log) and antilog ($alog) tables
     *
     * @param array<int, int> $log  Log table
     * @param array<int, int> $alog Anti-Log table
     * @param int   $ngf  Number of fields on log/antilog table (power of 2).
     * @param int   $vpp  The value of its prime modulus polynomial (301 for ECC200).
     */
    protected function genLogs(array &$log, array &$alog, int $ngf, int $vpp): void
    {
        for ($i = 1; $i < $ngf; ++$i) {
            $alog[$i] = $this->getArrayInt($alog, $i - 1) * 2;
            if ($alog[$i] >= $ngf) {
                $alog[$i] ^= $vpp;
            }

            $log[$alog[$i]] = $i;
        }

        \ksort($log);
    }
}
