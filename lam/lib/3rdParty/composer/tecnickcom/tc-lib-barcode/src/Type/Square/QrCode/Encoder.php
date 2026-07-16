<?php

declare(strict_types=1);

/**
 * Encoder.php
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

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Encoder
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Encoder extends \Com\Tecnick\Barcode\Type\Square\QrCode\Init
{
    protected function getRsBlockDataLength(int $row): int
    {
        return $this->rsblocks[$row]['dataLength'] ?? 0;
    }

    protected function getRsBlockCode(int $row, string $type, int $col): int
    {
        return $this->rsblocks[$row][$type][$col] ?? 0;
    }

    protected function getFrameOrd(int $xpos, int $ypos): int
    {
        return \ord($this->frame[$ypos][$xpos] ?? "\x00");
    }

    /**
     * Encode mask
     *
     * @param int   $maskNo   Mask number (masking mode)
     * @param array<int, int> $datacode Data code to encode
     *
     * @return array<int, string> Encoded Mask
     *
     * @throws BarcodeException in case of error
     * @throws \Random\RandomException in case of random generation error
     */
    public function encodeMask(int $maskNo, array $datacode): array
    {
        // initialize values
        $this->datacode = $datacode;
        $spec = $this->spc->getEccSpec($this->version, $this->level, [0, 0, 0, 0, 0]);
        $this->bv1 = $this->spc->rsBlockNum1($spec);
        $this->dataLength = $this->spc->rsDataLength($spec);
        $this->eccLength = \max(0, $this->spc->rsEccLength($spec));
        $this->ecccode = \array_fill(0, $this->eccLength, 0);
        $this->blocks = $this->spc->rsBlockNum($spec);
        $this->init($spec);
        $this->count = 0;
        $this->width = $this->spc->getWidth($this->version);
        $this->frame = $this->spc->createFrame($this->version);
        $this->xpos = $this->width - 1;
        $this->ypos = $this->width - 1;
        $this->dir = -1;
        $this->bit = -1;

        // interleaved data and ecc codes
        for ($idx = 0; $idx < ($this->dataLength + $this->eccLength); ++$idx) {
            $code = $this->getCode();
            $bit = 0x80;
            for ($jdx = 0; $jdx < 8; ++$jdx) {
                $addr = $this->getNextPosition();
                $this->setFrameAt($addr, 0x02 | ($bit & $code) !== 0);
                $bit >>= 1;
            }
        }

        // remainder bits
        $rbits = $this->spc->getRemainder($this->version);
        for ($idx = 0; $idx < $rbits; ++$idx) {
            $addr = $this->getNextPosition();
            $this->setFrameAt($addr, 0x02);
        }

        // masking
        $this->runLength = \array_fill(0, \max(0, Data::QRSPEC_WIDTH_MAX + 1), 0);
        if ($maskNo >= 0) {
            return $this->makeMask($this->width, $this->frame, $maskNo, $this->level);
        }

        if ($this->qr_find_best_mask) {
            return $this->mask($this->width, $this->frame, $this->level);
        }

        return $this->makeMask($this->width, $this->frame, $this->qr_default_mask % 8, $this->level);
    }

    /**
     * Return Reed-Solomon block code
     *
     * @return int rsblocks
     */
    protected function getCode(): int
    {
        if ($this->count < $this->dataLength) {
            $row = $this->count % $this->blocks;
            $col = (int) \floor($this->count / $this->blocks);
            if ($col >= $this->getRsBlockDataLength(0)) {
                $row += $this->bv1;
            }

            $ret = $this->getRsBlockCode($row, 'data', $col);
            ++$this->count;
            return $ret;
        }

        if ($this->count < ($this->dataLength + $this->eccLength)) {
            $row = ($this->count - $this->dataLength) % $this->blocks;
            $col = (int) \floor(($this->count - $this->dataLength) / $this->blocks);
            $ret = $this->getRsBlockCode($row, 'ecc', $col);
            ++$this->count;
            return $ret;
        }

        return 0;
    }

    /**
     * Set frame value at specified position
     *
     * @param array{'x': int, 'y': int} $pos X,Y position
     * @param int   $val Value of the character to set
     */
    protected function setFrameAt(array $pos, int $val): void
    {
        $this->frame[$pos['y']][$pos['x']] = \chr($val & 0xFF);
    }

    /**
     * Return the next frame position
     *
     * @return array{'x': int, 'y': int} of x,y coordinates
     *
     * @throws BarcodeException in case of error
     */
    protected function getNextPosition(): array
    {
        do {
            if ($this->bit === -1) {
                $this->bit = 0;
                return [
                    'x' => $this->xpos,
                    'y' => $this->ypos,
                ];
            }

            $xpos = $this->xpos;
            $ypos = $this->ypos;
            $wdt = $this->width;
            $this->getNextPositionB($xpos, $ypos, $wdt);
            if ($xpos < 0 || $ypos < 0) {
                throw new BarcodeException('Error getting next position');
            }

            $this->xpos = $xpos;
            $this->ypos = $ypos;
        } while (($this->getFrameOrd($xpos, $ypos) & 0x80) !== 0);

        return [
            'x' => $xpos,
            'y' => $ypos,
        ];
    }

    /**
     * Internal cycle for getNextPosition
     *
     * @param int $xpos X position
     * @param int $ypos Y position
     * @param int $wdt  Width
     */
    protected function getNextPositionB(int &$xpos, int &$ypos, int $wdt): void
    {
        $wasBitZero = $this->bit === 0;
        if ($wasBitZero) {
            --$xpos;
            ++$this->bit;
        }

        if (!$wasBitZero) {
            ++$xpos;
            $ypos += $this->dir;
            --$this->bit;
        }

        if ($this->dir < 0) {
            if ($ypos < 0) {
                $ypos = 0;
                $xpos -= 2;
                $this->dir = 1;
                if ($xpos === 6) {
                    --$xpos;
                    $ypos = 9;
                }
            }
            return;
        }

        if ($ypos === $wdt) {
            $ypos = $wdt - 1;
            $xpos -= 2;
            $this->dir = -1;
            if ($xpos === 6) {
                --$xpos;
                $ypos -= 8;
            }
        }
    }
}
