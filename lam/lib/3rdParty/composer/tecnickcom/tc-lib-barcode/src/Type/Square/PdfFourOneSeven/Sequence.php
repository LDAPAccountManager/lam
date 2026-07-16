<?php

declare(strict_types=1);

/**
 * Process.php
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

namespace Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven;

/**
 * Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven\Sequence
 *
 * Process for PdfFourOneSeven Barcode type class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Sequence extends \Com\Tecnick\Barcode\Type\Square
{
    /**
     * @return array<int, int>
     */
    protected function getRsFactors(int $ecl): array
    {
        $values = Data::RS_FACTORS[$ecl] ?? [];
        return \array_values($values);
    }

    /**
     * @param array<int, int> $data
     */
    protected function getArrayInt(array $data, int $index): int
    {
        return $data[$index] ?? 0;
    }

    /**
     * @param array<int, array{int, string}> $sequence_array
     */
    protected function getLastSequenceMode(array $sequence_array): int
    {
        if ($sequence_array === []) {
            return -1;
        }

        return $sequence_array[\count($sequence_array) - 1][0] ?? -1;
    }

    /**
     * Get the error correction level (0-8) to be used
     *
     * @param int $ecl   Error correction level
     * @param int $numcw Number of data codewords
     *
     * @return int error correction level
     */
    protected function getErrorCorrectionLevel(int $ecl, int $numcw): int
    {
        $maxecl = 8; // maximum error level
        $maxerrsize = 928 - $numcw; // available codewords for error
        while ($maxecl > 0 && $maxerrsize < (2 << $maxecl)) {
            --$maxecl;
        }

        if ($ecl < 0 || $ecl > 8) {
            $ecl = $maxecl;
            $ecl = match (true) {
                $numcw < 41 => 2,
                $numcw < 161 => 3,
                $numcw < 321 => 4,
                $numcw < 864 => 5,
                default => $ecl,
            };
        }

        return (int) \min($maxecl, $ecl);
    }

    /**
     * Get the error correction codewords
     *
     * @param array<int, int> $codewords  Array of codewords including Symbol Length Descriptor and pad
     * @param int   $ecl        Error correction level 0-8
     *
     * @return array<int, int> of error correction codewords
     */
    protected function getErrorCorrection(array $codewords, int $ecl): array
    {
        // get error correction coefficients
        $ecc = $this->getRsFactors($ecl);
        // number of error correction factors
        $eclsize = \max(0, 2 << $ecl);
        // maximum index for RS_FACTORS[$ecl]
        $eclmaxid = $eclsize - 1;
        // initialize array of error correction codewords
        $ecw = \array_fill(0, $eclsize, 0);
        // for each data codeword
        foreach ($codewords as $codeword) {
            $tk1 = ($codeword + $this->getArrayInt($ecw, $eclmaxid)) % 929;
            for ($idx = $eclmaxid; $idx > 0; --$idx) {
                $tk2 = ($tk1 * $this->getArrayInt($ecc, $idx)) % 929;
                $tk3 = 929 - $tk2;
                $ecw[$idx] = (int) (($this->getArrayInt($ecw, $idx - 1) + $tk3) % 929);
            }

            $tk2 = ($tk1 * $this->getArrayInt($ecc, 0)) % 929;
            $tk3 = 929 - $tk2;
            $ecw[0] = (int) ($tk3 % 929);
        }

        foreach ($ecw as $idx => $err) {
            if ($err === 0) {
                continue;
            }

            $ecw[$idx] = (int) (929 - $err);
        }

        return \array_reverse($ecw);
    }

    /**
     * Process a single sequence
     *
     * @param array<int, array{int, string}>  $sequence_array  Sequence to process
     * @param string $code            Data to process
     * @param int    $seq             Current sequence
     * @param int    $offset          Current code offset
     */
    protected function processSequence(array &$sequence_array, string $code, int $seq, int $offset): void
    {
        // extract text sequence before the number sequence
        $prevseq = \substr($code, $offset, $seq - $offset);
        $textseq = [];
        // get text sequences
        \preg_match_all('/([\x09\x0a\x0d\x20-\x7e]{5,})/', $prevseq, $textseq, PREG_OFFSET_CAPTURE);
        $textseq[1][] = ['', \strlen($prevseq)];
        $txtoffset = 0;
        foreach ($textseq[1] as $txtseq) {
            $txtSeqOffset = (int) $txtseq[1];
            $txtseqlen = \strlen($txtseq[0]);
            if ($txtSeqOffset > 0) {
                // extract byte sequence before the text sequence
                $prevtxtseq = \substr($prevseq, $txtoffset, $txtSeqOffset - $txtoffset);
                if (\strlen($prevtxtseq) > 0) {
                    // add BYTE sequence
                    $mode = 901;
                    $mode = match (true) {
                        \strlen($prevtxtseq) === 1 && $this->getLastSequenceMode($sequence_array) === 900 => 913,
                        (\strlen($prevtxtseq) % 6) === 0 => 924,
                        default => $mode,
                    };

                    $sequence_array[] = [$mode, $prevtxtseq];
                }
            }

            if ($txtseqlen > 0) {
                // add numeric sequence
                $sequence_array[] = [900, $txtseq[0]];
            }

            $txtoffset = $txtSeqOffset + $txtseqlen;
        }
    }

    /**
     * Get an array of sequences from input
     *
     * @param string $code Data to process
     *
     * @return array<int, array{int, string}>
     */
    protected function getInputSequences(string $code): array
    {
        $sequence_array = []; // array to be returned
        $numseq = [];
        // get numeric sequences
        \preg_match_all('/(\d{13,})/', $code, $numseq, PREG_OFFSET_CAPTURE);
        $numseq[1][] = ['', \strlen($code)];
        $offset = 0;
        foreach ($numseq[1] as $seq) {
            $seqlen = \strlen($seq[0]);
            $seqOffset = (int) $seq[1];
            if ($seqOffset > 0) {
                $this->processSequence($sequence_array, $code, $seqOffset, $offset);
            }

            if ($seqlen > 0) {
                // add numeric sequence
                $sequence_array[] = [902, $seq[0]];
            }

            $offset = $seqOffset + $seqlen;
        }

        return $sequence_array;
    }
}
