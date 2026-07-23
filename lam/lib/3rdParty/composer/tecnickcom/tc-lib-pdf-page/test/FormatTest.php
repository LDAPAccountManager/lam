<?php

/**
 * FormatTest.php
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

namespace Test;

/**
 * Format Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class FormatTest extends TestUtil
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

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetPageSize(): void
    {
        $page = $this->getTestObject();
        $dims = $page->getPageFormatSize('A0');
        $this->assertEquals([2383.937, 3370.394, 'P'], $dims);

        $dims = $page->getPageFormatSize('A4', '', 'in', 2);
        $this->assertEquals([8.27, 11.69, 'P'], $dims);

        $dims = $page->getPageFormatSize('LEGAL', '', 'mm', 0);
        $this->assertEquals([216, 356, 'P'], $dims);

        $dims = $page->getPageFormatSize('LEGAL', 'P', 'mm', 0);
        $this->assertEquals([216, 356, 'P'], $dims);

        $dims = $page->getPageFormatSize('LEGAL', 'L', 'mm', 0);
        $this->assertEquals([356, 216, 'L'], $dims);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetPageSizeEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $page = $this->getTestObject();
        $page->getPageFormatSize('*ERROR*');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetPageOrientedSize(): void
    {
        $page = $this->getTestObject();
        $dims = $page->getPageOrientedSize(10, 20);
        $this->assertEquals([10, 20, 'P'], $dims);

        $dims = $page->getPageOrientedSize(10, 20, 'P');
        $this->assertEquals([10, 20, 'P'], $dims);

        $dims = $page->getPageOrientedSize(10, 20, 'L');
        $this->assertEquals([20, 10, 'L'], $dims);

        $dims = $page->getPageOrientedSize(20, 10, 'P');
        $this->assertEquals([10, 20, 'P'], $dims);

        $dims = $page->getPageOrientedSize(20, 10, 'L');
        $this->assertEquals([20, 10, 'L'], $dims);

        $dims = $page->getPageOrientedSize(20, 10);
        $this->assertEquals([20, 10, 'L'], $dims);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetPageOrientation(): void
    {
        $page = $this->getTestObject();
        $orient = $page->getPageOrientation(10, 20);
        $this->assertEquals('P', $orient);

        $orient = $page->getPageOrientation(20, 10);
        $this->assertEquals('L', $orient);
    }
}
