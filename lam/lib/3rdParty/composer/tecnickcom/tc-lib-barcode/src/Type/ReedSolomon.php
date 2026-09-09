<?php

declare(strict_types=1);

/**
 * ReedSolomon.php
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type;

/**
 * Com\Tecnick\Barcode\Type\ReedSolomon
 *
 * Reed-Solomon error correction over GF(2^n)
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class ReedSolomon
{
    /**
     * Galois Field primitive by word size.
     *
     * @var array<int>
     */
    protected const GF = [
        4 => 19, // 10011  GF(16) (x^4 + x + 1) Aztec mode message
        5 => 37, // 100101  GF(32) (x^5 + x^2 + 1) Royal Mail Mailmark
        6 => 67, // 1000011  GF(64) (x^6 + x + 1) Aztec 01-02 layers, Australia Post 4-State
        8 => 301, // 100101101  GF(256) (x^8 + x^5 + x^3 + x^2 + 1) Aztec 03-08 layers
        10 => 1033, // 10000001001  GF(1024) (x^10 + x^3 + 1) Aztec 09-22 layers
        12 => 4201, // 1000001101001  GF(4096) (x^12 + x^6 + x^5 + x^3 + 1) Aztec 23-32 layers
    ];

    /**
     * Primitive polynomial of the GF(256) of QR Code and Micro QR Code:
     * x^8 + x^4 + x^3 + x^2 + 1. The word size 8 entry of GF is the Aztec one,
     * over a different polynomial, so this field is named on its own.
     *
     * @var int
     */
    public const GF_QRCODE = 285; // 100011101

    /**
     * Map the log and exp (inverse log) tables by word size.
     * NOTE: It is equal to 2^word_size.
     *
     * @var array<int>
     */
    protected const TSIZE = [
        4 => 16,
        5 => 32,
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
     * Generator polynomial coefficients, keyed by the number of error
     * correction codewords.
     *
     * @var array<int, array<int>>
     */
    protected array $gen = [];

    /**
     * Exponent of the first root of the generator polynomial.
     */
    protected int $firstroot = 1;

    /**
     * Initialize the Reed-Solomon Error Correction.
     *
     * @param int $wsize     Size of a word in bits.
     * @param int $primitive Primitive polynomial of the Galois field, or zero
     *                       for the one the word size selects in GF.
     * @param int $firstroot Exponent of the first root of the generator
     *                       polynomial, which is the product of x + a^i over
     *                       the roots. Aztec, Australia Post and Mailmark count
     *                       from 1, QR Code and Micro QR Code from 0.
     */
    public function __construct(int $wsize, int $primitive = 0, int $firstroot = 1)
    {
        $this->firstroot = $firstroot;
        $this->genTables($wsize, $primitive);
    }

    /**
     * Returns the Reed-Solomon Error Correction Codewords for the input data.
     *
     * @param array<int> $data   Array of data codewords to process.
     * @param int   $necc   Number of error correction bytes.
     *
     * @return list<int> Array of $necc error correction codewords.
     */
    public function checkwords(array $data, int $necc): array
    {
        $coeff = $this->getCoefficients($data, $necc);
        return \array_values(\array_pad($coeff, -$necc, 0));
    }

    /**
     * Generates log and exp (inverse log) tables.
     *
     * @param int $wsize     Size of the word in bits.
     * @param int $primitive Primitive polynomial of the Galois field, or zero
     *                       for the one the word size selects in GF.
     */
    protected function genTables(int $wsize, int $primitive = 0): void
    {
        $this->tsize = self::TSIZE[$wsize] ?? 0;
        $this->tlog = \array_fill(0, \max(0, $this->tsize), 0);
        $this->texp = $this->tlog;
        $primitive = $primitive > 0 ? $primitive : self::GF[$wsize] ?? 0;
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
        $gen = $this->getGenerator($necc);
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
     * Returns the coefficients of the generator polynomial, the product of
     * x + a^i over the roots. The result is cached per number of error
     * correction codewords.
     *
     * @param int $necc Number of error correction bytes.
     *
     * @return array<int> Array of coefficients.
     */
    protected function getGenerator(int $necc): array
    {
        if (isset($this->gen[$necc])) {
            return $this->gen[$necc];
        }

        $gen = [1];
        for ($idx = 0; $idx < $necc; ++$idx) {
            $gen = $this->multiplyCoeff([1, $this->texp[$this->firstroot + $idx] ?? 0], $gen);
        }

        return $this->gen[$necc] = $gen;
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
        $slen = \count($smaller);
        $llen = \count($larger);
        $lendiff = $llen - $slen;
        $coeff = \array_slice($larger, 0, $lendiff);
        for ($idx = $lendiff; $idx < $llen; ++$idx) {
            $coeff[$idx] = ($smaller[$idx - $lendiff] ?? 0) ^ ($larger[$idx] ?? 0);
        }

        return $this->trimCoefficients($coeff);
    }
}
