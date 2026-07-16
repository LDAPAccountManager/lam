<?php

/**
 * HslTest.php
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
 * Hsl Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class HslTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Model\Hsl
    {
        return new \Com\Tecnick\Color\Model\Hsl([
            'hue' => 0.583,
            'saturation' => 0.5,
            'lightness' => 0.5,
            'alpha' => 0.85,
        ]);
    }

    public function testGetType(): void
    {
        $hsl = $this->getTestObject();
        $type = $hsl->getType();
        $this->assertEquals('HSL', $type);
    }

    public function testGetNormalizedValue(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->getNormalizedValue(0.5, 255);
        $this->assertEquals(128, $res);
    }

    public function testGetHexValue(): void
    {
        $testObj = $this->getTestObject();
        $testObj = $this->getTestObject();

        $res = $testObj->getHexValue(0.5, 255);
        $this->assertEquals('80', $res);
    }

    public function testGetRgbaHexColor(): void
    {
        $hsl = $this->getTestObject();
        $rgbaHexColor = $hsl->getRgbaHexColor();
        $this->assertEquals('#4080bfd9', $rgbaHexColor);
    }

    public function testGetRgbHexColor(): void
    {
        $hsl = $this->getTestObject();
        $rgbHexColor = $hsl->getRgbHexColor();
        $this->assertEquals('#4080bf', $rgbHexColor);
    }

    public function testGetArray(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->getArray();
        $this->assertEquals(
            [
                'H' => 0.583,
                'S' => 0.5,
                'L' => 0.5,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testGetPDFacArray(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->getPDFacArray();
        $this->bcAssertEqualsWithDelta(
            [
                0.25,
                0.50,
                0.75,
            ],
            $res,
        );
    }

    public function testGetNormalizedArray(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->getNormalizedArray(255);
        $this->assertEquals(
            [
                'H' => 210,
                'S' => 0.5,
                'L' => 0.5,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testGetCssColor(): void
    {
        $hsl = $this->getTestObject();
        $cssColor = $hsl->getCssColor();
        $this->assertEquals('hsla(210,50%,50%,0.85)', $cssColor);

        $opaque = new \Com\Tecnick\Color\Model\Hsl([
            'hue' => 0.583,
            'saturation' => 0.5,
            'lightness' => 0.5,
            'alpha' => 1,
        ]);
        $this->assertEquals('hsl(210,50%,50%)', $opaque->getCssColor());
    }

    public function testGetJsPdfColor(): void
    {
        $testObj = $this->getTestObject();
        $res = $testObj->getJsPdfColor();
        $this->assertEquals('["RGB",0.250000,0.501000,0.750000]', $res);

        $hsl = new \Com\Tecnick\Color\Model\Hsl([
            'hue' => 0.583,
            'saturation' => 0.5,
            'lightness' => 0.5,
            'alpha' => 0,
        ]);
        $res = $hsl->getJsPdfColor();
        $this->assertEquals('["T"]', $res);
    }

    public function testGetComponentsString(): void
    {
        $hsl = $this->getTestObject();
        $componentsString = $hsl->getComponentsString();
        $this->assertEquals('0.250000 0.501000 0.750000', $componentsString);
    }

    public function testGetPdfColor(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->getPdfColor();
        $this->assertEquals('0.250000 0.501000 0.750000 rg' . "\n", $res);

        $res = $hsl->getPdfColor(false);
        $this->assertEquals('0.250000 0.501000 0.750000 rg' . "\n", $res);

        $res = $hsl->getPdfColor(true);
        $this->assertEquals('0.250000 0.501000 0.750000 RG' . "\n", $res);
    }

    public function testToGrayArray(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->toGrayArray();
        $this->bcAssertEqualsWithDelta([
            'gray' => 0.46561520,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToRgbArray(): void
    {
        $testObj = $this->getTestObject();
        $res = $testObj->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.25,
            'green' => 0.50,
            'blue' => 0.75,
            'alpha' => 0.85,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Hsl([
            'hue' => 0.583,
            'saturation' => 0.5,
            'lightness' => 0.4,
            'alpha' => 1,
        ]);
        $res = $col->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.199,
            'green' => 0.400,
            'blue' => 0.600,
            'alpha' => 1,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Hsl([
            'hue' => 0.583,
            'saturation' => 0,
            'lightness' => 0.4,
            'alpha' => 1,
        ]);
        $res = $col->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.400,
            'green' => 0.400,
            'blue' => 0.400,
            'alpha' => 1,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Hsl([
            'hue' => 0.01,
            'saturation' => 1,
            'lightness' => 0.4,
            'alpha' => 1,
        ]);
        $res = $col->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.8,
            'green' => 0.048,
            'blue' => 0,
            'alpha' => 1,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Hsl([
            'hue' => 1,
            'saturation' => 1,
            'lightness' => 0.4,
            'alpha' => 1,
        ]);
        $res = $col->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.8,
            'green' => 0,
            'blue' => 0,
            'alpha' => 1,
        ], $res);
    }

    public function testToHslArray(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0.583,
            'saturation' => 0.5,
            'lightness' => 0.5,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToCmykArray(): void
    {
        $hsl = $this->getTestObject();
        $res = $hsl->toCmykArray();
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
        $hsl = $this->getTestObject();
        $res = $hsl->toLabArray();
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
        $hsl = $this->getTestObject();
        $hsl->invertColor();

        $res = $hsl->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0.083,
            'saturation' => 0.5,
            'lightness' => 0.5,
            'alpha' => 0.85,
        ], $res);
    }
}
