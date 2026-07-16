<?php

/**
 * StackTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Test;

use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Buffer Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @SuppressWarnings("PHPMD.LongVariable")
 */
class StackTest extends TestUtil
{
    private function prepareTestEnvironment(): void
    {
        parent::setupTest();
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStack(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(0.75, true, true, true);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $cfont = $stack->insert($objnum, 'freesans', '', 12, -0.1, 0.9, '', null);
        $this->assertNotEmpty($cfont);
        $this->assertNotEmpty($cfont['cbbox']);

        $this->bcAssertEqualsWithDelta([0.162, 0.0, 7.0308, 8.748], $stack->getCharBBox(65), 0.0001);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'pdfa/pfb/PDFATimes.pfb');
        $afont = $stack->insert($objnum, 'times', '', 14, 0.3, 1.2, '', null);
        $this->assertNotEmpty($afont);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'pdfa/pfb/PDFAHelveticaBoldOblique.pfb');
        $bfont = $stack->insert($objnum, 'helvetica', 'BIUDO', null, null, null, '', null);
        $this->assertNotEmpty($bfont);

        $this->assertEquals("BT /F3 14.000000 Tf ET\r", $bfont['out']);
        $this->assertEquals('pdfahelveticaBI', $bfont['key']);
        $this->assertEquals('Type1', $bfont['type']);
        $this->bcAssertEqualsWithDelta(14, $bfont['size'], 0.0001);
        $this->bcAssertEqualsWithDelta(0.3, $bfont['spacing'], 0.0001);
        $this->bcAssertEqualsWithDelta(1.2, $bfont['stretching'], 0.0001);
        $this->bcAssertEqualsWithDelta(18.6667, $bfont['usize'], 0.0001);
        $this->bcAssertEqualsWithDelta(0.014, $bfont['cratio'], 0.0001);
        $this->bcAssertEqualsWithDelta(-1.554, $bfont['up'], 0.0001);
        $this->bcAssertEqualsWithDelta(0.966, $bfont['ut'], 0.0001);
        $this->bcAssertEqualsWithDelta(4.6704, $bfont['dw'], 0.0001);
        $this->bcAssertEqualsWithDelta(13.342, $bfont['ascent'], 0.0001);
        $this->bcAssertEqualsWithDelta(-3.08, $bfont['descent'], 0.0001);
        $this->bcAssertEqualsWithDelta(16.422, $bfont['height'], 0.0001);
        $this->bcAssertEqualsWithDelta(5.131, $bfont['midpoint'], 0.0001);
        $this->bcAssertEqualsWithDelta(10.136, $bfont['capheight'], 0.0001);
        $this->bcAssertEqualsWithDelta(7.56, $bfont['xheight'], 0.0001);
        $this->bcAssertEqualsWithDelta(9.492, $bfont['avgwidth'], 0.0001);
        $this->bcAssertEqualsWithDelta(16.8, $bfont['maxwidth'], 0.0001);
        $this->bcAssertEqualsWithDelta(4.6704, $bfont['missingwidth'], 0.0001);
        $this->bcAssertEqualsWithDelta([-1.092, -3.08, 18.5976, 13.342], $bfont['fbbox'], 0.0001);

        $fkey = $stack->getCurrentFontKey();
        $this->assertEquals('pdfahelveticaBI', $fkey);

        $font = $stack->getCurrentFont();
        $this->assertEquals($bfont, $font);

        $this->assertTrue($stack->isCharDefined(65));
        $this->assertFalse($stack->isCharDefined(300));

        $this->assertEquals(75, $stack->replaceChar(65, 75));
        $this->assertEquals(65, $stack->replaceChar(65, 300));

        $this->assertEquals([0, 0, 0, 0], $stack->getCharBBox(300));

        $this->bcAssertEqualsWithDelta(12.1296, $stack->getCharWidth(65), 0.0001);
        $this->bcAssertEqualsWithDelta(0, $stack->getCharWidth(173), 0.0001);
        $this->bcAssertEqualsWithDelta(4.6704, $stack->getCharWidth(300), 0.0001);

        $uniarr = [65, 173, 300];
        $this->bcAssertEqualsWithDelta(17.52, $stack->getOrdArrWidth($uniarr), 0.0001);

        $subs = [
            65 => [400, 75],
            173 => [76, 300],
            300 => [400, 77],
        ];
        $this->assertEquals([65, 173, 77], $stack->replaceMissingChars($uniarr, $subs));

        $font = $stack->popLastFont();
        $this->assertEquals($bfont, $font);

        $font = $stack->getCurrentFont();
        $this->assertEquals($afont, $font);

        $fkey = $stack->getCurrentFontKey();
        $this->assertEquals('pdfatimes', $fkey);

        $type = $stack->getCurrentFontType();
        $this->assertEquals('Type1', $type);

        $ftype = $stack->isCurrentUnicodeFont();
        $this->assertFalse($ftype);

        $ftype = $stack->isCurrentByteFont();
        $this->assertTrue($ftype);

        $uniarr = [65, 173, 300, 32, 65, 173, 300, 32, 65, 173, 300];
        $widths = $stack->getOrdArrDims($uniarr);
        $this->assertEquals(11, $widths['chars']);
        $this->assertEquals(2, $widths['spaces']);
        $this->bcAssertEqualsWithDelta(60.9384, $widths['totwidth'], 0.0001);
        $this->bcAssertEqualsWithDelta(8.76, $widths['totspacewidth'], 0.0001);
        $this->assertEquals(6, $widths['words']);

        $split = $widths['split'][5] ?? null;
        $this->assertIsArray($split);
        $this->assertEquals(11, $split['pos']);
        $this->assertEquals(8203, $split['ord']);
        $this->assertEquals('BN', $split['septype']);
        $this->bcAssertEqualsWithDelta(4.92, $split['wordwidth'], 0.0001);
        $this->assertEquals(2, $split['spaces']);
        $this->bcAssertEqualsWithDelta(60.9384, $split['totwidth'], 0.0001);
        $this->bcAssertEqualsWithDelta(8.76, $split['totspacewidth'], 0.0001);

        $outfont = $stack->getOutCurrentFont();
        $this->assertEquals("BT /F2 14.000000 Tf ET\r", $outfont);

        $font = $stack->cloneFont($objnum, null, null, 13, 0.3, 0.7);
        $this->assertEquals(13, $font['size']);
        $this->assertEquals(0.3, $font['spacing']);
        $this->assertEquals(0.7, $font['stretching']);

        $font = $stack->cloneFont($objnum, 0, 'BI', 17, 0.7, 1.3);
        $this->assertEquals('BI', $font['style']);
        $this->assertEquals(17, $font['size']);
        $this->assertEquals(0.7, $font['spacing']);
        $this->assertEquals(1.3, $font['stretching']);

        $fname = $stack->getFontFamilyName('unknown');
        $this->assertEquals('freesansBI', $fname);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'pdfa/pfb/PDFACourier.pfb');
        $bfont = $stack->insert($objnum, 'courier', '', null, null, null, '', null);
        $this->assertNotEmpty($bfont);

        $fname = $stack->getFontFamilyName('freesans');
        $this->assertEquals('freesans', $fname);

        $fname = $stack->getFontFamilyName('cursive');
        $this->assertEquals('pdfatimes', $fname);

        $fname = $stack->getFontFamilyName('unknown');
        $this->assertEquals('pdfacourier', $fname);
    }

    /** @throws FontException */
    public function testEmptyStack(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->prepareTestEnvironment();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $stack->popLastFont();
    }

    /** @throws FontException */
    public function testStackMissingFont(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->prepareTestEnvironment();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        $stack->insert($objnum, 'missing');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testHasCurrentFont(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(0.75, true, true, true);
        $this->assertFalse($stack->hasCurrentFont());
        $this->assertSame(0, $stack->getStackSize());
        $this->assertSame(-1, $stack->getCurrentFontIndex());

        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $font = $stack->insert($objnum, 'freesans', '', 12);
        $this->assertTrue($stack->hasCurrentFont());
        $this->assertSame(1, $stack->getStackSize());
        $this->assertSame(0, $stack->getCurrentFontIndex());
        $this->assertSame($font['out'], $stack->getOutCurrentFont());

        $stack->cloneFont($objnum, null, null, 13);
        $this->assertSame(2, $stack->getStackSize());
        $this->assertSame(1, $stack->getCurrentFontIndex());

        $stack->popLastFont();
        $this->assertTrue($stack->hasCurrentFont());
        $this->assertSame(1, $stack->getStackSize());
        $this->assertSame(0, $stack->getCurrentFontIndex());

        $stack->popLastFont();
        $this->assertFalse($stack->hasCurrentFont());
        $this->assertSame(0, $stack->getStackSize());
        $this->assertSame(-1, $stack->getCurrentFontIndex());
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testUnicodeOrdAddedToSubsetChars(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';
        $objnum = 1;

        $stack = new \Com\Tecnick\Pdf\Font\Stack(0.75, true, true, true);
        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $stack->insert($objnum, 'freesans', '', 12, 0, 1, '', true);

        // Use pi and almost-equal to ensure non-latin BMP code points are tracked.
        $stack->getOrdArrDims([960, 8776]);

        $fonts = $stack->getFonts();
        $fkey = $stack->getCurrentFontKey();
        $currentFont = $fonts[$fkey] ?? null;
        $this->assertIsArray($currentFont);
        $this->assertArrayHasKey(960, $currentFont['subsetchars']);
        $this->assertArrayHasKey(8776, $currentFont['subsetchars']);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testFractionalFontSize(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(0.75, true, true, true);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $font = $stack->insert($objnum, 'freesans', '', 10.5);

        $this->bcAssertEqualsWithDelta(10.5, $font['size'], 0.0001);
        $this->assertEquals("BT /F1 10.500000 Tf ET\r", $font['out']);

        $clone = $stack->cloneFont($objnum, null, null, 11.25);

        $this->bcAssertEqualsWithDelta(11.25, $clone['size'], 0.0001);
        $this->assertEquals("BT /F1 11.250000 Tf ET\r", $clone['out']);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testCloneFontRejectsOutOfRangeIndex(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';
        $objnum = 1;

        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        new \Com\Tecnick\Pdf\Font\Import($indir . 'core/Helvetica.afm');
        $stack->insert($objnum, 'helvetica');

        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $stack->cloneFont($objnum, 99);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testReplaceMissingCharsKeepsOriginalWhenNoSubstitutesProvided(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';
        $objnum = 1;

        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        new \Com\Tecnick\Pdf\Font\Import($indir . 'core/Helvetica.afm');
        $stack->insert($objnum, 'helvetica');

        $this->assertSame([400], $stack->replaceMissingChars([400], []));
    }

    /** @throws FontException */
    public function testGetFontFamilyNameRejectsEmptyString(): void
    {
        $this->prepareTestEnvironment();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);

        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $stack->getFontFamilyName('');
    }

    /** @throws FontException */
    public function testGetCharWidthFailsWithoutCurrentFont(): void
    {
        $this->prepareTestEnvironment();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);

        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $stack->getCharWidth(65);
    }

    /** @throws FontException */
    public function testMalformedCharBoxDataIsIgnored(): void
    {
        $this->prepareTestEnvironment();
        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);

        \file_put_contents(
            $this->getFontPath() . 'badbbox.json',
            '{"type":"Type1","desc":{"FontBBox":"[0 0 0 0]"},"cw":{"65":400},"cbbox":{"65":[1,2,3]}}',
        );

        $stack->insert($objnum, 'badbbox', '', null, null, null, $this->getFontPath() . 'badbbox.json', null);
        $this->assertSame([0.0, 0.0, 0.0, 0.0], $stack->getCharBBox(65));
    }
}
