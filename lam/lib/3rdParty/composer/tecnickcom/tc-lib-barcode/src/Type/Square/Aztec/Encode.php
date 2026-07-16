<?php

declare(strict_types=1);

/**
 * Encode.php
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

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Square\Aztec\Encode
 *
 * Encode for Aztec Barcode type class
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Encode extends \Com\Tecnick\Barcode\Type\Square\Aztec\Bitstream
{
    /**
     * Bidimensional grid containing the encoded data.
     *
     * @var array<int, array<int>>
     */
    protected array $grid = [];

    /**
     * Coordinate of the grid center.
     */
    protected int $gridcenter = 0;

    /**
     * Aztec main encoder.
     *
     * @param string $code The code to encode.
     * @param int    $ecc  The error correction code percentage of error check words.
     * @param int    $eci  The ECI mode to use.
     * @param string $hint The mode to use.
     * @param string $mode The mode to use (A = Automatic; F = Full Range mode).
     *
     * @throws BarcodeException
     */
    public function __construct(string $code, int $ecc = 33, int $eci = 0, string $hint = 'A', string $mode = 'A')
    {
        $this->highLevelEncoding($code, $eci, $hint);
        if (!$this->sizeAndBitStuffing($ecc, $mode)) {
            throw new BarcodeException('Data too long');
        }

        $wsize = $this->layer[2];
        $nbits = $this->layer[3];
        $numcdw = $this->addCheckWords($this->bitstream, $this->totbits, $nbits, $wsize);
        $this->setGrid();
        $this->drawMode($numcdw);
        $this->drawData();
    }

    /**
     * Returns the bidimensional grid containing the encoded data.
     *
     * @return array<int, array<int>>
     */
    public function getGrid(): array
    {
        return $this->grid;
    }

    /**
     * Returns the Check Codewords array for the given data words.
     *
     * @param array<int> $bitstream Array of bits.
     * @param int   $totbits   Number of bits in the bitstream.
     * @param int   $nbits     Number of bits per layer.
     * @param int   $wsize     Word size.
     *
     * @return int The number of data codewords.
     */
    protected function addCheckWords(array &$bitstream, int &$totbits, int $nbits, int $wsize): int
    {
        $cdw = $this->bitstreamToWords($bitstream, $totbits, $wsize);
        $numcdw = \count($cdw);
        $totwords = (int) ($nbits / $wsize);
        $eccwords = $totwords - $numcdw;
        $errorCorrection = new ErrorCorrection($wsize);
        $checkwords = $errorCorrection->checkwords($cdw, $eccwords);
        // append check codewords
        foreach ($checkwords as $checkword) {
            $this->appendWordToBitstream($bitstream, $totbits, $wsize, $checkword);
        }

        return $numcdw;
    }

    /**
     * Initialize the grid with all patterns.
     */
    protected function setGrid(): void
    {
        // initialize grid
        $size = $this->layer[0];
        $size = \max(0, $size);
        $row = \array_fill(0, $size, 0);
        $this->grid = \array_fill(0, $size, $row);
        // draw center
        $center = (int) (($size - 1) / 2);
        $this->gridcenter = $center;
        $this->grid[$center][$center] = 1;
        // draw finder pattern (bulls-eye)
        $bewidth = $this->compact ? 11 : 15;
        $bemid = (int) (($bewidth - 1) / 2);
        for ($rng = 2; $rng < $bemid; $rng += 2) {
            // center cross points
            $this->grid[$center + $rng][$center] = 1;
            $this->grid[$center - $rng][$center] = 1;
            $this->grid[$center][$center + $rng] = 1;
            $this->grid[$center][$center - $rng] = 1;
            // corner points
            $this->grid[$center + $rng][$center + $rng] = 1;
            $this->grid[$center + $rng][$center - $rng] = 1;
            $this->grid[$center - $rng][$center + $rng] = 1;
            $this->grid[$center - $rng][$center - $rng] = 1;
            for ($pos = 1; $pos < $rng; ++$pos) {
                // horizontal points
                $this->grid[$center + $rng][$center + $pos] = 1;
                $this->grid[$center + $rng][$center - $pos] = 1;
                $this->grid[$center - $rng][$center + $pos] = 1;
                $this->grid[$center - $rng][$center - $pos] = 1;
                // vertical points
                $this->grid[$center + $pos][$center + $rng] = 1;
                $this->grid[$center + $pos][$center - $rng] = 1;
                $this->grid[$center - $pos][$center + $rng] = 1;
                $this->grid[$center - $pos][$center - $rng] = 1;
            }
        }

        // draw orientation patterns
        $this->grid[$center - $bemid][$center - $bemid] = 1; // TL
        $this->grid[$center - $bemid][$center - $bemid + 1] = 1; // TL-R
        $this->grid[$center - $bemid + 1][$center - $bemid] = 1; // TL-B
        $this->grid[$center - $bemid][$center + $bemid] = 1; // TR-T
        $this->grid[$center - $bemid + 1][$center + $bemid] = 1; // TR-B
        $this->grid[$center + $bemid - 1][$center + $bemid] = 1; // BR
        if ($this->compact) {
            return;
        }

        // draw reference grid for full mode
        $halfsize = (int) (($size - 1) / 2);
        // central cross
        for ($pos = 8; $pos <= $halfsize; $pos += 2) {
            // horizontal
            $this->grid[$center][$center - $pos] = 1;
            $this->grid[$center][$center + $pos] = 1;
            // vertical
            $this->grid[$center - $pos][$center] = 1;
            $this->grid[$center + $pos][$center] = 1;
        }

        // grid lines
        for ($pos = 2; $pos <= $halfsize; $pos += 2) {
            for ($ref = 16; $ref <= $halfsize; $ref += 16) {
                // horizontal
                $this->grid[$center - $ref][$center - $pos] = 1;
                $this->grid[$center - $ref][$center + $pos] = 1;
                $this->grid[$center + $ref][$center - $pos] = 1;
                $this->grid[$center + $ref][$center + $pos] = 1;
                // vertical
                $this->grid[$center - $pos][$center - $ref] = 1;
                $this->grid[$center - $pos][$center + $ref] = 1;
                $this->grid[$center + $pos][$center - $ref] = 1;
                $this->grid[$center + $pos][$center + $ref] = 1;
            }
        }
    }

    /**
     * Add the mode message to the grid.
     *
     * @param int $numcdw Number of data codewords.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    protected function drawMode(int $numcdw): void
    {
        $modebs = [];
        $nbits = 0;
        $center = $this->gridcenter;
        $modebits = 40;
        $layersbits = 5;
        $codewordsbits = 11;
        $sidelen = 10;
        $srow = -7;
        $scol = -5;
        if ($this->compact) {
            $modebits = 28;
            $layersbits = 2;
            $codewordsbits = 6;
            $sidelen = 7;
            $srow = -5;
            $scol = -3;
        }

        $this->appendWordToBitstream($modebs, $nbits, $layersbits, $this->numlayers - 1);
        $this->appendWordToBitstream($modebs, $nbits, $codewordsbits, $numcdw - 1);
        $this->addCheckWords($modebs, $nbits, $modebits, 4);
        // draw the mode message in the grid clockwise starting from the top left corner
        $bit = 0;
        // top
        $ypos = $center + $srow;
        $xpos = $center + $scol;
        for ($pos = 0; $pos < $sidelen; ++$pos) {
            $xpos += $this->skipModeRefGrid($pos);
            $this->grid[$ypos][$xpos] = ($modebs[$bit++] ?? 0) === 0 ? 0 : 1;
            ++$xpos;
        }

        // right
        $ypos += 2;
        ++$xpos;
        for ($pos = 0; $pos < $sidelen; ++$pos) {
            $ypos += $this->skipModeRefGrid($pos);
            $this->grid[$ypos][$xpos] = ($modebs[$bit++] ?? 0) === 0 ? 0 : 1;
            ++$ypos;
        }

        // bottom
        ++$ypos;
        $xpos -= 2;
        for ($pos = 0; $pos < $sidelen; ++$pos) {
            $xpos -= $this->skipModeRefGrid($pos);
            $this->grid[$ypos][$xpos] = ($modebs[$bit++] ?? 0) === 0 ? 0 : 1;
            --$xpos;
        }

        // left
        $ypos -= 2;
        --$xpos;
        for ($pos = 0; $pos < $sidelen; ++$pos) {
            $ypos -= $this->skipModeRefGrid($pos);
            $this->grid[$ypos][$xpos] = ($modebs[$bit++] ?? 0) === 0 ? 0 : 1;
            --$ypos;
        }
    }

    /**
     * Returns a bit from the end of the bitstream and update the index.
     *
     * @param int $bit Index of the bit to pop.
     */
    protected function popBit(int &$bit): int
    {
        return ($this->bitstream[$bit--] ?? 0) === 0 ? 0 : 1;
    }

    /**
     * Returns 1 if the current position must be skipped in Full mode.
     *
     * @param int $pos Position in the grid.
     */
    protected function skipModeRefGrid(int $pos): int
    {
        return (int) (!$this->compact && $pos === 5);
    }

    /**
     * Returns the offset for the specified position to skip the reference grid.
     *
     * @param int $pos Position in the grid.
     */
    protected function skipRefGrid(int $pos): int
    {
        return (int) (!$this->compact && ($pos % 16) === 0);
    }

    /**
     * Draw the data bitstream in the grid in Full mode.
     */
    protected function drawData(): void
    {
        $center = $this->gridcenter;
        $llen = 16; // width of the first layer side
        $srow = -8; // start top row offset from the center (LSB)
        $scol = -7; // start top column offset from the center (LSB)
        if ($this->compact) {
            $llen = 13;
            $srow = -6;
            $scol = -5;
        }

        $skip = 0; // skip reference grid while drwaing dominoes
        $bit = $this->totbits - 1; // index of last bitstream bit (first to draw)
        for ($layer = 0; $layer < $this->numlayers; ++$layer) {
            // top
            $ypos = $center + $srow;
            $xpos = $center + $scol;
            for ($pos = 0; $pos < $llen; ++$pos) {
                $xpos += $this->skipRefGrid($xpos - $center); // skip reference grid
                $this->grid[$ypos][$xpos] = $this->popBit($bit);
                $this->grid[$ypos - 1 - $skip][$xpos] = $this->popBit($bit);
                ++$xpos;
            }

            // right
            ++$ypos;
            $xpos -= 2 + $skip;
            for ($pos = 0; $pos < $llen; ++$pos) {
                $ypos += $this->skipRefGrid($ypos - $center); // skip reference grid
                $this->grid[$ypos][$xpos] = $this->popBit($bit);
                $this->grid[$ypos][$xpos + 1 + $skip] = $this->popBit($bit);
                ++$ypos;
            }

            // bottom
            $ypos -= 2 + $skip;
            --$xpos;
            for ($pos = 0; $pos < $llen; ++$pos) {
                $xpos -= $this->skipRefGrid($xpos - $center); // skip reference grid
                $this->grid[$ypos][$xpos] = $this->popBit($bit);
                $this->grid[$ypos + 1 + $skip][$xpos] = $this->popBit($bit);
                --$xpos;
            }

            // left
            --$ypos;
            $xpos += 2 + $skip;
            for ($pos = 0; $pos < $llen; ++$pos) {
                $ypos -= $this->skipRefGrid($ypos - $center); // skip reference grid
                $this->grid[$ypos][$xpos] = $this->popBit($bit);
                $this->grid[$ypos][$xpos - 1 - $skip] = $this->popBit($bit);
                --$ypos;
            }

            $llen += 4;
            $srow = $ypos - $center;
            $srow -= $this->skipRefGrid($srow);
            $scol = $xpos - 1 - $center;
            $scol -= $this->skipRefGrid($scol);
            $skip = $this->skipRefGrid($srow - 1);
        }
    }
}
