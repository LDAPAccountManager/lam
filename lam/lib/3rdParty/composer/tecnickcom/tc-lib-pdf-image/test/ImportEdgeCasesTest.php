<?php

/**
 * ImportEdgeCasesTest.php
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
 * Import edge cases test
 *
 * @since     2026-04-19
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 */
class ImportEdgeCasesTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Image\Import
    {
        $encrypt = $this->getTestEncrypt();
        return new \Com\Tecnick\Pdf\Image\Import(0.75, $encrypt, $this->getTestFileHelper());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithZeroWidth(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/200x100_RGB.png', 0, 50);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithZeroHeight(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/200x100_RGB.png', 100, 0);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithOnlyWidthKeepsAspectRatio(): void
    {
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/200x100_RGB.png', 100, null);

        $key = $import->getKey(__DIR__ . '/images/200x100_RGB.png', 100, 0, 100);
        $data = $import->getImageDataByKey($key);
        $this->assertSame(100, $data['width']);
        $this->assertSame(50, $data['height']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithOnlyHeightKeepsAspectRatio(): void
    {
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/200x100_RGB.png', null, 50);

        $key = $import->getKey(__DIR__ . '/images/200x100_RGB.png', 0, 50, 100);
        $data = $import->getImageDataByKey($key);
        $this->assertSame(100, $data['width']);
        $this->assertSame(50, $data['height']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithQualityLow(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.jpg', null, null, false, 0);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithQualityHigh(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.jpg', null, null, false, 100);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithQualityExceeding100(): void
    {
        $import = $this->getTestObject();
        // Quality > 100 should be clamped to 100
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.jpg', null, null, false, 150);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithNegativeQuality(): void
    {
        $import = $this->getTestObject();
        // Negative quality should be clamped to 0
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.jpg', null, null, false, -50);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithMaskAndAlphaParameters(): void
    {
        $import = $this->getTestObject();
        // Add image as mask
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png', null, null, true);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithDefprintParameter(): void
    {
        $import = $this->getTestObject();
        // Add with defprint=true
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png', null, null, false, 100, true);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithAlternateImages(): void
    {
        $import = $this->getTestObject();
        $iid1 = $import->add(__DIR__ . '/images/200x100_RGB.png');
        $iid2 = $import->add(__DIR__ . '/images/200x100_GRAY.jpg');
        // Add with alternate images
        $iid3 = $import->add(__DIR__ . '/images/200x100_RGBALPHA.png', null, null, false, 100, false, [$iid1, $iid2]);
        $this->assertGreaterThan(0, $iid3);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddImageFromStringData(): void
    {
        $import = $this->getTestObject();
        $fileData = \file_get_contents(__DIR__ . '/images/200x100_RGB.png');
        $this->assertIsString($fileData);

        // Add image from raw data
        $iid = $import->add('@' . $fileData);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetKeyConsistency(): void
    {
        $import = $this->getTestObject();
        // Same image parameters should produce same key
        $key1 = $import->getKey('/path/image.png', 100, 200, 75);
        $key2 = $import->getKey('/path/image.png', 100, 200, 75);
        $this->assertEquals($key1, $key2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetKeyDifference(): void
    {
        $import = $this->getTestObject();
        // Different parameters should produce different keys
        $key1 = $import->getKey('/path/image.png', 100, 200, 75);
        $key2 = $import->getKey('/path/image.png', 100, 200, 80);
        $this->assertNotEquals($key1, $key2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testMultipleAddOperations(): void
    {
        $import = $this->getTestObject();
        $iid1 = $import->add(__DIR__ . '/images/200x100_RGB.png');
        $iid2 = $import->add(__DIR__ . '/images/200x100_GRAY.jpg');
        $iid3 = $import->add(__DIR__ . '/images/200x100_RGBALPHA.png');

        $this->assertEquals(1, $iid1);
        $this->assertEquals(2, $iid2);
        $this->assertEquals(3, $iid3);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testRepeatedImageAddition(): void
    {
        $import = $this->getTestObject();
        // Adding the same image twice with same params should reuse cache
        $iid1 = $import->add(__DIR__ . '/images/200x100_RGB.png', 100, 50);
        $iid2 = $import->add(__DIR__ . '/images/200x100_RGB.png', 100, 50);

        // Should have different image IDs but share same cached data
        $this->assertEquals(1, $iid1);
        $this->assertEquals(2, $iid2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithResizeDownscale(): void
    {
        $import = $this->getTestObject();
        // Downscale image
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png', 100, 50);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAddWithResizeUpscale(): void
    {
        $import = $this->getTestObject();
        // Upscale image
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png', 400, 200);
        $this->assertGreaterThan(0, $iid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageAfterMultipleAdds(): void
    {
        $import = $this->getTestObject();
        $iid1 = $import->add(__DIR__ . '/images/200x100_RGB.png');
        $iid2 = $import->add(__DIR__ . '/images/200x100_GRAY.jpg');
        $iid3 = $import->add(__DIR__ . '/images/200x100_RGBALPHA.png');

        $result1 = $import->getSetImage($iid1, 0, 0, 100, 100, 600);
        $result2 = $import->getSetImage($iid2, 0, 0, 100, 100, 600);
        $result3 = $import->getSetImage($iid3, 0, 0, 100, 100, 600);

        $this->assertNotEmpty($result1);
        $this->assertNotEmpty($result2);
        $this->assertNotEmpty($result3);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testInvalidImageFile(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/nonexistent.png');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testInvalidImageData(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->add('@invalidbinarydata');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDimensionsByKeyReturnsOriginal(): void
    {
        $import = $this->getTestObject();
        $image = __DIR__ . '/images/200x100_RGB.png';
        $import->add($image);
        $key = $import->getKey($image, 0, 0, 100);

        $this->assertSame(['width' => 200, 'height' => 100], $import->getImageDimensionsByKey($key));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDimensionsByKeyDerivesHeightFromWidth(): void
    {
        $import = $this->getTestObject();
        $image = __DIR__ . '/images/200x100_RGB.png';
        $import->add($image);
        $key = $import->getKey($image, 0, 0, 100);

        $this->assertSame(['width' => 100, 'height' => 50], $import->getImageDimensionsByKey($key, 100));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDimensionsByKeyDerivesWidthFromHeight(): void
    {
        $import = $this->getTestObject();
        $image = __DIR__ . '/images/200x100_RGB.png';
        $import->add($image);
        $key = $import->getKey($image, 0, 0, 100);

        $this->assertSame(['width' => 100, 'height' => 50], $import->getImageDimensionsByKey($key, 0, 50));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDimensionsByKeyKeepsBothWhenSpecified(): void
    {
        $import = $this->getTestObject();
        $image = __DIR__ . '/images/200x100_RGB.png';
        $import->add($image);
        $key = $import->getKey($image, 0, 0, 100);

        $this->assertSame(['width' => 80, 'height' => 80], $import->getImageDimensionsByKey($key, 80, 80));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDimensionsByKeyFitWithinBox(): void
    {
        $import = $this->getTestObject();
        $image = __DIR__ . '/images/200x100_RGB.png';
        $import->add($image);
        $key = $import->getKey($image, 0, 0, 100);

        // box 80x80, source 200x100: scale = min(0.4, 0.8) = 0.4 => 80x40
        $this->assertSame(['width' => 80, 'height' => 40], $import->getImageDimensionsByKey($key, 80, 80, true));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDimensionsByKeyUnknownKeyThrows(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->getImageDimensionsByKey('nonexistent-key');
    }
}
