<?php

declare(strict_types=1);

/**
 * GsOneDataBarStackedOmni.php
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

/**
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBarStackedOmni;
 *
 * GsOneDataBarStackedOmni Barcode type class
 * GS1 DataBar Stacked Omnidirectional (ISO/IEC 24724)
 *
 * Full height two-row variation of GS1 DataBar Omnidirectional, 50 modules wide
 * by 69 modules high: two rows of 33 modules divided by a separator pattern of
 * three module rows. The rows carry the same halves as GS1 DataBar Stacked.
 *
 * The outer rows of the separator pattern are the complement of the adjacent
 * symbol row, except over the finder pattern, where the ten modules that follow
 * its first element alternate. The middle row alternates over its whole width.
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
class GsOneDataBarStackedOmni extends \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarStacked
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATABARSTACKOMNI';

    /**
     * Height in modules of the top row
     */
    protected const TOP_HEIGHT = 33;

    /**
     * Height in modules of the bottom row
     */
    protected const BOTTOM_HEIGHT = 33;

    /**
     * Module of the top row where the left finder pattern begins:
     * the left guard pattern and the first data character precede it
     */
    protected const FINDER_TOP = 18;

    /**
     * Module of the bottom row where the right finder pattern ends:
     * the guard pattern, the fourth data character and the finder pattern precede it
     */
    protected const FINDER_BOTTOM = 31;

    /**
     * Number of modules of the finder pattern that alternate in the separator
     */
    protected const FINDER_ALTERNATING = 10;

    /**
     * Get the modules of a separator row adjacent to a symbol row.
     *
     * @param array<int, int> $row    Modules of the adjacent symbol row
     * @param int             $start  Module of the first element of the finder pattern
     * @param int             $step   1 when the finder elements are numbered left to right, -1 otherwise
     * @param int             $first  Width in modules of the first element of the finder pattern
     *
     * @return array<int, int> One entry per module, 1 for a bar and 0 for a space
     */
    protected function getSeparatorRow(array $row, int $start, int $step, int $first): array
    {
        $separator = [];
        foreach ($row as $pos => $module) {
            $separator[$pos] = 1 - $module;
        }

        $posx = $start + ($step * $first);
        $module = $separator[$posx - $step] ?? 0;
        for ($count = 0; $count < $this::FINDER_ALTERNATING; ++$count) {
            $module = 1 - $module;
            $separator[$posx] = $module;
            $posx += $step;
        }

        return $separator;
    }

    /**
     * Get the rows of the separator pattern.
     *
     * @param array<int, int> $top    Modules of the top row
     * @param array<int, int> $bottom Modules of the bottom row
     *
     * @return array<int, array<int, int>> One row per module of separator height
     */
    protected function getSeparator(array $top, array $bottom): array
    {
        $middle = [];
        $count = \count($top);
        for ($posx = 0; $posx < $count; ++$posx) {
            $middle[] = $posx % 2;
        }

        return [
            $this->getSeparatorRow($top, $this::FINDER_TOP, 1, $this->getFinderPattern($this->finder_left)[0] ?? 1),
            $middle,
            $this->getSeparatorRow(
                $bottom,
                $this::FINDER_BOTTOM,
                -1,
                $this->getFinderPattern($this->finder_right)[0] ?? 1,
            ),
        ];
    }
}
