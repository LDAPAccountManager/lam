<?php

declare(strict_types=1);

/**
 * CodeOneTwoEight.php
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
 * Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight;
 *
 * CodeOneTwoEight Barcode type class
 * CODE 128
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class CodeOneTwoEight extends \Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight\Process
{
    /**
     * Get the code point array
     *
     * @return array<int, int>
     *
     * @throws BarcodeException in case of error
     */
    protected function getCodeData(): array
    {
        $code = $this->code;
        // array of symbols
        $code_data = [];
        // split code into sequences
        $sequence = $this->getNumericSequence($code);
        // process the sequence
        $startid = 0;
        foreach ($sequence as $key => $seq) {
            switch ($seq[0]) {
                case 'A':
                    $this->processSequenceA($sequence, $code_data, $startid, $key, $seq);
                    break;
                case 'B':
                    $this->processSequenceB($sequence, $code_data, $startid, $key, $seq);
                    break;
                case 'C':
                    $this->processSequenceC($sequence, $code_data, $startid, $key, $seq);
                    break;
                default:
                    throw new BarcodeException('Invalid sequence mode');
            }
        }

        return $this->finalizeCodeData($code_data, $startid);
    }

    /**
     * @param array<int, array{0: string, 1: string, 2: int, 3?: string}> $sequence
     */
    protected function getSequenceMode(array $sequence, int $key): string
    {
        return $sequence[$key][0] ?? '';
    }

    /**
     * @param array<int, array{0: string, 1: string, 2: int, 3?: string}> $sequence
     */
    protected function hasSequenceShift(array $sequence, int $key): bool
    {
        return ($sequence[$key][3] ?? null) !== null;
    }

    /**
     * Process the A sequence
     *
     * @param array<int, array{0: string, 1: string, 2: int, 3?: string}>  $sequence   Sequence to process
     * @param array<int, int>  $code_data  Array of codepoints to alter
     * @param int    $startid    Start ID
     * @param int    $key        Sequence current key
     * @param array{string, string, int} $seq        Sequence current value
     *
     * @throws BarcodeException in case of error
     */
    protected function processSequenceA(array &$sequence, array &$code_data, int &$startid, int $key, array $seq): void
    {
        $prev_mode = $this->getSequenceMode($sequence, $key - 1);
        if ($key === 0) {
            $startid = 103;
        }

        if ($key !== 0 && $prev_mode !== 'A') {
            $hasPrevShift = $this->hasSequenceShift($sequence, $key - 1);
            $singleShift = $seq[2] === 1 && $prev_mode === 'B' && !$hasPrevShift;
            $codeSwitch = match (true) {
                $singleShift => 98,
                !$hasPrevShift => 101,
                default => null,
            };

            if ($codeSwitch !== null) {
                $code_data[] = $codeSwitch;
                if ($codeSwitch === 98) {
                    // mark single shift
                    $sequence[$key][3] = '';
                }
            }
        }

        $this->getCodeDataA($code_data, $seq[1], (int) $seq[2]);
    }

    /**
     * Process the B sequence
     *
     * @param array<int, array{0: string, 1: string, 2: int, 3?: string}>  $sequence   Sequence to process
     * @param array<int, int>  $code_data  Array of codepoints to alter
     * @param int    $startid    Start ID
     * @param int    $key        Sequence current key
     * @param array{string, string, int} $seq        Sequence current value
     *
     * @throws BarcodeException in case of error
     */
    protected function processSequenceB(array &$sequence, array &$code_data, int &$startid, int $key, array $seq): void
    {
        $prev_mode = $this->getSequenceMode($sequence, $key - 1);
        if ($key === 0) {
            $this->processSequenceBA($sequence, $code_data, $startid, $key, $seq);
        }

        if ($key !== 0 && $prev_mode !== 'B') {
            $this->processSequenceBB($sequence, $code_data, $key, $seq);
        }

        $this->getCodeDataB($code_data, $seq[1], (int) $seq[2]);
    }

    /**
     * Process the B-A sequence
     *
     * @param array<int, array{0: string, 1: string, 2: int, 3?: string}>  $sequence   Sequence to process
     * @param array<int, int>  $code_data  Array of codepoints to alter
     * @param int    $startid    Start ID
     * @param int    $key        Sequence current key
     * @param array{string, string, int} $seq        Sequence current value
     *
     * @throws BarcodeException in case of error
     */
    protected function processSequenceBA(array &$sequence, array &$code_data, int &$startid, int $key, array $seq): void
    {
        $tmpchr = \ord($seq[1][0]);
        $next_mode = $this->getSequenceMode($sequence, $key + 1);
        $startid = 104;
        if ($seq[2] === 1 && $tmpchr >= 241 && $tmpchr <= 244 && $next_mode !== '' && $next_mode !== 'B') {
            switch ($next_mode) {
                case 'A':
                    $startid = 103;
                    $sequence[$key][0] = 'A';
                    $code_data[] = $this->getFncAValue($tmpchr);
                    break;
                case 'C':
                    $startid = 105;
                    $sequence[$key][0] = 'C';
                    $code_data[] = $this->getFncAValue($tmpchr);
                    break;
            }
        }
    }

    /**
     * Process the B-B sequence
     *
     * @param array<int, array{0: string, 1: string, 2: int, 3?: string}>  $sequence   Sequence to process
     * @param array<int, int>  $code_data  Array of codepoints to alter
     * @param int    $key        Sequence current key
     * @param array{string, string, int} $seq        Sequence current value
     */
    protected function processSequenceBB(array &$sequence, array &$code_data, int $key, array $seq): void
    {
        $prev_mode = $this->getSequenceMode($sequence, $key - 1);
        $hasPrevShift = $this->hasSequenceShift($sequence, $key - 1);
        $singleShift = $seq[2] === 1 && $prev_mode === 'A' && !$hasPrevShift;
        $codeSwitch = match (true) {
            $singleShift => 98,
            !$hasPrevShift => 100,
            default => null,
        };

        if ($codeSwitch !== null) {
            $code_data[] = $codeSwitch;
            if ($codeSwitch === 98) {
                // mark single shift
                $sequence[$key][3] = '';
            }
        }
    }

    /**
     * Process the C sequence
     *
     * @param array<int, array{0: string, 1: string, 2: int, 3?: string}>  $sequence   Sequence to process
     * @param array<int, int>  $code_data  Array of codepoints to alter
     * @param int    $startid    Start ID
     * @param int    $key        Sequence current key
     * @param array{string, string, int} $seq        Sequence current value
     *
     * @throws BarcodeException in case of error
     */
    protected function processSequenceC(array &$sequence, array &$code_data, int &$startid, int $key, array $seq): void
    {
        $prev_mode = $this->getSequenceMode($sequence, $key - 1);
        if ($key === 0) {
            $startid = 105;
        }

        if ($key !== 0 && $prev_mode !== 'C') {
            $code_data[] = 99;
        }

        $this->getCodeDataC($code_data, $seq[1]);
    }

    protected function getBarPattern(int $value): string
    {
        return $this::CHBAR[$value] ?? '';
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $code_data = $this->getCodeData();
        $this->ncols = 0;
        $this->nrows = 1;
        $this->bars = [];
        foreach ($code_data as $val) {
            $seq = $this->getBarPattern($val);
            for ($pos = 0; $pos < 6; ++$pos) {
                $bar_width = (int) ($seq[$pos] ?? '0');
                if (($pos % 2) === 0 && $bar_width > 0) {
                    $this->bars[] = [$this->ncols, 0, $bar_width, 1];
                }

                $this->ncols += $bar_width;
            }
        }
    }
}
