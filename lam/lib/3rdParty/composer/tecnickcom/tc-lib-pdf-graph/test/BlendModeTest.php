<?php

/**
 * BlendModeTest.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * This file is part of tc-lib-pdf-graph software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Graph\BlendMode;

/**
 * BlendMode enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
class BlendModeTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Graph\Draw
    {
        return new \Com\Tecnick\Pdf\Graph\Draw(1, 0, 0, new \Com\Tecnick\Color\Pdf(), $this->getEncryptObject(), false);
    }

    public function testCaseBackingValues(): void
    {
        $this->assertSame('Normal', BlendMode::Normal->value);
        $this->assertSame('Multiply', BlendMode::Multiply->value);
        $this->assertSame('Luminosity', BlendMode::Luminosity->value);
        $this->assertCount(16, BlendMode::cases());
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(BlendMode::Multiply, BlendMode::fromLoose('Multiply'));
        $this->assertSame(BlendMode::ColorBurn, BlendMode::fromLoose('ColorBurn'));
    }

    public function testFromLooseStripsLeadingSlash(): void
    {
        $this->assertSame(BlendMode::Screen, BlendMode::fromLoose('/Screen'));
    }

    public function testFromLooseFallsBackToNormal(): void
    {
        $this->assertSame(BlendMode::Normal, BlendMode::fromLoose('bogus'));
        $this->assertSame(BlendMode::Normal, BlendMode::fromLoose(''));
        // case sensitive: wrong case is unknown and falls back
        $this->assertSame(BlendMode::Normal, BlendMode::fromLoose('multiply'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(BlendMode::HardLight, BlendMode::fromLoose(BlendMode::HardLight));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (BlendMode::cases() as $case) {
            $this->assertSame($case, BlendMode::fromLoose($case->value));
        }
    }

    /**
     * The widened getAlpha() accepts a BlendMode enum; equal blend modes dedupe
     * to the same ExtGState reference, different ones do not.
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetAlphaAcceptsEnum(): void
    {
        $draw = $this->getTestObject();
        $fromEnum = $draw->getAlpha(1.0, BlendMode::Multiply);
        $this->assertSame($fromEnum, $draw->getAlpha(1.0, 'Multiply'));
        $this->assertSame($fromEnum, $draw->getAlpha(1.0, '/Multiply'));
        $this->assertNotSame($fromEnum, $draw->getAlpha(1.0, BlendMode::Screen));
        // unknown blend mode falls back to Normal
        $this->assertSame($draw->getAlpha(1.0, 'Normal'), $draw->getAlpha(1.0, 'bogus'));
    }
}
