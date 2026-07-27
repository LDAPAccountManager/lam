<?php

declare(strict_types=1);

/**
 * FontType.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\FontType
 *
 * Backed enum for the font type accepted by Import::__construct. The backing
 * value of each case is the canonical type name; the empty string selects
 * autodetection.
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
enum FontType: string
{
    /** Autodetect the font type from the file contents. */
    case Auto = '';

    /** Adobe Font Metrics (one of the 14 Core fonts). */
    case Core = 'Core';

    case TrueType = 'TrueType';

    case TrueTypeUnicode = 'TrueTypeUnicode';

    case Type1 = 'Type1';

    /** CID-0 Japanese. */
    case Cid0Jp = 'CID0JP';

    /** CID-0 Korean. */
    case Cid0Kr = 'CID0KR';

    /** CID-0 Chinese Simplified. */
    case Cid0Cs = 'CID0CS';

    /** CID-0 Chinese Traditional. */
    case Cid0Ct = 'CID0CT';

    /**
     * Resolve a loose font type value to the matching enum case.
     *
     * Accepts the canonical type name (or the empty string for autodetection)
     * or an enum instance (returned unchanged). Unknown values throw, matching
     * the closed set validated by Import::getFontType().
     *
     * @param string|self $value Font type name or enum case.
     *
     * @throws FontException if the value does not match a known font type.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new FontException('unknown or unsupported font type: ' . $value);
    }
}
