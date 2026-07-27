<?php

declare(strict_types=1);

/**
 * PageBoxType.php
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
 * Com\Tecnick\Pdf\Page\PageBoxType
 *
 * Backed enum for the PDF page boundary box names. The backing value of each
 * case matches an entry of Box::BOX validated by Box::setBox().
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
enum PageBoxType: string
{
    case MediaBox = 'MediaBox';

    case CropBox = 'CropBox';

    case BleedBox = 'BleedBox';

    case TrimBox = 'TrimBox';

    case ArtBox = 'ArtBox';

    /**
     * Resolve a loose page box value to the matching enum case.
     *
     * Accepts the exact box name (as validated by Box::setBox) or an enum
     * instance (returned unchanged). Unknown values throw.
     *
     * @param string|self $value Page box name or enum case.
     *
     * @throws PageException if the value does not match a known page box.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new PageException('unknown page box type: ' . $value);
    }
}
