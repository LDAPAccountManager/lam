<?php

declare(strict_types=1);

/**
 * DatamatrixShape.php
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Datamatrix;

/**
 * Com\Tecnick\Barcode\Type\Square\Datamatrix\DatamatrixShape
 *
 * Backed enum for the Data Matrix symbol shape: S (square, default) or R
 * (rectangular).
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum DatamatrixShape: string
{
    /** Square (default). */
    case Square = 'S';

    /** Rectangular. */
    case Rectangular = 'R';

    /**
     * Resolve a loose Data Matrix shape value to the matching enum case.
     *
     * Accepts the canonical letter or an enum instance (returned unchanged).
     * Unknown values fall back to Square, matching the lenient behavior of
     * Datamatrix.
     *
     * @param string|self $value Shape letter or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::Square;
    }
}
