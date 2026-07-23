<?php

declare(strict_types=1);

/**
 * ErrorCorrection.php
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Aztec;

/**
 * Com\Tecnick\Barcode\Type\Square\Aztec\ErrorCorrection
 *
 * ErrorCorrection for Aztec Barcode type class
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class ErrorCorrection
{
    /**
     * Galois Field primitive by word size.
     *
     * @var array<int>
     */
    protected const GF = [
        4 => 19, // 10011  GF(16) (x^4 + x + 1) Mode message
        6 => 67, // 1000011  GF(64) (x^6 + x + 1) 01–02 layers
        8 => 301, // 100101101  GF(256) (x^8 + x^5 + x^3 + x^2 + 1) 03–08 layers
        10 => 1033, // 10000001001  GF(1024) (x^10 + x^3 + 1) 09–22 layers
        12 => 4201, // 1000001101001  GF(4096) (x^12 + x^6 + x^5 + x^3 + 1) 23–32 layers
    ];

    /**
     * Map the log and exp (inverse log) tables by word size.
     * NOTE: It is equal to 2^word_size.
     *
     * @var array<int>
     */
    protected const TSIZE = [
        4 => 16,
        6 => 64,
        8 => 256,
        10 => 1024,
        12 => 4096,
    ];

    /**
     * Log table.
     *
     * @var array<int>
     */
    protected array $tlog = [];

    /**
     * Exponential (inverse log) table.
     *
     * @var array<int>
     */
    protected array $texp = [];

    /**
     * Size of the log and exp tables.
     */
    protected int $tsize = 0;

    /**
     * Initialize the the Reed-Solomon Error Correction.
     *
     * @param int $wsize Size of a word in bits.
     */
    public function __construct(int $wsize)
    {
        $this->genTables($wsize);
    }

    /**
     * Returns the Reed-Solomon Error Correction Codewords added to the input data.
     *
     * @param array<int> $data   Array of data codewords to process.
     * @param int   $necc   Number of error correction bytes.
     *
     * @return array<int>
     */
    public function checkwords(array $data, int $necc): array
    {
        $coeff = $this->getCoefficients($data, $necc);
        return \array_pad($coeff, -$necc, 0); // @phpstan-ignore return.type
    }

    /**
     * Generates log and exp (inverse log) tables.
     *
     * @param int $wsize Size of the word in bits.
     */
    protected function genTables(int $wsize): void
    {
        $this->tsize = self::TSIZE[$wsize] ?? 0;
        $this->tlog = \array_fill(0, \max(0, $this->tsize), 0);
        $this->texp = $this->tlog;
        $primitive = self::GF[$wsize] ?? 0;
        $val = 1;
        $sizeminusone = $this->tsize - 1;
        for ($idx = 0; $idx < $this->tsize; ++$idx) {
            $this->texp[$idx] = $val;
            $val <<= 1; // multiply by 2
            if ($val >= $this->tsize) {
                $val ^= $primitive;
                $val &= $sizeminusone;
            }
        }

        for ($idx = 0; $idx < ($this->tsize - 1); ++$idx) {
            $exp = $this->texp[$idx] ?? 0;
            $this->tlog[$exp] = $idx;
        }
    }

    /**
     * Calculates the coefficients of the error correction polynomial.
     *
     * @param array<int> $data   Array of data codewords to process.
     * @param int   $necc   Number of error correction bytes.
     *
     * @return array<int> Array of coefficients.
     */
    protected function getCoefficients(array $data, int $necc): array
    {
        $gen = [1];
        for ($idx = 1; $idx <= $necc; ++$idx) {
            $gen = $this->multiplyCoeff([1, $this->texp[$idx] ?? 0], $gen);
        }

        $deg = $necc + 1;
        $coeff = $this->multiplyByMonomial($data, 1, $necc);
        $len = \count($coeff);
        while ($len >= $deg && ($coeff[0] ?? 0) !== 0) {
            $scale = $this->multiply($coeff[0] ?? 0, 1);
            $largercoeffs = $this->multiplyByMonomial($gen, $scale, $len - $deg);
            $coeff = $this->addOrSubtract($coeff, $largercoeffs);
            $len = \count($coeff);
        }

        return $coeff;
    }

    /**
     * Returns the product of two coefficient arrays.
     *
     * @param array<int> $acf First array of coefficients.
     * @param array<int> $bcf Second array of coefficients.
     *
     * @return array<int> Array of coefficients.
     */
    protected function multiplyCoeff(array $acf, array $bcf): array
    {
        $alen = \count($acf);
        $blen = \count($bcf);
        $coeff = \array_fill(0, \max(0, $alen + $blen - 1), 0);
        for ($aid = 0; $aid < $alen; ++$aid) {
            for ($bid = 0; $bid < $blen; ++$bid) {
                $coeff[$aid + $bid] = ($coeff[$aid + $bid] ?? 0) ^ $this->multiply($acf[$aid] ?? 0, $bcf[$bid] ?? 0);
            }
        }

        return $this->trimCoefficients($coeff);
    }

    /**
     * Returns the product of $aval and $bval in GF(size).
     *
     * @param int $aval First value.
     * @param int $bval Second value.
     */
    protected function multiply(int $aval, int $bval): int
    {
        if ($aval === 0 || $bval === 0) {
            return 0;
        }

        $sizeMinusOne = $this->tsize - 1;
        if ($sizeMinusOne <= 0) {
            return 0;
        }

        $index = (($this->tlog[$aval] ?? 0) + ($this->tlog[$bval] ?? 0)) % $sizeMinusOne;

        return $this->texp[$index] ?? 0;
    }

    /**
     * Left-trim coefficients array.
     *
     * @param array<int> $coeff Array of coefficients.
     *
     * @return array<int> Array of coefficients.
     */
    protected function trimCoefficients(array $coeff): array
    {
        while ($coeff !== [] && ($coeff[0] ?? 0) === 0) {
            \array_shift($coeff);
        }

        return $coeff;
    }

    /**
     * Returns the product of a polynomial by a monomial.
     *
     * @param array<int> $coeff  Array of polynomial coefficients.
     * @param int   $mon    Monomial.
     * @param int   $deg    Degree of the monomial.
     *
     * @return array<int> Array of coefficients.
     */
    protected function multiplyByMonomial(array $coeff, int $mon, int $deg): array
    {
        // if ($mon == 0) {
        //     return array(0);
        // }
        $ncf = \count($coeff);
        $prod = \array_fill(0, \max(0, $ncf + $deg), 0);
        for ($idx = 0; $idx < $ncf; ++$idx) {
            $prod[$idx] = $this->multiply($coeff[$idx] ?? 0, $mon);
        }

        return $this->trimCoefficients($prod);
    }

    /**
     * Adds or subtracts two coefficient arrays.
     *
     * @param array<int> $smaller The smaller array of coefficients.
     * @param array<int> $larger  The larger array of coefficients.
     *
     * @return array<int> Array of coefficients.
     */
    protected function addOrSubtract(array $smaller, array $larger): array
    {
        // if ($smaller[0] == 0) {
        //     return $larger;
        // }
        // if ($larger[0] == 0) {
        //     return $smaller;
        // }
        $slen = \count($smaller);
        $llen = \count($larger);
        // if ($slen > $llen) {
        //     // swap arrays
        //     list($smaller, $larger) = array($larger, $smaller);
        //     list($slen, $llen) = array($llen, $slen);
        // }
        $lendiff = $llen - $slen;
        $coeff = \array_slice($larger, 0, $lendiff);
        for ($idx = $lendiff; $idx < $llen; ++$idx) {
            $coeff[$idx] = ($smaller[$idx - $lendiff] ?? 0) ^ ($larger[$idx] ?? 0);
        }

        return $this->trimCoefficients($coeff);
    }
}
