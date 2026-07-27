<?php

/**
 * PageLayoutTest.php
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

use Com\Tecnick\Pdf\Page\PageLayout;

/**
 * PageLayout enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class PageLayoutTest extends TestUtil
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
        $this->assertSame('SinglePage', PageLayout::SinglePage->value);
        $this->assertSame('OneColumn', PageLayout::OneColumn->value);
        $this->assertSame('TwoColumnLeft', PageLayout::TwoColumnLeft->value);
        $this->assertSame('TwoColumnRight', PageLayout::TwoColumnRight->value);
        $this->assertSame('TwoPageLeft', PageLayout::TwoPageLeft->value);
        $this->assertSame('TwoPageRight', PageLayout::TwoPageRight->value);
    }

    public function testFromLooseCanonicalAndAliases(): void
    {
        $this->assertSame(PageLayout::SinglePage, PageLayout::fromLoose('SinglePage'));
        $this->assertSame(PageLayout::SinglePage, PageLayout::fromLoose('single'));
        $this->assertSame(PageLayout::SinglePage, PageLayout::fromLoose('default'));
        $this->assertSame(PageLayout::OneColumn, PageLayout::fromLoose('onecolumn'));
        $this->assertSame(PageLayout::OneColumn, PageLayout::fromLoose('continuous'));
        $this->assertSame(PageLayout::TwoColumnLeft, PageLayout::fromLoose('two'));
        $this->assertSame(PageLayout::TwoColumnRight, PageLayout::fromLoose('TWOCOLUMNRIGHT'));
    }

    public function testFromLooseFallsBackToSinglePage(): void
    {
        $this->assertSame(PageLayout::SinglePage, PageLayout::fromLoose(''));
        $this->assertSame(PageLayout::SinglePage, PageLayout::fromLoose('nonsense'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(PageLayout::TwoPageLeft, PageLayout::fromLoose(PageLayout::TwoPageLeft));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (PageLayout::cases() as $case) {
            $this->assertSame($case, PageLayout::fromLoose($case->value));
        }
    }

    /**
     * The widened getLayout() accepts a PageLayout enum, staying consistent with
     * the legacy alias resolution.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetLayoutAcceptsEnum(): void
    {
        $page = $this->getTestObject();
        $this->assertSame('TwoColumnLeft', $page->getLayout(PageLayout::TwoColumnLeft));
        $this->assertSame($page->getLayout('two'), $page->getLayout(PageLayout::TwoColumnLeft));
    }
}
