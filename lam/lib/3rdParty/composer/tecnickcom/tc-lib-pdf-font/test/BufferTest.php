<?php

/**
 * BufferTest.php
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
class BufferTest extends TestUtil
{
    public function testSubsetModeDisabledByDefault(): void
    {
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $this->assertFalse($stack->isSubsetMode());
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testAddSubsetCharOnMissingFontThrows(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $stack->addSubsetChar('missing', 65);
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testStackMissingKey(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $stack->getFont('missing');
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testStackMissingFontName(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        $stack->add($objnum, '');
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testStackIFileMissing(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        $stack->add($objnum, 'something', '', '/missing/nothere.json');
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testStackIFileNotJson(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        $stack->add($objnum, 'something', '', __DIR__ . '/StackTest.php');
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testStackIFileWrongFormat(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        \file_put_contents($this->getFontPath() . 'badformat.json', '{"bad":"format"}');
        $stack->add($objnum, 'something', '', $this->getFontPath() . 'badformat.json');
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadDeafultWidthA(): void
    {
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        \file_put_contents($this->getFontPath() . 'test.json', '{"type":"Type1","cw":{"0":100}}');
        $stack->add($objnum, 'test', '', $this->getFontPath() . 'test.json');
        $font = $stack->getFont('test');
        $this->assertEquals(600, $font['dw']);
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadDeafultWidthB(): void
    {
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        \file_put_contents($this->getFontPath() . 'test.json', '{"type":"Type1","cw":{"32":123}}');
        $stack->add($objnum, 'test', '', $this->getFontPath() . 'test.json');
        $font = $stack->getFont('test');
        $this->assertEquals(123, $font['dw']);
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadDeafultWidthC(): void
    {
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        \file_put_contents(
            $this->getFontPath() . 'test.json',
            '{"type":"Type1","desc":{"MissingWidth":234},"cw":{"0":600}}',
        );
        $stack->add($objnum, 'test', '', $this->getFontPath() . 'test.json');
        $font = $stack->getFont('test');
        $this->assertEquals(234, $font['dw']);
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadWrongType(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        \file_put_contents($this->getFontPath() . 'test.json', '{"type":"WRONG","cw":{"0":600}}');
        $stack->add($objnum, 'test', '', $this->getFontPath() . 'test.json');
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadCidOnPdfa(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1, false, true, true);
        $objnum = 1;
        \file_put_contents($this->getFontPath() . 'test.json', '{"type":"cidfont0","cw":{"0":600}}');
        $stack->add($objnum, 'test', '', $this->getFontPath() . 'test.json', false);
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadArtificialStyles(): void
    {
        $this->setupTest();
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        $objnum = 1;
        \file_put_contents(
            $this->getFontPath() . 'test.json',
            '{"type":"Core","cw":{"0":600},"mode":{"bold":true,"italic":true}}',
        );
        $key = $stack->add($objnum, 'symbol', '', $this->getFontPath() . 'test.json');
        $this->assertNotEmpty($key);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Font\Exception
     * @throws \RangeException
     */
    public function testBuffer(): void
    {
        $this->setupTest();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1, false, true, false);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'pdfa/pfb/PDFASymbol.pfb', '', 'Type1', 'symbol');
        $stack->add($objnum, 'pdfasymbol');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'core/Helvetica.afm');
        $stack->add($objnum, 'helvetica');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'core/Helvetica-Bold.afm');
        $stack->add($objnum, 'helvetica', 'B');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'core/Helvetica-BoldOblique.afm');
        $stack->add($objnum, 'helveticaBI');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'core/Helvetica-Oblique.afm');
        $stack->add($objnum, 'helvetica', 'I');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $stack->add($objnum, 'freesans', '');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSansBold.ttf');
        $stack->add($objnum, 'freesans', 'B');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSansOblique.ttf');
        $stack->add($objnum, 'freesans', 'I');

        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSansBoldOblique.ttf');
        $stack->add($objnum, 'freesans', 'BIUDO', '', true);

        $fontkey = $stack->add($objnum, 'freesans', 'BI', '', true);
        $this->assertEquals('freesansBI', $fontkey);

        $this->assertEquals(10, $objnum);
        $this->assertCount(9, $stack->getFonts());
        $this->assertCount(1, $stack->getEncDiffs());

        $font = $stack->getFont('freesansB');
        $this->assertNotEmpty($font);
        $this->assertEquals('FreeSansBold', $font['name']);
        $this->assertEquals('TrueTypeUnicode', $font['type']);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'core/ZapfDingbats.afm');
        $stack->add($objnum, 'zapfdingbats', 'BIUDO');
        $font = $stack->getFont('zapfdingbats');
        $this->assertNotEmpty($font);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Font\Exception
     * @throws \RangeException
     */
    public function testBufferPdfa(): void
    {
        $this->setupTest();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1, true, false, true);

        new \Com\Tecnick\Pdf\Font\Import($indir . 'pdfa/pfb/PDFAHelveticaBoldOblique.pfb');
        $stack->add($objnum, 'arial', 'BIUDO', '', true);
        $font = $stack->getFont('pdfahelveticaBI');
        $this->assertNotEmpty($font);
    }
}
