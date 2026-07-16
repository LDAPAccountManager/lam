<?php

declare(strict_types=1);

/**
 * MaskNum.php
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
 * Com\Tecnick\Barcode\Type\Square\QrCode\MaskNum
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class MaskNum
{
    /**
     * @param array<int, string> $rows
     */
    protected function getRow(array $rows, int $ypos): string
    {
        return $rows[$ypos] ?? '';
    }

    protected function getRowCharOrd(string $row, int $xpos): int
    {
        return \ord($row[$xpos] ?? "\x00");
    }

    /**
     * @param array<int, array<int, int>> $bitMask
     */
    protected function getMaskBit(array $bitMask, int $ypos, int $xpos): int
    {
        return $bitMask[$ypos][$xpos] ?? 0;
    }

    /**
     * Make Mask Number
     *
     * @param int   $maskNo Mask number
     * @param int   $width  Width
     * @param array<int, string> $frame  Frame
     * @param array<int, string> $mask   Mask
     *
     * @return int mask number
     */
    protected function makeMaskNo(int $maskNo, int $width, array $frame, array &$mask): int
    {
        $bnum = 0;
        $bitMask = $this->generateMaskNo($maskNo, $width, $frame);
        $mask = $frame;
        for ($ypos = 0; $ypos < $width; ++$ypos) {
            for ($xpos = 0; $xpos < $width; ++$xpos) {
                $mask_bit = $this->getMaskBit($bitMask, $ypos, $xpos);
                if ($mask_bit === 1) {
                    $frame_row = $this->getRow($frame, $ypos);
                    $mask_row = $this->getRow($mask, $ypos);
                    $mask[$ypos] = \substr_replace(
                        $mask_row,
                        \chr(($this->getRowCharOrd($frame_row, $xpos) ^ $mask_bit) & 0xFF),
                        $xpos,
                        1,
                    );
                }

                $bnum += $this->getRowCharOrd($this->getRow($mask, $ypos), $xpos) & 1;
            }
        }

        return $bnum;
    }

    /**
     * Return bit mask
     *
     * @param int   $maskNo Mask number
     * @param int   $width  Width
     * @param array<int, string> $frame  Frame
     *
     * @return array<int, array<int, int>> bit mask
     */
    protected function generateMaskNo(int $maskNo, int $width, array $frame): array
    {
        $mask_width = \max(0, $width);
        $bitMask = \array_fill(0, $mask_width, \array_fill(0, $mask_width, 0));
        for ($ypos = 0; $ypos < $width; ++$ypos) {
            $frame_row = $this->getRow($frame, $ypos);
            for ($xpos = 0; $xpos < $width; ++$xpos) {
                if (($this->getRowCharOrd($frame_row, $xpos) & 0x80) !== 0) {
                    $bitMask[$ypos][$xpos] = 0;
                    continue;
                }
                $maskFunc = match ($maskNo) {
                    0 => ($xpos + $ypos) & 1,
                    1 => $ypos & 1,
                    2 => $xpos % 3,
                    3 => ($xpos + $ypos) % 3,
                    4 => ((int) ($ypos / 2) + (int) ($xpos / 3)) & 1,
                    5 => (($xpos * $ypos) & 1) + (($xpos * $ypos) % 3),
                    6 => ((($xpos * $ypos) & 1) + (($xpos * $ypos) % 3)) & 1,
                    7 => ((($xpos * $ypos) % 3) + (($xpos + $ypos) & 1)) & 1,
                    default => 1,
                };
                $bitMask[$ypos][$xpos] = $maskFunc === 0 ? 1 : 0;
            }
        }

        return $bitMask;
    }
}
