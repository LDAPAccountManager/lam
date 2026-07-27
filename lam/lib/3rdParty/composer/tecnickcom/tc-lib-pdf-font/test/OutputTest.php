<?php

/**
 * OutputTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Test;

use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\Pdf\Encrypt\Encrypt;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Output Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @SuppressWarnings("PHPMD.LongVariable")
 *
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 */
class OutputTest extends TestUtil
{
    /** @throws \ReflectionException */
    private function createEncrypt(): Encrypt
    {
        $reflector = new \ReflectionClass(Encrypt::class);
        $encrypt = $reflector->newInstanceWithoutConstructor();

        \assert($encrypt instanceof Encrypt, 'Failed to create Encrypt instance');

        return $encrypt;
    }

    private function prepareTestEnvironment(): void
    {
        parent::setupTest();
    }

    /**
     * @return TFontData
     */
    private function getFontTemplate(): array
    {
        return [
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
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     * @throws \ReflectionException
     */
    public function testOutput(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);

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

        new \Com\Tecnick\Pdf\Font\Import($indir . 'cid0/cid0jp.ttf', '', 'CID0JP');
        $stack->add($objnum, 'cid0jp');

        $fonts = $stack->getFonts();
        $this->assertCount(10, $fonts);

        $encrypt = $this->createEncrypt();
        $output = new \Com\Tecnick\Pdf\Font\Output($fonts, $objnum, $encrypt, null);

        $this->assertEquals(37, $output->getObjectNumber());

        $this->assertNotEmpty($output->getFontsBlock());

        $this->assertNotEmpty($output->getOutFontDict());

        $keys = [];
        foreach ($fonts as $font) {
            $keys[] = $font['key'];
        }

        $this->assertNotEmpty($output->getOutFontDictByKeys($keys));
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \ReflectionException
     */
    public function testOutputWithNoFontsReturnsEmptyStrings(): void
    {
        // Empty font array: constructor still runs without error; all output methods
        // return empty strings because there is nothing to iterate over.
        $encrypt = $this->createEncrypt();
        $output = new \Com\Tecnick\Pdf\Font\Output([], 1, $encrypt, null);

        $this->assertSame('', $output->getFontsBlock());
        $this->assertSame('', $output->getOutFontDict());
        $this->assertSame('', $output->getOutFontDictByKeys([]));
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \ReflectionException
     */
    public function testOutputGetFontDefinitionsThrowsOnUnknownFontType(): void
    {
        // A font entry with an unrecognised type triggers the default branch of the
        // match expression inside getFontDefinitions, which throws FontException.
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);

        $encrypt = $this->createEncrypt();

        // Build a minimal font array with an unknown type so that getFontDefinitions
        // reaches the default throw branch.
        $fonts = ['unknown_key' => $this->getFontTemplate()];
        $fonts['unknown_key']['type'] = 'UnknownType';

        new \Com\Tecnick\Pdf\Font\Output($fonts, 1, $encrypt, null);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     * @throws \ReflectionException
     */
    public function testSubsetTrueTypeUnicodeOutputUsesValidCidSystemInfoAndFontStream(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $stack->add($objnum, 'freesans', '', '', true);

        // Ensure at least a few glyphs are included in the subset.
        foreach ([32, 65, 66, 67, 937, 960] as $ord) {
            $stack->addSubsetChar('freesans', $ord);
        }

        $encrypt = $this->createEncrypt();
        $output = new \Com\Tecnick\Pdf\Font\Output($stack->getFonts(), $objnum, $encrypt, null);
        $out = $output->getFontsBlock();

        $this->assertStringNotContainsString('/Registry () /Ordering ()', $out);
        $this->assertStringContainsString('/Registry (Adobe) /Ordering (Identity) /Supplement 0', $out);

        $matches = [];
        \preg_match_all('/\\/Length1\\s+(\\d+)/', $out, $matches);
        $lengthMatches = $matches[1] ?? [];
        $lengths = \array_map('intval', $lengthMatches);
        $this->assertNotEmpty($lengths);
        $this->assertGreaterThan(1000, \max($lengths));
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     * @throws \ReflectionException
     */
    public function testSubsetCharMergePreservesUnicodeKeys(): void
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $objnum = 1;
        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $stack->add($objnum, 'freesans', '', '', true);

        $fonts = $stack->getFonts();
        if (!isset($fonts['freesans'])) {
            $this->fail('Expected freesans font data');
        }

        $base = $fonts['freesans'];
        $base['key'] = 'freesans_dup';
        $base['i'] += 1000;
        $base['n'] += 1000;
        $base['subsetchars'] = [8776 => true, 9999 => false];
        $primary = $fonts['freesans'];
        $primary['subsetchars'] = [960 => true];

        $fonts = \array_replace($fonts, [
            'freesans' => $primary,
            'freesans_dup' => $base,
        ]);

        $encrypt = $this->createEncrypt();
        $output = new \Com\Tecnick\Pdf\Font\Output($fonts, $objnum, $encrypt, null);

        $ref = new \ReflectionClass($output);
        $prop = $ref->getProperty('subchars');
        /** @var array<int, array<int, bool>> $subchars */
        $subchars = $prop->getValue($output);

        $this->assertIsArray($subchars);
        $this->assertNotEmpty($subchars);
        $first = \array_values($subchars)[0] ?? null;
        $this->assertIsArray($first);
        $this->assertArrayHasKey(960, $first);
        $this->assertArrayHasKey(8776, $first);
        $this->assertArrayNotHasKey(9999, $first);
    }

    public function testUniToCidPreservesNumericCidKeys(): void
    {
        $outfont = new OutputTestOutFont();

        $font = $this->getFontTemplate();
        $font['cidinfo'] = [
            'Ordering' => 'Identity',
            'Registry' => 'Adobe',
            'Supplement' => 0,
            'uni2cid' => [960 => 853, 8776 => 3283],
        ];
        $font['cw'] = [32 => 250, 960 => 500, 8776 => 600];
        $font['i'] = 1;
        $font['n'] = 1;
        $font['name'] = 'test';
        $font['subset'] = true;

        $outfont->runUniToCid($font, 0);

        $this->assertArrayHasKey(853, $font['cw']);
        $this->assertArrayHasKey(3283, $font['cw']);
        $this->assertSame(500, $font['cw'][853] ?? null);
        $this->assertSame(600, $font['cw'][3283] ?? null);
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testGetFontFullPathThrowsForMissingFile(): void
    {
        $this->setupTest();
        $outfont = new OutputTestOutFont();
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);
        $outfont->runGetFontFullPath($this->getFontPath(), 'not-here.bin');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \ReflectionException
     */
    public function testOutputRejectsSubsetFromPlainFileData(): void
    {
        $this->prepareTestEnvironment();
        $this->bcExpectException(\Com\Tecnick\Pdf\Font\Exception::class);

        $tmpfile = $this->getFontPath() . 'plain-font.bin';
        \file_put_contents($tmpfile, 'not-gzip-data');

        $font = $this->getFontTemplate();
        $font['key'] = 'plain';
        $font['name'] = 'Plain';
        $font['i'] = 1;
        $font['n'] = 1;
        $font['file'] = 'plain-font.bin';
        $font['dir'] = $this->getFontPath();
        $font['subset'] = true;
        $font['subsetchars'] = [65 => true];

        $encrypt = $this->createEncrypt();
        \set_error_handler(static fn(): bool => true);
        try {
            new \Com\Tecnick\Pdf\Font\Output(['plain' => $font], 1, $encrypt, null);
        } finally {
            \restore_error_handler();
        }
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \ReflectionException
     */
    public function testOutputBuildsTrueTypeDefinitionWithDefaultEncoding(): void
    {
        $font = $this->getFontTemplate();
        $font['key'] = 'truetypefont';
        $font['name'] = 'TrueTypeFont';
        $font['type'] = 'TrueType';
        $font['i'] = 1;
        $font['n'] = 1;
        $font['enc'] = 'cp1252';
        $font['dw'] = 600;
        $font['cw'] = [32 => 250, 65 => 700];

        $encrypt = $this->createEncrypt();
        $output = new \Com\Tecnick\Pdf\Font\Output(['truetypefont' => $font], 1, $encrypt, null);
        $block = $output->getFontsBlock();

        $this->assertStringContainsString('/Subtype /TrueType', $block);
        $this->assertStringContainsString('/Encoding /WinAnsiEncoding', $block);
    }

    public function testGetKeyValOutFormatsFloatValues(): void
    {
        $outfont = new OutputTestOutFont();
        $out = $outfont->runGetKeyValOut('ItalicAngle', 12.5);
        $this->assertSame(' /ItalicAngle 12.500000', $out);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \ReflectionException
     */
    public function testOutputBuildsCidFont0WhenGlyphOneIsNotDefined(): void
    {
        $font = $this->getFontTemplate();
        $font['key'] = 'cidfont0';
        $font['name'] = 'CIDFont0Test';
        $font['type'] = 'CIDFont0';
        $font['i'] = 1;
        $font['n'] = 1;
        $font['enc'] = 'Identity-H';
        $font['dw'] = 600;
        $font['cw'] = [32 => 500, 65 => 700];
        $font['cidinfo'] = [
            'Registry' => 'Adobe',
            'Ordering' => 'Identity',
            'Supplement' => 0,
            'uni2cid' => [],
        ];

        $encrypt = $this->createEncrypt();
        $output = new \Com\Tecnick\Pdf\Font\Output(['cidfont0' => $font], 1, $encrypt, null);
        $block = $output->getFontsBlock();

        $this->assertStringContainsString('/Subtype /Type0', $block);
        $this->assertStringContainsString('/Subtype /CIDFontType0', $block);
    }

    /**
     * Build a subset-enabled FreeSans font stack for cache tests.
     *
     * @return array<string, TFontData>
     *
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    private function buildSubsetFonts(int $objnum): array
    {
        $this->prepareTestEnvironment();
        $indir = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/';

        $stack = new \Com\Tecnick\Pdf\Font\Stack(1);
        new \Com\Tecnick\Pdf\Font\Import($indir . 'freefont/FreeSans.ttf');
        $stack->add($objnum, 'freesans', '', '', true);

        foreach ([32, 65, 66, 67, 937, 960] as $ord) {
            $stack->addSubsetChar('freesans', $ord);
        }

        return $stack->getFonts();
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     * @throws \ReflectionException
     */
    public function testSubsetCacheStoresOnMissAndReusesOnHit(): void
    {
        $objnum = 1;
        $fonts = $this->buildSubsetFonts($objnum);
        $cache = new SpyFontSubsetCache();
        $encrypt = $this->createEncrypt();

        $firstObj = new \Com\Tecnick\Pdf\Font\Output($fonts, $objnum, $encrypt, null, $cache);
        $first = $firstObj->getFontsBlock();

        $secondObj = new \Com\Tecnick\Pdf\Font\Output($fonts, $objnum, $encrypt, null, $cache);
        $second = $secondObj->getFontsBlock();

        $this->assertSame($first, $second);
        $this->assertCount(1, $cache->store, 'subset should be stored exactly once');

        $storedKey = \array_key_first($cache->store);
        $this->assertIsString($storedKey);
        $this->assertSame([$storedKey], $cache->setCalls, 'subset should be computed and stored exactly once');
        $this->assertSame(
            [$storedKey, $storedKey],
            $cache->getCalls,
            'cache should be consulted on every pass (miss then hit) for the same key',
        );
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     * @throws \ReflectionException
     */
    public function testSubsetCacheHitProducesIdenticalOutputToNoCache(): void
    {
        $objnum = 1;
        $fonts = $this->buildSubsetFonts($objnum);
        $encrypt = $this->createEncrypt();

        $noCacheObj = new \Com\Tecnick\Pdf\Font\Output($fonts, $objnum, $encrypt, null, null);
        $noCache = $noCacheObj->getFontsBlock();

        $cache = new SpyFontSubsetCache();
        // First pass populates the cache (miss), second pass reads from it (hit).
        new \Com\Tecnick\Pdf\Font\Output($fonts, $objnum, $encrypt, null, $cache);
        $hitObj = new \Com\Tecnick\Pdf\Font\Output($fonts, $objnum, $encrypt, null, $cache);
        $hit = $hitObj->getFontsBlock();

        $this->assertSame($noCache, $hit, 'a cache hit must reproduce the no-cache output byte-for-byte');
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \ReflectionException
     */
    public function testSubsetCacheKeyIsStableAndDiscriminating(): void
    {
        $this->setupTest();
        $encrypt = $this->createEncrypt();
        $output = new OutputTestOutput([], 1, $encrypt, null);

        $font = $this->getFontTemplate();
        $font['platform_id'] = 3;
        $font['encoding_id'] = 1;
        $font['type'] = 'TrueTypeUnicode';
        $data = 'FONT-PROGRAM-BYTES';

        $keyA = $output->runSubsetCacheKey($data, $font, [65 => true, 66 => true]);

        $keyOrder = $output->runSubsetCacheKey($data, $font, [66 => true, 65 => true]);
        $this->assertSame($keyA, $keyOrder, 'char insertion order must not change the key');

        $keyDisabled = $output->runSubsetCacheKey($data, $font, [65 => true, 66 => true, 67 => false]);
        $this->assertSame($keyA, $keyDisabled, 'disabled chars must not change the key');

        $keyChars = $output->runSubsetCacheKey($data, $font, [65 => true]);
        $this->assertNotSame($keyA, $keyChars, 'a different char set must change the key');

        $keyBytes = $output->runSubsetCacheKey('OTHER-BYTES', $font, [65 => true, 66 => true]);
        $this->assertNotSame($keyA, $keyBytes, 'different font bytes must change the key');

        $fontAlt = $font;
        $fontAlt['encoding_id'] = 10;
        $keyCmap = $output->runSubsetCacheKey($data, $fontAlt, [65 => true, 66 => true]);
        $this->assertNotSame($keyA, $keyCmap, 'different cmap selection metrics must change the key');
    }
}
