<?php

declare(strict_types=1);

/**
 * GsOneDataBarExpandedStacked.php
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
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBarExpandedStacked;
 *
 * GsOneDataBarExpandedStacked Barcode type class
 * GS1 DataBar Expanded Stacked (ISO/IEC 24724)
 *
 * Multi-row variation of GS1 DataBar Expanded of 2 to 11 rows, each 2 to 20
 * segments wide and 34 modules high, divided by separator patterns of three
 * module rows. The encoding is identical to GS1 DataBar Expanded; the symbol
 * characters are laid out over the rows in order, every row but the last one
 * carrying the same even number of them.
 *
 * The second row and the following even rows are mirrored when they hold a
 * number of segments that is a multiple of four, so that they start with a bar.
 * An incomplete last row with an odd number of finder patterns is moved one
 * module to the right, because its guard patterns would otherwise be symmetric.
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
class GsOneDataBarExpandedStacked extends \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarExpanded
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATABAREXPSTACK';

    /**
     * Height in modules of a row
     */
    protected const ROW_HEIGHT = 34;

    /**
     * Height in modules of the separator pattern between two rows
     */
    protected const SEPARATOR_HEIGHT = 3;

    /**
     * Number of modules at each end of a separator row that are always spaces
     */
    protected const SEPARATOR_MARGIN = 4;

    /**
     * Smallest and largest number of segments of a row
     */
    protected const MIN_SEGMENTS = 2;

    /**
     * Largest number of segments of a row
     */
    protected const MAX_SEGMENTS = 20;

    /**
     * Number of segments of a row when no parameter selects it
     */
    protected const DEFAULT_SEGMENTS = 4;

    /**
     * Number of segments of every row but the last one
     */
    protected int $segments = self::DEFAULT_SEGMENTS;

    /**
     * Set extra (optional) parameters.
     * The second parameter is the number of segments of a row, which must be
     * even.
     *
     * @throws BarcodeException if the number of segments is invalid
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        if (($this->params[1] ?? null) === null || $this->params[1] === '') {
            return;
        }

        $segments = (int) $this->params[1];
        if (($segments % 2) !== 0 || $segments < $this::MIN_SEGMENTS || $segments > $this::MAX_SEGMENTS) {
            throw new BarcodeException(
                'The number of segments of a row must be an even number between '
                . $this::MIN_SEGMENTS
                . ' and '
                . $this::MAX_SEGMENTS
                . ': '
                . $segments,
            );
        }

        $this->segments = $segments;
    }

    /**
     * Get the number of symbol characters that holds the given binary string.
     * One more symbol character is used when the last row would hold a single
     * one.
     *
     * @param int $bits    Length of the binary string
     * @param int $fixed   Fixed number of symbol characters, or zero
     * @param int $minimum Smallest number of symbol characters
     *
     * @throws BarcodeException if the data does not fit a symbol
     */
    protected function getCharacterCount(int $bits, int $fixed, int $minimum): int
    {
        $chars = parent::getCharacterCount($bits, $fixed, $minimum);
        if ($fixed === 0 && ($chars % $this->segments) === 1) {
            $chars = parent::getCharacterCount($bits, 0, $chars + 1);
        }

        return $chars;
    }

    /**
     * Get the modules of a row and the mask of the finder pattern regions that
     * the separator pattern treats differently.
     *
     * @param array<int, array<int, int>> $chars   Element widths of the symbol characters of the row
     * @param array<int, int>             $finders Finder patterns of the row
     * @param bool                        $mirror  True to reverse the row
     * @param bool                        $shift   True to move the row one module to the right
     *
     * @return array{array<int, int>, array<int, bool>}
     */
    protected function getRow(array $chars, array $finders, bool $mirror, bool $shift): array
    {
        $elements = $this->getRowElements($chars, $finders);
        $mask = \array_fill(0, \max(0, \count($elements)), false);
        // the guard pattern and the first symbol character precede the first
        // finder pattern, and every following finder pattern comes after a
        // symbol character, a finder pattern and another symbol character
        $offset = 2 + 8;
        foreach ($finders as $finder) {
            // the wide part of a finder pattern is its first three elements
            // when it starts with a space, and its last three when it starts
            // with a bar
            $start = $offset + (($finder % 2) === 0 ? 0 : 2);
            for ($pos = 0; $pos < 3; ++$pos) {
                $mask[$start + $pos] = true;
            }

            $offset += 8 + 5 + 8;
        }

        $modules = [];
        $regions = [];
        foreach ($elements as $pos => $width) {
            $bar = ($pos % 2) === 0 ? 0 : 1;
            $modules = \array_merge($modules, \array_fill(0, \max(0, $width), $bar));
            $regions = \array_merge($regions, \array_fill(0, \max(0, $width), $mask[$pos] ?? false));
        }

        if ($mirror) {
            $modules = \array_reverse($modules);
            $regions = \array_reverse($regions);
        }

        if ($shift) {
            \array_unshift($modules, 0);
            \array_unshift($regions, false);
        }

        return [$modules, $regions];
    }

    /**
     * Get the modules of the separator row adjacent to a symbol row.
     * Every module is the complement of the symbol module next to it, except
     * over the wide part of a finder pattern, where the spaces alternate, and at
     * the four modules of each end of the row, which are always spaces.
     *
     * @param array<int, int>  $modules Modules of the symbol row
     * @param array<int, bool> $regions Finder pattern regions of the symbol row
     * @param int              $width   Width in modules of the separator pattern
     *
     * @return array<int, int>
     */
    protected function getSeparatorRow(array $modules, array $regions, int $width): array
    {
        $separator = \array_fill(0, \max(0, $width), 0);
        $count = \count($modules);
        $offset = 0;
        for ($pos = 0; $pos < $count; ++$pos) {
            $module = $modules[$pos] ?? 0;
            if (($regions[$pos] ?? false) && $module === 0) {
                $separator[$pos] = ($offset % 2) === 0 ? 1 : 0;
                ++$offset;
                continue;
            }

            $offset = 0;
            $separator[$pos] = 1 - $module;
        }

        for ($pos = 0; $pos < $this::SEPARATOR_MARGIN; ++$pos) {
            $separator[$pos] = 0;
            $separator[$count - 1 - $pos] = 0;
        }

        return $separator;
    }

    /**
     * Get the rows of the separator pattern between two symbol rows.
     *
     * @param array{array<int, int>, array<int, bool>} $above Modules and finder regions of the row above
     * @param array{array<int, int>, array<int, bool>} $below Modules and finder regions of the row below
     * @param int                                      $width Width in modules of the separator pattern
     *
     * @return array<int, array<int, int>>
     */
    protected function getSeparator(array $above, array $below, int $width): array
    {
        $middle = [];
        for ($pos = 0; $pos < $width; ++$pos) {
            $middle[] = $pos % 2;
        }

        for ($pos = 0; $pos < $this::SEPARATOR_MARGIN; ++$pos) {
            $middle[$pos] = 0;
            $middle[$width - 1 - $pos] = 0;
        }

        return [
            $this->getSeparatorRow($above[0], $above[1], $width),
            $middle,
            $this->getSeparatorRow($below[0], $below[1], $width),
        ];
    }

    /**
     * Split the symbol characters over the rows.
     *
     * @param array<int, array<int, int>> $chars   Element widths of the symbol characters
     * @param array<int, int>             $finders Finder patterns of the symbol
     *
     * @return array<int, array{array<int, int>, array<int, bool>}>
     *
     * @throws BarcodeException in case of error
     */
    protected function getRows(array $chars, array $finders): array
    {
        $count = \count($chars);
        $rows = [];
        for ($first = 0; $first < $count; $first += $this->segments) {
            $size = \min($this->segments, $count - $first);
            $index = \count($rows) + 1;
            $last = ($first + $size) >= $count;
            $used = (int) \ceil($size / 2);
            $rows[] = $this->getRow(
                \array_slice($chars, $first, $size),
                \array_slice($finders, (int) ($first / 2), $used),
                ($index % 2) === 0 && ($size % 4) === 0,
                $last && $size < $this->segments && ($used % 2) === 1,
            );
        }

        return $rows;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $chars = $this->getCharacters();
        $rows = $this->getRows($chars, Data::EXPANDED_SEQUENCE[\count($chars)] ?? []);

        $this->ncols = 0;
        foreach ($rows as $row) {
            $this->ncols = \max($this->ncols, \count($row[0]));
        }

        $this->nrows = (\count($rows) * $this::ROW_HEIGHT) + ((\count($rows) - 1) * $this::SEPARATOR_HEIGHT);
        $this->bars = [];

        $posy = 0;
        foreach ($rows as $index => $row) {
            if ($index > 0) {
                $above = $rows[$index - 1] ?? $row;
                foreach ($this->getSeparator($above, $row, $this->ncols) as $line) {
                    $this->addModuleBars($line, $posy, 1);
                    ++$posy;
                }
            }

            $this->addModuleBars($row[0], $posy, $this::ROW_HEIGHT);
            $posy += $this::ROW_HEIGHT;
        }
    }
}
