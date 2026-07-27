<?php

declare(strict_types=1);

/**
 * PageLayout.php
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

/**
 * Com\Tecnick\Pdf\Page\PageLayout
 *
 * Backed enum for the PDF /PageLayout name. The backing value of each case is
 * the canonical name produced by Mode::getLayout().
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
enum PageLayout: string
{
    case SinglePage = 'SinglePage';

    case OneColumn = 'OneColumn';

    case TwoColumnLeft = 'TwoColumnLeft';

    case TwoColumnRight = 'TwoColumnRight';

    case TwoPageLeft = 'TwoPageLeft';

    case TwoPageRight = 'TwoPageRight';

    /**
     * Resolve a loose page layout value to the matching enum case.
     *
     * Accepts the canonical name, its legacy aliases and the empty-string
     * default, case-insensitively, or an enum instance (returned unchanged).
     * Unknown values fall back to SinglePage, matching Mode::getLayout().
     *
     * @param string|self $value Page layout name or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return match (\strtolower($value)) {
            'singlepage', 'default', 'single' => self::SinglePage,
            'onecolumn', 'continuous' => self::OneColumn,
            'twocolumnleft', 'two' => self::TwoColumnLeft,
            'twocolumnright' => self::TwoColumnRight,
            'twopageleft' => self::TwoPageLeft,
            'twopageright' => self::TwoPageRight,
            default => self::SinglePage,
        };
    }
}
