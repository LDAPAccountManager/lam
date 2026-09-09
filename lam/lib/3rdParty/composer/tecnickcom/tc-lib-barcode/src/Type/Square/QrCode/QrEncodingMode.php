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
 * case is the token accepted as the second parameter of QrCode. Only KJ changes
 * the encoder behaviour, by adding the kanji mode to the modes it may use.
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
    /** Variable, the numeric, alphanumeric and byte modes mixed. */
    case NL = 'NL';

    /** Numeric, kept for compatibility and equivalent to NL. */
    case NM = 'NM';

    /** Alphanumeric, kept for compatibility and equivalent to NL. */
    case AN = 'AN';

    /** 8-bit byte, kept for compatibility and equivalent to NL. */
    case Byte = '8B';

    /** Kanji, which adds the kanji mode to the modes of NL. */
    case KJ = 'KJ';

    /** Structured append, which is not implemented and is equivalent to NL. */
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
