<?php

declare(strict_types=1);

/**
 * FourState.php
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
 * Com\Tecnick\Barcode\Type\Linear\FourState
 *
 * Shared layer of the four state postal symbologies
 *
 * A bar occupies one of three rows: the ascender, the tracker and the
 * descender. The four states are the full bar, the ascender with the tracker,
 * the tracker with the descender and the tracker alone. Bars are one module
 * wide and separated by one module.
 *
 * Subclasses name the three states that are not the tracker with the FULL,
 * ASCENDER and DESCENDER constants; every other bar identifier is a tracker.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class FourState extends \Com\Tecnick\Barcode\Type\Linear
{
    /**
     * Number of rows of a four state symbol
     */
    protected const ROWS = 3;

    /**
     * Bar identifier of the ascender, the tracker and the descender
     *
     * @var string
     */
    protected const FULL = '';

    /**
     * Bar identifier of the ascender and the tracker
     *
     * @var string
     */
    protected const ASCENDER = '';

    /**
     * Bar identifier of the tracker and the descender
     *
     * @var string
     */
    protected const DESCENDER = '';

    /**
     * Append one bar and advance to the next bar position.
     *
     * @param string $value Bar identifier
     */
    protected function addBar(string $value): void
    {
        $this->bars[] = match ($value) {
            $this::FULL => [$this->ncols, 0, 1, 3],
            $this::ASCENDER => [$this->ncols, 0, 1, 2],
            $this::DESCENDER => [$this->ncols, 1, 1, 2],
            default => [$this->ncols, 1, 1, 1],
        };

        $this->ncols += 2;
    }

    /**
     * Draw the whole symbol from a string of bar identifiers.
     * The trailing separator of the last bar is not part of the symbol.
     *
     * @param string $values Bar identifiers, one per bar
     */
    protected function setStateBars(string $values): void
    {
        $this->ncols = 0;
        $this->nrows = $this::ROWS;
        $this->bars = [];

        $vlen = \strlen($values);
        for ($pos = 0; $pos < $vlen; ++$pos) {
            $this->addBar($values[$pos]);
        }

        --$this->ncols;
    }
}
