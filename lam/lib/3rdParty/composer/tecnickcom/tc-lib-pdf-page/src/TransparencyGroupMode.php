<?php

declare(strict_types=1);

/**
 * TransparencyGroupMode.php
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
 * Com\Tecnick\Pdf\Page\TransparencyGroupMode
 *
 * Backed enum for the per-page transparency /Group emission policy: 'auto'
 * (only on pages flagged as using transparency), 'always' or 'never'.
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
enum TransparencyGroupMode: string
{
    case Auto = 'auto';

    case Always = 'always';

    case Never = 'never';

    /**
     * Resolve a loose transparency group mode value to the matching enum case.
     *
     * Accepts the canonical value (case-insensitively) or an enum instance
     * (returned unchanged). Unknown values fall back to Auto, matching the
     * lenient behavior of Page::setPageTransparencyGroupMode().
     *
     * @param string|self $value Transparency group mode or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom(\strtolower($value)) ?? self::Auto;
    }
}
