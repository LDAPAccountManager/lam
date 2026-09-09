<?php

declare(strict_types=1);

/**
 * GsOneDataBarExpanded.php
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
use Com\Tecnick\Barcode\Type\GsOneElementString;
use Com\Tecnick\Barcode\Type\Linear\GsOneDataBar\Compaction;
use Com\Tecnick\Barcode\Type\Linear\GsOneDataBar\Data;
use Com\Tecnick\Barcode\Type\Linear\GsOneDataBar\Encodation;

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBarExpanded;
 *
 * GsOneDataBarExpanded Barcode type class
 * GS1 DataBar Expanded (ISO/IEC 24724)
 *
 * Variable length symbol of 4 to 22 symbol characters carrying any sequence of
 * GS1 Application Identifier element strings. The input is the bracketed form
 * "(ai)value(ai)value...", which is also the human readable interpretation.
 *
 * The data is turned into a binary string of 12 bits per data character: the
 * linkage flag, the encodation method, the two bits that repeat the symbol
 * length, the compressed data field of the method and the general purpose data
 * compaction field. The binary string is cut into (17,4) symbol characters, and
 * the first symbol character is a check character carrying the symbol length
 * and the modulo 211 checksum of the data character elements.
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
class GsOneDataBarExpanded extends \Com\Tecnick\Barcode\Type\Linear\GsOneDataBar
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATABAREXP';

    /**
     * Checksum modulus
     */
    protected const CHECKSUM_MODULUS = 211;

    /**
     * Symbol height in modules
     */
    protected const SYMBOL_HEIGHT = 34;

    /**
     * Smallest number of symbol characters
     */
    protected const MIN_CHARACTERS = 4;

    /**
     * Largest number of symbol characters
     */
    protected const MAX_CHARACTERS = 22;

    /**
     * Number of symbol characters above which the group size bit is set
     */
    protected const GROUP_SIZE = 14;

    /**
     * Characteristics of the (17,4) symbol characters, one row per group: sum of
     * the previous groups, odd and even subset modules, odd and even widest
     * element, and odd and even subset total values.
     *
     * @var array<int, array{int, int, int, int, int, int, int}>
     */
    protected const CHARACTER = [
        [0,     12, 5,  7, 2, 87, 4],
        [348,   10, 7,  5, 4, 52, 20],
        [1_388, 8,  9,  4, 5, 30, 52],
        [2_948, 6,  11, 3, 6, 10, 104],
        [3_988, 4,  13, 1, 8, 1,  204],
    ];

    /**
     * Get the bracketed source of the element strings.
     */
    protected function getBracketedCode(): string
    {
        return $this->code;
    }

    /**
     * Parse the element strings, build the human readable interpretation and the
     * binary string that precedes the general purpose data compaction field.
     *
     * @return array{string, string, int, int, int} Binary string, characters left to the
     *                                              general purpose data compaction, fixed
     *                                              number of symbol characters or zero,
     *                                              smallest number of symbol characters, and
     *                                              offset of the two bits that repeat the
     *                                              symbol length, or -1 when they are absent
     *
     * @throws BarcodeException if the element strings cannot be encoded
     */
    protected function getPrefix(): array
    {
        $parser = new GsOneElementString();
        $elements = $parser->parse($this->getBracketedCode());
        $this->extcode = $parser->getHumanReadable($elements);

        [$method, $compressed, $data, $fixed, $minimum] = (new Encodation())->getEncodation($elements);
        // the two bits that repeat the symbol length are only present in the
        // methods that admit a variable number of symbol characters, and they
        // follow the encodation method, ahead of the compressed data field
        $variable = $fixed === 0 ? '00' : '';
        $offset = $fixed === 0 ? \strlen($this->linkage . $method) : -1;

        return [$this->linkage . $method . $variable . $compressed, $data, $fixed, $minimum, $offset];
    }

    /**
     * Get the number of symbol characters that holds the given binary string.
     *
     * @param int $bits    Length of the binary string
     * @param int $fixed   Fixed number of symbol characters, or zero
     * @param int $minimum Smallest number of symbol characters
     *
     * @throws BarcodeException if the data does not fit a symbol
     */
    protected function getCharacterCount(int $bits, int $fixed, int $minimum): int
    {
        $chars = $fixed > 0 ? $fixed : \max($minimum, (int) \ceil($bits / Compaction::CHARACTER_BITS) + 1);

        if ($chars > $this::MAX_CHARACTERS) {
            throw new BarcodeException(
                'The data is too long: '
                . $bits
                . ' bits (maximum '
                . (Compaction::CHARACTER_BITS * ($this::MAX_CHARACTERS - 1))
                . ' for '
                . $this::FORMAT
                . ')',
            );
        }

        return $chars;
    }

    /**
     * Get the number of symbol characters that holds the given binary string,
     * as the general purpose data compaction counts them while it runs.
     *
     * @param int $bits  Length of the binary string
     * @param int $least Smallest number of symbol characters
     *
     * @throws BarcodeException if the data does not fit a symbol
     */
    protected function getCompactionCharacterCount(int $bits, int $least): int
    {
        return $this->getCharacterCount($bits, 0, $least);
    }

    /**
     * Get the values of the data characters.
     *
     * @return array<int, int>
     *
     * @throws BarcodeException if the data cannot be encoded
     */
    protected function getDataValues(): array
    {
        [$bits, $data, $fixed, $minimum, $offset] = $this->getPrefix();
        $compaction = new Compaction();
        // the compaction sizes the trailing digit against the symbol it ends,
        // which is the one this type counts and not always the smallest that fits
        $compaction->setCharacterCount($this->getCompactionCharacterCount(...));
        $mode = Compaction::NUMERIC;
        $prefix = \strlen($bits);
        if ($fixed === 0) {
            $bits .= $compaction->encode($data, $prefix, $minimum, $mode);
        }

        $chars = $this->getCharacterCount(\strlen($bits), $fixed, $minimum);
        if ($offset >= 0) {
            // the two bits after the encodation method repeat the symbol length
            $bits[$offset] = (string) ($chars % 2);
            $bits[$offset + 1] = $chars > $this::GROUP_SIZE ? '1' : '0';
        }

        $capacity = Compaction::CHARACTER_BITS * ($chars - 1);
        $bits .= $compaction->getPadding($capacity - \strlen($bits), $mode);

        $values = [];
        for ($pos = 0; $pos < $capacity; $pos += Compaction::CHARACTER_BITS) {
            $values[] = (int) \bindec(\substr($bits, $pos, Compaction::CHARACTER_BITS));
        }

        return $values;
    }

    /**
     * Get the elements of a (17,4) symbol character, ordered from the element
     * farthest from the adjacent finder pattern.
     * The odd elements need an element of one module, the even ones do not.
     *
     * @return array<int, int> Element widths in modules
     *
     * @throws BarcodeException if the value cannot be encoded
     */
    protected function getSymbolCharacter(int $value): array
    {
        foreach (\array_reverse($this::CHARACTER) as $group) {
            [$gsum, $mododd, $modeven, $maxodd, $maxeven, , $teven] = $group;
            if ($value < $gsum) {
                continue;
            }

            $odd = $this->getSubsetWidths(\intdiv($value - $gsum, $teven), $mododd, 4, $maxodd, true);
            $even = $this->getSubsetWidths(($value - $gsum) % $teven, $modeven, 4, $maxeven, false);
            return $this->mergeSubsets($odd, $even);
        }

        throw new BarcodeException('The symbol character value is out of range: ' . $value);
    }

    /**
     * Get the value of the check character: the symbol length and the modulo 211
     * checksum of the data character elements.
     *
     * The weight of an element is a power of 3 modulo 211 selected by the finder
     * pattern the data character is adjacent to and by the side it is on, in the
     * canonical finder pattern order. The check character itself is on the left
     * of the first finder pattern and carries no weight.
     *
     * @param array<int, array<int, int>> $chars   Element widths of the data characters
     * @param array<int, int>             $finders Sequence of finder patterns of the symbol
     */
    protected function getCheckValue(array $chars, array $finders): int
    {
        $sum = 0;
        foreach ($chars as $pos => $elements) {
            // the data characters follow the check character, so the first one
            // is the second symbol character
            $index = $pos + 2;
            $finder = $finders[(int) \ceil($index / 2) - 1] ?? 0;
            $row = (2 * $finder) + (($index % 2) === 0 ? 1 : 0);
            $weight = 1;
            for ($step = 8 * ($row - 1); $step > 0; --$step) {
                $weight = ($weight * 3) % $this::CHECKSUM_MODULUS;
            }

            foreach ($elements as $width) {
                $sum += $width * $weight;
                $weight = ($weight * 3) % $this::CHECKSUM_MODULUS;
            }
        }

        return (
            ($this::CHECKSUM_MODULUS * (\count($chars) + 1 - $this::MIN_CHARACTERS)) + ($sum % $this::CHECKSUM_MODULUS)
        );
    }

    /**
     * Get the element widths of every symbol character, the check character
     * first, each ordered from the element farthest from the adjacent finder.
     *
     * @return array<int, array<int, int>>
     *
     * @throws BarcodeException if the data cannot be encoded
     */
    protected function getCharacters(): array
    {
        $chars = [];
        foreach ($this->getDataValues() as $value) {
            $chars[] = $this->getSymbolCharacter($value);
        }

        $finders = Data::EXPANDED_SEQUENCE[\count($chars) + 1] ?? [];
        \array_unshift($chars, $this->getSymbolCharacter($this->getCheckValue($chars, $finders)));
        return $chars;
    }

    /**
     * Get the element widths of a row of the symbol, from left to right.
     * The symbol characters are laid out in triplets of a finder pattern between
     * two symbol characters, and the elements of every second symbol character
     * are mirrored so that they are ordered towards the adjacent finder pattern.
     *
     * @param array<int, array<int, int>> $chars   Element widths of the symbol characters of the row
     * @param array<int, int>             $finders Finder patterns of the row
     *
     * @return array<int, int>
     */
    protected function getRowElements(array $chars, array $finders): array
    {
        $elements = [1, 1];
        foreach ($finders as $pos => $finder) {
            $elements = \array_merge(
                $elements,
                $chars[2 * $pos] ?? [],
                Data::EXPANDED_FINDER[$finder] ?? [],
                \array_reverse($chars[(2 * $pos) + 1] ?? []),
            );
        }

        return \array_merge($elements, [1, 1]);
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $chars = $this->getCharacters();
        $elements = $this->getRowElements($chars, Data::EXPANDED_SEQUENCE[\count($chars)] ?? []);
        $this->ncols = \array_sum($elements);
        $this->nrows = $this::SYMBOL_HEIGHT;
        $this->bars = [];
        $this->addElementBars($elements, 0, 0, $this->nrows, true);
    }
}
