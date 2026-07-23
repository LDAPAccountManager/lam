<?php

/**
 * BarcodeTest.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test;

/**
 * Barcode class test
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class BarcodeTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Barcode\Barcode
    {
        return new \Com\Tecnick\Barcode\Barcode();
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetBarcodeObjException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $barcode->getBarcodeObj('ERROR', '01001100011100001111,10110011100011110000', -2, -2, 'purple');
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testSetPaddingException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        /** @var array{0: int, 1: int, 2: int, 3: int} $invalidPadding */
        $invalidPadding = [10];
        $barcode->getBarcodeObj(
            'LRAW,AB,12,E3F',
            '01001100011100001111,10110011100011110000',
            -2,
            -2,
            'purple',
            $invalidPadding,
        );
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testEmptyColumns(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $barcode->getBarcodeObj('LRAW', '');
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testEmptyInput(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $barcode->getBarcodeObj('LRAW', '');
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testSpotColor(): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj('LRAW', '01001100011100001111,10110011100011110000', -2, -2, 'all', [
            -2,
            3,
            0,
            1,
        ]);
        $bobjarr = $type->getArray();
        $this->assertEquals('#000000ff', $bobjarr['color_obj']->getRgbaHexColor());
        $this->assertNUll($bobjarr['bg_color_obj']);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testBackgroundColor(): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj('LRAW', '01001100011100001111,10110011100011110000', -2, -2, 'all', [
            -2,
            3,
            0,
            1,
        ])->setBackgroundColor('mediumaquamarine');
        $bobjarr = $type->getArray();
        $this->assertNotNull($bobjarr['bg_color_obj']);
        $this->assertEquals('#66cdaaff', $bobjarr['bg_color_obj']->getRgbaHexColor());
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testNoColorException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $barcode->getBarcodeObj('LRAW', '01001100011100001111,10110011100011110000', -2, -2, '', [-2, 3, 0, 1]);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testExportMethods(): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj(
            'LRAW,AB,12,E3F',
            '01001100011100001111,10110011100011110000',
            -2,
            -2,
            'purple',
            [-2, 3, 0, 1],
        );

        $this->assertEquals('01001100011100001111,10110011100011110000', $type->getExtendedCode());

        $barr = $type->getArray();
        $this->assertEquals('linear', $barr['type']);
        $this->assertEquals('LRAW', $barr['format']);
        $this->assertEquals(['AB', '12', 'E3F'], $barr['params']);
        $this->assertEquals('01001100011100001111,10110011100011110000', $barr['code']);
        $this->assertEquals('01001100011100001111,10110011100011110000', $barr['extcode']);
        $this->assertEquals(20, $barr['ncols']);
        $this->assertEquals(2, $barr['nrows']);
        $this->assertEquals(40, $barr['width']);
        $this->assertEquals(4, $barr['height']);
        $this->assertEquals(2, $barr['width_ratio']);
        $this->assertEquals(2, $barr['height_ratio']);
        $this->assertEquals(
            [
                'T' => 4,
                'R' => 3,
                'B' => 0,
                'L' => 1,
            ],
            $barr['padding'],
        );
        $this->assertEquals(44, $barr['full_width']);
        $this->assertEquals(8, $barr['full_height']);

        $expected = [
            [1, 0, 1, 1],
            [4, 0, 2, 1],
            [9, 0, 3, 1],
            [16, 0, 4, 1],
            [0, 1, 1, 1],
            [2, 1, 2, 1],
            [6, 1, 3, 1],
            [12, 1, 4, 1],
        ];
        $this->assertEquals($expected, $barr['bars']);
        $this->assertEquals('#800080ff', $barr['color_obj']->getRgbaHexColor());

        $grid = $type->getGrid('A', 'B');
        $expected = "ABAABBAAABBBAAAABBBB\nBABBAABBBAAABBBBAAAA\n";
        $this->assertEquals($expected, $grid);

        $svg = $type->setBackgroundColor('yellow')->getSvgCode();
        $expected =
            '<?xml version="1.0" standalone="no" ?>
<svg'
            . ' version="1.2"'
            . ' baseProfile="full"'
            . ' xmlns="http://www.w3.org/2000/svg"'
            . ' xmlns:xlink="http://www.w3.org/1999/xlink"'
            . ' xmlns:ev="http://www.w3.org/2001/xml-events"'
            . ' width="44.000000"'
            . ' height="8.000000"'
            . ' viewBox="0 0 44.000000 8.000000">
	<desc>01001100011100001111,10110011100011110000</desc>
	<rect x="0" y="0" width="44.000000" height="8.000000" fill="#ffff00"'
            . ' stroke="none" stroke-width="0" stroke-linecap="square" />
	<g id="bars" fill="#800080" stroke="none" stroke-width="0" stroke-linecap="square">
		<rect x="3.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="9.000000" y="4.000000" width="4.000000" height="2.000000" />
		<rect x="19.000000" y="4.000000" width="6.000000" height="2.000000" />
		<rect x="33.000000" y="4.000000" width="8.000000" height="2.000000" />
		<rect x="1.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="5.000000" y="6.000000" width="4.000000" height="2.000000" />
		<rect x="13.000000" y="6.000000" width="6.000000" height="2.000000" />
		<rect x="25.000000" y="6.000000" width="8.000000" height="2.000000" />
		<rect x="1.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="3.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="5.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="7.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="9.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="11.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="13.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="15.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="17.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="19.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="21.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="23.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="25.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="27.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="29.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="31.000000" y="6.000000" width="2.000000" height="2.000000" />
		<rect x="33.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="35.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="37.000000" y="4.000000" width="2.000000" height="2.000000" />
		<rect x="39.000000" y="4.000000" width="2.000000" height="2.000000" />
	</g>
</svg>
';
        $this->assertEquals($expected, $svg);

        $hdiv = $type->setBackgroundColor('lightcoral')->getHtmlDiv();
        $expected =
            '<div style="width:44.000000px;height:8.000000px;position:relative;font-size:0;'
            . 'border:none;padding:0;margin:0;background-color:rgb(94%,50%,50%);">
	<div style="background-color:rgb(50%,0%,50%);left:3.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:9.000000px;top:4.000000px;'
            . 'width:4.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:19.000000px;top:4.000000px;'
            . 'width:6.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:33.000000px;top:4.000000px;'
            . 'width:8.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:1.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:5.000000px;top:6.000000px;'
            . 'width:4.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:13.000000px;top:6.000000px;'
            . 'width:6.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:25.000000px;top:6.000000px;'
            . 'width:8.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:1.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:3.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:5.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:7.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:9.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:11.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:13.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:15.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:17.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:19.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:21.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:23.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:25.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:27.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:29.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:31.000000px;top:6.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:33.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:35.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:37.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
	<div style="background-color:rgb(50%,0%,50%);left:39.000000px;top:4.000000px;'
            . 'width:2.000000px;height:2.000000px;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>
</div>
';
        $this->assertEquals($expected, $hdiv);

        if (\extension_loaded('imagick')) {
            $pngik = $type->setBackgroundColor('white')->getPngData(true);
            $this->assertEquals('PNG', \substr($pngik, 1, 3));
        }

        $pnggd = $type->setBackgroundColor('white')->getPngData(false);
        $this->assertEquals('PNG', \substr($pnggd, 1, 3));

        $pnggd = $type->setBackgroundColor('')->getPngData(false);
        $this->assertEquals('PNG', \substr($pnggd, 1, 3));
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetSvg(): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj(
            'LRAW,AB,12,E3F',
            '01001100011100001111,10110011100011110000',
            -2,
            -2,
            'purple',
        );

        // empty filename
        \ob_start();
        $type->getSvg();
        $svg = \ob_get_clean();
        $this->assertNotFalse($svg);
        $this->assertEquals('114f33435c265345f7c6cdf673922292', \md5($svg));
        $headers = $this->getResponseHeaders();
        $this->assertEquals(
            'Content-Disposition: inline; filename="114f33435c265345f7c6cdf673922292.svg";',
            $headers[5] ?? '',
        );

        // invalid filename
        \ob_start();
        $type->getSvg('#~');
        $svg = \ob_get_clean();
        $this->assertNotFalse($svg);
        $this->assertEquals('114f33435c265345f7c6cdf673922292', \md5($svg));
        $headers = $this->getResponseHeaders();
        $this->assertEquals(
            'Content-Disposition: inline; filename="114f33435c265345f7c6cdf673922292.svg";',
            $headers[5] ?? '',
        );

        // valid filename
        \ob_start();
        $type->getSvg('test_SVG_filename-001');
        $svg = \ob_get_clean();
        $this->assertNotFalse($svg);
        $this->assertEquals('114f33435c265345f7c6cdf673922292', \md5($svg));
        $headers = $this->getResponseHeaders();
        $this->assertEquals('Content-Disposition: inline; filename="test_SVG_filename-001.svg";', $headers[5] ?? '');
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetPng(): void
    {
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj(
            'LRAW,AB,12,E3F',
            '01001100011100001111,10110011100011110000',
            -2,
            -2,
            'purple',
        );

        // empty filename
        \ob_start();
        $type->getPng();
        $png = \ob_get_clean();
        $this->assertNotFalse($png);
        $this->assertEquals('PNG', \substr($png, 1, 3));
        $headers = $this->getResponseHeaders();
        $this->assertNotEmpty($headers[5] ?? '');

        // invalid filename
        \ob_start();
        $type->getPng('#~');
        $png = \ob_get_clean();
        $this->assertNotFalse($png);
        $this->assertEquals('PNG', \substr($png, 1, 3));
        $headers = $this->getResponseHeaders();
        $this->assertNotEmpty($headers[5] ?? '');

        // valid filename
        \ob_start();
        $type->getPng('test_PNG_filename-001');
        $png = \ob_get_clean();
        $this->assertNotFalse($png);
        $this->assertEquals('PNG', \substr($png, 1, 3));
        $headers = $this->getResponseHeaders();
        $this->assertEquals('Content-Disposition: inline; filename="test_PNG_filename-001.png";', $headers[5] ?? '');
    }

    /**
     * Regression: an over-long payload must be rejected early.
     *
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testPayloadTooLong(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $barcode->getBarcodeObj('QRCODE', \str_pad('', 30001, 'A'));
    }

    /**
     * Regression: a pathological size multiplier must not trigger a huge image allocation.
     *
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testImageTooLarge(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj('QRCODE', 'test', -100000, -100000);
        $type->getGd();
    }
}
