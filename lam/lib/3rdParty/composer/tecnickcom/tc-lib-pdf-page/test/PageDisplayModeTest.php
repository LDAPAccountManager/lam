<?php

/**
 * PageDisplayModeTest.php
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

namespace Test;

use Com\Tecnick\Pdf\Page\PageDisplayMode;

/**
 * PageDisplayMode enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class PageDisplayModeTest extends TestUtil
{
    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    protected function getTestObject(): \Com\Tecnick\Pdf\Page\Page
    {
        $pdf = new \Com\Tecnick\Color\Pdf();
        $encrypt = $this->getEncryptObject();
        return new \Com\Tecnick\Pdf\Page\Page('mm', $pdf, $encrypt, false, false);
    }

    public function testCaseBackingValues(): void
    {
        $this->assertSame('UseNone', PageDisplayMode::UseNone->value);
        $this->assertSame('UseOutlines', PageDisplayMode::UseOutlines->value);
        $this->assertSame('UseThumbs', PageDisplayMode::UseThumbs->value);
        $this->assertSame('FullScreen', PageDisplayMode::FullScreen->value);
        $this->assertSame('UseOC', PageDisplayMode::UseOC->value);
        $this->assertSame('UseAttachments', PageDisplayMode::UseAttachments->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(PageDisplayMode::UseThumbs, PageDisplayMode::fromLoose('usethumbs'));
        $this->assertSame(PageDisplayMode::FullScreen, PageDisplayMode::fromLoose('FullScreen'));
        $this->assertSame(PageDisplayMode::UseOC, PageDisplayMode::fromLoose('USEOC'));
    }

    public function testFromLooseEmptyMapsToUseAttachments(): void
    {
        // Preserves the legacy Mode::DISPLAY quirk: '' maps to UseAttachments.
        $this->assertSame(PageDisplayMode::UseAttachments, PageDisplayMode::fromLoose(''));
    }

    public function testFromLooseUnknownFallsBackToUseNone(): void
    {
        $this->assertSame(PageDisplayMode::UseNone, PageDisplayMode::fromLoose('nonsense'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(PageDisplayMode::UseOutlines, PageDisplayMode::fromLoose(PageDisplayMode::UseOutlines));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (PageDisplayMode::cases() as $case) {
            $this->assertSame($case, PageDisplayMode::fromLoose($case->value));
        }
    }

    /**
     * The widened getDisplay() accepts a PageDisplayMode enum.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetDisplayAcceptsEnum(): void
    {
        $page = $this->getTestObject();
        $this->assertSame('UseThumbs', $page->getDisplay(PageDisplayMode::UseThumbs));
        $this->assertSame($page->getDisplay('usethumbs'), $page->getDisplay(PageDisplayMode::UseThumbs));
    }
}
