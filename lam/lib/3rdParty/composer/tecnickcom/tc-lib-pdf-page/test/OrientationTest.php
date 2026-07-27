<?php

/**
 * OrientationTest.php
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

use Com\Tecnick\Pdf\Page\Orientation;

/**
 * Orientation enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class OrientationTest extends TestUtil
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
        $this->assertSame('P', Orientation::Portrait->value);
        $this->assertSame('L', Orientation::Landscape->value);
        $this->assertSame('', Orientation::Auto->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(Orientation::Portrait, Orientation::fromLoose('P'));
        $this->assertSame(Orientation::Landscape, Orientation::fromLoose('L'));
        $this->assertSame(Orientation::Auto, Orientation::fromLoose(''));
    }

    public function testFromLooseFallsBackToAuto(): void
    {
        // Matching legacy behavior: only exact 'P'/'L' select an orientation.
        $this->assertSame(Orientation::Auto, Orientation::fromLoose('p'));
        $this->assertSame(Orientation::Auto, Orientation::fromLoose('portrait'));
        $this->assertSame(Orientation::Auto, Orientation::fromLoose('X'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(Orientation::Landscape, Orientation::fromLoose(Orientation::Landscape));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (Orientation::cases() as $case) {
            $this->assertSame($case, Orientation::fromLoose($case->value));
        }
    }

    /**
     * The widened getPageOrientedSize() accepts an Orientation enum.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetPageOrientedSizeAcceptsEnum(): void
    {
        $page = $this->getTestObject();
        $this->assertSame(
            $page->getPageOrientedSize(100, 200, 'L'),
            $page->getPageOrientedSize(100, 200, Orientation::Landscape),
        );
        $this->assertSame(
            $page->getPageOrientedSize(100, 200, ''),
            $page->getPageOrientedSize(100, 200, Orientation::Auto),
        );
    }
}
