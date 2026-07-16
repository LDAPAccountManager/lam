<?php

declare(strict_types=1);

/**
 * Layers.php
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
 * Com\Tecnick\Barcode\Type\Square\Aztec\Layers
 *
 * Layers for Aztec Barcode type class
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Layers extends \Com\Tecnick\Barcode\Type\Square\Aztec\Codeword
{
    /**
     * @param array<int, array{0: int, 1: int, 2: int, 3: int, 4: int, 5: int, 6: int}> $data
     */
    protected function getLayerMaxBits(array $data): int
    {
        if ($data === []) {
            return 0;
        }

        $last = \array_values($data)[\count($data) - 1] ?? [0, 0, 0, 0, 0, 0, 0];
        return $last[3] ?? 0;
    }

    /**
     * True for compact mode (up to 4 layers), false for full-range mode (up to 32 layers).
     */
    protected bool $compact = true;

    /**
     * Number of data layers.
     */
    protected int $numlayers = 0;

    /**
     * Size data for the selected layer.
     *
     * @var array{int, int, int, int, int, int, int}
     */
    protected array $layer = [0, 0, 0, 0, 0, 0, 0];

    /**
     * Returns the minimum number of layers required.
     *
     * @param array<int, array{int, int, int, int, int, int, int}> $data
     *        Either the Data::SIZE_COMPACT or Data::SIZE_FULL array.
     * @param int   $numbits The number of bits to encode.
     */
    protected function getMinLayers(array $data, int $numbits): int
    {
        if ($numbits <= $this->getLayerMaxBits($data)) {
            foreach ($data as $numlayers => $size) {
                if ($numbits <= $size[3]) {
                    return $numlayers;
                }
            }
        }

        return 0;
    }

    /**
     * Select the layer by the number of bits to encode.
     *
     * @param int    $numbits The number of bits to encode.
     * @param string $mode    The mode to use (A = Automatic; F = Full Range mode).
     *
     * @return bool Returns true if the size computation was successful, false otherwise.
     */
    protected function setLayerByBits(int $numbits, string $mode = 'A'): bool
    {
        $this->numlayers = 0;
        if ($mode === 'A') {
            $this->compact = true;
            $this->numlayers = $this->getMinLayers(Data::SIZE_COMPACT, $numbits);
        }

        if ($this->numlayers === 0) {
            $this->compact = false;
            $this->numlayers = $this->getMinLayers(Data::SIZE_FULL, $numbits);
        }

        if ($this->numlayers === 0) {
            return false;
        }

        if ($this->compact) {
            $compactLayer = Data::SIZE_COMPACT[$this->numlayers] ?? null;
            if ($compactLayer === null) {
                return false;
            }
            $this->layer = $compactLayer;
            return true;
        }

        $fullLayer = Data::SIZE_FULL[$this->numlayers] ?? null;
        if ($fullLayer === null) {
            return false;
        }
        $this->layer = $fullLayer;
        return true;
    }

    /**
     * Computes the type and number of required layers and performs bit stuffing
     *
     * @param int    $ecc  The error correction level.
     * @param string $mode The mode to use (A = Automatic; F = Full Range mode).
     *
     * @return bool Returns true if the size computation was successful, false otherwise.
     */
    protected function sizeAndBitStuffing(int $ecc, string $mode = 'A'): bool
    {
        $nsbits = 0;
        $eccbits = 11 + (int) (($this->totbits * $ecc) / 100);
        do {
            if (!$this->setLayerByBits($this->totbits + $nsbits + $eccbits, $mode)) {
                return false;
            }

            $nsbits = $this->bitStuffing();
        } while (($nsbits + $eccbits) > $this->layer[3]);

        $this->bitstream = [];
        $this->totbits = 0;
        $this->mergeTmpCwdRaw();
        return true;
    }

    /**
     * Bit-stuffing the bitstream into Reed–Solomon codewords.
     * The resulting codewords are stored in the temporary tmpCdws array.
     *
     * @return int The number of bits in the bitstream after bit stuffing.
     */
    protected function bitStuffing(): int
    {
        $nsbits = 0;
        $wsize = $this->layer[2];
        $mask = (1 << $wsize) - 2; // b-1 bits at 1 and last bit at 0
        $this->tmpCdws = [];
        for ($wid = 0; $wid < $this->totbits; $wid += $wsize) {
            $word = 0;
            for ($idx = 0; $idx < $wsize; ++$idx) {
                $bid = $wid + $idx;
                if ($this->getBitstreamBit($this->bitstream, $bid) === 1) {
                    $word |= 1 << ($wsize - 1 - $idx); // the first bit is MSB
                }
            }

            // If the first b−1 bits of a code word have the same value,
            // an extra bit with the complementary value is inserted into the data stream.
            $maskedWord = $word & $mask;
            [$word, $wid] = match (true) {
                $maskedWord === $mask => [$word & $mask, $wid - 1],
                $maskedWord === 0 => [$word | 1, $wid - 1],
                default => [$word, $wid],
            };

            $this->tmpCdws[] = [$wsize, $word];
            $nsbits += $wsize;
        }

        return $nsbits;
    }
}
