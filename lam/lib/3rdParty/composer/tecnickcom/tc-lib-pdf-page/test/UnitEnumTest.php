<?php

/**
 * UnitEnumTest.php
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

use Com\Tecnick\Pdf\Page\Unit;

/**
 * Unit enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class UnitEnumTest extends TestUtil
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
        $this->assertSame('pt', Unit::Point->value);
        $this->assertSame('mm', Unit::Millimeter->value);
        $this->assertSame('cm', Unit::Centimeter->value);
        $this->assertSame('in', Unit::Inch->value);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseCanonicalAliasesAndDefault(): void
    {
        $this->assertSame(Unit::Point, Unit::fromLoose(''));
        $this->assertSame(Unit::Point, Unit::fromLoose('pt'));
        $this->assertSame(Unit::Point, Unit::fromLoose('points'));
        $this->assertSame(Unit::Point, Unit::fromLoose('px'));
        $this->assertSame(Unit::Millimeter, Unit::fromLoose('mm'));
        $this->assertSame(Unit::Millimeter, Unit::fromLoose('millimeters'));
        $this->assertSame(Unit::Centimeter, Unit::fromLoose('cm'));
        $this->assertSame(Unit::Centimeter, Unit::fromLoose('centimeters'));
        $this->assertSame(Unit::Inch, Unit::fromLoose('in'));
        $this->assertSame(Unit::Inch, Unit::fromLoose('inches'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseIsCaseInsensitive(): void
    {
        $this->assertSame(Unit::Millimeter, Unit::fromLoose('MM'));
        $this->assertSame(Unit::Inch, Unit::fromLoose('Inches'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(Unit::Centimeter, Unit::fromLoose(Unit::Centimeter));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseRoundTrip(): void
    {
        foreach (Unit::cases() as $case) {
            $this->assertSame($case, Unit::fromLoose($case->value));
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testFromLooseUnknownThrows(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        Unit::fromLoose('parsec');
    }

    /**
     * The widened getUnitRatio()/convertPoints() accept a Unit enum.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testUnitRatioAcceptsEnum(): void
    {
        $page = $this->getTestObject();
        $this->assertSame($page->getUnitRatio('in'), $page->getUnitRatio(Unit::Inch));
        $this->assertSame($page->convertPoints(72, 'in', 3), $page->convertPoints(72, Unit::Inch, 3));
    }
}
