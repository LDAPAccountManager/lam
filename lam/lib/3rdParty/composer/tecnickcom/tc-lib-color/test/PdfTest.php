<?php

/**
 * PdfTest.php
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

/**
 * Pdf Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class PdfTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Pdf
    {
        return new \Com\Tecnick\Color\Pdf();
    }

    public function testGetJsColorString(): void
    {
        $pdf = $this->getTestObject();
        $res = $pdf->getJsColorString('t()');
        $this->assertEquals('color.transparent', $res);
        $res = $pdf->getJsColorString('["T"]');
        $this->assertEquals('color.transparent', $res);
        $res = $pdf->getJsColorString('transparent');
        $this->assertEquals('color.transparent', $res);
        $res = $pdf->getJsColorString('color.transparent');
        $this->assertEquals('color.transparent', $res);
        $res = $pdf->getJsColorString('magenta');
        $this->assertEquals('color.magenta', $res);
        $res = $pdf->getJsColorString('#1a2b3c4d');
        $this->assertEquals('["RGB",0.101961,0.168627,0.235294]', $res);
        $res = $pdf->getJsColorString('#1a2b3c');
        $this->assertEquals('["RGB",0.101961,0.168627,0.235294]', $res);
        $res = $pdf->getJsColorString('#1234');
        $this->assertEquals('["RGB",0.066667,0.133333,0.200000]', $res);
        $res = $pdf->getJsColorString('#123');
        $this->assertEquals('["RGB",0.066667,0.133333,0.200000]', $res);
        $res = $pdf->getJsColorString('["G",0.5]');
        $this->assertEquals('["G",0.500000]', $res);
        $res = $pdf->getJsColorString('["RGB",0.25,0.50,0.75]');
        $this->assertEquals('["RGB",0.250000,0.500000,0.750000]', $res);
        $res = $pdf->getJsColorString('["CMYK",0.666,0.333,0,0.25]');
        $this->assertEquals('["CMYK",0.666000,0.333000,0.000000,0.250000]', $res);
        $res = $pdf->getJsColorString('g(50%)');
        $this->assertEquals('["G",0.500000]', $res);
        $res = $pdf->getJsColorString('g(128)');
        $this->assertEquals('["G",0.501961]', $res);
        $res = $pdf->getJsColorString('rgb(25%,50%,75%)');
        $this->assertEquals('["RGB",0.250000,0.500000,0.750000]', $res);
        $res = $pdf->getJsColorString('rgb(64,128,191)');
        $this->assertEquals('["RGB",0.250980,0.501961,0.749020]', $res);
        $res = $pdf->getJsColorString('rgba(25%,50%,75%,0.85)');
        $this->assertEquals('["RGB",0.250000,0.500000,0.750000]', $res);
        $res = $pdf->getJsColorString('rgba(64,128,191,0.85)');
        $this->assertEquals('["RGB",0.250980,0.501961,0.749020]', $res);
        $res = $pdf->getJsColorString('hsl(210,50%,50%)');
        $this->assertEquals('["RGB",0.250000,0.500000,0.750000]', $res);
        $res = $pdf->getJsColorString('hsla(210,50%,50%,0.85)');
        $this->assertEquals('["RGB",0.250000,0.500000,0.750000]', $res);
        $res = $pdf->getJsColorString('cmyk(67%,33%,0,25%)');
        $this->assertEquals('["CMYK",0.670000,0.330000,0.000000,0.250000]', $res);
        $res = $pdf->getJsColorString('cmyk(67,33,0,25)');
        $this->assertEquals('["CMYK",0.670000,0.330000,0.000000,0.250000]', $res);
        $res = $pdf->getJsColorString('cmyka(67,33,0,25,0.85)');
        $this->assertEquals('["CMYK",0.670000,0.330000,0.000000,0.250000]', $res);
        $res = $pdf->getJsColorString('cmyka(67%,33%,0,25%,0.85)');
        $this->assertEquals('["CMYK",0.670000,0.330000,0.000000,0.250000]', $res);
        $res = $pdf->getJsColorString('lab(52% 0 -39)');
        $this->assertEquals('["RGB",0.252784,0.499848,0.747328]', $res);
        $res = $pdf->getJsColorString('lab(52% 0 -39 / 0.85)');
        $this->assertEquals('["RGB",0.252784,0.499848,0.747328]', $res);
        $res = $pdf->getJsColorString('g(-)');
        $this->assertEquals('color.transparent', $res);
        $res = $pdf->getJsColorString('rgb(-)');
        $this->assertEquals('color.transparent', $res);
        $res = $pdf->getJsColorString('hsl(-)');
        $this->assertEquals('color.transparent', $res);
        $res = $pdf->getJsColorString('cmyk(-)');
        $this->assertEquals('color.transparent', $res);
    }

    public function testGetColorObject(): void
    {
        $pdf = $this->getTestObject();
        $res = $pdf->getColorObject('');
        $this->assertNull($res);
        $res = $pdf->getColorObject('[*');
        $this->assertNull($res);
        $res = $pdf->getColorObject('t()');
        $this->assertNull($res);
        $res = $pdf->getColorObject('["T"]');
        $this->assertNull($res);
        $res = $pdf->getColorObject('transparent');
        $this->assertNull($res);
        $res = $pdf->getColorObject('color.transparent');
        $this->assertNull($res);
        $res = $pdf->getColorObject('#1a2b3c4d');
        $this->assertNotNull($res);
        $this->assertEquals('#1a2b3c4d', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('#1a2b3c');
        $this->assertNotNull($res);
        $this->assertEquals('#1a2b3cff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('#1234');
        $this->assertNotNull($res);
        $this->assertEquals('#11223344', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('#123');
        $this->assertNotNull($res);
        $this->assertEquals('#112233ff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('["G",0.5]');
        $this->assertNotNull($res);
        $this->assertEquals('#808080ff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('["RGB",0.25,0.50,0.75]');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('["CMYK",0.666,0.333,0,0.25]');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('g(50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#808080ff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('g(128)');
        $this->assertNotNull($res);
        $this->assertEquals('#808080ff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('rgb(25%,50%,75%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('rgb(64,128,191)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('rgba(25%,50%,75%,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfd9', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('rgba(64,128,191,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfd9', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('hsl(210,50%,50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('hsla(210,50%,50%,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfd9', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('cmyk(67%,33%,0,25%)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('cmyk(67,33,0,25)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('cmyka(67,33,0,25,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfd9', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('cmyka(67%,33%,0,25%,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfd9', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('lab(52% 0 -39)');
        $this->assertNotNull($res);
        $this->assertEquals('#407fbfff', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('lab(52% 0 -39 / 0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#407fbfd9', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('lab(52 0 -39 / 85%)');
        $this->assertNotNull($res);
        $this->assertEquals('#407fbfd9', $res->getRgbaHexColor());
        $res = $pdf->getColorObject('none');
        $this->assertNotNull($res);
        $this->assertEquals('0.000000 0.000000 0.000000 0.000000 k' . "\n", $res->getPdfColor());
        $res = $pdf->getColorObject('all');
        $this->assertNotNull($res);
        $this->assertEquals('1.000000 1.000000 1.000000 1.000000 k' . "\n", $res->getPdfColor());
        $res = $pdf->getColorObject('["G"]');
        $this->assertNull($res);
        $res = $pdf->getColorObject('["RGB"]');
        $this->assertNull($res);
        $res = $pdf->getColorObject('["CMYK"]');
        $this->assertNull($res);
        $res = $pdf->getColorObject('g(-)');
        $this->assertNull($res);
        $res = $pdf->getColorObject('rgb(-)');
        $this->assertNull($res);
        $res = $pdf->getColorObject('hsl(-)');
        $this->assertNull($res);
        $res = $pdf->getColorObject('cmyk(-)');
        $this->assertNull($res);
    }

    public function testGetPdfColor(): void
    {
        $pdf = $this->getTestObject();
        $res = $pdf->getPdfColor('magenta', false, 1);
        $this->assertEquals('/CS1 cs 1.000000 scn' . "\n", $res);
        $res = $pdf->getPdfColor('magenta', true, 1);
        $this->assertEquals('/CS1 CS 1.000000 SCN' . "\n", $res);
        $res = $pdf->getPdfColor('magenta', false, 0.5);
        $this->assertEquals('/CS1 cs 0.500000 scn' . "\n", $res);
        $res = $pdf->getPdfColor('magenta', true, 0.5);
        $this->assertEquals('/CS1 CS 0.500000 SCN' . "\n", $res);

        $res = $pdf->getPdfColor('t()', false, 1);
        $this->assertEquals('', $res);
        $res = $pdf->getPdfColor('["T"]', false, 1);
        $this->assertEquals('', $res);
        $res = $pdf->getPdfColor('transparent', false, 1);
        $this->assertEquals('', $res);
        $res = $pdf->getPdfColor('color.transparent', false, 1);
        $this->assertEquals('', $res);
        $res = $pdf->getPdfColor('magenta', false, 1);
        $this->assertEquals('/CS1 cs 1.000000 scn' . "\n", $res);
        $res = $pdf->getPdfColor('#1a2b3c4d', false, 1);
        $this->assertEquals('0.101961 0.168627 0.235294 rg' . "\n", $res);
        $res = $pdf->getPdfColor('#1a2b3c', false, 1);
        $this->assertEquals('0.101961 0.168627 0.235294 rg' . "\n", $res);
        $res = $pdf->getPdfColor('#1234', false, 1);
        $this->assertEquals('0.066667 0.133333 0.200000 rg' . "\n", $res);
        $res = $pdf->getPdfColor('#123', false, 1);
        $this->assertEquals('0.066667 0.133333 0.200000 rg' . "\n", $res);
        $res = $pdf->getPdfColor('["G",0.5]', false, 1);
        $this->assertEquals('0.500000 g' . "\n", $res);
        $res = $pdf->getPdfColor('["RGB",0.25,0.50,0.75]', false, 1);
        $this->assertEquals('0.250000 0.500000 0.750000 rg' . "\n", $res);
        $res = $pdf->getPdfColor('["CMYK",0.666,0.333,0,0.25]', false, 1);
        $this->assertEquals('0.666000 0.333000 0.000000 0.250000 k' . "\n", $res);
        $res = $pdf->getPdfColor('g(50%)', false, 1);
        $this->assertEquals('0.500000 g' . "\n", $res);
        $res = $pdf->getPdfColor('g(128)', false, 1);
        $this->assertEquals('0.501961 g' . "\n", $res);
        $res = $pdf->getPdfColor('rgb(25%,50%,75%)', false, 1);
        $this->assertEquals('0.250000 0.500000 0.750000 rg' . "\n", $res);
        $res = $pdf->getPdfColor('rgb(64,128,191)', false, 1);
        $this->assertEquals('0.250980 0.501961 0.749020 rg' . "\n", $res);
        $res = $pdf->getPdfColor('rgba(25%,50%,75%,0.85)', false, 1);
        $this->assertEquals('0.250000 0.500000 0.750000 rg' . "\n", $res);
        $res = $pdf->getPdfColor('rgba(64,128,191,0.85)', false, 1);
        $this->assertEquals('0.250980 0.501961 0.749020 rg' . "\n", $res);
        $res = $pdf->getPdfColor('hsl(210,50%,50%)', false, 1);
        $this->assertEquals('0.250000 0.500000 0.750000 rg' . "\n", $res);
        $res = $pdf->getPdfColor('hsla(210,50%,50%,0.85)', false, 1);
        $this->assertEquals('0.250000 0.500000 0.750000 rg' . "\n", $res);
        $res = $pdf->getPdfColor('cmyk(67%,33%,0,25%)', false, 1);
        $this->assertEquals('0.670000 0.330000 0.000000 0.250000 k' . "\n", $res);
        $res = $pdf->getPdfColor('cmyk(67,33,0,25)', false, 1);
        $this->assertEquals('0.670000 0.330000 0.000000 0.250000 k' . "\n", $res);
        $res = $pdf->getPdfColor('cmyka(67,33,0,25,0.85)', false, 1);
        $this->assertEquals('0.670000 0.330000 0.000000 0.250000 k' . "\n", $res);
        $res = $pdf->getPdfColor('cmyka(67%,33%,0,25%,0.85)', false, 1);
        $this->assertEquals('0.670000 0.330000 0.000000 0.250000 k' . "\n", $res);
        $res = $pdf->getPdfColor('lab(52% 0 -39)', false, 1);
        $this->assertEquals('0.252784 0.499848 0.747328 rg' . "\n", $res);
        $res = $pdf->getPdfColor('lab(52% 0 -39 / 0.85)', false, 1);
        $this->assertEquals('0.252784 0.499848 0.747328 rg' . "\n", $res);
        $res = $pdf->getPdfColor('g(-)');
        $this->assertEquals('', $res);
        $res = $pdf->getPdfColor('rgb(-)');
        $this->assertEquals('', $res);
        $res = $pdf->getPdfColor('hsl(-)');
        $this->assertEquals('', $res);
        $res = $pdf->getPdfColor('cmyk(-)');
        $this->assertEquals('', $res);
    }

    public function testGetPdfRgbComponents(): void
    {
        $pdf = $this->getTestObject();
        $res = $pdf->getPdfRgbComponents('');
        $this->assertEquals('', $res);

        $res = $pdf->getPdfRgbComponents('red');
        $this->assertEquals('1.000000 0.000000 0.000000', $res);

        $res = $pdf->getPdfRgbComponents('#00ff00');
        $this->assertEquals('0.000000 1.000000 0.000000', $res);

        $res = $pdf->getPdfRgbComponents('rgb(0,0,255)');
        $this->assertEquals('0.000000 0.000000 1.000000', $res);
    }

    public function testGetPdfStrokeColor(): void
    {
        $pdf = $this->getTestObject();

        $res = $pdf->getPdfStrokeColor('magenta', 0.5);
        $this->assertEquals('/CS1 CS 0.500000 SCN' . "\n", $res);

        $res = $pdf->getPdfStrokeColor('rgb(64,128,191)');
        $this->assertEquals('0.250980 0.501961 0.749020 RG' . "\n", $res);

        $res = $pdf->getPdfStrokeColor('g(-)');
        $this->assertEquals('', $res);
    }

    public function testGetPdfFillColor(): void
    {
        $pdf = $this->getTestObject();

        $res = $pdf->getPdfFillColor('magenta', 0.5);
        $this->assertEquals('/CS1 cs 0.500000 scn' . "\n", $res);

        $res = $pdf->getPdfFillColor('rgb(64,128,191)');
        $this->assertEquals('0.250980 0.501961 0.749020 rg' . "\n", $res);

        $res = $pdf->getPdfFillColor('g(-)');
        $this->assertEquals('', $res);
    }

    public function testGetPdfCmykComponents(): void
    {
        $pdf = $this->getTestObject();
        $res = $pdf->getPdfCmykComponents('');
        $this->assertEquals('', $res);

        $res = $pdf->getPdfCmykComponents('red');
        $this->assertEquals('0.000000 1.000000 1.000000 0.000000', $res);

        $res = $pdf->getPdfCmykComponents('rgb(64,128,191)');
        $this->assertEquals('0.664921 0.329843 0.000000 0.250980', $res);

        $res = $pdf->getPdfCmykComponents('cyan');
        $this->assertEquals('1.000000 0.000000 0.000000 0.000000', $res);
    }

    public function testReadAccessorsDoNotRegisterSpotColors(): void
    {
        $pdf = $this->getTestObject();

        // Querying components for a default spot-color name must NOT register a
        // spot color (which would otherwise leak into the PDF spot resources).
        $pdf->getColorObject('red');
        $pdf->getPdfRgbComponents('green');
        $pdf->getPdfCmykComponents('cyan');
        $this->assertSame([], $pdf->getSpotColors());

        $pon = 0;
        $this->assertSame('', $pdf->getPdfSpotObjects($pon));
        $this->assertSame(0, $pon);
        $this->assertSame('', $pdf->getPdfSpotResources());

        // getPdfColor() is the explicit "emit a separation" path and DOES register.
        $pdf->getPdfColor('magenta');
        $this->assertArrayHasKey('magenta', $pdf->getSpotColors());
    }

    public function testGetColorObjectResolvesRegisteredSpotColor(): void
    {
        $pdf = $this->getTestObject();

        // A custom spot color is not part of the default spot color table, so the
        // read-only accessor can only resolve it from the internal registry. This
        // exercises the "already registered" early return of resolveSpotColorData().
        $cmyk = new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => 0.1,
            'magenta' => 0.2,
            'yellow' => 0.3,
            'key' => 0.4,
            'alpha' => 1.0,
        ]);
        $pdf->addSpotColor('My Custom Spot', $cmyk);

        $res = $pdf->getColorObject('My Custom Spot');
        $this->assertNotNull($res);
        $this->assertSame($cmyk, $res);

        // The read-only lookup must not have added or duplicated any registration.
        $spotColors = $pdf->getSpotColors();
        $this->assertCount(1, $spotColors);
        $this->assertArrayHasKey('mycustomspot', $spotColors);
    }
}
