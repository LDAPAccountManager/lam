<?php

/**
 * ImportOutputPngEdgeCasesTest.php
 *
 * @since     2026-05-21
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-pdf-image software library.
 */

declare(strict_types=1);

namespace Test;

require_once __DIR__ . '/ImportProtectedMethodsHarness.php';
require_once __DIR__ . '/PngChunkParsingHarness.php';

/**
 * Extra branch-focused tests for import, output and PNG chunk parsing.
 *
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 * @phpstan-import-type ImageRawData from \Com\Tecnick\Pdf\Image\Import
 */
class ImportOutputPngEdgeCasesTest extends TestUtil
{
    /**
     * @return ImageBaseData
     */
    protected function getBaseData(string $key = 'test'): array
    {
        return [
            'bits' => 8,
            'channels' => 3,
            'colspace' => 'DeviceRGB',
            'data' => 'abc',
            'exturl' => false,
            'file' => '',
            'filter' => 'FlateDecode',
            'height' => 1,
            'icc' => '',
            'ismask' => false,
            'key' => $key,
            'mapto' => IMAGETYPE_PNG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => 'raw',
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_PNG,
            'width' => 1,
        ];
    }

    /**
     * @return ImageRawData
     */
    protected function getRawData(string $key = 'test'): array
    {
        return $this->getBaseData($key);
    }

    protected function getImportHarness(bool $pdfa = false): ImportProtectedMethodsHarness
    {
        return new ImportProtectedMethodsHarness(
            0.75,
            $this->getTestEncrypt(),
            $this->getTestFileHelper(),
            $pdfa,
            false,
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function testGetDataThrowsWhenImageIsNotNative(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);

        $import = $this->getImportHarness();
        $data = $this->getRawData();
        $data['native'] = false;
        $import->callGetData($data, 10, 10, 90);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function testGetDataThrowsWhenNativeTypeIsUnknown(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);

        $import = $this->getImportHarness();
        $data = $this->getRawData();
        $data['type'] = IMAGETYPE_GIF;
        $import->callGetData($data, 10, 10, 90);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function testGetResizedRawDataRejectsInvalidRawImage(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);

        $import = $this->getImportHarness();
        $data = $this->getBaseData();
        $data['raw'] = 'not-image-binary';

        \set_error_handler(static fn(): bool => true);
        try {
            $import->callGetResizedRawData($data, 10, 10, true, 90);
        } finally {
            \restore_error_handler();
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function testGetAlphaChannelRawDataRejectsInvalidRawImage(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);

        $import = $this->getImportHarness();
        $data = $this->getBaseData();
        $data['raw'] = 'not-image-binary';

        \set_error_handler(static fn(): bool => true);
        try {
            $import->callGetAlphaChannelRawData($data);
        } finally {
            \restore_error_handler();
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetOutImageSupportsExternalStreamWithFilter(): void
    {
        $import = $this->getImportHarness();
        $import->setPon(10);

        $img = [
            'iid' => 1,
            'key' => 'ext',
            'width' => 10,
            'height' => 5,
            'defprint' => false,
            'altimgs' => [],
        ];
        $data = $this->getRawData('ext');
        $data['exturl'] = true;
        $data['filter'] = 'ASCIIHexDecode';
        $data['width'] = 10;
        $data['height'] = 5;

        $import->setCacheEntry('ext', $data);
        $out = $import->callGetOutImage($img, $data);

        $this->assertStringContainsString('/Length 0 /F << /FS /URL /F (true) >>', $out);
        $this->assertStringContainsString('/FFilter /ASCIIHexDecode', $out);
    }

    public function testGetXobjectDictByKeysFallsBackToPlainAndMaskEntries(): void
    {
        $import = $this->getImportHarness();
        $import->setXobjdict([
            'IMGplain2' => 42,
            'IMGmask3' => 43,
        ]);

        $out = $import->getXobjectDictByKeys([2, 3]);
        $this->assertSame(' /IMGplain2 42 0 R /IMGmask3 43 0 R', $out);
    }

    public function testGetOutAltImagesSkipsUnknownOrUnbuiltAlternateImages(): void
    {
        $import = $this->getImportHarness();
        $import->setPon(20);

        $import->setImageEntry(2, [
            'iid' => 2,
            'key' => 'alt-no-object',
            'width' => 10,
            'height' => 5,
            'defprint' => true,
            'altimgs' => [],
        ]);

        $altData = $this->getRawData('alt-no-object');
        $altData['obj'] = 0;
        $import->setCacheEntry('alt-no-object', $altData);

        $img = [
            'iid' => 1,
            'key' => 'main',
            'width' => 10,
            'height' => 5,
            'defprint' => false,
            'altimgs' => [999, 2],
        ];
        $data = $this->getRawData('main');

        $out = $import->callGetOutAltImages($img, $data);

        $this->assertStringContainsString('21 0 obj', $out);
        $this->assertStringContainsString('[ ]', $out);
    }

    public function testGetOutTransparencyIndexedSkipsNonZeroValues(): void
    {
        $import = $this->getImportHarness();
        $data = $this->getRawData();
        $data['colspace'] = 'Indexed';
        $data['trns'] = [0 => 0, 1 => 9, 2 => 0];

        // Indexed: the fully-transparent palette indices (alpha 0) are masked.
        $out = $import->callGetOutTransparency($data);
        $this->assertSame('0 0 2 2 ', $out);
    }

    public function testGetOutTransparencyRgbUsesColorValues(): void
    {
        $import = $this->getImportHarness();
        $data = $this->getRawData();
        $data['colspace'] = 'DeviceRGB';
        // trns holds the transparent colour sample values (R, G, B).
        $data['trns'] = [255, 128, 0];

        $out = $import->callGetOutTransparency($data);
        $this->assertSame('255 255 128 128 0 0 ', $out);
    }

    public function testGetOutTransparencyGrayUsesColorValue(): void
    {
        $import = $this->getImportHarness();
        $data = $this->getRawData();
        $data['colspace'] = 'DeviceGray';
        $data['trns'] = [128];

        $out = $import->callGetOutTransparency($data);
        $this->assertSame('128 128 ', $out);
    }

    public function testGetTrnsChunkHandlesGrayAndRgbFormats(): void
    {
        $png = new PngChunkParsingHarness();

        $gray = $this->getBaseData();
        $gray['colspace'] = 'DeviceGray';
        $gray['raw'] = "\x00\x7f";
        $gray['trns'] = [];
        $grayOffset = 0;
        $gray = $png->callGetTrnsChunk($gray, $grayOffset, 2);
        $this->assertSame([127], $gray['trns']);
        $this->assertSame(6, $grayOffset);

        $rgb = $this->getBaseData();
        $rgb['colspace'] = 'DeviceRGB';
        $rgb['raw'] = "\x00\x11\x00\x22\x00\x33";
        $rgb['trns'] = [];
        $rgbOffset = 0;
        $rgb = $png->callGetTrnsChunk($rgb, $rgbOffset, 6);
        $this->assertSame([17, 34, 51], $rgb['trns']);
        $this->assertSame(10, $rgbOffset);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \RangeException
     */
    public function testGetIccpChunkThrowsOnInvalidCompressedProfile(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);

        $png = new PngChunkParsingHarness();
        $data = $this->getBaseData();
        $data['raw'] = "ICC\x00\x00BAD";
        $offset = 0;
        $byte = new \Com\Tecnick\File\Byte($data['raw']);

        \set_error_handler(static fn(): bool => true);
        try {
            $png->callGetIccpChunk($byte, $data, $offset, 8);
        } finally {
            \restore_error_handler();
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     */
    public function testGetResizedRawDataPreservesIndexedTransparency(): void
    {
        $import = $this->getImportHarness();

        $raw = \file_get_contents(__DIR__ . '/images/200x100_INDEXALPHA.png');
        $this->assertIsString($raw);

        $data = $this->getBaseData();
        $data['raw'] = $raw;
        $data['width'] = 200;
        $data['height'] = 100;

        // alpha=false exercises the indexed-palette transparency branch.
        \set_error_handler(static fn(): bool => true);
        try {
            $resized = $import->callGetResizedRawData($data, 100, 50, false, 90);
        } finally {
            \restore_error_handler();
        }

        $this->assertSame(100, $resized['width']);
        $this->assertSame(50, $resized['height']);
        $this->assertTrue($resized['recoded']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDimensionsByKeyFitWithZeroSourceReturnsBox(): void
    {
        $import = $this->getImportHarness();

        $data = $this->getRawData('zero-source');
        $data['width'] = 0;
        $data['height'] = 0;
        $import->setCacheEntry('zero-source', $data);

        // A zero-sized source falls back to the requested bounding box as-is.
        $this->assertSame(
            ['width' => 80, 'height' => 80],
            $import->getImageDimensionsByKey('zero-source', 80, 80, true),
        );
    }
}
