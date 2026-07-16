<?php

/**
 * JpegTest.php
 *
 * @since     2026-04-19
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-pdf-image software library.
 */

namespace Test;

/**
 * Jpeg class test
 *
 * @since     2026-04-19
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 */
class JpegTest extends TestUtil
{
    /**
     * @throws \RangeException
     */
    public function testGetDataRgbJpeg(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_RGB.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 3,
            'colspace' => 'DeviceRGB',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_RGB.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals('DCTDecode', $result['filter']);
        $this->assertNotEmpty($result['data']);
        $this->assertEquals($file, $result['data']);
    }

    /**
     * @throws \RangeException
     */
    public function testGetDataGrayJpeg(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_GRAY.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 1,
            'colspace' => 'DeviceGray',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_GRAY.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals('DCTDecode', $result['filter']);
        $this->assertEquals(1, $result['channels']);
    }

    /**
     * @throws \RangeException
     */
    public function testGetDataCmykJpeg(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_CMYK.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 4,
            'colspace' => 'DeviceCMYK',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_CMYK.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals('DCTDecode', $result['filter']);
        $this->assertEquals(4, $result['channels']);
        $this->assertEquals('DeviceCMYK', $result['colspace']);
    }

    /**
     * @throws \RangeException
     */
    public function testGetDataJpegWithIcc(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_RGBICC.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 3,
            'colspace' => 'DeviceRGB',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_RGBICC.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals('DCTDecode', $result['filter']);
        $this->assertNotEmpty($result['icc']);
    }

    /**
     * @throws \RangeException
     */
    public function testGetDataJpegWithoutIcc(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_RGB.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 3,
            'colspace' => 'DeviceRGB',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_RGB.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals('DCTDecode', $result['filter']);
        $this->assertEmpty($result['icc']);
    }

    /**
     * @throws \RangeException
     */
    public function testGetDataPreservesRawData(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_RGB.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 3,
            'colspace' => 'DeviceRGB',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_RGB.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals($file, $result['raw']);
        $this->assertEquals($file, $result['data']);
    }

    /**
     * @throws \RangeException
     */
    public function testGetDataWithMissingIcc(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_RGB.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 3,
            'colspace' => 'DeviceRGB',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_RGB.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals('DCTDecode', $result['filter']);
        $this->assertEquals('DeviceRGB', $result['colspace']);
    }

    /**
     * @throws \RangeException
     */
    public function testGetDataPreservesChannels(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $file = \file_get_contents(__DIR__ . '/images/200x100_GRAY.jpg');
        $this->assertIsString($file);

        $data = [
            'bits' => 8,
            'channels' => 1,
            'colspace' => 'DeviceGray',
            'data' => '',
            'exturl' => false,
            'file' => __DIR__ . '/images/200x100_GRAY.jpg',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $file,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];

        $result = $import->getData($data);

        $this->assertEquals(1, $result['channels']);
        $this->assertEquals($file, $result['data']);
    }

    /**
     * A crafted "ICC_PROFILE" marker preceded by a 2-byte length of 0 yields a
     * negative segment length. The scanner must still move forward instead of
     * re-finding the same marker forever (DoS regression).
     *
     * @throws \RangeException
     */
    public function testGetDataDoesNotHangOnMalformedIccMarker(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $raw = 'XX' . "\x00\x00" . 'ICC_PROFILE' . "\x00\x01\x01" . 'PAYLOAD';
        $data = $this->makeJpegData($raw);

        $result = $import->getData($data);

        $this->assertSame('DCTDecode', $result['filter']);
        $this->assertSame('', $result['icc']);
        $this->assertSame($raw, $result['data']);
    }

    /**
     * A marker at the very start of the stream has too few bytes before it to
     * carry a length: it must be skipped without throwing or looping.
     *
     * @throws \RangeException
     */
    public function testGetDataIgnoresIccMarkerWithoutLengthPrefix(): void
    {
        $import = new \Com\Tecnick\Pdf\Image\Import\Jpeg();
        $raw = 'ICC_PROFILE' . "\x00" . 'rest';
        $data = $this->makeJpegData($raw);

        $result = $import->getData($data);

        $this->assertSame('', $result['icc']);
    }

    /**
     * Build a minimal JPEG base-data array wrapping the given raw bytes.
     *
     * @return ImageBaseData
     */
    private function makeJpegData(string $raw): array
    {
        return [
            'bits' => 8,
            'channels' => 3,
            'colspace' => 'DeviceRGB',
            'data' => '',
            'exturl' => false,
            'file' => '',
            'filter' => '',
            'height' => 100,
            'icc' => '',
            'ismask' => false,
            'key' => 'test',
            'mapto' => IMAGETYPE_JPEG,
            'native' => true,
            'obj' => 0,
            'obj_alt' => 0,
            'obj_icc' => 0,
            'obj_pal' => 0,
            'pal' => '',
            'parms' => '',
            'raw' => $raw,
            'recode' => false,
            'recoded' => false,
            'splitalpha' => false,
            'trns' => [],
            'type' => IMAGETYPE_JPEG,
            'width' => 200,
        ];
    }
}
