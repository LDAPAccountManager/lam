<?php

/**
 * TypeRuntimeBranchTest.php
 *
 * @since     2026-04-30
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Test;

class TypeRuntimeBranchTest extends TestUtil
{
    private function readUInt16(string $data, int $offset, bool $littleEndian): int
    {
        $bytes = substr($data, $offset, 2);
        if (strlen($bytes) !== 2) {
            return 0;
        }

        $byte0 = ord($bytes[0]);
        $byte1 = ord($bytes[1]);

        if ($littleEndian) {
            return $byte0 | ($byte1 << 8);
        }

        return ($byte0 << 8) | $byte1;
    }

    private function readUInt32(string $data, int $offset, bool $littleEndian): int
    {
        $bytes = substr($data, $offset, 4);
        if (strlen($bytes) !== 4) {
            return 0;
        }

        $byte0 = ord($bytes[0]);
        $byte1 = ord($bytes[1]);
        $byte2 = ord($bytes[2]);
        $byte3 = ord($bytes[3]);

        if ($littleEndian) {
            return $byte0 | ($byte1 << 8) | ($byte2 << 16) | ($byte3 << 24);
        }

        return ($byte0 << 24) | ($byte1 << 16) | ($byte2 << 8) | $byte3;
    }

    private function extractTiffStrip(string $tiffBlob): string
    {
        $byteOrder = substr($tiffBlob, 0, 2);
        $littleEndian = $byteOrder === 'II';
        if (!$littleEndian && $byteOrder !== 'MM') {
            return '';
        }

        $ifdOffset = $this->readUInt32($tiffBlob, 4, $littleEndian);
        if ($ifdOffset <= 0) {
            return '';
        }

        $numTags = $this->readUInt16($tiffBlob, $ifdOffset, $littleEndian);
        $cursor = $ifdOffset + 2;
        $stripOffset = 0;
        $stripByteCount = 0;

        for ($i = 0; $i < $numTags; ++$i) {
            $tag = $this->readUInt16($tiffBlob, $cursor, $littleEndian);
            $type = $this->readUInt16($tiffBlob, $cursor + 2, $littleEndian);
            $count = $this->readUInt32($tiffBlob, $cursor + 4, $littleEndian);
            $value = $this->readUInt32($tiffBlob, $cursor + 8, $littleEndian);

            if ($count === 1 && $tag === 273) {
                $stripOffset = $type === 3 ? $value & 0xFFFF : $value;
            }

            if ($count === 1 && $tag === 279) {
                $stripByteCount = $type === 3 ? $value & 0xFFFF : $value;
            }

            $cursor += 12;
        }

        if ($stripOffset <= 0 || $stripByteCount <= 0) {
            return '';
        }

        return substr($tiffBlob, $stripOffset, $stripByteCount);
    }

    protected function setUp(): void
    {
        parent::setUp();
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::reset();
    }

    protected function tearDown(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::reset();
        parent::tearDown();
    }

    public function testJpxMissingImagickPathViaShim(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$imagickLoaded = false;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\Jpx();
        $obj->decode('not-empty');
    }

    public function testCcittFaxMissingImagickPathViaShim(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$imagickLoaded = false;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\CcittFax();
        $obj->decode('not-empty');
    }

    public function testCcittFaxSuccessPathWithGeneratedCcittData(): void
    {
        if (!\extension_loaded('imagick')) {
            $this->markTestSkipped('ext-imagick is not available');
        }

        $img = new \Imagick();
        $img->newImage(1, 1, 'white');
        $img->setImageType(\Imagick::IMGTYPE_BILEVEL);
        $img->setImageDepth(1);
        $img->setImageCompression(\Imagick::COMPRESSION_GROUP4);
        $img->setImageFormat('tiff');

        $tiffBlob = $img->getImageBlob();
        $ccittData = $this->extractTiffStrip($tiffBlob);
        $this->assertNotSame('', $ccittData);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\CcittFax([
            'K' => 0,
            'Columns' => 1,
            'Rows' => 1,
            'BlackIs1' => false,
        ]);

        try {
            $decoded = $obj->decode($ccittData);
        } catch (\Com\Tecnick\Pdf\Filter\Exception $e) {
            if (str_contains($e->getMessage(), 'no decode delegate')) {
                $this->markTestSkipped('Imagick is available but TIFF/CCITT decode delegate is missing');
            }

            throw $e;
        }

        $this->assertStringStartsWith("\x89PNG", $decoded);
    }

    public function testJbigTwoTempFileCreationFailure(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamFail = true;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $obj->decode('payload');
    }

    public function testJbigTwoLaunchFailure(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamSequence = ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock'];
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$procOpenFail = true;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $obj->decode('payload');
    }

    public function testJbigTwoExitCodeFailure(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamSequence = ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock'];
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$procCloseCode = 1;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileExists = true;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $obj->decode('payload');
    }

    public function testJbigTwoMissingOutputFileFailure(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamSequence = ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock'];
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$procCloseCode = 0;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileExists = false;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $obj->decode('payload');
    }

    public function testJbigTwoOutputReadFailure(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamSequence = ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock'];
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$procCloseCode = 0;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileExists = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileGetContents = false;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $obj->decode('payload');
    }

    public function testJbigTwoSuccessPathViaShim(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamSequence = ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock'];
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$procCloseCode = 0;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileExists = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileGetContents = 'decoded-ok';

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $result = $obj->decode('payload');

        $this->assertSame('decoded-ok', $result);
        $this->assertSame(
            ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock'],
            \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$unlinked,
        );
    }

    public function testJbigTwoInputWriteFailure(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamSequence = ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock'];
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$putResult = false;

        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $obj->decode('payload');
    }

    public function testJbigTwoGlobalsSuccessPathViaShim(): void
    {
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$enabled = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$shellExecOutput = '/usr/bin/jbig2dec';
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$tempnamSequence = [
            '/tmp/jbig2in_mock',
            '/tmp/jbig2out_mock',
            '/tmp/jbig2glob_mock',
        ];
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$procCloseCode = 0;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileExists = true;
        \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$fileGetContents = 'decoded-ok';

        $obj = new \Com\Tecnick\Pdf\Filter\Type\JbigTwo();
        $result = $obj->decode('payload', ['JBIG2Globals' => 'shared-segments']);

        $this->assertSame('decoded-ok', $result);
        // The globals temp file is created and cleaned up alongside the in/out files.
        $this->assertSame(
            ['/tmp/jbig2in_mock', '/tmp/jbig2out_mock', '/tmp/jbig2glob_mock'],
            \Com\Tecnick\Pdf\Filter\Type\RuntimeShim::$unlinked,
        );
    }
}
