<?php

/**
 * ConvertTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Test;

use Com\Tecnick\Unicode\Data\Latin;
use PHPUnit\Framework\Attributes\DataProvider;
use PHPUnit\Framework\TestCase;

/**
 * Convert Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class ConvertTest extends TestCase
{
    protected function getTestObject(): \Com\Tecnick\Unicode\Convert
    {
        return new \Com\Tecnick\Unicode\Convert();
    }

    private static function decodeJsonString(string $json): string
    {
        /** @var string */
        return \json_decode($json);
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    #[DataProvider('chrDataProvider')]
    public function testChr(int $ord, mixed $expected): void
    {
        $convert = $this->getTestObject();
        $chr = $convert->chr($ord);
        $this->assertEquals($expected, $chr);
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    #[DataProvider('chrDataProvider')]
    public function testOrd(mixed $expected, string $chr): void
    {
        $convert = $this->getTestObject();
        $ord = $convert->ord($chr);
        $this->assertEquals($expected, $ord);
    }

    /**
     * @return array<int, array{0:int,1:string}>
     */
    public static function chrDataProvider(): array
    {
        return [
            [32, ' '],
            [48, '0'],
            [65, 'A'],
            [182, '¶'],
            [255, 'ÿ'],
            [256, 'Ā'],
            [544, 'Ƞ'],
            [916, 'Δ'],
            [1488, 'א'],
            [21488, '台'],
            [49436, '서'],
            [70039, '𑆗'],
            [195101, '𪘀'],
        ];
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testStrToChrArr(): void
    {
        $convert = $this->getTestObject();
        $res = $convert->strToChrArr('0A¶ÿĀȠΔא台서');
        $this->assertEquals(['0', 'A', '¶', 'ÿ', 'Ā', 'Ƞ', 'Δ', 'א', '台', '서'], $res);
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testChrArrToOrdArr(): void
    {
        $convert = $this->getTestObject();
        $res = $convert->chrArrToOrdArr(['0', 'A', '¶', 'ÿ', 'Ā', 'Ƞ', 'Δ', 'א', '台', '서']);
        $this->assertEquals([48, 65, 182, 255, 256, 544, 916, 1488, 21488, 49436], $res);
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testOrdArrToChrArr(): void
    {
        $convert = $this->getTestObject();
        $res = $convert->ordArrToChrArr([48, 65, 182, 255, 256, 544, 916, 1488, 21488, 49436]);
        $this->assertEquals(['0', 'A', '¶', 'ÿ', 'Ā', 'Ƞ', 'Δ', 'א', '台', '서'], $res);
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testStrToOrdArr(): void
    {
        $convert = $this->getTestObject();
        $res = $convert->strToOrdArr('0A¶ÿĀȠΔא台서');
        $this->assertEquals([48, 65, 182, 255, 256, 544, 916, 1488, 21488, 49436], $res);
    }

    /**
     * Empty inputs must short-circuit to an empty array in every direction.
     *
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testConvertArrEmpty(): void
    {
        $convert = $this->getTestObject();
        $this->assertSame([], $convert->strToOrdArr(''));
        $this->assertSame([], $convert->ordArrToChrArr([]));
        $this->assertSame([], $convert->chrArrToOrdArr([]));
    }

    /**
     * Supplementary-plane (4-byte) code points must round-trip through every conversion.
     *
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testConvertArrSupplementaryPlane(): void
    {
        $convert = $this->getTestObject();
        // Includes two supplementary-plane (4-byte UTF-8) code points: U+11197 and U+2A600.
        $str = "A\u{11197}\u{00B6}\u{2A600}\u{53F0}";
        $ords = [65, 70039, 182, 173568, 21488];
        $chrs = ['A', "\u{11197}", "\u{00B6}", "\u{2A600}", "\u{53F0}"];

        $this->assertSame($ords, $convert->strToOrdArr($str));
        $this->assertSame($chrs, $convert->ordArrToChrArr($ords));
        $this->assertSame($ords, $convert->chrArrToOrdArr($chrs));
        $this->assertSame($str, \implode('', $convert->ordArrToChrArr($convert->strToOrdArr($str))));
    }

    /**
     * Malformed UTF-8 is substituted with U+003F rather than throwing, matching mbstring's
     * default substitution for the bulk conversion.
     *
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testStrToOrdArrSubstitutesMalformed(): void
    {
        $convert = $this->getTestObject();
        $this->assertSame([63, 63], $convert->strToOrdArr("\xff\xfe"));
    }

    public function testGetSubUniArrStr(): void
    {
        $convert = $this->getTestObject();
        $res = $convert->getSubUniArrStr(['0', 'A', '¶', 'ÿ', 'Ā', 'Ƞ', 'Δ', 'א', '台', '서']);
        $this->assertEquals('0A¶ÿĀȠΔא台서', $res);

        $res = $convert->getSubUniArrStr(['0', 'A', '¶', 'ÿ', 'Ā', 'Ƞ', 'Δ', 'א', '台', '서'], 2, 8);
        $this->assertEquals('¶ÿĀȠΔא', $res);
    }

    public function testUniArrToLatinArr(): void
    {
        $convert = $this->getTestObject();
        $uniarr = \array_keys(Latin::SUBSTITUTE);
        $uniarr[] = 65533; // 0xFFFD - character to ignore
        $uniarr[] = 123456; // undefined char
        $uniarr[] = 65; // ASCII char
        $latarr = \array_values(Latin::SUBSTITUTE);
        $latarr[] = 63;
        $latarr[] = 65;
        $res = $convert->uniArrToLatinArr($uniarr);
        $this->assertEquals($latarr, $res);
    }

    public function testLatinArrToStr(): void
    {
        $convert = $this->getTestObject();
        $res = $convert->latinArrToStr([48, 57, 65, 90, 97, 122]);
        $this->assertEquals('09AZaz', $res);
    }

    #[DataProvider('strToHexDataProvider')]
    public function testStrToHex(string $str, mixed $hex): void
    {
        $convert = $this->getTestObject();
        $res = $convert->strToHex($str);
        $this->assertEquals($hex, $res);
    }

    #[DataProvider('strToHexDataProvider')]
    public function testHexToStr(mixed $str, string $hex): void
    {
        $convert = $this->getTestObject();
        $res = $convert->hexToStr($hex);
        $this->assertEquals($str, $res);
    }

    /**
     * @return array<int, array<string>>
     */
    public static function strToHexDataProvider(): array
    {
        return [
            ['', ''],
            ['A', '41'],
            ['AB', '4142'],
            ['ABC', '414243'],
            ["\n", '0a'],
        ];
    }

    #[DataProvider('toUTF16BEDataProvider')]
    public function testToUTF16BE(string $str, mixed $exp): void
    {
        $convert = $this->getTestObject();
        $res = $convert->toUTF16BE($str);
        $this->assertEquals($exp, $convert->strToHex($res));
    }

    /**
     * @return array<int, array<string>>
     */
    public static function toUTF16BEDataProvider(): array
    {
        return [
            ['', ''],
            ['ABC', '004100420043'],
            [self::decodeJsonString('"\u0010\uffff\u00ff\uff00"'), '0010ffff00ffff00'],
        ];
    }

    #[DataProvider('toUTF8DataProvider')]
    public function testToUTF8(string $str, mixed $exp, ?string $enc = null): void
    {
        $convert = $this->getTestObject();
        $res = $convert->toUTF8($str, $enc);
        $this->assertEquals($exp, $res);
    }

    /**
     * @return array<int, array<string>>
     */
    public static function toUTF8DataProvider(): array
    {
        return [
            ['', ''],
            ['òèìòù', 'òèìòù'],
            ['òèìòù', 'Ã²Ã¨Ã¬Ã²Ã¹', 'ISO-8859-1'],
        ];
    }

    public function testToUTF8WithUndetectableEncoding(): void
    {
        // BASE64 and HTML-ENTITIES cannot detect a plain ASCII string,
        // causing mb_detect_encoding() to return false; the method must
        // handle this gracefully by falling back to a null source encoding.
        $convert = $this->getTestObject();
        $res = $convert->toUTF8('abc', ['BASE64', 'HTML-ENTITIES']);
        $this->assertEquals('abc', $res);
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testOrdException(): void
    {
        $this->expectException(\Com\Tecnick\Unicode\Exception::class);
        $convert = $this->getTestObject();
        // An empty string produces a zero-length UCS-4BE buffer,
        // making unpack() fail and triggering the exception.
        $convert->ord('');
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testStrToChrArrException(): void
    {
        $this->expectException(\Com\Tecnick\Unicode\Exception::class);
        $convert = $this->getTestObject();
        // Invalid UTF-8 bytes cause preg_split() with the /u flag to return
        // false, triggering the exception.
        $convert->strToChrArr("\xff\xfe");
    }
}
