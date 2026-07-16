<?php

/**
 * CoreMetricsTest.php
 *
 * @since     2026-05-05
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Test\Import;

use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\Import\Core;
use PHPUnit\Framework\Attributes\DataProvider;
use PHPUnit\Framework\TestCase;

/**
 * Verifies that Import\Core correctly maps AFM widths to WinAnsi byte positions (cw)
 * and to Unicode codepoints (cwu) for Helvetica.
 *
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 */
class CoreMetricsTest extends TestCase
{
    private const HELVETICA_AFM = __DIR__ . '/../../util/vendor/tecnickcom/tc-font-mirror/core/Helvetica.afm';

    /** @var TFontData */
    private static array $fdtTemplate = [
        'Ascender' => 0,
        'Ascent' => 0,
        'AvgWidth' => 0.0,
        'CapHeight' => 0,
        'CharacterSet' => '',
        'Descender' => 0,
        'Descent' => 0,
        'EncodingScheme' => '',
        'FamilyName' => '',
        'Flags' => 0,
        'FontBBox' => [],
        'FontName' => '',
        'FullName' => '',
        'IsFixedPitch' => false,
        'ItalicAngle' => 0,
        'Leading' => 0,
        'MaxWidth' => 0,
        'MissingWidth' => 0,
        'StdHW' => 0,
        'StdVW' => 0,
        'StemH' => 0,
        'StemV' => 0,
        'UnderlinePosition' => 0,
        'UnderlineThickness' => 0,
        'Version' => '',
        'Weight' => '',
        'XHeight' => 0,
        'bbox' => '',
        'cbbox' => [],
        'cidinfo' => ['Ordering' => '', 'Registry' => '', 'Supplement' => 0, 'uni2cid' => []],
        'compress' => false,
        'ctg' => '',
        'ctgdata' => [],
        'cw' => [],
        'cwu' => [],
        'datafile' => '',
        'desc' => [
            'Ascent' => 0,
            'AvgWidth' => 0,
            'CapHeight' => 0,
            'Descent' => 0,
            'Flags' => 0,
            'FontBBox' => '',
            'ItalicAngle' => 0,
            'Leading' => 0,
            'MaxWidth' => 0,
            'MissingWidth' => 0,
            'StemH' => 0,
            'StemV' => 0,
            'XHeight' => 0,
        ],
        'diff' => '',
        'diff_n' => 0,
        'dir' => '',
        'dw' => 0,
        'enc' => '',
        'enc_map' => [],
        'encodingTables' => [],
        'encoding_id' => 0,
        'encrypted' => '',
        'fakestyle' => false,
        'family' => '',
        'file' => '',
        'file_n' => 0,
        'file_name' => '',
        'i' => 0,
        'ifile' => '',
        'indexToLoc' => [],
        'input_file' => '',
        'isUnicode' => false,
        'italicAngle' => 0,
        'key' => '',
        'lenIV' => 0,
        'length1' => 0,
        'length2' => 0,
        'linked' => false,
        'mode' => [
            'bold' => false,
            'italic' => false,
            'linethrough' => false,
            'overline' => false,
            'underline' => false,
        ],
        'n' => 0,
        'name' => '',
        'numGlyphs' => 0,
        'numHMetrics' => 0,
        'originalsize' => 0,
        'pdfa' => false,
        'platform_id' => 0,
        'settype' => '',
        'short_offset' => false,
        'size1' => 0,
        'size2' => 0,
        'style' => '',
        'subset' => false,
        'subsetchars' => [],
        'table' => [],
        'tot_num_glyphs' => 0,
        'type' => '',
        'underlinePosition' => 0,
        'underlineThickness' => 0,
        'unicode' => false,
        'unitsPerEm' => 0,
        'up' => 0,
        'urk' => 0.0,
        'ut' => 0,
        'weight' => '',
    ];

    /** @return array<string, array{int, int}> */
    public static function provideHelveticaWinAnsiWidths(): array
    {
        return [
            // [ winansi_cid, expected_width ]
            // Printable ASCII — unchanged by fix, sanity checks
            'space' => [0x20, 278],
            'A' => [0x41, 667],
            'hyphen' => [0x2D, 333],
            // WinAnsi 0x80–0x9F range (formerly wrong/missing)
            'Euro' => [0x80, 556],
            'quotesinglbase' => [0x82, 222],
            'florin' => [0x83, 556],
            'quotedblbase' => [0x84, 333],
            'ellipsis' => [0x85, 1000],
            'dagger' => [0x86, 556],
            'daggerdbl' => [0x87, 556],
            'circumflex' => [0x88, 333],
            'perthousand' => [0x89, 1000],
            'Scaron' => [0x8A, 667],
            'guilsinglleft' => [0x8B, 333],
            'OE' => [0x8C, 1000],
            'Zcaron' => [0x8E, 611],
            'quoteleft' => [0x91, 222],
            'quoteright' => [0x92, 222],
            'quotedblleft' => [0x93, 333],
            'quotedblright' => [0x94, 333],
            'bullet' => [0x95, 350],
            'endash' => [0x96, 556],
            'emdash' => [0x97, 1000],
            'tilde' => [0x98, 333],
            'trademark' => [0x99, 1000],
            'scaron' => [0x9A, 500],
            'guilsinglright' => [0x9B, 333],
            'oe' => [0x9C, 944],
            'zcaron' => [0x9E, 500],
            'Ydieresis' => [0x9F, 667],
        ];
    }

    /** @throws FontException */
    #[DataProvider('provideHelveticaWinAnsiWidths')]
    public function testHelveticaWinAnsiWidth(int $cid, int $expected): void
    {
        $fdt = $this->getHelveticaMetrics();
        $this->assertSame($expected, $fdt['cw'][$cid] ?? null, 'cw[0x' . \dechex($cid) . ']');
    }

    /** @return array<string, array{int, int}> */
    public static function provideHelveticaUnicodeWidths(): array
    {
        return [
            // [ unicode_codepoint, expected_width ]
            'emdash' => [0x2014, 1000],
            'endash' => [0x2013, 556],
            'quoteleft' => [0x2018, 222],
            'quoteright' => [0x2019, 222],
            'quotedblleft' => [0x201C, 333],
            'quotedblright' => [0x201D, 333],
            'bullet' => [0x2022, 350],
            'ellipsis' => [0x2026, 1000],
            'trademark' => [0x2122, 1000],
            'Euro' => [0x20AC, 556],
            'OE' => [0x0152, 1000],
            'oe' => [0x0153, 944],
            'Scaron' => [0x0160, 667],
            'scaron' => [0x0161, 500],
            'Ydieresis' => [0x0178, 667],
            'Zcaron' => [0x017D, 611],
            'zcaron' => [0x017E, 500],
            // fi/fl are not WinAnsi-encoded but must appear in cwu
            'fi' => [0xFB01, 500],
            'fl' => [0xFB02, 500],
        ];
    }

    /** @throws FontException */
    #[DataProvider('provideHelveticaUnicodeWidths')]
    public function testHelveticaUnicodeWidth(int $codepoint, int $expected): void
    {
        $fdt = $this->getHelveticaMetrics();
        $this->assertArrayHasKey('cwu', $fdt);
        $this->assertSame(
            $expected,
            $fdt['cwu'][$codepoint] ?? null,
            'cwu[U+' . \strtoupper(\dechex($codepoint)) . ']',
        );
    }

    /** @throws FontException */
    public function testGetFontMetricsHandlesAfmWithoutCharMetrics(): void
    {
        // an AFM with zero "C" lines must not trigger a DivisionByZeroError in setCharWidths
        $afm =
            "StartFontMetrics 4.1\n"
            . "FontName TestEmpty\n"
            . "FullName Test Empty\n"
            . "FontBBox 0 -200 1000 700\n"
            . "CapHeight 700\n"
            . "Ascender 700\n"
            . "Descender -200\n"
            . "StartCharMetrics 0\n"
            . "EndCharMetrics\n"
            . "EndFontMetrics\n";

        $core = new Core($afm, self::$fdtTemplate, new \Com\Tecnick\File\File());
        $fdt = $core->getFontMetrics();

        $this->assertSame(0, $fdt['AvgWidth']);
    }

    /**
     * @return TFontData
     *
     * @throws FontException
     */
    private function getHelveticaMetrics(): array
    {
        $content = \file_get_contents(self::HELVETICA_AFM);
        $this->assertIsString($content);

        $core = new Core($content, self::$fdtTemplate, new \Com\Tecnick\File\File());

        return $core->getFontMetrics();
    }
}
