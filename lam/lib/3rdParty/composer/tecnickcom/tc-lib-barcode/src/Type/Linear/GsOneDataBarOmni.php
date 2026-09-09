<?php

declare(strict_types=1);

/**
 * GsOneDataBarOmni.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBarOmni;
 *
 * GsOneDataBarOmni Barcode type class
 * GS1 DataBar Omnidirectional (ISO/IEC 24724)
 *
 * Symbol of 46 elements and 96 modules encoding the element string of the
 * Global Trade Item Number, Application Identifier (01). The input is the plain
 * digit string: a code of the full length must carry a valid check digit, a
 * shorter one is left-padded with zeros and the check digit is appended.
 *
 * The value of the symbol is the linkage flag followed by the 13 data digits of
 * the key. It is split into a left and a right data character pair, and each
 * pair into an outside (16,4) and an inside (15,4) data character. The two
 * finder patterns between them carry the modulo 79 checksum of the data
 * character elements.
 *
 * GS1 and GS1 DataBar are registered trademarks of GS1 AISBL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class GsOneDataBarOmni extends \Com\Tecnick\Barcode\Type\Linear\GsOneDataBar
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATABAR';

    /**
     * Number of values of an inside (15,4) data character
     */
    protected const INSIDE_VALUES = 1_597;

    /**
     * Number of values of a data character pair
     */
    protected const PAIR_VALUES = 4_537_077;

    /**
     * Checksum modulus, the number of usable finder pattern pairs
     */
    protected const CHECKSUM_MODULUS = 79;

    /**
     * Characteristics of the (16,4) outside data characters, one row per group:
     * sum of the previous groups, odd and even subset modules, odd and even
     * widest element, and odd and even subset total values.
     *
     * @var array<int, array{int, int, int, int, int, int, int}>
     */
    protected const OUTSIDE = [
        [0,     12, 4,  8, 1, 161, 1],
        [161,   10, 6,  6, 3, 80,  10],
        [961,   8,  8,  4, 5, 31,  34],
        [2_015, 6,  10, 3, 6, 10,  70],
        [2_715, 4,  12, 1, 8, 1,   126],
    ];

    /**
     * Characteristics of the (15,4) inside data characters, in the same order
     * as OUTSIDE.
     *
     * @var array<int, array{int, int, int, int, int, int, int}>
     */
    protected const INSIDE = [
        [0,     5,  10, 2, 7, 4,  84],
        [336,   7,  8,  4, 5, 20, 35],
        [1_036, 9,  6,  6, 3, 48, 10],
        [1_516, 11, 4,  8, 1, 81, 1],
    ];

    /**
     * Element widths of the nine finder patterns, numbered from the outside of
     * the symbol to its centre.
     *
     * @var array<int, array{int, int, int, int, int}>
     */
    protected const FINDER = [
        [3, 8, 2, 1, 1],
        [3, 5, 5, 1, 1],
        [3, 3, 7, 1, 1],
        [3, 1, 9, 1, 1],
        [2, 7, 4, 1, 1],
        [2, 5, 6, 1, 1],
        [2, 3, 8, 1, 1],
        [1, 5, 7, 1, 1],
        [1, 3, 9, 1, 1],
    ];

    /**
     * Symbol height in modules
     */
    protected const SYMBOL_HEIGHT = 33;

    /**
     * Value of the left finder pattern
     */
    protected int $finder_left = 0;

    /**
     * Value of the right finder pattern
     */
    protected int $finder_right = 0;

    /**
     * Get the group characteristics that contain the given data character value.
     *
     * @param array<int, array{int, int, int, int, int, int, int}> $groups Group table
     *
     * @return array{int, int, int, int, int, int, int}
     *
     * @throws BarcodeException if no group contains the value
     */
    protected function getCharacterGroup(int $value, array $groups): array
    {
        foreach (\array_reverse($groups) as $group) {
            if ($value >= $group[0]) {
                return $group;
            }
        }

        throw new BarcodeException('The data character value is out of range: ' . $value);
    }

    /**
     * Get the elements of an outside (16,4) data character, ordered from the
     * element farthest from the adjacent finder pattern.
     * The odd elements need no element of one module, the even ones do.
     *
     * @return array<int, int> Element widths in modules
     *
     * @throws BarcodeException if the value cannot be encoded
     */
    protected function getOutsideCharacter(int $value): array
    {
        [$gsum, $mododd, $modeven, $maxodd, $maxeven, , $teven] = $this->getCharacterGroup($value, $this::OUTSIDE);
        $odd = $this->getSubsetWidths(\intdiv($value - $gsum, $teven), $mododd, 4, $maxodd, false);
        $even = $this->getSubsetWidths(($value - $gsum) % $teven, $modeven, 4, $maxeven, true);
        return $this->mergeSubsets($odd, $even);
    }

    /**
     * Get the elements of an inside (15,4) data character, ordered from the
     * element farthest from the adjacent finder pattern.
     * The odd elements need an element of one module, the even ones do not.
     *
     * @return array<int, int> Element widths in modules
     *
     * @throws BarcodeException if the value cannot be encoded
     */
    protected function getInsideCharacter(int $value): array
    {
        [$gsum, $mododd, $modeven, $maxodd, $maxeven, $todd] = $this->getCharacterGroup($value, $this::INSIDE);
        $odd = $this->getSubsetWidths(($value - $gsum) % $todd, $mododd, 4, $maxodd, true);
        $even = $this->getSubsetWidths(\intdiv($value - $gsum, $todd), $modeven, 4, $maxeven, false);
        return $this->mergeSubsets($odd, $even);
    }

    /**
     * Get the elements of the four data characters, each ordered from the
     * element farthest from the adjacent finder pattern.
     *
     * @return array<int, array<int, int>> Element widths in modules, indexed by data character
     *
     * @throws BarcodeException if the value cannot be encoded
     */
    protected function getDataCharacters(): array
    {
        $value = $this->getSymbolValue();
        $left = $this->divNumeric($value, (string) $this::PAIR_VALUES);
        $right = (string) $this->modNumeric($value, (string) $this::PAIR_VALUES);
        $inside = (string) $this::INSIDE_VALUES;

        return [
            $this->getOutsideCharacter((int) $this->divNumeric($left, $inside)),
            $this->getInsideCharacter($this->modNumeric($left, $inside)),
            $this->getOutsideCharacter((int) $this->divNumeric($right, $inside)),
            $this->getInsideCharacter($this->modNumeric($right, $inside)),
        ];
    }

    /**
     * Get the values of the left and right finder patterns.
     * The checksum is spread over the 79 usable pairs, skipping the pairs 0,8
     * and 8,0 that a single edge error would turn into each other.
     *
     * @param array<int, array<int, int>> $chars Element widths of the four data characters
     *
     * @return array{int, int}
     */
    protected function getFinderValues(array $chars): array
    {
        $elements = \array_merge($chars[0] ?? [], $chars[1] ?? [], $chars[2] ?? [], $chars[3] ?? []);
        $temp = $this->getWeightedChecksum($elements, $this::CHECKSUM_MODULUS);
        if ($temp >= 8) {
            ++$temp;
        }

        if ($temp >= 72) {
            ++$temp;
        }

        return [\intdiv($temp, 9), $temp % 9];
    }

    /**
     * Get the element widths of a finder pattern.
     *
     * @return array<int, int>
     */
    protected function getFinderPattern(int $value): array
    {
        return $this::FINDER[$value] ?? [];
    }

    /**
     * Get the element widths of the two halves of the symbol.
     *
     * The left half, from left to right, is the left guard pattern, the first
     * data character, the left finder pattern and the second data character, and
     * its first element is a space. The right half is the fourth data character,
     * the right finder pattern, the third data character and the right guard
     * pattern, and its first element is a bar.
     *
     * @return array{array<int, int>, array<int, int>}
     *
     * @throws BarcodeException if the value cannot be encoded
     */
    protected function getHalves(): array
    {
        $chars = $this->getDataCharacters();
        [$this->finder_left, $this->finder_right] = $this->getFinderValues($chars);

        return [
            \array_merge(
                [1, 1],
                $chars[0] ?? [],
                $this->getFinderPattern($this->finder_left),
                \array_reverse($chars[1] ?? []),
            ),
            \array_merge(
                $chars[3] ?? [],
                \array_reverse($this->getFinderPattern($this->finder_right)),
                \array_reverse($chars[2] ?? []),
                [1, 1],
            ),
        ];
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->formatCode();
        [$left, $right] = $this->getHalves();
        $elements = \array_merge($left, $right);
        $this->ncols = \array_sum($elements);
        $this->nrows = $this::SYMBOL_HEIGHT;
        $this->bars = [];
        $this->addElementBars($elements, 0, 0, $this->nrows, true);
    }
}
