<?php

/**
 * ImportTest.php
 *
 * @since     2011-05-23
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

use PHPUnit\Framework\Attributes\DataProvider;

/**
 * Unit Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 */
class ImportTest extends TestUtil
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
    public function testGetKey(): void
    {
        $import = $this->getTestObject();
        $result = $import->getKey('/images/200x100_RGB.png', 200, 100, 100);
        $this->assertEquals('0i9dDNrAwOZdFa6L6u1zfg', $result);

        // The parts are delimited, so inputs that would concatenate to the same
        // string ('img' + 12 + 3 vs 'img1' + 2 + 3) must not collide.
        $this->assertNotEquals($import->getKey('img', 12, 3, 100), $import->getKey('img1', 2, 3, 100));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testConstructorUsesConfiguredFileHelper(): void
    {
        $encrypt = $this->getTestEncrypt();
        $allowedHosts = ['localhost', 'example.test'];
        $allowedPaths = [__DIR__ . '/images'];
        $maxRemoteSize = 10485760;
        $curlopts = [CURLOPT_TIMEOUT => 5];
        $defaultCurlOpts = [CURLOPT_TIMEOUT => 12];
        $fixedCurlOpts = [CURLOPT_SSL_VERIFYHOST => 2, CURLOPT_SSL_VERIFYPEER => true];

        $fileHelper = new \Com\Tecnick\File\File(
            allowedHosts: $allowedHosts,
            maxRemoteSize: $maxRemoteSize,
            curlopts: $curlopts,
            defaultCurlOpts: $defaultCurlOpts,
            fixedCurlOpts: $fixedCurlOpts,
            allowedPaths: $allowedPaths,
        );

        $import = new \Com\Tecnick\Pdf\Image\Import(0.75, $encrypt, $fileHelper);

        $method = new \ReflectionProperty($import, 'fileHelper');
        /** @var \Com\Tecnick\File\File $file */
        $file = $method->getValue($import);

        $ref = new \ReflectionClass($file);
        $allowedHostsProp = $ref->getProperty('allowedHosts');
        $allowedPathsProp = $ref->getProperty('allowedPaths');
        $maxRemoteSizeProp = $ref->getProperty('maxRemoteSize');
        $curloptsProp = $ref->getProperty('curlopts');
        $defaultProp = $ref->getProperty('defaultCurlOpts');
        $fixedProp = $ref->getProperty('fixedCurlOpts');

        $this->assertSame($allowedHosts, $allowedHostsProp->getValue($file));
        $this->assertSame($allowedPaths, $allowedPathsProp->getValue($file));
        $this->assertSame($maxRemoteSize, $maxRemoteSizeProp->getValue($file));
        $this->assertSame($curlopts, $curloptsProp->getValue($file));
        $this->assertSame($defaultCurlOpts, $defaultProp->getValue($file));
        $this->assertSame($fixedCurlOpts, $fixedProp->getValue($file));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetImageDataByKeyError(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->getImageDataByKey('missing');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetSetImageError(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->getSetImage(1, 2, 3, 5, 7, 17);
    }

    /**
     * @return array<int, array<int, string>>
     */
    public static function getBadAddValues(): array
    {
        return [
            [''],
            [__DIR__ . '/images/missing.png'],
            ['@'],
            ['@garbage'],
            ['*'],
            ['*http://www.example.com/image.png'],
        ];
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    #[DataProvider('getBadAddValues')]
    public function testAddError(string $bad): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Image\Exception::class);
        $import = $this->getTestObject();
        $import->add($bad);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Image\Exception
     * @throws \Com\Tecnick\File\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testAdd(): void
    {
        $import = $this->getTestObject();
        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG1 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        $iid = $import->add(__DIR__ . '/images/200x100_GRAY.jpg');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG2 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        $iid = $import->add(__DIR__ . '/images/200x100_GRAY.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG3 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        $iid = $import->add(__DIR__ . '/images/200x100_INDEX16.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG4 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        $iid = $import->add(__DIR__ . '/images/200x100_INDEX256.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG5 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        $iid = $import->add(__DIR__ . '/images/200x100_RGB.jpg');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG6 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG7 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        $iid = $import->add(__DIR__ . '/images/200x100_RGBALPHA.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMGplain8 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 200, 100, 600));

        $iid = $import->add(__DIR__ . '/images/200x100_INDEXALPHA.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG9 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            200,
            100,
            600,
        ));

        // resize

        $iid = $import->add(__DIR__ . '/images/200x100_RGB.png', 100, 50, true, 75, true);
        $this->assertEquals('q 75.000000 0 0 37.500000 2.250000 408.750000 cm /IMGmask10 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 100, 50, 600));

        $iid = $import->add(__DIR__ . '/images/200x100_RGBALPHA.png', 100, 50, true, 75, true);
        $this->assertEquals('q 75.000000 0 0 37.500000 2.250000 408.750000 cm /IMGmask11 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 100, 50, 600));

        $iid = $import->add(__DIR__ . '/images/200x100_INDEXALPHA.png', 100, 50, true, 75, true);
        $this->assertEquals('q 75.000000 0 0 37.500000 2.250000 408.750000 cm /IMGmask12 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 100, 50, 600));

        $iid = $import->add(__DIR__ . '/images/200x100_RGB.jpg', 100, 50, false, 75, true, [1, 2, 3]);
        $this->assertEquals('q 75.000000 0 0 37.500000 2.250000 408.750000 cm /IMG13 Do Q' . "\n", $import->getSetImage(
            $iid,
            3,
            5,
            100,
            50,
            600,
        ));

        // ICC

        $iid = $import->add(__DIR__ . '/images/200x100_RGBICC.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG14 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 200, 100, 600));

        $iid = $import->add(__DIR__ . '/images/200x100_RGBICC.jpg');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG15 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 200, 100, 600));

        $iid = $import->add(__DIR__ . '/images/200x100_RGBINT.png');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMGplain16 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 200, 100, 600));

        $iid = $import->add(__DIR__ . '/images/200x100_CMYK.jpg');
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG17 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 200, 100, 600));

        $key = $import->getKey(__DIR__ . '/images/200x100_INDEX256.png');
        $data = $import->getImageDataByKey($key);
        $this->assertEquals($key, $data['key']);

        $iid = $import->add('@' . $data['raw']);
        $this->assertEquals('q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG18 Do Q'
        . "\n", $import->getSetImage($iid, 3, 5, 200, 100, 600));

        // disabled because of libpngerror
        // $iid = $testObj->add('*http://localhost:8000/200x100_INDEX16.png');
        // $this->assertEquals(
        //     'q 150.000000 0 0 75.000000 2.250000 371.250000 cm /IMG18 Do Q' . "\n",
        //     $testObj->getSetImage($iid, 3, 5, 200, 100, 600)
        // );

        $out = $import->getOutImagesBlock(10);
        $this->assertNotEmpty($out);

        $this->assertEquals(37, $import->getObjectNumber());

        $xobjectDict = $import->getXobjectDict();
        $this->assertEquals(
            ' /IMG1 11 0 R /IMG2 12 0 R /IMG3 13 0 R /IMG4 15 0 R'
            . ' /IMG5 17 0 R /IMG6 18 0 R /IMG7 11 0 R /IMGplain8 20 0 R'
            . ' /IMG9 22 0 R /IMGmask10 23 0 R /IMGmask11 24 0 R'
            . ' /IMGmask12 25 0 R /IMG13 27 0 R /IMG14 29 0 R'
            . ' /IMG15 31 0 R /IMGplain16 33 0 R /IMG17 35 0 R /IMG18 37 0 R',
            $xobjectDict,
        );

        $xdByKeys = $import->getXobjectDictByKeys([2, 3]);
        $this->assertEquals(' /IMG2 12 0 R /IMG3 13 0 R', $xdByKeys);
    }
}
