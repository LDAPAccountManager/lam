<?php

/**
 * SpotTest.php
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

namespace Test;

use Com\Tecnick\Color\Model\Lab;
use ReflectionProperty;

/**
 * Spot Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class SpotTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Spot
    {
        return new \Com\Tecnick\Color\Spot();
    }

    public function testGetSpotColors(): void
    {
        $spot = $this->getTestObject();
        $res = $spot->getSpotColors();
        $this->assertEquals(0, \count($res));
    }

    public function testNormalizeSpotColorName(): void
    {
        $spot = $this->getTestObject();
        $res = $spot->normalizeSpotColorName('abc.FG12!-345');
        $this->assertEquals('abcfg12345', $res);
    }

    public function testEncodeSpotColorName(): void
    {
        $spot = $this->getTestObject();
        // spaces and uppercase letters are preserved/encoded, not stripped
        $this->assertEquals('SPOTTYPE#20279#20C', $spot->encodeSpotColorName('SPOTTYPE 279 C'));
        // plain ASCII names are returned unchanged
        $this->assertEquals('Reflex#20Blue', $spot->encodeSpotColorName('Reflex Blue'));
        // the number sign and the PDF delimiters are escaped
        $this->assertEquals('a#23b#28c#29#2Fd', $spot->encodeSpotColorName('a#b(c)/d'));
        // tabs and other control/whitespace bytes are escaped
        $this->assertEquals('x#09y', $spot->encodeSpotColorName("x\ty"));
        // bytes outside printable ASCII (e.g. UTF-8 "é") are escaped per byte
        $this->assertEquals('caf#C3#A9', $spot->encodeSpotColorName('café'));
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetPdfSpotObjectsEncodesName(): void
    {
        $spot = $this->getTestObject();
        $spot->addSpotColorFromArray('SPOTTYPE 279 C', [
            'cyan' => 0.66,
            'magenta' => 0.28,
            'yellow' => 0,
            'key' => 0,
            'alpha' => 1,
        ]);

        $obj = 1;
        $res = $spot->getPdfSpotObjects($obj);
        $this->assertStringContainsString('[/Separation /SPOTTYPE#20279#20C /DeviceCMYK', $res);
        $this->assertStringNotContainsString('SPOTTYPE279c', $res);
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetSpotColor(): void
    {
        $spot = $this->getTestObject();
        $res = $spot->getSpotColor('none');
        $this->assertEquals('0.000000 0.000000 0.000000 0.000000 k' . "\n", $res['color']->getPdfColor());
        $res = $spot->getSpotColor('all');
        $this->assertEquals('1.000000 1.000000 1.000000 1.000000 k' . "\n", $res['color']->getPdfColor());
        $res = $spot->getSpotColor('red');
        $this->assertEquals('0.000000 1.000000 1.000000 0.000000 K' . "\n", $res['color']->getPdfColor(true));
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetSpotColorNotFound(): void
    {
        $spot = $this->getTestObject();
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $spot->getSpotColor('missing-color');
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetSpotColorObj(): void
    {
        $spot = $this->getTestObject();
        $res = $spot->getSpotColorObj('none');
        $this->assertEquals('0.000000 0.000000 0.000000 0.000000 k' . "\n", $res->getPdfColor());
        $res = $spot->getSpotColorObj('all');
        $this->assertEquals('1.000000 1.000000 1.000000 1.000000 k' . "\n", $res->getPdfColor());
        $res = $spot->getSpotColorObj('red');
        $this->assertEquals('0.000000 1.000000 1.000000 0.000000 K' . "\n", $res->getPdfColor(true));
    }

    public function testAddSpotColor(): void
    {
        $spot = $this->getTestObject();
        $cmyk = new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 0.85,
        ]);
        $spot->addSpotColor('test', $cmyk);
        $res = $spot->getSpotColors();
        $this->assertArrayHasKey('test', $res);
        $testColor = $res['test'] ?? null;
        $this->assertNotNull($testColor);

        $this->assertEquals(1, $testColor['i']);
        $this->assertEquals('test', $testColor['name']);
        $this->assertEquals('0.666000 0.333000 0.000000 0.250000 k' . "\n", $testColor['color']->getPdfColor());

        // test overwrite
        $cmyk = new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => 0.25,
            'magenta' => 0.35,
            'yellow' => 0.45,
            'key' => 0.55,
            'alpha' => 0.65,
        ]);
        $key = $spot->addSpotColor('test', $cmyk);
        $this->assertEquals('test', $key);

        $res = $spot->getSpotColors();
        $this->assertArrayHasKey('test', $res);
        $testColor = $res['test'] ?? null;
        $this->assertNotNull($testColor);

        $this->assertEquals(1, $testColor['i']);
        $this->assertEquals('test', $testColor['name']);
        $this->assertEquals('0.250000 0.350000 0.450000 0.550000 k' . "\n", $testColor['color']->getPdfColor());
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testAddSpotColorFromArray(): void
    {
        $spot = $this->getTestObject();
        $key = $spot->addSpotColorFromArray('Brand Spot', [
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 0.85,
        ]);

        $this->assertEquals('brandspot', $key);
        $res = $spot->getSpotColor('Brand Spot');
        $this->assertEquals('0.666000 0.333000 0.000000 0.250000 k' . "\n", $res['color']->getPdfColor());
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testAddSpotLabColor(): void
    {
        $spot = $this->getTestObject();
        $key = $spot->addSpotLabColor('Brand Orange', 64.25, 58.5, 71.2);
        $this->assertEquals('brandorange', $key);

        $res = $spot->getSpotColor('Brand Orange');
        $this->assertEquals(1, $res['i']);
        $this->assertEquals('Brand Orange', $res['name']);
        $this->assertEquals('Lab', $res['space']);
        $this->assertInstanceOf(\Com\Tecnick\Color\Model\Cmyk::class, $res['color']);
        $this->assertIsArray($res['lab']);
        $this->assertEquals([0.9505, 1.0, 1.089], $res['lab']['whitepoint']);
        $this->assertEquals([100.0, 0.0, 0.0], $res['lab']['c0']);
        $this->assertInstanceOf(\Com\Tecnick\Color\Model\Lab::class, $res['lab']['model']);
        $this->bcAssertEqualsWithDelta([
            'lstar' => 64.25,
            'astar' => 58.5,
            'bstar' => 71.2,
            'alpha' => 1,
        ], $res['lab']['model']->toLabArray());
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetSpotLabColorObj(): void
    {
        $spot = $this->getTestObject();
        $spot->addSpotLabColor('Brand Orange', 64.25, 58.5, 71.2);

        $res = $spot->getSpotLabColorObj('Brand Orange');
        $this->assertInstanceOf(\Com\Tecnick\Color\Model\Lab::class, $res);
        $this->bcAssertEqualsWithDelta([
            'lstar' => 64.25,
            'astar' => 58.5,
            'bstar' => 71.2,
            'alpha' => 1,
        ], $res->toLabArray());
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetSpotColorObjFromLab(): void
    {
        $spot = $this->getTestObject();
        $spot->addSpotLabColor('Brand Orange', 64.25, 58.5, 71.2);

        $cmyk = $spot->getSpotColorObj('Brand Orange');
        $this->assertInstanceOf(\Com\Tecnick\Color\Model\Cmyk::class, $cmyk);
        $this->assertEquals('0.000000 0.596375 0.949051 0.000000 k' . "\n", $cmyk->getPdfColor());
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetSpotLabColorObjFromCmyk(): void
    {
        $spot = $this->getTestObject();
        $spot->getSpotColor('cyan');

        $lab = $spot->getSpotLabColorObj('cyan');
        $this->assertInstanceOf(\Com\Tecnick\Color\Model\Lab::class, $lab);
    }

    public function testGetPdfSpotObjectsEmpty(): void
    {
        $spot = $this->getTestObject();
        $obj = 1;
        $res = $spot->getPdfSpotObjects($obj);
        $this->assertEquals(1, $obj);
        $this->assertEquals('', $res);
    }

    public function testGetPdfSpotResourcesEmpty(): void
    {
        $spot = $this->getTestObject();
        $pdfSpotResources = $spot->getPdfSpotResources();
        $this->assertEquals('', $pdfSpotResources);
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetPdfSpotObjects(): void
    {
        $spot = $this->getTestObject();
        $cmyk = new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => 0.666,
            'magenta' => 0.333,
            'yellow' => 0,
            'key' => 0.25,
            'alpha' => 0.85,
        ]);
        $spot->addSpotColor('test', $cmyk);
        $spot->getSpotColor('cyan');
        $spot->getSpotColor('magenta');
        $spot->getSpotColor('yellow');
        $spot->getSpotColor('key');

        $obj = 1;
        $res = $spot->getPdfSpotObjects($obj);
        $this->assertEquals(6, $obj);
        $this->assertEquals(
            '2 0 obj'
            . "\n"
            . '[/Separation /test /DeviceCMYK <</Range [0 1 0 1 0 1 0 1] /C0 [0 0 0 0]'
            . ' /C1 [0.666000 0.333000 0.000000 0.250000] /FunctionType 2 /Domain [0 1] /N 1>>]'
            . "\n"
            . 'endobj'
            . "\n"
            . '3 0 obj'
            . "\n"
            . '[/Separation /Cyan /DeviceCMYK <</Range [0 1 0 1 0 1 0 1] /C0 [0 0 0 0]'
            . ' /C1 [1.000000 0.000000 0.000000 0.000000] /FunctionType 2 /Domain [0 1] /N 1>>]'
            . "\n"
            . 'endobj'
            . "\n"
            . '4 0 obj'
            . "\n"
            . '[/Separation /Magenta /DeviceCMYK <</Range [0 1 0 1 0 1 0 1] /C0 [0 0 0 0]'
            . ' /C1 [0.000000 1.000000 0.000000 0.000000] /FunctionType 2 /Domain [0 1] /N 1>>]'
            . "\n"
            . 'endobj'
            . "\n"
            . '5 0 obj'
            . "\n"
            . '[/Separation /Yellow /DeviceCMYK <</Range [0 1 0 1 0 1 0 1] /C0 [0 0 0 0]'
            . ' /C1 [0.000000 0.000000 1.000000 0.000000] /FunctionType 2 /Domain [0 1] /N 1>>]'
            . "\n"
            . 'endobj'
            . "\n"
            . '6 0 obj'
            . "\n"
            . '[/Separation /Key /DeviceCMYK <</Range [0 1 0 1 0 1 0 1] /C0 [0 0 0 0]'
            . ' /C1 [0.000000 0.000000 0.000000 1.000000] /FunctionType 2 /Domain [0 1] /N 1>>]'
            . "\n"
            . 'endobj'
            . "\n",
            $res,
        );

        $res = $spot->getPdfSpotResources();
        $this->assertEquals('/ColorSpace << /CS1 2 0 R /CS2 3 0 R /CS3 4 0 R /CS4 5 0 R /CS5 6 0 R >>' . "\n", $res);

        $resk = $spot->getPdfSpotResourcesByKeys(['cyan', 'yellow']);
        $this->assertEquals('/ColorSpace << /CS2 3 0 R /CS4 5 0 R >>' . "\n", $resk);

        $resk_empty = $spot->getPdfSpotResourcesByKeys([]);
        $this->assertEquals('', $resk_empty);

        // Test with non-existent keys to trigger continue statement
        $resk_mixed = $spot->getPdfSpotResourcesByKeys(['cyan', 'nonexistent', 'yellow', 'invalid']);
        $this->assertEquals('/ColorSpace << /CS2 3 0 R /CS4 5 0 R >>' . "\n", $resk_mixed);
    }

    public function testGetPdfSpotObjectsLab(): void
    {
        $spot = $this->getTestObject();
        $spot->addSpotLabColor('Brand Orange', 64.25, 58.5, 71.2);

        $obj = 7;
        $res = $spot->getPdfSpotObjects($obj);
        $this->assertEquals(8, $obj);
        $this->assertEquals(
            '8 0 obj'
            . "\n"
            . '[/Separation /Brand#20Orange [/Lab << /WhitePoint [0.950500 1.000000 1.089000]'
            . ' /BlackPoint [0.000000 0.000000 0.000000] /Range [-128.000000 127.000000 -128.000000 127.000000]>>] <<'
            . ' /FunctionType 2 /Domain [0 1] /C0 [100.000000 0.000000 0.000000]'
            . ' /C1 [64.250000 58.500000 71.200000] /N 1>>]'
            . "\n"
            . 'endobj'
            . "\n",
            $res,
        );

        $pdfSpotResources = $spot->getPdfSpotResources();
        $this->assertEquals('/ColorSpace << /CS1 8 0 R >>' . "\n", $pdfSpotResources);
    }

    public function testGetPdfSpotObjectsSkipsInvalidNonCmykColor(): void
    {
        $spot = $this->getTestObject();

        $property = new ReflectionProperty($spot, 'spot_colors');
        $property->setValue($spot, [
            'invalid' => [
                'i' => 1,
                'n' => 0,
                'name' => 'Invalid',
                'color' => new Lab([
                    'lstar' => 50,
                    'astar' => 0,
                    'bstar' => 0,
                    'alpha' => 1,
                ]),
                'space' => 'DeviceCMYK',
                'lab' => null,
            ],
        ]);

        $obj = 10;
        // An invalid (non-CMYK, non-Lab) entry is skipped entirely: no object is
        // emitted and the PDF object counter is left unchanged.
        $this->assertSame('', $spot->getPdfSpotObjects($obj));
        $this->assertSame(10, $obj);
    }
}
