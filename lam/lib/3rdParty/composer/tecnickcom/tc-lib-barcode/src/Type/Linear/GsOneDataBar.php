<?php

declare(strict_types=1);

/**
 * GsOneDataBar.php
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

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBar;
 *
 * GsOneDataBar Barcode type class
 * Common structure of the GS1 DataBar family (ISO/IEC 24724)
 *
 * Every GS1 DataBar symbol is a continuous sequence of elements whose widths
 * come from (n,k) subsets: a symbol character of n modules is split into an odd
 * and an even subset of k elements each, and the value of the character selects
 * one combination of element widths from each subset. This class holds the
 * subset combination algorithm, the weighted checksum and the drawing helpers
 * shared by the seven variations.
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
abstract class GsOneDataBar extends \Com\Tecnick\Barcode\Type\Linear
{
    use FixedLengthCheckDigit;

    /**
     * Number of combinations of a subset, keyed by the remaining modules,
     * elements, widest element and pending narrow element requirement.
     *
     * @var array<string, int>
     */
    private array $subset_count = [];

    /**
     * Linkage flag: 1 when a 2D Composite Component adjoins the symbol
     */
    protected int $linkage = 0;

    /**
     * Set extra (optional) parameters.
     * The first parameter is the linkage flag.
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        if (($this->params[0] ?? null) !== null && (int) $this->params[0] === 1) {
            $this->linkage = 1;
        }
    }

    /**
     * Count the combinations of element widths of a subset.
     * The elements are ordered from the one farthest from the adjacent finder
     * pattern, and every element is at least one module wide.
     *
     * @param int  $modules  Modules left to distribute
     * @param int  $elements Elements left to fill
     * @param int  $widest   Widest element in modules
     * @param bool $narrow   True while the subset still needs an element of one module
     */
    protected function countSubset(int $modules, int $elements, int $widest, bool $narrow): int
    {
        if ($elements === 0) {
            return $modules === 0 && !$narrow ? 1 : 0;
        }

        $key = $modules . ':' . $elements . ':' . $widest . ':' . ($narrow ? '1' : '0');
        if (isset($this->subset_count[$key])) {
            return $this->subset_count[$key];
        }

        $count = 0;
        $limit = \min($widest, $modules - $elements + 1);
        for ($width = 1; $width <= $limit; ++$width) {
            $count += $this->countSubset($modules - $width, $elements - 1, $widest, $narrow && $width !== 1);
        }

        return $this->subset_count[$key] = $count;
    }

    /**
     * Get the element widths of a subset.
     * The combinations are numbered in ascending order of the width sequence, so
     * the value 0 is the combination with the narrowest leading elements.
     *
     * @param int  $value    Subset value
     * @param int  $modules  Total modules of the subset
     * @param int  $elements Number of elements of the subset
     * @param int  $widest   Widest element in modules
     * @param bool $narrow   True when the subset must contain an element of one module
     *
     * @return array<int, int> Element widths in modules
     *
     * @throws BarcodeException if the value is outside the range of the subset
     */
    protected function getSubsetWidths(int $value, int $modules, int $elements, int $widest, bool $narrow): array
    {
        $widths = [];
        $left = $value;
        for ($pos = $elements; $pos > 0; --$pos) {
            $found = 0;
            $limit = \min($widest, $modules - $pos + 1);
            for ($width = 1; $width <= $limit; ++$width) {
                $count = $this->countSubset($modules - $width, $pos - 1, $widest, $narrow && $width !== 1);
                if ($left < $count) {
                    $found = $width;
                    break;
                }

                $left -= $count;
            }

            if ($found === 0) {
                throw new BarcodeException('The subset value is out of range: ' . $value);
            }

            $widths[] = $found;
            $narrow = $narrow && $found !== 1;
            $modules -= $found;
        }

        return $widths;
    }

    /**
     * Interleave the odd and even subsets into the elements of a symbol character.
     * The first element of the character is the first odd element.
     *
     * @param array<int, int> $odd  Odd subset element widths
     * @param array<int, int> $even Even subset element widths
     *
     * @return array<int, int> Element widths in modules
     */
    protected function mergeSubsets(array $odd, array $even): array
    {
        $widths = [];
        foreach ($odd as $pos => $width) {
            $widths[] = $width;
            $widths[] = $even[$pos] ?? 0;
        }

        return $widths;
    }

    /**
     * Calculate the weighted checksum of the symbol character elements.
     * The weights are the ascending powers of 3 modulo the given modulus.
     *
     * @param array<int, int> $elements Element widths of the symbol characters, in character order
     * @param int             $modulus  Checksum modulus
     */
    protected function getWeightedChecksum(array $elements, int $modulus): int
    {
        $sum = 0;
        $weight = 1;
        foreach ($elements as $width) {
            $sum += $width * $weight;
            $weight = ($weight * 3) % $modulus;
        }

        return $sum % $modulus;
    }

    /**
     * Calculate the GS1 modulo 10 check digit of a numeric string.
     *
     * @param string $code Data digits without the check digit
     */
    protected function getCheckDigit(string $code): int
    {
        return (new GsOneElementString())->getCheckDigit($code);
    }

    /**
     * Fixed code length of the encoded key, check digit included
     */
    protected const CODE_LENGTH = 14;

    /**
     * Application Identifier of the encoded element string
     *
     * @var string
     */
    protected const APPID = '01';

    /**
     * Encoded key, check digit included
     */
    protected string $key = '';

    /**
     * Check the input code and expand it to the full length key.
     * A code of the full length must carry a valid check digit, a shorter one is
     * left-padded with zeros and the check digit is appended.
     * The human readable interpretation is the bracketed element string.
     *
     * @throws BarcodeException if the code cannot be encoded
     */
    protected function formatCode(): void
    {
        if (!\ctype_digit($this->code)) {
            throw new BarcodeException('Input code must be a number');
        }

        if (\strlen($this->code) > $this::CODE_LENGTH) {
            throw new BarcodeException(
                'The code is too long: '
                . \strlen($this->code)
                . ' digits (maximum '
                . $this::CODE_LENGTH
                . ' for '
                . $this::FORMAT
                . ')',
            );
        }

        $this->key = $this->getCheckedCode($this->code, $this::CODE_LENGTH);
        $this->extcode = '(' . $this::APPID . ')' . $this->key;
    }

    /**
     * Get the value encoded by the symbol: the linkage flag followed by the key
     * without its check digit.
     *
     * @return numeric-string
     */
    protected function getSymbolValue(): string
    {
        /** @var numeric-string */
        return $this->linkage . \substr($this->key, 0, $this::CODE_LENGTH - 1);
    }

    /**
     * Add the bars of a sequence of elements.
     *
     * @param array<int, int> $elements Element widths in modules
     * @param int             $posx     Horizontal offset in modules of the first element
     * @param int             $posy     Vertical offset in modules
     * @param int             $height   Height in modules
     * @param bool            $space    True when the first element is a space
     */
    protected function addElementBars(array $elements, int $posx, int $posy, int $height, bool $space): void
    {
        foreach ($elements as $pos => $width) {
            if ((($pos % 2) === 0) !== $space) {
                $this->bars[] = [$posx, $posy, $width, $height];
            }

            $posx += $width;
        }
    }

    /**
     * Expand a sequence of elements into one value per module.
     *
     * @param array<int, int> $elements Element widths in modules
     * @param bool            $space    True when the first element is a space
     *
     * @return array<int, int> One entry per module, 1 for a bar and 0 for a space
     */
    protected function getElementModules(array $elements, bool $space): array
    {
        $modules = [];
        foreach ($elements as $pos => $width) {
            $bar = (($pos % 2) === 0) !== $space ? 1 : 0;
            $modules = \array_merge($modules, \array_fill(0, \max(0, $width), $bar));
        }

        return $modules;
    }

    /**
     * Add the bars of a row of modules.
     *
     * @param array<int, int> $modules One entry per module, 1 for a bar and 0 for a space
     * @param int             $posy    Vertical offset in modules
     * @param int             $height  Height in modules
     */
    protected function addModuleBars(array $modules, int $posy, int $height): void
    {
        $start = -1;
        $count = \count($modules);
        for ($posx = 0; $posx <= $count; ++$posx) {
            if (($modules[$posx] ?? 0) === 1) {
                if ($start < 0) {
                    $start = $posx;
                }

                continue;
            }

            if ($start >= 0) {
                $this->bars[] = [$start, $posy, $posx - $start, $height];
                $start = -1;
            }
        }
    }
}
