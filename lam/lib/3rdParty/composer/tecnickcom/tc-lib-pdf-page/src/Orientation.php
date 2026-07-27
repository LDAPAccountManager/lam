<?php

declare(strict_types=1);

/**
 * Orientation.php
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
 * Com\Tecnick\Pdf\Page\Orientation
 *
 * Backed enum for the requested page orientation: 'P' (portrait), 'L'
 * (landscape) or '' (auto-detect from the page dimensions).
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
enum Orientation: string
{
    case Portrait = 'P';

    case Landscape = 'L';

    case Auto = '';

    /**
     * Resolve a loose orientation value to the matching enum case.
     *
     * Accepts an enum instance (returned unchanged) or a string. Matching the
     * legacy behavior of getPageOrientedSize(), only the exact values 'P' and
     * 'L' select an orientation; anything else means auto (never throws).
     *
     * @param string|self $value Orientation identifier or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return match ($value) {
            'P' => self::Portrait,
            'L' => self::Landscape,
            default => self::Auto,
        };
    }
}
