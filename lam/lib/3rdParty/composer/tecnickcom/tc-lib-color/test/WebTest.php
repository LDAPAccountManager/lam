<?php

/**
 * WebTest.php
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

use PHPUnit\Framework\Attributes\DataProvider;

/**
 * Web Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class WebTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Web
    {
        return new \Com\Tecnick\Color\Web();
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetHexFromName(): void
    {
        $web = $this->getTestObject();
        $res = $web->getHexFromName('aliceblue');
        $this->assertEquals('f0f8ffff', $res);
        $res = $web->getHexFromName('color.yellowgreen');
        $this->assertEquals('9acd32ff', $res);
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetHexFromNameInvalid(): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        $web->getHexFromName('invalid');
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetNameFromHex(): void
    {
        $web = $this->getTestObject();
        $res = $web->getNameFromHex('f0f8ffff');
        $this->assertEquals('aliceblue', $res);
        $res = $web->getNameFromHex('9acd32ff');
        $this->assertEquals('yellowgreen', $res);
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetNameFromHexBad(): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        $web->getNameFromHex('012345');
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testExtractHexCode(): void
    {
        $web = $this->getTestObject();
        $res = $web->extractHexCode('abc');
        $this->assertEquals('aabbccff', $res);
        $res = $web->extractHexCode('#abc');
        $this->assertEquals('aabbccff', $res);
        $res = $web->extractHexCode('abcd');
        $this->assertEquals('aabbccdd', $res);
        $res = $web->extractHexCode('#abcd');
        $this->assertEquals('aabbccdd', $res);
        $res = $web->extractHexCode('112233');
        $this->assertEquals('112233ff', $res);
        $res = $web->extractHexCode('#112233');
        $this->assertEquals('112233ff', $res);
        $res = $web->extractHexCode('11223344');
        $this->assertEquals('11223344', $res);
        $res = $web->extractHexCode('#11223344');
        $this->assertEquals('11223344', $res);
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testExtractHexCodeBad(): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        $web->extractHexCode('');
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testExtractHexCodeInvalidLength(): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        // 5-character hex code is accepted by regex but not handled by switch
        $web->extractHexCode('12345');
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testExtractHexCodeInvalidLength7(): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        // 7-character hex code is accepted by regex but not handled by switch
        $web->extractHexCode('1234567');
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetRgbObjFromHex(): void
    {
        $web = $this->getTestObject();
        $rgb = $web->getRgbObjFromHex('#87ceebff');
        $this->assertEquals('#87ceebff', $rgb->getRgbaHexColor());
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetRgbObjFromHexBad(): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        $web->getRgbObjFromHex('xx');
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetRgbObjFromName(): void
    {
        $web = $this->getTestObject();
        $rgb = $web->getRgbObjFromName('skyblue');
        $this->assertEquals('#87ceebff', $rgb->getRgbaHexColor());
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetRgbObjFromNameBad(): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        $web->getRgbObjFromName('xx');
    }

    public function testNormalizeValue(): void
    {
        $web = $this->getTestObject();
        $res = $web->normalizeValue('50%', 50);
        $this->assertEquals(0.5, $res);
        $res = $web->normalizeValue(128, 255);
        $this->bcAssertEqualsWithDelta(0.5, $res);
    }

    public function testNormalizeValueInvalidPercent(): void
    {
        $web = $this->getTestObject();
        $this->assertSame(0.0, $web->normalizeValue('abc%', 255));
    }

    public function testNormalizeValueUnsupportedType(): void
    {
        $web = $this->getTestObject();
        $this->assertSame(0.0, $web->normalizeValue([], 255));
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetColorObj(): void
    {
        $web = $this->getTestObject();
        $res = $web->getColorObj('');
        $this->assertNull($res);
        $res = $web->getColorObj('t()');
        $this->assertNull($res);
        $res = $web->getColorObj('["T"]');
        $this->assertNull($res);
        $res = $web->getColorObj('transparent');
        $this->assertNull($res);
        $res = $web->getColorObj('color.transparent');
        $this->assertNull($res);
        $res = $web->getColorObj('royalblue');
        $this->assertNotNull($res);
        $this->assertEquals('#4169e1ff', $res->getRgbaHexColor());
        $res = $web->getColorObj('#1a2b3c4d');
        $this->assertNotNull($res);
        $this->assertEquals('#1a2b3c4d', $res->getRgbaHexColor());
        $res = $web->getColorObj('#1a2b3c');
        $this->assertNotNull($res);
        $this->assertEquals('#1a2b3cff', $res->getRgbaHexColor());
        $res = $web->getColorObj('#1234');
        $this->assertNotNull($res);
        $this->assertEquals('#11223344', $res->getRgbaHexColor());
        $res = $web->getColorObj('#123');
        $this->assertNotNull($res);
        $this->assertEquals('#112233ff', $res->getRgbaHexColor());
        $res = $web->getColorObj('["G",0.5]');
        $this->assertNotNull($res);
        $this->assertEquals('#808080ff', $res->getRgbaHexColor());
        $res = $web->getColorObj('["RGB",0.25,0.50,0.75]');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('["CMYK",0.666,0.333,0,0.25]');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('g(50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#808080ff', $res->getRgbaHexColor());
        $res = $web->getColorObj('g(128)');
        $this->assertNotNull($res);
        $this->assertEquals('#808080ff', $res->getRgbaHexColor());
        $res = $web->getColorObj('rgb(25%,50%,75%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('rgb(64,128,191)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('rgba(25%,50%,75%,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfd9', $res->getRgbaHexColor());
        $res = $web->getColorObj('rgba(64,128,191,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfd9', $res->getRgbaHexColor());
        $res = $web->getColorObj('hsl(210,50%,50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        // saturation/lightness without "%" are treated as CSS percentages (÷100),
        // so the bare form is equivalent to the "%" form.
        $res = $web->getColorObj('hsl(210,50,50)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        // decimal percentages are accepted
        $res = $web->getColorObj('hsl(210,50.0%,50.0%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('hsla(210,50%,50%,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfd9', $res->getRgbaHexColor());
        $res = $web->getColorObj('cmyk(67%,33%,0,25%)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('cmyk(67,33,0,25)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('cmyka(67,33,0,25,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfd9', $res->getRgbaHexColor());
        $res = $web->getColorObj('cmyka(67%,33%,0,25%,0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfd9', $res->getRgbaHexColor());
        // decimal percentages are accepted for CMYK components
        $res = $web->getColorObj('cmyk(66.5%,33%,0,25%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('lab(52% 0 -39)');
        $this->assertNotNull($res);
        $this->assertEquals('#407fbfff', $res->getRgbaHexColor());
        $res = $web->getColorObj('lab(52% 0 -39 / 0.85)');
        $this->assertNotNull($res);
        $this->assertEquals('#407fbfd9', $res->getRgbaHexColor());
        $res = $web->getColorObj('lab(52 0 -39 / 85%)');
        $this->assertNotNull($res);
        $this->assertEquals('#407fbfd9', $res->getRgbaHexColor());
    }

    /**
     * Modern CSS syntax: space-separated components and percentage alpha.
     *
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetColorObjModernSyntax(): void
    {
        $web = $this->getTestObject();

        // space-separated components are equivalent to the comma-separated form
        $res = $web->getColorObj('rgb(64 128 191)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());

        $res = $web->getColorObj('hsl(210 50% 50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bfff', $res->getRgbaHexColor());

        $res = $web->getColorObj('cmyk(67% 33% 0% 25%)');
        $this->assertNotNull($res);
        $this->assertEquals('#3f80bfff', $res->getRgbaHexColor());

        // percentage alpha is accepted (50% == 0.5)
        $res = $web->getColorObj('rgba(64,128,191,50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bf80', $res->getRgbaHexColor());

        // modern space syntax with a "/" alpha separator
        $res = $web->getColorObj('rgb(64 128 191 / 50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bf80', $res->getRgbaHexColor());

        $res = $web->getColorObj('hsla(210,50%,50%,50%)');
        $this->assertNotNull($res);
        $this->assertEquals('#4080bf80', $res->getRgbaHexColor());
    }

    /**
     * @return array<string[]>
     */
    public static function getBadColor(): array
    {
        return [['g(-)'], ['rgb(-)'], ['hsl(-)'], ['cmyk(-)'], ['lab(-)']];
    }

    /**
     * @throws \Com\Tecnick\Color\Exception
     */
    #[DataProvider('getBadColor')]
    public function testGetColorObjBad(string $bad): void
    {
        $this->bcExpectException(\Com\Tecnick\Color\Exception::class);
        $web = $this->getTestObject();
        $web->getColorObj($bad);
    }

    public function testTryGetColorObj(): void
    {
        $web = $this->getTestObject();

        $ok = $web->tryGetColorObj('royalblue');
        $this->assertNotNull($ok);
        $this->assertEquals('#4169e1ff', $ok->getRgbaHexColor());

        $bad = $web->tryGetColorObj('g(-)');
        $this->assertNull($bad);
    }

    public function testGetRgbSquareDistance(): void
    {
        $web = $this->getTestObject();
        $cola = [
            'red' => 0,
            'green' => 0,
            'blue' => 0,
        ];
        $colb = [
            'red' => 1,
            'green' => 1,
            'blue' => 1,
        ];
        $dist = $web->getRgbSquareDistance($cola, $colb);
        $this->assertEquals(3, $dist);

        $cola = [
            'red' => 0.5,
            'green' => 0.5,
            'blue' => 0.5,
        ];
        $colb = [
            'red' => 0.5,
            'green' => 0.5,
            'blue' => 0.5,
        ];
        $dist = $web->getRgbSquareDistance($cola, $colb);
        $this->assertEquals(0, $dist);

        $cola = [
            'red' => 0.25,
            'green' => 0.50,
            'blue' => 0.75,
        ];
        $colb = [
            'red' => 0.50,
            'green' => 0.75,
            'blue' => 1.00,
        ];
        $dist = $web->getRgbSquareDistance($cola, $colb);
        $this->assertEquals(0.1875, $dist);
    }

    public function testGetClosestWebColor(): void
    {
        $web = $this->getTestObject();
        $col = [
            'red' => 1,
            'green' => 0,
            'blue' => 0,
        ];
        $color = $web->getClosestWebColor($col);
        $this->assertEquals('red', $color);

        $col = [
            'red' => 0,
            'green' => 1,
            'blue' => 0,
        ];
        $color = $web->getClosestWebColor($col);
        $this->assertEquals('lime', $color);

        $col = [
            'red' => 0,
            'green' => 0,
            'blue' => 1,
        ];
        $color = $web->getClosestWebColor($col);
        $this->assertEquals('blue', $color);

        $col = [
            'red' => 0.33,
            'green' => 0.4,
            'blue' => 0.18,
        ];
        $color = $web->getClosestWebColor($col);
        $this->assertEquals('darkolivegreen', $color);

        // On an exact tie between duplicate hex codes (aqua/cyan both 00ffff),
        // the first/canonical name is returned.
        $col = [
            'red' => 0,
            'green' => 1,
            'blue' => 1,
        ];
        $color = $web->getClosestWebColor($col);
        $this->assertEquals('aqua', $color);
    }

    public function testGetClosestWebColorFromString(): void
    {
        $web = $this->getTestObject();

        $color = $web->getClosestWebColorFromString('rgb(255,0,0)');
        $this->assertEquals('red', $color);

        $color = $web->getClosestWebColorFromString('#0000ff');
        $this->assertEquals('blue', $color);

        $color = $web->getClosestWebColorFromString('invalid-color-value');
        $this->assertEquals('', $color);
    }
}
