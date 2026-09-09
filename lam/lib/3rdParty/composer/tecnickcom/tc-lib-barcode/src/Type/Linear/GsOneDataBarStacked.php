<?php

declare(strict_types=1);

/**
 * GsOneDataBarStacked.php
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
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBarStacked;
 *
 * GsOneDataBarStacked Barcode type class
 * GS1 DataBar Stacked (ISO/IEC 24724)
 *
 * Reduced height two-row variation of GS1 DataBar Omnidirectional, 50 modules
 * wide by 13 modules high. The top row is the left half of the Omnidirectional
 * symbol followed by a guard pattern of a one module bar and a one module
 * space, the bottom row is the same guard pattern followed by the right half,
 * and a one module high separator pattern divides them. The encoding is
 * identical to GS1 DataBar Omnidirectional.
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
class GsOneDataBarStacked extends \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarOmni
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATABARSTACK';

    /**
     * Height in modules of the top row
     */
    protected const TOP_HEIGHT = 5;

    /**
     * Height in modules of the bottom row
     */
    protected const BOTTOM_HEIGHT = 7;

    /**
     * Guard pattern added to the inner end of each row: a bar and a space of one module
     *
     * @var array<int, int>
     */
    protected const ROW_GUARD = [1, 1];

    /**
     * Get the modules of the two rows of the symbol.
     *
     * @return array{array<int, int>, array<int, int>} One entry per module, 1 for a bar and 0 for a space
     *
     * @throws BarcodeException if the value cannot be encoded
     */
    protected function getRows(): array
    {
        [$left, $right] = $this->getHalves();

        return [
            $this->getElementModules(\array_merge($left, $this::ROW_GUARD), true),
            $this->getElementModules(\array_merge($this::ROW_GUARD, $right), false),
        ];
    }

    /**
     * Number of modules at each end of the separator pattern that are always spaces
     */
    protected const SEPARATOR_MARGIN = 4;

    /**
     * Get the rows of the separator pattern.
     *
     * Every module is the complement of the two symbol modules above and below
     * it when they have the same colour, and the complement of the separator
     * module on its left when they do not. The four modules at each end of the
     * pattern are spaces.
     *
     * @param array<int, int> $top    Modules of the top row
     * @param array<int, int> $bottom Modules of the bottom row
     *
     * @return array<int, array<int, int>> One row per module of separator height
     */
    protected function getSeparator(array $top, array $bottom): array
    {
        $separator = [0];
        $count = \count($top);
        for ($posx = 1; $posx < $count; ++$posx) {
            $above = $top[$posx] ?? 0;
            $below = $bottom[$posx] ?? 0;
            $separator[] = $above === $below ? 1 - $above : 1 - ($separator[$posx - 1] ?? 0);
        }

        for ($pos = 0; $pos < $this::SEPARATOR_MARGIN; ++$pos) {
            $separator[$pos] = 0;
            $separator[$count - 1 - $pos] = 0;
        }

        return [$separator];
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->formatCode();
        [$top, $bottom] = $this->getRows();
        $separator = $this->getSeparator($top, $bottom);

        $this->ncols = \count($top);
        $this->nrows = $this::TOP_HEIGHT + \count($separator) + $this::BOTTOM_HEIGHT;
        $this->bars = [];

        $this->addModuleBars($top, 0, $this::TOP_HEIGHT);
        foreach ($separator as $pos => $row) {
            $this->addModuleBars($row, $this::TOP_HEIGHT + $pos, 1);
        }

        $this->addModuleBars($bottom, $this::TOP_HEIGHT + \count($separator), $this::BOTTOM_HEIGHT);
    }
}
