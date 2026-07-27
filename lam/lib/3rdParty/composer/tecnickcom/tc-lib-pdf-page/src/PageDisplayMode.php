<?php

declare(strict_types=1);

/**
 * PageDisplayMode.php
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
 * Com\Tecnick\Pdf\Page\PageDisplayMode
 *
 * Backed enum for the PDF /PageMode name. The backing value of each case is the
 * canonical name produced by Mode::getDisplay().
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
enum PageDisplayMode: string
{
    case UseNone = 'UseNone';

    case UseOutlines = 'UseOutlines';

    case UseThumbs = 'UseThumbs';

    case FullScreen = 'FullScreen';

    case UseOC = 'UseOC';

    case UseAttachments = 'UseAttachments';

    /**
     * Resolve a loose display mode value to the matching enum case.
     *
     * Accepts the canonical name (case-insensitively) or an enum instance
     * (returned unchanged). The empty string maps to UseAttachments and any
     * other unknown value falls back to UseNone, matching Mode::getDisplay().
     *
     * @param string|self $value Display mode name or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return match (\strtolower($value)) {
            'usenone' => self::UseNone,
            'useoutlines' => self::UseOutlines,
            'usethumbs' => self::UseThumbs,
            'fullscreen' => self::FullScreen,
            'useoc' => self::UseOC,
            'useattachments', '' => self::UseAttachments,
            default => self::UseNone,
        };
    }
}
