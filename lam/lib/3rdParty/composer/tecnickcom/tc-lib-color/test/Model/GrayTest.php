<?php

/**
 * GrayTest.php
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
 * Gray Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class GrayTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Model\Gray
    {
        return new \Com\Tecnick\Color\Model\Gray([
            'gray' => 0.75,
            'alpha' => 0.85,
        ]);
    }

    public function testGetType(): void
    {
        $gray = $this->getTestObject();
        $type = $gray->getType();
        $this->assertEquals('GRAY', $type);
    }

    public function testGetNormalizedValue(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->getNormalizedValue(0.5, 255);
        $this->assertEquals(128, $res);
    }

    public function testGetHexValue(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->getHexValue(0.5, 255);
        $this->assertEquals('80', $res);
    }

    public function testGetRgbaHexColor(): void
    {
        $gray = $this->getTestObject();
        $rgbaHexColor = $gray->getRgbaHexColor();
        $this->assertEquals('#bfbfbfd9', $rgbaHexColor);
    }

    public function testGetRgbHexColor(): void
    {
        $gray = $this->getTestObject();
        $rgbHexColor = $gray->getRgbHexColor();
        $this->assertEquals('#bfbfbf', $rgbHexColor);
    }

    public function testGetArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->getArray();
        $this->assertEquals(
            [
                'G' => 0.75,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testGetPDFacArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->getPDFacArray();
        $this->assertEquals(
            [
                0.75,
            ],
            $res,
        );
    }

    public function testGetNormalizedArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->getNormalizedArray(255);
        $this->assertEquals(
            [
                'G' => 191,
                'A' => 0.85,
            ],
            $res,
        );
    }

    public function testGetCssColor(): void
    {
        $gray = $this->getTestObject();
        $cssColor = $gray->getCssColor();
        $this->assertEquals('rgba(75%,75%,75%,0.85)', $cssColor);

        $opaque = new \Com\Tecnick\Color\Model\Gray([
            'gray' => 0.75,
            'alpha' => 1,
        ]);
        $this->assertEquals('g(75%)', $opaque->getCssColor());
    }

    public function testGetJsPdfColor(): void
    {
        $testObj = $this->getTestObject();
        $res = $testObj->getJsPdfColor();
        $this->assertEquals('["G",0.750000]', $res);

        $gray = new \Com\Tecnick\Color\Model\Gray([
            'gray' => 0.5,
            'alpha' => 0,
        ]);
        $res = $gray->getJsPdfColor();
        $this->assertEquals('["T"]', $res);
    }

    public function testGetComponentsString(): void
    {
        $gray = $this->getTestObject();
        $componentsString = $gray->getComponentsString();
        $this->assertEquals('0.750000', $componentsString);
    }

    public function testGetPdfColor(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->getPdfColor();
        $this->assertEquals('0.750000 g' . "\n", $res);

        $res = $gray->getPdfColor(false);
        $this->assertEquals('0.750000 g' . "\n", $res);

        $res = $gray->getPdfColor(true);
        $this->assertEquals('0.750000 G' . "\n", $res);
    }

    public function testToGrayArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->toGrayArray();
        $this->assertEquals(
            [
                'gray' => 0.75,
                'alpha' => 0.85,
            ],
            $res,
        );
    }

    public function testToRgbArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->toRgbArray();
        $this->assertEquals(
            [
                'red' => 0.75,
                'green' => 0.75,
                'blue' => 0.75,
                'alpha' => 0.85,
            ],
            $res,
        );
    }

    public function testToHslArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->toHslArray();
        $this->assertEquals(
            [
                'hue' => 0,
                'saturation' => 0,
                'lightness' => 0.75,
                'alpha' => 0.85,
            ],
            $res,
        );
    }

    public function testToCmykArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->toCmykArray();
        $this->assertEquals(
            [
                'cyan' => 0,
                'magenta' => 0,
                'yellow' => 0,
                'key' => 0.25,
                'alpha' => 0.85,
            ],
            $res,
        );
    }

    public function testToLabArray(): void
    {
        $gray = $this->getTestObject();
        $res = $gray->toLabArray();
        $this->bcAssertEqualsWithDelta(
            [
                'lstar' => 77,
                'astar' => 0,
                'bstar' => 0,
                'alpha' => 0.85,
            ],
            $res,
            1.5,
        );
    }

    public function testInvertColor(): void
    {
        $gray = $this->getTestObject();
        $gray->invertColor();

        $res = $gray->toGrayArray();
        $this->assertEquals(
            [
                'gray' => 0.25,
                'alpha' => 0.85,
            ],
            $res,
        );
    }
}
