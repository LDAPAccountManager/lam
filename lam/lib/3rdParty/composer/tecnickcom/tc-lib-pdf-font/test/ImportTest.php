<?php

/**
 * ImportTest.php
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
use PHPUnit\Framework\Attributes\DataProvider;

/**
 * Import Test
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
 *
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 */
class ImportTest extends TestUtil
{
    private function expectFontException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportForbiddenProtocol(): void
    {
        $this->expectFontException();
        new \Com\Tecnick\Pdf\Font\Import('phar://test.txt');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportParentDir(): void
    {
        $this->expectFontException();
        new \Com\Tecnick\Pdf\Font\Import('/tmp/something/../test.txt');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportEmptyName(): void
    {
        $this->expectFontException();
        new \Com\Tecnick\Pdf\Font\Import('');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportExist(): void
    {
        $this->expectFontException();
        $fin = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/core/Helvetica.afm';
        $outdir = \dirname(__DIR__) . '/target/tmptest/';
        \system('rm -rf ' . $outdir . ' && mkdir -p ' . $outdir);
        new \Com\Tecnick\Pdf\Font\Import($fin, $outdir);
        new \Com\Tecnick\Pdf\Font\Import($fin, $outdir);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportWrongFile(): void
    {
        $this->expectFontException();
        new \Com\Tecnick\Pdf\Font\Import(\dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/core/Missing.afm');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportDefaultOutput(): void
    {
        $this->expectFontException();
        new \Com\Tecnick\Pdf\Font\Import(\dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/core/Missing.afm');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportUnsupportedType(): void
    {
        $this->expectFontException();
        $fin = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/core/Helvetica.afm';
        $outdir = \dirname(__DIR__) . '/target/tmptest/core/';
        \system('rm -rf ' . $outdir . ' && mkdir -p ' . $outdir);
        new \Com\Tecnick\Pdf\Font\Import($fin, $outdir, 'ERROR');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testImportUnsupportedOpenType(): void
    {
        $this->expectFontException();
        $outdir = \dirname(__DIR__) . '/target/tmptest/core/';
        \system('rm -rf ' . $outdir . ' && mkdir -p ' . $outdir);
        \file_put_contents($outdir . 'test.ttf', 'OTTO 1234');
        new \Com\Tecnick\Pdf\Font\Import($outdir . 'test.ttf', $outdir);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \JsonException
     * @throws \RangeException
     */
    #[DataProvider('importDataProvider')]
    public function testImport(
        string $fontdir,
        string $font,
        string $outname,
        string $type = '',
        string $encoding = '',
    ): void {
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/' . $fontdir . '/';
        $outdir = \dirname(__DIR__) . '/target/tmptest/' . $fontdir . '/';
        \system('rm -rf ' . \dirname(__DIR__) . '/target/tmptest/ && mkdir -p ' . $outdir);

        $import = new \Com\Tecnick\Pdf\Font\Import($indir . $font, $outdir, $type, $encoding);
        $this->assertEquals($outname, $import->getFontName());

        $file = \file_get_contents($outdir . $outname . '.json');
        $this->assertNotFalse($file);

        /**
         * @var array{
         *     type: mixed,
         *     name: mixed,
         *     up: mixed,
         *     ut: mixed,
         *     dw: mixed,
         *     diff: mixed,
         *     desc: array{
         *         Flags: mixed,
         *         FontBBox: mixed,
         *         ItalicAngle: mixed,
         *         Ascent: mixed,
         *         Descent: mixed,
         *         Leading: mixed,
         *         CapHeight: mixed,
         *         XHeight: mixed,
         *         StemV: mixed,
         *         StemH: mixed,
         *         AvgWidth: mixed,
         *         MaxWidth: mixed,
         *         MissingWidth: mixed
         *     }
         * } $json
         */
        $json = \json_decode($file, true, 512, JSON_THROW_ON_ERROR);

        $this->assertArrayHasKey('type', $json);
        $this->assertArrayHasKey('name', $json);
        $this->assertArrayHasKey('up', $json);
        $this->assertArrayHasKey('ut', $json);
        $this->assertArrayHasKey('dw', $json);
        $this->assertArrayHasKey('diff', $json);
        $this->assertArrayHasKey('desc', $json);
        $this->assertArrayHasKey('Flags', $json['desc']);

        $metric = $import->getFontMetrics();

        $this->assertEquals('[' . $metric['bbox'] . ']', $json['desc']['FontBBox']);
        $this->assertEqualsWithDelta($metric['italicAngle'], $json['desc']['ItalicAngle'], 0.001);
        $this->assertEquals($metric['Ascent'], $json['desc']['Ascent']);
        $this->assertEquals($metric['Descent'], $json['desc']['Descent']);
        $this->assertEquals($metric['Leading'], $json['desc']['Leading']);
        $this->assertEquals($metric['CapHeight'], $json['desc']['CapHeight']);
        $this->assertEquals($metric['XHeight'], $json['desc']['XHeight']);
        $this->assertEquals($metric['StemV'], $json['desc']['StemV']);
        $this->assertEquals($metric['StemH'], $json['desc']['StemH']);
        $this->assertEquals($metric['AvgWidth'], $json['desc']['AvgWidth']);
        $this->assertEquals($metric['MaxWidth'], $json['desc']['MaxWidth']);
        $this->assertEquals($metric['MissingWidth'], $json['desc']['MissingWidth']);
    }

    /**
     * @return array<array<string>>
     */
    public static function importDataProvider(): array
    {
        return [
            ['core', 'Courier.afm', 'courier'],
            ['core', 'Courier-Bold.afm', 'courierb'],
            ['core', 'Courier-BoldOblique.afm', 'courierbi'],
            ['core', 'Courier-Oblique.afm', 'courieri'],
            ['core', 'Helvetica.afm', 'helvetica'],
            ['core', 'Helvetica-Bold.afm', 'helveticab'],
            ['core', 'Helvetica-BoldOblique.afm', 'helveticabi'],
            ['core', 'Helvetica-Oblique.afm', 'helveticai'],
            ['core', 'Symbol.afm', 'symbol'],
            ['core', 'Times.afm', 'times'],
            ['core', 'Times-Bold.afm', 'timesb'],
            ['core', 'Times-BoldItalic.afm', 'timesbi'],
            ['core', 'Times-Italic.afm', 'timesi'],
            ['core', 'ZapfDingbats.afm', 'zapfdingbats'],
            ['pdfa/pfb', 'PDFACourierBoldOblique.pfb', 'pdfacourierbi', '', ''],
            ['pdfa/pfb', 'PDFACourierBold.pfb', 'pdfacourierb', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFACourierOblique.pfb', 'pdfacourieri', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFACourier.pfb', 'pdfacourier', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFAHelveticaBoldOblique.pfb', 'pdfahelveticabi', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFAHelveticaBold.pfb', 'pdfahelveticab', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFAHelveticaOblique.pfb', 'pdfahelveticai', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFAHelvetica.pfb', 'pdfahelvetica', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFASymbol.pfb', 'pdfasymbol', '', 'symbol'],
            ['pdfa/pfb', 'PDFATimesBoldItalic.pfb', 'pdfatimesbi', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFATimesBold.pfb', 'pdfatimesb', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFATimesItalic.pfb', 'pdfatimesi', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFATimes.pfb', 'pdfatimes', 'Type1', 'cp1252'],
            ['pdfa/pfb', 'PDFAZapfDingbats.pfb', 'pdfazapfdingbats'],
            ['freefont', 'FreeMonoBoldOblique.ttf', 'freemonobi'],
            ['freefont', 'FreeMonoBold.ttf', 'freemonob'],
            ['freefont', 'FreeMonoOblique.ttf', 'freemonoi'],
            ['freefont', 'FreeMono.ttf', 'freemono'],
            ['freefont', 'FreeSansBoldOblique.ttf', 'freesansbi'],
            ['freefont', 'FreeSansBold.ttf', 'freesansb'],
            ['freefont', 'FreeSansOblique.ttf', 'freesansi'],
            ['freefont', 'FreeSans.ttf', 'freesans'],
            ['freefont', 'FreeSerifBoldItalic.ttf', 'freeserifbi'],
            ['freefont', 'FreeSerifBold.ttf', 'freeserifb'],
            ['freefont', 'FreeSerifItalic.ttf', 'freeserifi'],
            ['freefont', 'FreeSerif.ttf', 'freeserif'],
            ['unifont', 'unifont.ttf', 'unifont'],
            ['cid0', 'cid0cs.ttf', 'cid0cs', 'CID0CS'],
            ['cid0', 'cid0ct.ttf', 'cid0ct', 'CID0CT'],
            ['cid0', 'cid0jp.ttf', 'cid0jp', 'CID0JP'],
            ['cid0', 'cid0kr.ttf', 'cid0kr', 'CID0KR'],
            ['dejavu/ttf', 'DejaVuSans.ttf', 'dejavusans'],
            ['dejavu/ttf', 'DejaVuSans-BoldOblique.ttf', 'dejavusansbi'],
            ['dejavu/ttf', 'DejaVuSans-Bold.ttf', 'dejavusansb'],
            ['dejavu/ttf', 'DejaVuSans-Oblique.ttf', 'dejavusansi'],
            ['dejavu/ttf', 'DejaVuSansCondensed.ttf', 'dejavusanscondensed'],
            ['dejavu/ttf', 'DejaVuSansCondensed-BoldOblique.ttf', 'dejavusanscondensedbi'],
            ['dejavu/ttf', 'DejaVuSansCondensed-Bold.ttf', 'dejavusanscondensedb'],
            ['dejavu/ttf', 'DejaVuSansCondensed-Oblique.ttf', 'dejavusanscondensedi'],
            ['dejavu/ttf', 'DejaVuSansMono.ttf', 'dejavusansmono'],
            ['dejavu/ttf', 'DejaVuSansMono-BoldOblique.ttf', 'dejavusansmonobi'],
            ['dejavu/ttf', 'DejaVuSansMono-Bold.ttf', 'dejavusansmonob'],
            ['dejavu/ttf', 'DejaVuSansMono-Oblique.ttf', 'dejavusansmonoi'],
            ['dejavu/ttf', 'DejaVuSans-ExtraLight.ttf', 'dejavusansextralight'],
            ['dejavu/ttf', 'DejaVuSerif.ttf', 'dejavuserif'],
            ['dejavu/ttf', 'DejaVuSerif-BoldItalic.ttf', 'dejavuserifbi'],
            ['dejavu/ttf', 'DejaVuSerif-Bold.ttf', 'dejavuserifb'],
            ['dejavu/ttf', 'DejaVuSerif-Italic.ttf', 'dejavuserifi'],
            ['dejavu/ttf', 'DejaVuSerifCondensed.ttf', 'dejavuserifcondensed'],
            ['dejavu/ttf', 'DejaVuSerifCondensed-BoldItalic.ttf', 'dejavuserifcondensedbi'],
            ['dejavu/ttf', 'DejaVuSerifCondensed-Bold.ttf', 'dejavuserifcondensedb'],
            ['dejavu/ttf', 'DejaVuSerifCondensed-Italic.ttf', 'dejavuserifcondensedi'],
        ];
    }
}
