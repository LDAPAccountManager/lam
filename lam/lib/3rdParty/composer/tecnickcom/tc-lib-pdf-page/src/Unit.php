<?php

declare(strict_types=1);

/**
 * Unit.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 *
 * This file is part of tc-lib-pdf-page software library.
 */

namespace Com\Tecnick\Pdf\Page;

use Com\Tecnick\Pdf\Page\Exception as PageException;

/**
 * Com\Tecnick\Pdf\Page\Unit
 *
 * Backed enum for the document unit of measure. The backing value of each case
 * is the canonical unit name used as a key of Format::UNITRATIO.
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
enum Unit: string
{
    case Point = 'pt';

    case Millimeter = 'mm';

    case Centimeter = 'cm';

    case Inch = 'in';

    /**
     * Resolve a loose unit value to the matching enum case.
     *
     * Accepts the canonical value, its legacy aliases and the empty-string
     * default, case-insensitively, or an enum instance (returned unchanged).
     * Unknown values throw, matching Format::getUnitRatio().
     *
     * @param string|self $value Unit name or enum case.
     *
     * @throws PageException if the value does not match a known unit.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return match (\strtolower($value)) {
            '', 'pt', 'points', 'px' => self::Point,
            'mm', 'millimeters' => self::Millimeter,
            'cm', 'centimeters' => self::Centimeter,
            'in', 'inches' => self::Inch,
            default => throw new PageException('unknown unit: ' . $value),
        };
    }
}
