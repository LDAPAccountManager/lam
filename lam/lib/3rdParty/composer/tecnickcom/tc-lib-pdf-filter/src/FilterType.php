<?php

declare(strict_types=1);

/**
 * FilterType.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter;

use Com\Tecnick\Pdf\Filter\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Filter\FilterType
 *
 * Backed enum for the standard PDF stream filters (PDF 32000-2008 - 7.4 Filters).
 * The backing value of each case is the exact filter name validated by Filter::decode().
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
enum FilterType: string
{
    case AsciiHexDecode = 'ASCIIHexDecode';

    case Ascii85Decode = 'ASCII85Decode';

    case LzwDecode = 'LZWDecode';

    case FlateDecode = 'FlateDecode';

    case RunLengthDecode = 'RunLengthDecode';

    case CcittFaxDecode = 'CCITTFaxDecode';

    case Jbig2Decode = 'JBIG2Decode';

    case DctDecode = 'DCTDecode';

    case JpxDecode = 'JPXDecode';

    case Crypt = 'Crypt';

    /**
     * Resolve a loose filter name to the matching enum case.
     *
     * Accepts the exact PDF filter name (as validated by Filter::decode) or an
     * enum instance (returned unchanged). Unknown names throw, matching the
     * closed set enforced by Filter::decode.
     *
     * @param string|self $value PDF filter name or enum case.
     *
     * @throws PPException if the value does not match a known PDF filter.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new PPException('unknown filter: ' . $value);
    }
}
