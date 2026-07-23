<?php

/**
 * RgbTest.php
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
 * Rgb Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class RgbTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Model\Rgb
    {
        return new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0.25,
            'green' => 0.50,
            'blue' => 0.75,
            'alpha' => 0.85,
        ]);
    }

    public function testGetType(): void
    {
        $rgb = $this->getTestObject();
        $type = $rgb->getType();
        $this->assertEquals('RGB', $type);
    }

    public function testGetNormalizedValue(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->getNormalizedValue(0.5, 255);
        $this->assertEquals(128, $res);
    }

    public function testGetHexValue(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->getHexValue(0.5, 255);
        $this->assertEquals('80', $res);
    }

    public function testGetRgbaHexColor(): void
    {
        $rgb = $this->getTestObject();
        $rgbaHexColor = $rgb->getRgbaHexColor();
        $this->assertEquals('#4080bfd9', $rgbaHexColor);
    }

    public function testGetRgbHexColor(): void
    {
        $rgb = $this->getTestObject();
        $rgbHexColor = $rgb->getRgbHexColor();
        $this->assertEquals('#4080bf', $rgbHexColor);
    }

    public function testGetArray(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->getArray();
        $this->assertEquals(
            [
                'R' => 0.25,
                'G' => 0.50,
                'B' => 0.75,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testGetPDFacArray(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->getPDFacArray();
        $this->assertEquals(
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
        $rgb = $this->getTestObject();
        $res = $rgb->getNormalizedArray(255);
        $this->assertEquals(
            [
                'R' => 64,
                'G' => 128,
                'B' => 191,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testGetCssColor(): void
    {
        $rgb = $this->getTestObject();
        $cssColor = $rgb->getCssColor();
        $this->assertEquals('rgba(25%,50%,75%,0.85)', $cssColor);

        $opaque = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0.25,
            'green' => 0.50,
            'blue' => 0.75,
            'alpha' => 1,
        ]);
        $this->assertEquals('rgb(25%,50%,75%)', $opaque->getCssColor());
    }

    public function testGetJsPdfColor(): void
    {
        $testObj = $this->getTestObject();
        $res = $testObj->getJsPdfColor();
        $this->assertEquals('["RGB",0.250000,0.500000,0.750000]', $res);

        $rgb = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0.25,
            'green' => 0.50,
            'blue' => 0.75,
            'alpha' => 0,
        ]);
        $res = $rgb->getJsPdfColor();
        $this->assertEquals('["T"]', $res);
    }

    public function testGetComponentsString(): void
    {
        $rgb = $this->getTestObject();
        $componentsString = $rgb->getComponentsString();
        $this->assertEquals('0.250000 0.500000 0.750000', $componentsString);
    }

    public function testGetPdfColor(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->getPdfColor();
        $this->assertEquals('0.250000 0.500000 0.750000 rg' . "\n", $res);

        $res = $rgb->getPdfColor(false);
        $this->assertEquals('0.250000 0.500000 0.750000 rg' . "\n", $res);

        $res = $rgb->getPdfColor(true);
        $this->assertEquals('0.250000 0.500000 0.750000 RG' . "\n", $res);
    }

    public function testToGrayArray(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->toGrayArray();
        $this->bcAssertEqualsWithDelta([
            'gray' => 0.465,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToRgbArray(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.25,
            'green' => 0.50,
            'blue' => 0.75,
            'alpha' => 0.85,
        ], $res);
    }

    public function testToHslArray(): void
    {
        $testObj = $this->getTestObject();
        $res = $testObj->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0.583,
            'saturation' => 0.5,
            'lightness' => 0.5,
            'alpha' => 0.85,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0,
            'green' => 0,
            'blue' => 0,
            'alpha' => 1,
        ]);
        $res = $col->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0,
            'saturation' => 0,
            'lightness' => 0,
            'alpha' => 1,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0.1,
            'green' => 0.3,
            'blue' => 0.2,
            'alpha' => 1,
        ]);
        $res = $col->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0.416,
            'saturation' => 0.500,
            'lightness' => 0.200,
            'alpha' => 1,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0.3,
            'green' => 0.2,
            'blue' => 0.1,
            'alpha' => 1,
        ]);
        $res = $col->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0.0833,
            'saturation' => 0.500,
            'lightness' => 0.200,
            'alpha' => 1,
        ], $res);

        $col = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 1,
            'green' => 0.1,
            'blue' => 0.9,
            'alpha' => 1,
        ]);
        $res = $col->toHslArray();
        $this->bcAssertEqualsWithDelta([
            'hue' => 0.852,
            'saturation' => 1,
            'lightness' => 0.55,
            'alpha' => 1,
        ], $res);
    }

    public function testToCmykArray(): void
    {
        $testObj = $this->getTestObject();
        $res = $testObj->toCmykArray();
        $this->bcAssertEqualsWithDelta([
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 0.85,
        ], $res);

        $rgb = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0,
            'green' => 0,
            'blue' => 0,
            'alpha' => 1,
        ]);
        $res = $rgb->toCmykArray();
        $this->bcAssertEqualsWithDelta([
            'cyan' => 0,
            'magenta' => 0,
            'yellow' => 0,
            'key' => 1,
            'alpha' => 1,
        ], $res);
    }

    public function testToLabArray(): void
    {
        $rgb = $this->getTestObject();
        $res = $rgb->toLabArray();
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

    public function testToLabArrayLowValueBranch(): void
    {
        $rgb = new \Com\Tecnick\Color\Model\Rgb([
            'red' => 0,
            'green' => 0,
            'blue' => 0,
            'alpha' => 1,
        ]);

        $lab = $rgb->toLabArray();
        $this->assertSame(0.0, $lab['lstar'] ?? 0.0);
        $this->assertEqualsWithDelta(0.0, $lab['astar'] ?? 0.0, 0.05);
        $this->assertEqualsWithDelta(0.0, $lab['bstar'] ?? 0.0, 0.05);
        $this->assertEquals(1.0, $lab['alpha'] ?? 0.0);
    }

    public function testInvertColor(): void
    {
        $rgb = $this->getTestObject();
        $rgb->invertColor();

        $res = $rgb->toRgbArray();
        $this->bcAssertEqualsWithDelta([
            'red' => 0.75,
            'green' => 0.50,
            'blue' => 0.25,
            'alpha' => 0.85,
        ], $res);
    }
}
