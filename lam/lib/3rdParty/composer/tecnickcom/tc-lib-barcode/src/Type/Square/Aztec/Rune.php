<?php

declare(strict_types=1);

/**
 * Rune.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Aztec;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\ReedSolomon;

/**
 * Com\Tecnick\Barcode\Type\Square\Aztec\Rune
 *
 * Encode for the Aztec Rune Barcode type class
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Rune
{
    /**
     * Symbol size in modules.
     */
    protected const SIZE = 11;

    /**
     * Radius of the outermost ring, the one carrying the mode message.
     */
    protected const RING = 5;

    /**
     * Number of bits in a mode message word.
     */
    protected const WORD_BITS = 4;

    /**
     * Number of check words in the mode message.
     */
    protected const CHECK_WORDS = 5;

    /**
     * Value combined by exclusive-or with each mode message word so that a rune
     * is not decoded as a compact Aztec Code symbol.
     */
    protected const WORD_MASK = 0xA;

    /**
     * Number of mode message bits on each side of the symbol.
     */
    protected const SIDE_BITS = 7;

    /**
     * Bidimensional grid containing the encoded symbol.
     *
     * @var array<int, array<int>>
     */
    protected array $grid = [];

    /**
     * Aztec Rune encoder.
     *
     * @param int $value The value to encode, from 0 to 255.
     *
     * @throws BarcodeException
     */
    public function __construct(int $value)
    {
        if ($value < 0 || $value > 255) {
            throw new BarcodeException('The value must be between 0 and 255');
        }

        $this->setGrid();
        $this->drawMode($value);
    }

    /**
     * Returns the bidimensional grid containing the encoded symbol.
     *
     * @return array<int, array<int>>
     */
    public function getGrid(): array
    {
        return $this->grid;
    }

    /**
     * Initialize the grid with the bullseye and the orientation patterns.
     */
    protected function setGrid(): void
    {
        $center = (int) ((self::SIZE - 1) / 2);
        $this->grid = [];
        for ($row = 0; $row < self::SIZE; ++$row) {
            for ($col = 0; $col < self::SIZE; ++$col) {
                // concentric rings of the bullseye: the even ones are dark
                $ring = \max(\abs($row - $center), \abs($col - $center));
                $this->grid[$row][$col] = (int) ($ring < self::RING && ($ring % 2) === 0);
            }
        }

        $last = self::SIZE - 1;
        $this->grid[0][0] = 1; // TL
        $this->grid[0][1] = 1; // TL-R
        $this->grid[1][0] = 1; // TL-B
        $this->grid[0][$last] = 1; // TR-T
        $this->grid[1][$last] = 1; // TR-B
        $this->grid[$last - 1][$last] = 1; // BR
    }

    /**
     * Add the mode message to the grid.
     *
     * @param int $value The value to encode, from 0 to 255.
     */
    protected function drawMode(int $value): void
    {
        $center = (int) ((self::SIZE - 1) / 2);
        $half = (int) ((self::SIDE_BITS - 1) / 2); // offset of the first bit of a side from the center

        // the mode message is drawn clockwise starting from the top left corner:
        // side 0 is the top, left to right, side 1 the right, top to bottom,
        // side 2 the bottom, right to left, and side 3 the left, bottom to top
        foreach ($this->modeBits($value) as $pos => $bit) {
            $side = (int) ($pos / self::SIDE_BITS);
            $idx = $pos % self::SIDE_BITS;
            $row = match ($side) {
                0 => $center - self::RING,
                1 => $center - $half + $idx,
                2 => $center + self::RING,
                default => $center + $half - $idx,
            };
            $col = match ($side) {
                0 => $center - $half + $idx,
                1 => $center + self::RING,
                2 => $center + $half - $idx,
                default => $center - self::RING,
            };
            $this->grid[$row][$col] = $bit;
        }
    }

    /**
     * Returns the bits of the mode message, in the order they are drawn.
     *
     * @param int $value The value to encode, from 0 to 255.
     *
     * @return list<int> Array of bits.
     */
    protected function modeBits(int $value): array
    {
        $words = [
            ($value >> self::WORD_BITS) & 0xF,
            $value & 0xF,
        ];
        $errorCorrection = new ReedSolomon(self::WORD_BITS);
        $words = \array_merge($words, $errorCorrection->checkwords($words, self::CHECK_WORDS));

        $bits = [];
        foreach ($words as $word) {
            $word ^= self::WORD_MASK;
            for ($idx = self::WORD_BITS - 1; $idx >= 0; --$idx) {
                $bits[] = ($word >> $idx) & 1;
            }
        }

        return $bits;
    }
}
