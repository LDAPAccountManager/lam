<?php

declare(strict_types=1);

/**
 * Mask.php
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

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Mask
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Mask extends \Com\Tecnick\Barcode\Type\Square\QrCode\MaskNum
{
    /**
     * Run length
     *
     * @var array<int, int>
     */
    protected array $runLength = [];

    /**
     * Spec class object
     */
    protected Spec $spc;

    /**
     * @param array<int, string> $frame
     */
    protected function getFrameRow(array $frame, int $index): string
    {
        return $frame[$index] ?? '';
    }

    /**
     * @param array<int, string> $frame
     */
    protected function replaceFrameRow(
        array &$frame,
        int $index,
        string $replacement,
        int $offset,
        ?int $length = null,
    ): void {
        $replaceLength = $length === null ? null : \max(0, $length);
        $frame[$index] = \substr_replace($this->getFrameRow($frame, $index), $replacement, $offset, $replaceLength);
    }

    protected function getRowChar(string $row, int $index): string
    {
        return $row[$index] ?? "\0";
    }

    protected function getRunLengthValue(int $index): int
    {
        return $this->runLength[$index] ?? 0;
    }

    protected function incrementRunLength(int $index): void
    {
        $this->runLength[$index] = $this->getRunLengthValue($index) + 1;
    }

    /**
     * Initialize
     *
     * @param int  $version       Code version
     * @param int  $level         Error Correction Level
     * @param int $qr_find_from_random If negative, checks all masks available,
     *                            otherwise the value indicates the number of masks to be checked,
     *                            mask ids are random
     * @param bool $qr_find_best_mask If true, estimates best mask (slow)
     * @param int $qr_default_mask Default mask used when $fbm is false
     */
    public function __construct(
        /**
         * QR code version.
         * The Size of QRcode is defined as version. Version is an integer value from 1 to 40.
         * Version 1 is 21*21 matrix. And 4 modules increases whenever 1 version increases.
         * So version 40 is 177*177 matrix.
         */
        public int $version,
        /**
         * Error correction level
         */
        protected int $level,
        protected int $qr_find_from_random = -1,
        protected bool $qr_find_best_mask = true,
        protected int $qr_default_mask = 2,
    ) {
        $this->spc = new Spec();
    }

    /**
     * Get the best mask
     *
     * @param int   $width Width
     * @param array<int, string> $frame Frame
     * @param int   $level Error Correction lLevel
     *
     * @return array<int, string> best mask
     *
     * @throws \Random\RandomException in case random mask selection fails
     */
    protected function mask(int $width, array $frame, int $level): array
    {
        $minDemerit = PHP_INT_MAX;
        $bestMask = [];
        $checked_masks = [0, 1, 2, 3, 4, 5, 6, 7];
        if ($this->qr_find_from_random >= 0) {
            // keep at least one candidate mask so a mask and its format info are always applied
            $howManuOut = \min(7, 8 - ($this->qr_find_from_random % 9));
            for ($idx = 0; $idx < $howManuOut; ++$idx) {
                $maxpos = \count($checked_masks) - 1;
                $remPos = $maxpos > 0 ? \random_int(0, $maxpos) : 0;
                unset($checked_masks[$remPos]);
                $checked_masks = \array_values($checked_masks);
            }
        }

        $bestMask = $frame;
        foreach ($checked_masks as $checked_mask) {
            $mask = \array_fill(0, \max(0, $width), \str_repeat("\0", \max(0, $width)));
            $demerit = 0;
            $blacks = $this->makeMaskNo($checked_mask, $width, $frame, $mask);
            $blacks += $this->writeFormatInformation($width, $mask, $checked_mask, $level);
            $blacks = (int) ((100 * $blacks) / ($width * $width));
            $demerit = (int) (\abs($blacks - 50) / 5) * Data::N4;
            $demerit += $this->evaluateSymbol($width, $mask);
            if ($demerit < $minDemerit) {
                $minDemerit = $demerit;
                $bestMask = $mask;
            }
        }

        return $bestMask;
    }

    /**
     * Make a mask
     *
     * @param int   $width  Mask width
     * @param array<int, string> $frame  Frame
     * @param int   $maskNo Mask number
     * @param int   $level  Error Correction level
     *
     * @return array<int, string> mask
     */
    protected function makeMask(int $width, array $frame, int $maskNo, int $level): array
    {
        $mask = [];
        $this->makeMaskNo($maskNo, $width, $frame, $mask);
        $this->writeFormatInformation($width, $mask, $maskNo, $level);
        return $mask;
    }

    /**
     * Write Format Information on the frame and returns the number of black bits
     *
     * @param int   $width  Mask width
     * @param array<int, string> $frame  Frame
     * @param int   $maskNo Mask number
     * @param int   $level  Error Correction level
     *
     * @return int blacks
     */
    protected function writeFormatInformation(int $width, array &$frame, int $maskNo, int $level): int
    {
        $blacks = 0;
        $spec = new Spec();
        $format = $spec->getFormatInfo($maskNo, $level);
        for ($idx = 0; $idx < 8; ++$idx) {
            $val = 0x84;
            if (($format & 1) !== 0) {
                $blacks += 2;
                $val = 0x85;
            }

            $this->replaceFrameRow($frame, 8, \chr($val & 0xFF), $width - 1 - $idx, 1);
            if ($idx < 6) {
                $this->replaceFrameRow($frame, $idx, \chr($val & 0xFF), 8, 1);
                $format >>= 1;
                continue;
            }

            $this->replaceFrameRow($frame, $idx + 1, \chr($val & 0xFF), 8, 1);

            $format >>= 1;
        }

        for ($idx = 0; $idx < 7; ++$idx) {
            $val = 0x84;
            if (($format & 1) !== 0) {
                $blacks += 2;
                $val = 0x85;
            }

            $this->replaceFrameRow($frame, $width - 7 + $idx, \chr($val & 0xFF), 8, 1);
            if ($idx === 0) {
                $this->replaceFrameRow($frame, 8, \chr($val & 0xFF), 7, 1);
                $format >>= 1;
                continue;
            }

            $this->replaceFrameRow($frame, 8, \chr($val & 0xFF), 6 - $idx, 1);

            $format >>= 1;
        }

        return $blacks;
    }

    /**
     * Evaluate Symbol and returns demerit value.
     *
     * @param int   $width Width
     * @param array<int, string> $frame Frame
     */
    protected function evaluateSymbol(int $width, array $frame): int
    {
        // horizontal direction: accumulate the per-row demerit over every row
        $demerit = 0;
        for ($ypos = 0; $ypos < $width; ++$ypos) {
            $frameY = $this->getFrameRow($frame, $ypos);
            $frameYM = $ypos > 0 ? $this->getFrameRow($frame, $ypos - 1) : $frameY;
            $demerit += $this->evaluateSymbolB($ypos, $width, $frameY, $frameYM);
        }

        // vertical direction: accumulate the per-column demerit over every column
        for ($xpos = 0; $xpos < $width; ++$xpos) {
            $head = 0;
            $this->runLength[0] = 1;
            for ($ypos = 0; $ypos < $width; ++$ypos) {
                if ($ypos === 0 && \ord($this->getRowChar($this->getFrameRow($frame, $ypos), $xpos)) & 1) {
                    $this->runLength[0] = -1;
                    $head = 1;
                    $this->runLength[$head] = 1;
                    continue;
                }

                if ($ypos > 0) {
                    if (
                        (
                            (
                                \ord($this->getRowChar($this->getFrameRow($frame, $ypos), $xpos))
                                ^ \ord($this->getRowChar($this->getFrameRow($frame, $ypos - 1), $xpos))
                            )
                            & 1
                        )
                        !== 0
                    ) {
                        ++$head;
                        $this->runLength[$head] = 1;
                        continue;
                    }

                    $this->incrementRunLength($head);
                }
            }

            $demerit += $this->calcN1N3($head + 1);
        }

        return $demerit;
    }

    /**
     * Evaluate Symbol
     *
     * @param int   $ypos   Y position
     * @param int   $width  Width
     *
     * @return int demerit
     */
    protected function evaluateSymbolB(int $ypos, int $width, string $frameY, string $frameYM): int
    {
        $head = 0;
        $demerit = 0;
        $this->runLength[0] = 1;
        for ($xpos = 0; $xpos < $width; ++$xpos) {
            if ($xpos > 0 && $ypos > 0) {
                $b22 =
                    \ord($this->getRowChar($frameY, $xpos))
                    & \ord($this->getRowChar($frameY, $xpos - 1))
                    & \ord($this->getRowChar($frameYM, $xpos))
                    & \ord($this->getRowChar($frameYM, $xpos - 1));
                $w22 =
                    \ord($this->getRowChar($frameY, $xpos))
                    | \ord($this->getRowChar($frameY, $xpos - 1))
                    | \ord($this->getRowChar($frameYM, $xpos))
                    | \ord($this->getRowChar($frameYM, $xpos - 1));
                if ((($b22 | ($w22 ^ 1)) & 1) !== 0) {
                    $demerit += Data::N2;
                }
            }

            if ($xpos === 0 && \ord($this->getRowChar($frameY, $xpos)) & 1) {
                $this->runLength[0] = -1;
                $head = 1;
                $this->runLength[$head] = 1;
                continue;
            }

            if ($xpos > 0) {
                if (
                    ((\ord($this->getRowChar($frameY, $xpos)) ^ \ord($this->getRowChar($frameY, $xpos - 1))) & 1) !== 0
                ) {
                    ++$head;
                    $this->runLength[$head] = 1;
                    continue;
                }

                $this->incrementRunLength($head);
            }
        }

        return $demerit + $this->calcN1N3($head + 1);
    }

    /**
     * Calc N1 N3
     *
     * @param int $length Length
     *
     * @return int demerit
     */
    protected function calcN1N3(int $length): int
    {
        $demerit = 0;
        for ($idx = 0; $idx < $length; ++$idx) {
            $runLen = $this->getRunLengthValue($idx);
            if ($runLen >= 5) {
                $demerit += Data::N1 + ($runLen - 5);
            }

            if ($idx & 1 && $idx >= 3 && $idx < ($length - 2) && ($runLen % 3) === 0) {
                $demerit += $this->calcN1N3delta($length, $idx);
            }
        }

        return $demerit;
    }

    /**
     * Calc N1 N3 delta
     *
     * @param int $length Length
     * @param int $idx    Index
     *
     * @return int demerit delta
     */
    protected function calcN1N3delta(int $length, int $idx): int
    {
        $fact = (int) ($this->getRunLengthValue($idx) / 3);
        if (
            $this->getRunLengthValue($idx - 2) === $fact
            && $this->getRunLengthValue($idx - 1) === $fact
            && $this->getRunLengthValue($idx + 1) === $fact
            && $this->getRunLengthValue($idx + 2) === $fact
        ) {
            if ($this->getRunLengthValue($idx - 3) < 0 || $this->getRunLengthValue($idx - 3) >= (4 * $fact)) {
                return Data::N3;
            }

            if (($idx + 3) >= $length || $this->getRunLengthValue($idx + 3) >= (4 * $fact)) {
                return Data::N3;
            }
        }

        return 0;
    }
}
