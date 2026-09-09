<?php

declare(strict_types=1);

/**
 * Mode.php
 *
 * @since     2011-05-23
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
 * Com\Tecnick\Pdf\Page\Mode
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
abstract class Mode extends \Com\Tecnick\Pdf\Page\Format
{
    /**
     * Get the canonical page layout name.
     * Accepted names, aliases and the fallback are defined by PageLayout::fromLoose().
     *
     * @param string|PageLayout $name Page layout name or PageLayout enum case.
     *
     * @return string Canonical page layout name.
     */
    public function getLayout(string|PageLayout $name = ''): string
    {
        return PageLayout::fromLoose($name)->value;
    }

    /**
     * Get the canonical page display mode.
     * Accepted names and the fallback are defined by PageDisplayMode::fromLoose().
     *
     * @param string|PageDisplayMode $mode Display mode name or PageDisplayMode enum case.
     *
     * @return string Canonical page display mode.
     */
    public function getDisplay(string|PageDisplayMode $mode = ''): string
    {
        return PageDisplayMode::fromLoose($mode)->value;
    }
}
