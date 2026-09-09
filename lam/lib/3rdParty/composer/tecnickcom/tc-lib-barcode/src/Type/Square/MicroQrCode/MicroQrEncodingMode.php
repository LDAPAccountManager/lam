<?php

declare(strict_types=1);

/**
 * MicroQrEncodingMode.php
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
 * Com\Tecnick\Barcode\Type\Square\MicroQrCode\MicroQrEncodingMode
 *
 * Backed enum for the Micro QR Code encodation mode. The Kanji, ECI, FNC1 and
 * structured append modes are not available.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum MicroQrEncodingMode: string
{
    /** Mode that yields the shortest bit stream. */
    case Auto = '';

    /** Numeric. */
    case NM = 'NM';

    /** Alphanumeric. */
    case AN = 'AN';

    /** 8-bit byte. */
    case Byte = '8B';

    /**
     * Resolve a loose encodation mode value to the matching enum case.
     *
     * Accepts the canonical token or an enum instance (returned unchanged).
     * Unknown values fall back to the automatic mode.
     *
     * @param string|self $value Encodation mode token or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::Auto;
    }

    /**
     * Returns the mode value of Table 2 of ISO/IEC 18004, or a negative value
     * for the automatic mode.
     */
    public function getMode(): int
    {
        return match ($this) {
            self::NM => Data::MODE_NUMERIC,
            self::AN => Data::MODE_ALPHANUM,
            self::Byte => Data::MODE_BYTE,
            default => -1,
        };
    }
}
