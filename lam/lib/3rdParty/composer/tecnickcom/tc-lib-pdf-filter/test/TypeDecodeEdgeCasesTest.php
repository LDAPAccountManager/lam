<?php

/**
 * TypeDecodeEdgeCasesTest.php
 *
 * @since     2026-04-19
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

class TypeDecodeEdgeCasesTest extends TestUtil
{
    /**
     * @return array<int, string>
     */
    private function getInitialLzwDictionary(): array
    {
        $dictionary = [];
        for ($i = 0; $i < 256; ++$i) {
            $dictionary[$i] = \chr($i);
        }

        return $dictionary;
    }

    /**
     * @param array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string} $state
     *
     * @return array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string}
     */
    private function callLzwProcessIndex(\Com\Tecnick\Pdf\Filter\Type\Lzw $obj, int $index, array $state): array
    {
        /**
         * @var \Closure(\Com\Tecnick\Pdf\Filter\Type\Lzw, int, array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string}): array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string}|null $caller
         */
        $caller = \Closure::bind(
            /** @param array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string} $lzwState */
            static fn(
                \Com\Tecnick\Pdf\Filter\Type\Lzw $target,
                int $code,
                array $lzwState,
            ): array => $target->processIndex($code, $lzwState),
            null,
            \Com\Tecnick\Pdf\Filter\Type\Lzw::class,
        );

        if ($caller === null) {
            return $state;
        }

        return $caller($obj, $index, $state);
    }

    private function getCcittTagValue(string $tiff, int $tagId): int
    {
        // TIFF header: 2-byte order + 2-byte version + 4-byte IFD offset.
        $ifdHeader = \unpack('Voffset', \substr($tiff, 4, 4));
        $ifdOffset = (int) ($ifdHeader['offset'] ?? 0);

        $header = \unpack('vcount', \substr($tiff, $ifdOffset, 2));
        $count = (int) ($header['count'] ?? 0);
        $cursor = $ifdOffset + 2;

        for ($i = 0; $i < $count; ++$i) {
            $entry = \unpack('vtag/vtype/Vitems/Vvalue', \substr($tiff, $cursor, 12));
            if ((int) ($entry['tag'] ?? 0) === $tagId) {
                return (int) ($entry['value'] ?? 0);
            }

            $cursor += 12;
        }

        return 0;
    }

    private function buildCcittHeader(\Com\Tecnick\Pdf\Filter\Type\CcittFax $obj, string $data): string
    {
        /** @var \Closure(\Com\Tecnick\Pdf\Filter\Type\CcittFax, string): string|null $builder */
        $builder = \Closure::bind(
            static fn(
                \Com\Tecnick\Pdf\Filter\Type\CcittFax $target,
                string $ccittData,
            ): string => $target->buildTiffHeader($ccittData),
            null,
            \Com\Tecnick\Pdf\Filter\Type\CcittFax::class,
        );

        if ($builder === null) {
            return '';
        }

        return $builder($obj, $data);
    }

    public function testTypeDecodeEmptyInput(): void
    {
        $types = [
            new \Com\Tecnick\Pdf\Filter\Type\AsciiEightFive(),
            new \Com\Tecnick\Pdf\Filter\Type\AsciiHex(),
            new \Com\Tecnick\Pdf\Filter\Type\CcittFax(),
            new \Com\Tecnick\Pdf\Filter\Type\Crypt(),
            new \Com\Tecnick\Pdf\Filter\Type\Dct(),
            new \Com\Tecnick\Pdf\Filter\Type\Flate(),
            new \Com\Tecnick\Pdf\Filter\Type\JbigTwo(),
            new \Com\Tecnick\Pdf\Filter\Type\Jpx(),
            new \Com\Tecnick\Pdf\Filter\Type\Lzw(),
            new \Com\Tecnick\Pdf\Filter\Type\RunLength(),
        ];

        foreach ($types as $type) {
            $this->assertSame('', $type->decode(''));
        }
    }

    public function testAsciiEightFiveInvalidZInsideGroup(): void
    {
        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);
        $obj = new \Com\Tecnick\Pdf\Filter\Type\AsciiEightFive();
        $obj->decode('!z~>');
    }

    public function testAsciiEightFiveLastTupleCases(): void
    {
        $obj = new \Com\Tecnick\Pdf\Filter\Type\AsciiEightFive();

        // Ends with an incomplete 4-char tuple.
        $res4 = $obj->decode('!!!!~>');
        $this->assertSame(3, \strlen($res4));

        // Ends with an incomplete 2-char tuple.
        $res2 = $obj->decode('!!~>');
        $this->assertSame(1, \strlen($res2));

        // Ends with an incomplete 3-char tuple.
        $res3 = $obj->decode('!!!~>');
        $this->assertSame(2, \strlen($res3));
    }

    public function testAsciiEightFiveSingleTrailingCharThrows(): void
    {
        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);
        $obj = new \Com\Tecnick\Pdf\Filter\Type\AsciiEightFive();
        $obj->decode('!~>');
    }

    public function testFlateDecodeRawDeflatePayload(): void
    {
        $obj = new \Com\Tecnick\Pdf\Filter\Type\Flate();
        $expected = 'raw-deflate-fallback';
        $rawDeflate = (string) \gzdeflate($expected);

        $this->assertSame($expected, $obj->decode($rawDeflate));
    }

    public function testFlateDecodeHeaderStrippedFallback(): void
    {
        $obj = new \Com\Tecnick\Pdf\Filter\Type\Flate();
        $expected = 'header-stripped-fallback';
        $rawDeflate = (string) \gzdeflate($expected);
        // Build malformed zlib-like data: header + raw payload without Adler-32 trailer.
        $payload = "\x78\x9c" . $rawDeflate;

        $this->assertSame($expected, $obj->decode($payload));
    }

    public function testFlateDecodeGzipWrappedPayload(): void
    {
        $obj = new \Com\Tecnick\Pdf\Filter\Type\Flate();
        $expected = 'gzip-fallback';
        $gzipPayload = (string) \gzencode($expected);

        $this->assertSame($expected, $obj->decode($gzipPayload));
    }

    public function testLzwProcessIndexOutsideDictionary(): void
    {
        $obj = new LzwProxy();

        $decoded = '';
        $bitstring = '000000000';
        $bitlen = 9;
        $dataLength = 9;
        $index = 300;
        $dictionary = [];
        for ($i = 0; $i < 256; ++$i) {
            $dictionary[$i] = \chr($i);
        }

        $dix = 258;
        $prevIndex = 65;
        $obj->callProcess($decoded, $bitstring, $bitlen, $dataLength, $index, $dictionary, $dix, $prevIndex);

        $this->assertSame('AA', $decoded);
        $this->assertSame('AA', $dictionary[258]);
        $this->assertSame(259, $dix);
    }

    public function testLzwProcessBitLengthThresholds(): void
    {
        $obj = new LzwProxy();

        $cases = [
            [510, 10],
            [1022, 11],
            [2046, 12],
        ];

        foreach ($cases as [$startDix, $expectedBitlen]) {
            $decoded = '';
            $bitstring = '000000000';
            $bitlen = 9;
            $dataLength = 9;
            $index = 65;
            $dictionary = [];
            for ($i = 0; $i < 256; ++$i) {
                $dictionary[$i] = \chr($i);
            }

            $dix = $startDix;
            $prevIndex = 66;

            $obj->callProcess($decoded, $bitstring, $bitlen, $dataLength, $index, $dictionary, $dix, $prevIndex);

            $this->assertSame($expectedBitlen, $bitlen);
        }
    }

    public function testLzwProcessIndexAtOrBeyondDictionarySize(): void
    {
        $obj = new \Com\Tecnick\Pdf\Filter\Type\Lzw();
        $state = [
            'bitlen' => 9,
            'dix' => 259,
            'prev_index' => 65,
            'dictionary' => $this->getInitialLzwDictionary(),
            'decoded' => '',
        ];

        $updated = $this->callLzwProcessIndex($obj, 300, $state);

        $this->assertSame('AA', $updated['decoded']);
        $this->assertSame('AA', $updated['dictionary'][259]);
        $this->assertSame(260, $updated['dix']);
    }

    public function testLzwProcessIndexBitLengthMatchBranches(): void
    {
        $obj = new \Com\Tecnick\Pdf\Filter\Type\Lzw();
        $cases = [
            [510, 10],
            [1022, 11],
            [2046, 12],
        ];

        foreach ($cases as [$startDix, $expectedBitlen]) {
            $state = [
                'bitlen' => 9,
                'dix' => $startDix,
                'prev_index' => 66,
                'dictionary' => $this->getInitialLzwDictionary(),
                'decoded' => '',
            ];

            $updated = $this->callLzwProcessIndex($obj, 65, $state);
            $this->assertSame($expectedBitlen, $updated['bitlen']);
        }
    }

    public function testCcittFaxConstructorNormalizesParams(): void
    {
        $ccittData = "\xAA\xBB";

        $asTrue = new \Com\Tecnick\Pdf\Filter\Type\CcittFax([
            'BlackIs1' => 'YeS',
            'K' => -1,
            'Columns' => 10,
            'Rows' => 0,
        ]);

        // K = -1 => Group 4 (T.6) => TIFF compression 4, no T4Options.
        $tiffTrue = $this->buildCcittHeader($asTrue, $ccittData);
        $this->assertSame(4, $this->getCcittTagValue($tiffTrue, 259));
        $this->assertSame(1, $this->getCcittTagValue($tiffTrue, 262));
        $this->assertSame(10, $this->getCcittTagValue($tiffTrue, 256));
        $this->assertSame(2, $this->getCcittTagValue($tiffTrue, 257));
        $this->assertSame(0, $this->getCcittTagValue($tiffTrue, 292));

        $asFalse = new \Com\Tecnick\Pdf\Filter\Type\CcittFax([
            'BlackIs1' => ['unexpected'],
            'Columns' => 0,
            'Rows' => -5,
        ]);

        // K = 0 (default) => Group 3 1-D => TIFF compression 3, T4Options = 0.
        $tiffFalse = $this->buildCcittHeader($asFalse, $ccittData);
        $this->assertSame(3, $this->getCcittTagValue($tiffFalse, 259));
        $this->assertSame(0, $this->getCcittTagValue($tiffFalse, 262));
        $this->assertSame(1, $this->getCcittTagValue($tiffFalse, 256));
        $this->assertSame(16, $this->getCcittTagValue($tiffFalse, 257));
        $this->assertSame(0, $this->getCcittTagValue($tiffFalse, 292));
    }

    public function testCcittFaxGroup3TwoDimensionalSetsT4Options(): void
    {
        $ccittData = "\xAA\xBB";

        // K > 0 => Group 3 2-D => TIFF compression 3 with T4Options bit 0 set.
        $mixed = new \Com\Tecnick\Pdf\Filter\Type\CcittFax(['K' => 4, 'Columns' => 8]);
        $tiff = $this->buildCcittHeader($mixed, $ccittData);
        $this->assertSame(3, $this->getCcittTagValue($tiff, 259));
        $this->assertSame(1, $this->getCcittTagValue($tiff, 292));
    }

    public function testCcittFaxConstructorAcceptsNumericAndBooleanBlackIs1(): void
    {
        $ccittData = "\x0F";

        $numericTrue = new \Com\Tecnick\Pdf\Filter\Type\CcittFax(['BlackIs1' => 2]);
        $tiffNumeric = $this->buildCcittHeader($numericTrue, $ccittData);
        $this->assertSame(1, $this->getCcittTagValue($tiffNumeric, 262));

        $boolFalse = new \Com\Tecnick\Pdf\Filter\Type\CcittFax(['BlackIs1' => false]);
        $tiffBool = $this->buildCcittHeader($boolFalse, $ccittData);
        $this->assertSame(0, $this->getCcittTagValue($tiffBool, 262));
    }

    public function testCcittFaxDecodeWrapsImagickException(): void
    {
        if (!\extension_loaded('imagick')) {
            $this->markTestSkipped('ext-imagick is not available');
        }

        $obj = new class extends \Com\Tecnick\Pdf\Filter\Type\CcittFax {
            protected function newImagick(): \Imagick
            {
                throw new \ImagickException('forced imagick failure');
            }
        };

        try {
            $obj->decode("\xAA");
            $this->fail('Expected wrapped exception when Imagick throws');
        } catch (\Com\Tecnick\Pdf\Filter\Exception $e) {
            $this->assertStringContainsString('CCITTFaxDecode: Imagick failed to decode the stream:', $e->getMessage());
            $this->assertStringContainsString('forced imagick failure', $e->getMessage());
        }
    }

    public function testJpxDecodeWrapsImagickException(): void
    {
        if (!\extension_loaded('imagick')) {
            $this->markTestSkipped('ext-imagick is not available');
        }

        $obj = new class extends \Com\Tecnick\Pdf\Filter\Type\Jpx {
            protected function newImagick(): \Imagick
            {
                throw new \ImagickException('forced imagick failure');
            }
        };

        try {
            $obj->decode("\xAA");
            $this->fail('Expected wrapped exception when Imagick throws');
        } catch (\Com\Tecnick\Pdf\Filter\Exception $e) {
            $this->assertStringContainsString('JPXDecode: Imagick failed to decode the stream:', $e->getMessage());
            $this->assertStringContainsString('forced imagick failure', $e->getMessage());
        }
    }

    public function testRunLengthTruncatedRunDoesNotWarn(): void
    {
        $obj = new \Com\Tecnick\Pdf\Filter\Type\RunLength();

        // "\x02XYZ" copies 3 literal bytes; the trailing "\xC8" (200) is a run-length
        // marker with no following byte to repeat. The decoder must stop cleanly
        // instead of reading past the end of the string.
        $errors = [];
        \set_error_handler(static function (int $_errno, string $errstr) use (&$errors): bool {
            $errors[] = $errstr;
            return true;
        });

        try {
            $result = $obj->decode("\x02XYZ\xC8");
        } finally {
            \restore_error_handler();
        }

        $this->assertSame('XYZ', $result);
        $this->assertSame([], $errors);
    }
}
