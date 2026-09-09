<?php

declare(strict_types=1);

/**
 * GsOneDataBarLimited.php
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
use Com\Tecnick\Barcode\Type\Linear\GsOneDataBar\Data;

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBarLimited;
 *
 * GsOneDataBarLimited Barcode type class
 * GS1 DataBar Limited (ISO/IEC 24724)
 *
 * Symbol of 47 elements and 79 modules encoding the element string of the
 * Global Trade Item Number, Application Identifier (01), for small items that
 * are not read by omnidirectional point of sale scanners. Only the Indicator
 * digits 0 and 1 are encodable.
 *
 * The value of the symbol is the 13 data digits of the key, offset by a fixed
 * amount when the linkage flag is set. It is split into a left and a right
 * (26,7) data character, and the (18,7) check character between them carries
 * the modulo 89 checksum of the data character elements.
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
class GsOneDataBarLimited extends \Com\Tecnick\Barcode\Type\Linear\GsOneDataBar
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATABARLIMITED';

    /**
     * Number of values of a (26,7) data character
     */
    protected const CHARACTER_VALUES = 2_013_571;

    /**
     * Offset added to the symbol value when the linkage flag is set.
     * The two ranges are chosen so that the presence of a 2D Composite Component
     * can be told from the modules of the left data character alone.
     *
     * @var numeric-string
     */
    protected const LINKAGE_OFFSET = '2015133531096';

    /**
     * Checksum modulus
     */
    protected const CHECKSUM_MODULUS = 89;

    /**
     * Symbol height in modules
     */
    protected const SYMBOL_HEIGHT = 10;

    /**
     * Highest Indicator digit that GS1 DataBar Limited can encode
     */
    protected const MAX_INDICATOR = 1;

    /**
     * Characteristics of the (26,7) data characters, one row per group: sum of
     * the previous groups, odd and even subset modules, odd and even widest
     * element, and odd and even subset total values.
     *
     * @var array<int, array{int, int, int, int, int, int, int}>
     */
    protected const CHARACTER = [
        [0,         17, 9,  6, 3, 6_538,  28],
        [183_064,   13, 13, 5, 4, 875,    728],
        [820_064,   9,  17, 3, 6, 28,     6_454],
        [1_000_776, 15, 11, 5, 4, 2_415,  203],
        [1_491_021, 11, 15, 4, 5, 203,    2_408],
        [1_979_845, 19, 7,  8, 1, 17_094, 1],
        [1_996_939, 7,  19, 1, 8, 1,      16_632],
    ];

    /**
     * Check the input code and reject the Indicator digits that the symbology
     * cannot encode.
     *
     * @throws BarcodeException if the code cannot be encoded
     */
    protected function formatCode(): void
    {
        parent::formatCode();

        if ((int) $this->key[0] > $this::MAX_INDICATOR) {
            throw new BarcodeException(
                'The Indicator digit of '
                . $this::FORMAT
                . ' must not be greater than '
                . $this::MAX_INDICATOR
                . ': '
                . $this->key[0],
            );
        }
    }

    /**
     * Get the value encoded by the symbol: the key without its check digit,
     * offset when the linkage flag is set.
     *
     * @return numeric-string
     */
    protected function getSymbolValue(): string
    {
        /** @var numeric-string $value */
        $value = \substr($this->key, 0, $this::CODE_LENGTH - 1);
        return $this->linkage === 1 ? $this->addNumeric($value, $this::LINKAGE_OFFSET) : $value;
    }

    /**
     * Get the elements of a (26,7) data character, ordered from left to right.
     * The odd elements are the spaces and need no element of one module, the
     * even ones are the bars and do.
     *
     * @return array<int, int> Element widths in modules
     *
     * @throws BarcodeException if the value cannot be encoded
     */
    protected function getDataCharacter(int $value): array
    {
        foreach (\array_reverse($this::CHARACTER) as $group) {
            [$gsum, $mododd, $modeven, $maxodd, $maxeven, , $teven] = $group;
            if ($value < $gsum) {
                continue;
            }

            $odd = $this->getSubsetWidths(\intdiv($value - $gsum, $teven), $mododd, 7, $maxodd, false);
            $even = $this->getSubsetWidths(($value - $gsum) % $teven, $modeven, 7, $maxeven, true);
            return $this->mergeSubsets($odd, $even);
        }

        throw new BarcodeException('The data character value is out of range: ' . $value);
    }

    /**
     * Get the element widths of the check character.
     *
     * @param array<int, int> $elements Element widths of the two data characters
     *
     * @return array<int, int>
     */
    protected function getCheckCharacter(array $elements): array
    {
        $value = $this->getWeightedChecksum($elements, $this::CHECKSUM_MODULUS);
        return Data::LIMITED_CHECK[$value] ?? [];
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->formatCode();
        $value = $this->getSymbolValue();
        $divisor = (string) $this::CHARACTER_VALUES;
        $left = $this->getDataCharacter((int) $this->divNumeric($value, $divisor));
        $right = $this->getDataCharacter($this->modNumeric($value, $divisor));

        $elements = \array_merge(
            [1, 1],
            $left,
            $this->getCheckCharacter(\array_merge($left, $right)),
            $right,
            [1, 1, 5],
        );

        $this->ncols = \array_sum($elements);
        $this->nrows = $this::SYMBOL_HEIGHT;
        $this->bars = [];
        $this->addElementBars($elements, 0, 0, $this->nrows, true);
    }
}
