<?php

/**
 * OutputTest.php
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
 * Output class test
 *
 * @since     2026-04-19
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 */
class OutputTest extends TestUtil
{
    /**
     * @throws \RuntimeException
     */
    public function throwWithFileHelperCallbackError(): void
    {
        throw new \RuntimeException('withFileHelper callback error');
    }

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
    public function testGetObjectNumber(): void
    {
        $import = $this->getTestObject();
        // Add some images and check object number increases after calling getOutImagesBlock
        $import->add(__DIR__ . '/images/200x100_RGB.png');
        $import->getOutImagesBlock(10);
        $this->assertGreaterThan(10, $import->getObjectNumber());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testWithFileHelperRestoresAfterSuccess(): void
    {
        $import = $this->getTestObject();
        $fileHelperProp = new \ReflectionProperty($import, 'fileHelper');
        /** @var \Com\Tecnick\File\File $originalFileHelper */
        $originalFileHelper = $fileHelperProp->getValue($import);

        $tempFileHelper = new \Com\Tecnick\File\File(allowedHosts: ['localhost'], allowedPaths: ['*']);

        $result = $import->withFileHelper($tempFileHelper, function () use (
            $import,
            $fileHelperProp,
            $tempFileHelper,
        ): string {
            $this->assertSame($tempFileHelper, $fileHelperProp->getValue($import));
            return 'ok';
        });

        $this->assertSame('ok', $result);
        $this->assertSame($originalFileHelper, $fileHelperProp->getValue($import));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testWithFileHelperRestoresAfterException(): void
    {
        $import = $this->getTestObject();
        $fileHelperProp = new \ReflectionProperty($import, 'fileHelper');
        /** @var \Com\Tecnick\File\File $originalFileHelper */
        $originalFileHelper = $fileHelperProp->getValue($import);

        $tempFileHelper = new \Com\Tecnick\File\File(allowedHosts: ['example.test'], allowedPaths: ['*']);

        $this->bcExpectException(\RuntimeException::class);

        try {
            $import->withFileHelper($tempFileHelper, [$this, 'throwWithFileHelperCallbackError']);
        } finally {
            $this->assertSame($originalFileHelper, $fileHelperProp->getValue($import));
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetXobjectDictEmpty(): void
    {
        $import = $this->getTestObject();
        $this->assertEquals('', $import->getXobjectDict());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetXobjectDictByKeysEmpty(): void
    {
        $import = $this->getTestObject();
        $this->assertEquals('', $import->getXobjectDictByKeys([]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetXobjectDictByKeysInvalid(): void
    {
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/200x100_RGB.png');
        $import->getOutImagesBlock(10);

        // Test with non-existent image IDs
        $result = $import->getXobjectDictByKeys([999]);
        $this->assertEquals('', $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetXobjectDictByKeysWithValidImage(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png');
        $import->getOutImagesBlock(10);

        $result = $import->getXobjectDictByKeys([$iid]);
        $this->assertNotEmpty($result);
        $this->assertStringContainsString('IMG' . $iid, $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetOutImagesBlockMultiple(): void
    {
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/200x100_RGB.png');
        $import->add(__DIR__ . '/images/200x100_GRAY.jpg');

        $result = $import->getOutImagesBlock(10);
        $this->assertNotEmpty($result);
        $this->assertStringContainsString('XObject', $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetOutImagesBlockObjectNumberIncrement(): void
    {
        $import = $this->getTestObject();
        $import->add(__DIR__ . '/images/200x100_RGB.png');
        $import->add(__DIR__ . '/images/200x100_RGBALPHA.png');

        $import->getOutImagesBlock(20);
        $this->assertGreaterThan(20, $import->getObjectNumber());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageWithMask(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGBALPHA.png', 100, 50, true, 75, true);

        $result = $import->getSetImage($iid, 3, 5, 100, 50, 600);
        $this->assertStringContainsString('IMGmask' . $iid, $result);
        $this->assertStringContainsString('cm /IMGmask' . $iid . ' Do Q', $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageRealOutput(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png');

        $result = $import->getSetImage($iid, 3, 5, 200, 100, 600);
        $this->assertStringContainsString('q ', $result);
        $this->assertStringContainsString('Do Q', $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageWithoutMaskOrPlain(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png');

        $result = $import->getSetImage($iid, 3, 5, 200, 100, 600);
        $this->assertStringContainsString('/IMG' . $iid . ' Do Q', $result);
        $this->assertStringNotContainsString('IMGplain', $result);
        $this->assertStringNotContainsString('IMGmask', $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageWithCoordinates(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png');

        // Test with different coordinates and page height
        $result = $import->getSetImage($iid, 10, 20, 100, 50, 800);
        $this->assertStringContainsString('q 75.000000 0 0 37.500000', $result);
        $this->assertStringContainsString('/IMG' . $iid . ' Do Q', $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageErrorInvalidId(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->getSetImage(999, 0, 0, 100, 100, 600);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageMultipleImages(): void
    {
        $import = $this->getTestObject();
        $iid1 = $import->add(__DIR__ . '/images/200x100_RGB.png');
        $iid2 = $import->add(__DIR__ . '/images/200x100_GRAY.jpg');

        $result1 = $import->getSetImage($iid1, 0, 0, 100, 100, 600);
        $result2 = $import->getSetImage($iid2, 10, 10, 150, 150, 600);

        $this->assertNotEmpty($result1);
        $this->assertNotEmpty($result2);
        $this->assertStringContainsString('IMG' . $iid1, $result1);
        $this->assertStringContainsString('IMG' . $iid2, $result2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageVariousCoordinates(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png');

        // Test with zero coordinates
        $result = $import->getSetImage($iid, 0, 0, 100, 100, 600);
        $this->assertStringContainsString('0.000000', $result);
        $this->assertStringContainsString('cm /IMG', $result);

        // Test with large coordinates
        $result2 = $import->getSetImage($iid, 100, 200, 300, 400, 1000);
        $this->assertStringContainsString('300.000000', $result2);
        $this->assertStringContainsString('cm /IMG', $result2);
    }
}
