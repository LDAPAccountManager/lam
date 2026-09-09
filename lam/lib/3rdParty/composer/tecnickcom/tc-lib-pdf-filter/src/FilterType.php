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
 * Backed enum of the standard PDF stream filters (PDF 32000-1:2008 §7.4).
 * The backing value of each case is the PDF filter name.
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
     * Accepts an enum case (returned unchanged), the PDF filter name, the
     * inline-image abbreviation (PDF 32000-1:2008 Table 93), and either name
     * written with the leading solidus of a PDF name object. Names are
     * case-sensitive.
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

        $name = \str_starts_with($value, '/') ? \substr($value, 1) : $value;

        return (
            self::tryFrom($name) ?? self::fromAbbreviation($name) ?? throw new PPException('unknown filter: ' . $value)
        );
    }

    /**
     * Resolve an inline-image filter abbreviation (PDF 32000-1:2008, Table 93).
     *
     * @param string $name Abbreviated filter name.
     */
    private static function fromAbbreviation(string $name): ?self
    {
        return match ($name) {
            'AHx' => self::AsciiHexDecode,
            'A85' => self::Ascii85Decode,
            'LZW' => self::LzwDecode,
            'Fl' => self::FlateDecode,
            'RL' => self::RunLengthDecode,
            'CCF' => self::CcittFaxDecode,
            'DCT' => self::DctDecode,
            default => null,
        };
    }
}
