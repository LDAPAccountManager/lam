<?php

declare(strict_types=1);

/**
 * DmreSize.php
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

namespace Com\Tecnick\Barcode\Type\Square\Dmre;

/**
 * Com\Tecnick\Barcode\Type\Square\Dmre\DmreSize
 *
 * Backed enum for the DMRE symbol size: the eighteen sizes of ISO/IEC 21471 as
 * rows by columns, or Auto to choose the smallest one that fits the data.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum DmreSize: string
{
    /** Smallest size that fits the data (default). */
    case Auto = '';

    /** 8 rows by 48 columns, 18 data codewords. */
    case Size8x48 = '8x48';

    /** 8 rows by 64 columns, 24 data codewords. */
    case Size8x64 = '8x64';

    /** 8 rows by 80 columns, 32 data codewords. */
    case Size8x80 = '8x80';

    /** 8 rows by 96 columns, 38 data codewords. */
    case Size8x96 = '8x96';

    /** 8 rows by 120 columns, 49 data codewords. */
    case Size8x120 = '8x120';

    /** 8 rows by 144 columns, 63 data codewords. */
    case Size8x144 = '8x144';

    /** 12 rows by 64 columns, 43 data codewords. */
    case Size12x64 = '12x64';

    /** 12 rows by 88 columns, 64 data codewords. */
    case Size12x88 = '12x88';

    /** 16 rows by 64 columns, 62 data codewords. */
    case Size16x64 = '16x64';

    /** 20 rows by 36 columns, 44 data codewords. */
    case Size20x36 = '20x36';

    /** 20 rows by 44 columns, 56 data codewords. */
    case Size20x44 = '20x44';

    /** 20 rows by 64 columns, 84 data codewords. */
    case Size20x64 = '20x64';

    /** 22 rows by 48 columns, 72 data codewords. */
    case Size22x48 = '22x48';

    /** 24 rows by 48 columns, 80 data codewords. */
    case Size24x48 = '24x48';

    /** 24 rows by 64 columns, 108 data codewords. */
    case Size24x64 = '24x64';

    /** 26 rows by 40 columns, 70 data codewords. */
    case Size26x40 = '26x40';

    /** 26 rows by 48 columns, 90 data codewords. */
    case Size26x48 = '26x48';

    /** 26 rows by 64 columns, 118 data codewords. */
    case Size26x64 = '26x64';

    /**
     * Resolve a loose DMRE size value to the matching enum case.
     *
     * Accepts the rows and columns separated by an upper or lower case X, with
     * or without surrounding spaces, or an enum instance (returned unchanged).
     * Unknown values fall back to Auto, matching the lenient behavior of
     * Datamatrix.
     *
     * @param string|self $value Symbol size or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom(\strtolower(\trim($value))) ?? self::Auto;
    }
}
