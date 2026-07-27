<?php

declare(strict_types=1);

/**
 * QrEncodingMode.php
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
 * Com\Tecnick\Barcode\Type\Square\QrCode\QrEncodingMode
 *
 * Backed enum for the QR Code data encoding mode hint. The backing value of each
 * case is the token used as a key of Data::ENC_MODES.
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum QrEncodingMode: string
{
    /** Terminator / no data. */
    case NL = 'NL';

    /** Numeric. */
    case NM = 'NM';

    /** Alphanumeric. */
    case AN = 'AN';

    /** 8-bit byte. */
    case Byte = '8B';

    /** Kanji. */
    case KJ = 'KJ';

    /** Structured append. */
    case ST = 'ST';

    /**
     * Resolve a loose encoding mode value to the matching enum case.
     *
     * Accepts the canonical token or an enum instance (returned unchanged).
     * Unknown values fall back to Byte (8B), matching the lenient behavior of
     * QrCode.
     *
     * @param string|self $value Encoding mode token or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::Byte;
    }
}
