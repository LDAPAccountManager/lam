<?php

/**
 * ModeTest.php
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
 * Mode Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class ModeTest extends TestUtil
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
    public function testGetLayout(): void
    {
        $page = $this->getTestObject();
        $this->assertEquals('TwoColumnLeft', $page->getLayout('two'));
        $this->assertEquals('SinglePage', $page->getLayout(''));
        $this->assertEquals('SinglePage', $page->getLayout());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetDisplay(): void
    {
        $page = $this->getTestObject();
        $this->assertEquals('UseThumbs', $page->getDisplay('usethumbs'));
        $this->assertEquals('UseAttachments', $page->getDisplay(''));
        $this->assertEquals('UseNone', $page->getDisplay('something'));
    }
}
