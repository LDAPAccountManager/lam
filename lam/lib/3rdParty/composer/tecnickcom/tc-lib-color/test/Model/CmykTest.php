<?php

/**
 * CmykTest.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Test\Model;

use Test\TestUtil;

/**
 * Cmyk Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class CmykTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Model\Cmyk
    {
        return new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 0.85,
        ]);
    }

    public function testGetType(): void
    {
        $cmyk = $this->getTestObject();
        $type = $cmyk->getType();
        $this->assertEquals('CMYK', $type);
    }

    public function testGetNormalizedValue(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->getNormalizedValue(0.5, 255);
        $this->assertEquals(128, $res);
    }

    public function testGetHexValue(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->getHexValue(0.5, 255);
        $this->assertEquals('80', $res);
    }

    public function testGetRgbaHexColor(): void
    {
        $cmyk = $this->getTestObject();
        $rgbaHexColor = $cmyk->getRgbaHexColor();
        $this->assertEquals('#4080bfd9', $rgbaHexColor);
    }

    public function testGetRgbHexColor(): void
    {
        $cmyk = $this->getTestObject();
        $rgbHexColor = $cmyk->getRgbHexColor();
        $this->assertEquals('#4080bf', $rgbHexColor);
    }

    public function testGetArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->getArray();
        $this->assertEquals(
            [
                'C' => 0.666,
                'M' => 0.333,
                'Y' => 0,
                'K' => 0.25,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testPDFacArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->getPDFacArray();
        $this->assertEquals(
            [
                0.666,
                0.333,
                0,
                0.25,
            ],
            $res,
        );
    }

    public function testGetNormalizedArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->getNormalizedArray(100);
        $this->assertEquals(
            [
                'C' => 67,
                'M' => 33,
                'Y' => 0,
                'K' => 25,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testGetCssColor(): void
    {
        $cmyk = $this->getTestObject();
        $cssColor = $cmyk->getCssColor();
        $this->assertEquals('cmyka(67%,33%,0%,25%,0.85)', $cssColor);

        $opaque = new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 1,
        ]);
        $this->assertEquals('cmyk(67%,33%,0%,25%)', $opaque->getCssColor());
    }

    public function testGetJsPdfColor(): void
    {
        $testObj = $this->getTestObject();
        $res = $testObj->getJsPdfColor();
        $this->assertEquals('["CMYK",0.666000,0.333000,0.000000,0.250000]', $res);

        $cmyk = new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 0,
        ]);
        $res = $cmyk->getJsPdfColor();
        $this->assertEquals('["T"]', $res);
    }

    public function testGetComponentsString(): void
    {
        $cmyk = $this->getTestObject();
        $componentsString = $cmyk->getComponentsString();
        $this->assertEquals('0.666000 0.333000 0.000000 0.250000', $componentsString);
    }

    public function testGetPdfColor(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->getPdfColor();
        $this->assertEquals('0.666000 0.333000 0.000000 0.250000 k' . "\n", $res);

        $res = $cmyk->getPdfColor(false);
        $this->assertEquals('0.666000 0.333000 0.000000 0.250000 k' . "\n", $res);

        $res = $cmyk->getPdfColor(true);
        $this->assertEquals('0.666000 0.333000 0.000000 0.250000 K' . "\n", $res);
    }

    public function testToGrayArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->toGrayArray();
        $this->bcAssertEqualsWithDelta([
            'gray' => 0.46518510,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToRgbArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.25,
            'green' => 0.50,
            'blue' => 0.75,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToHslArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0.583,
            'saturation' => 0.5,
            'lightness' => 0.5,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToCmykArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->toCmykArray();
        $this->bcAssertEqualsWithDelta([
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToLabArray(): void
    {
        $cmyk = $this->getTestObject();
        $res = $cmyk->toLabArray();
        $this->bcAssertEqualsWithDelta(
            [
                'lstar' => 52,
                'astar' => 0,
                'bstar' => -39,
                'alpha' => 0.85,
            ],
            $res,
            1.5,
        );
    }

    public function testInvertColor(): void
    {
        $cmyk = $this->getTestObject();
        $cmyk->invertColor();

        $res = $cmyk->toCmykArray();
        $this->bcAssertEqualsWithDelta([
            'cyan' => 0.333,
            'magenta' => 0.666,
            'yellow' => 1,
            'key' => 0.75,
            'alpha' => 0.85,
        ], $res);
    }
}
