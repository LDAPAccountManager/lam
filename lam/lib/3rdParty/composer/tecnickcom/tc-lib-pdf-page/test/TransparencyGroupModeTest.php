<?php

/**
 * TransparencyGroupModeTest.php
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

use Com\Tecnick\Pdf\Page\TransparencyGroupMode;

/**
 * TransparencyGroupMode enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class TransparencyGroupModeTest extends TestUtil
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
        $this->assertSame('auto', TransparencyGroupMode::Auto->value);
        $this->assertSame('always', TransparencyGroupMode::Always->value);
        $this->assertSame('never', TransparencyGroupMode::Never->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(TransparencyGroupMode::Auto, TransparencyGroupMode::fromLoose('auto'));
        $this->assertSame(TransparencyGroupMode::Always, TransparencyGroupMode::fromLoose('always'));
        $this->assertSame(TransparencyGroupMode::Never, TransparencyGroupMode::fromLoose('never'));
    }

    public function testFromLooseIsCaseInsensitive(): void
    {
        $this->assertSame(TransparencyGroupMode::Always, TransparencyGroupMode::fromLoose('ALWAYS'));
    }

    public function testFromLooseUnknownFallsBackToAuto(): void
    {
        $this->assertSame(TransparencyGroupMode::Auto, TransparencyGroupMode::fromLoose('sometimes'));
        $this->assertSame(TransparencyGroupMode::Auto, TransparencyGroupMode::fromLoose(''));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(TransparencyGroupMode::Never, TransparencyGroupMode::fromLoose(TransparencyGroupMode::Never));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (TransparencyGroupMode::cases() as $case) {
            $this->assertSame($case, TransparencyGroupMode::fromLoose($case->value));
        }
    }

    /**
     * The widened setter accepts a TransparencyGroupMode enum and stays chainable.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetPageTransparencyGroupModeAcceptsEnum(): void
    {
        $page = $this->getTestObject();
        $this->assertSame($page, $page->setPageTransparencyGroupMode(TransparencyGroupMode::Always));
        $this->assertSame($page, $page->setPageTransparencyGroupMode('never'));
    }
}
