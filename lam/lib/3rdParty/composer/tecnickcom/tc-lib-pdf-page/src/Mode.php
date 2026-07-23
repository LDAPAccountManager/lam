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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
abstract class Mode extends \Com\Tecnick\Pdf\Page\Format
{
    /**
     * Map layouts with their canonical names
     *
     * @var array<string, string>
     */
    protected const LAYOUT = [
        'singlepage' => 'SinglePage', // Display one page at a time
        'default' => 'SinglePage',
        'single' => 'SinglePage',
        'onecolumn' => 'OneColumn', // Display the pages in one column
        'continuous' => 'OneColumn',
        'twocolumnleft' => 'TwoColumnLeft', // Display the pages in two columns, with odd-numbered pages on the left
        'two' => 'TwoColumnLeft',
        'twocolumnright' => 'TwoColumnRight', // Display the pages in two columns, with odd-numbered pages on the right
        'twopageleft' => 'TwoPageLeft', // Display the pages two at a time, with odd-numbered pages on the left
        'twopageright' => 'TwoPageRight', // Display the pages two at a time, with odd-numbered pages on the right
    ];

    /**
     * Map display modes with their canonical names
     *
     * @var array<string, string>
     */
    protected const DISPLAY = [
        'usenone' => 'UseNone', // Neither document outline nor thumbnail images visible
        'useoutlines' => 'UseOutlines', // Document outline visible
        'usethumbs' => 'UseThumbs', // Thumbnail images visible
        'fullscreen' => 'FullScreen', // Full-screen mode, with no menu bar or window controls
        'useoc' => 'UseOC', // (PDF 1.5) Optional content group panel visible
        'useattachments' => 'UseAttachments', // (PDF 1.6) Attachments panel visible
        '' => 'UseAttachments', // (PDF 1.6) Attachments panel visible
    ];

    /**
     * Get the canonical page layout name.
     *
     * @param string $name Page layout name.
     *
     * @return string Canonical page layout name.
     */
    public function getLayout(string $name = ''): string
    {
        $name = \strtolower($name);
        return self::LAYOUT[$name] ?? 'SinglePage';
    }

    /**
     * Get the canonical page display mode.
     *
     * @param string $mode A name object specifying how the document should be displayed when opened.
     *
     * @return string Canonical page display mode.
     */
    public function getDisplay(string $mode = ''): string
    {
        $mode = \strtolower($mode);
        return self::DISPLAY[$mode] ?? 'UseNone';
    }
}
