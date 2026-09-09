<?php

declare(strict_types=1);

/**
 * MicroQrEccLevel.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\MicroQrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\MicroQrCode\MicroQrEccLevel
 *
 * Backed enum for the Micro QR Code error correction level. The level H of
 * QR Code is not available; the version M1 carries error detection only and is
 * selected by the automatic level.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum MicroQrEccLevel: string
{
    /** Smallest symbol able to carry the data. */
    case Auto = '';

    /** Low. */
    case L = 'L';

    /** Medium. */
    case M = 'M';

    /** Quartile. */
    case Q = 'Q';

    /**
     * Resolve a loose error correction level value to the matching enum case.
     *
     * Accepts the canonical letter or an enum instance (returned unchanged).
     * Unknown values fall back to the automatic level.
     *
     * @param string|self $value Error correction level letter or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::Auto;
    }
}
