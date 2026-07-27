<?php

/**
 * PageBoxTypeTest.php
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

use Com\Tecnick\Pdf\Page\PageBoxType;

/**
 * PageBoxType enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class PageBoxTypeTest extends TestUtil
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
        $this->assertSame('MediaBox', PageBoxType::MediaBox->value);
        $this->assertSame('CropBox', PageBoxType::CropBox->value);
        $this->assertSame('BleedBox', PageBoxType::BleedBox->value);
        $this->assertSame('TrimBox', PageBoxType::TrimBox->value);
        $this->assertSame('ArtBox', PageBoxType::ArtBox->value);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseCanonical(): void
    {
        $this->assertSame(PageBoxType::MediaBox, PageBoxType::fromLoose('MediaBox'));
        $this->assertSame(PageBoxType::ArtBox, PageBoxType::fromLoose('ArtBox'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(PageBoxType::TrimBox, PageBoxType::fromLoose(PageBoxType::TrimBox));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseRoundTrip(): void
    {
        foreach (PageBoxType::cases() as $case) {
            $this->assertSame($case, PageBoxType::fromLoose($case->value));
        }
    }

    /**
     * Box names are case sensitive, so a wrong-case value is unknown.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseIsCaseSensitive(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        PageBoxType::fromLoose('mediabox');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseUnknownThrows(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        PageBoxType::fromLoose('WobbleBox');
    }

    /**
     * The widened setBox() accepts a PageBoxType enum and keys the dimensions
     * array on the same string as the legacy call.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetBoxAcceptsEnum(): void
    {
        $page = $this->getTestObject();
        $fromEnum = $page->setBox([], PageBoxType::CropBox, 0.0, 0.0, 100.0, 200.0);
        $fromString = $page->setBox([], 'CropBox', 0.0, 0.0, 100.0, 200.0);
        $this->assertArrayHasKey('CropBox', $fromEnum);
        $this->assertSame($fromString, $fromEnum);
    }
}
