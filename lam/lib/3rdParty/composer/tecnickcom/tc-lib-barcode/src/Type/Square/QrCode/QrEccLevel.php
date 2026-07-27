<?php

declare(strict_types=1);

/**
 * QrEccLevel.php
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

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\QrEccLevel
 *
 * Backed enum for the QR Code error correction level. The backing value of each
 * case is the letter used as a key of Data::ECC_LEVELS.
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum QrEccLevel: string
{
    /** Low (7%). */
    case L = 'L';

    /** Medium (15%). */
    case M = 'M';

    /** Quartile (25%). */
    case Q = 'Q';

    /** High (30%). */
    case H = 'H';

    /**
     * Resolve a loose ECC level value to the matching enum case.
     *
     * Accepts the canonical letter or an enum instance (returned unchanged).
     * Unknown values fall back to L, matching the lenient behavior of QrCode.
     *
     * @param string|self $value ECC level letter or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::L;
    }
}
