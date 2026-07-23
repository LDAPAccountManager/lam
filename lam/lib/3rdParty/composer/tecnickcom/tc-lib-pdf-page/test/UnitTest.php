<?php

/**
 * UnitTest.php
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
 * Unit Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class UnitTest extends TestUtil
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
        $val = $page->convertPoints(72, 'in', 3);
        $this->assertEquals(1, $val);

        $val = $page->convertPoints(72, 'mm', 3);
        $this->assertEquals(25.4, $val);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetPageSizeEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $page = $this->getTestObject();
        $page->convertPoints(1, '*ERROR*', 2);
    }
}
