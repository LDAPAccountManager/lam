<?php

/**
 * XrefTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Pdfparser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Parser\Exception as PPException;

class XrefTest extends TestCase
{
    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testProcessObjIndexesHandlesInUseAndCompressedObjects(): void
    {
        $xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => [],
        ];
        $obj_num = 3;
        $sdata = [
            [1, 42, 0],
            [2, 7, 4],
            [0, 0, 0],
        ];

        $parser = new XrefStreamHarness();
        $parser->processObjIndexesPublic($xref, $obj_num, $sdata);

        $this->assertSame(6, $obj_num);
        $this->assertSame(42, $xref['xref']['3_0'] ?? null);
        $this->assertSame('7_0_4', $xref['xref']['4_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testPngUnpredictorDecodesAndRejectsUnknownPredictor(): void
    {
        $parser = new XrefStreamHarness();

        $decoded = [];
        $parser->pngUnpredictorPublic([[0, 10]], $decoded, 1, [0]);
        if (!isset($decoded[0][0])) {
            $this->fail('Decoded first row value is missing.');
        }
        $this->assertSame(10, $decoded[0][0]);

        $decoded = [];
        $parser->pngUnpredictorPublic([[1, 5]], $decoded, 1, [0]);
        if (!isset($decoded[0][0])) {
            $this->fail('Decoded first row value is missing.');
        }
        $this->assertSame(5, $decoded[0][0]);

        $decoded = [];
        $parser->pngUnpredictorPublic([[2, 3]], $decoded, 1, [4]);
        if (!isset($decoded[0][0])) {
            $this->fail('Decoded first row value is missing.');
        }
        $this->assertSame(7, $decoded[0][0]);

        $decoded = [];
        $parser->pngUnpredictorPublic([[4, 8]], $decoded, 1, [1]);
        if (!isset($decoded[0][0])) {
            $this->fail('Decoded first row value is missing.');
        }
        $this->assertSame(9, $decoded[0][0]);

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Unknown PNG predictor');
        $decoded = [];
        $parser->pngUnpredictorPublic([[9, 1]], $decoded, 1, [0]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefParsesEntriesAndTrailer(): void
    {
        $pdf =
            "xref\r\n"
            . "0 2\r\n"
            . "0000000000 65535 f\r\n"
            . "0000000017 00000 n\r\n"
            . "trailer << /Size 2 /Root 1 0 R /Info 2 0 R /Encrypt 3 0 R /ID [<AA><BB>] >>\r\n";

        $parser = new XrefHarness();
        $parser->setPdfDataPublic($pdf);
        $xref = $parser->decodeXrefPublic(0, ['xref' => []]);

        $this->assertSame(17, $xref['xref']['1_0'] ?? null);
        $this->assertSame(2, $xref['trailer']['size']);
        $this->assertSame('1_0', $xref['trailer']['root']);
        $this->assertSame('2_0', $xref['trailer']['info']);
        $this->assertSame('3_0', $xref['trailer']['encrypt'] ?? null);
        $this->assertSame(['AA', 'BB'], $xref['trailer']['id']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefLeavesEncryptUnsetWhenMissing(): void
    {
        $pdf =
            "xref\r\n"
            . "0 1\r\n"
            . "0000000017 00000 n\r\n"
            . "trailer << /Size 1 /Root 1 0 R /Info 2 0 R /ID [<AA><BB>] >>\r\n";

        $parser = new XrefHarness();
        $parser->setPdfDataPublic($pdf);
        $xref = $parser->decodeXrefPublic(0, ['xref' => []]);

        $this->assertArrayNotHasKey('encrypt', $xref['trailer']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamParsesRowsAndTrailer(): void
    {
        $stream_data = \pack('C*', 0, 1, 10, 0, 0, 2, 5, 1);

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '5_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '1', 0], ['numeric', '1', 0]], 0],
                    ['/', 'Index', 0],
                    ['[', [['numeric', '0', 0], ['numeric', '2', 0]], 0],
                    ['/', 'Size', 0],
                    ['numeric', '2', 0],
                    ['/', 'Root', 0],
                    ['objref', '1_0', 0],
                    ['/', 'ID', 0],
                    ['[', [['hex', 'AA', 0], ['hex', 'BB', 0]], 0],
                    ['/', 'DecodeParms', 0],
                    ['<<', [['/', 'Columns', 0], ['numeric', '3', 0], ['/', 'Predictor', 0], ['numeric', '10', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $xref = $parser->decodeXrefStreamPublic(100, ['xref' => []]);

        $this->assertSame(10, $xref['xref']['0_0'] ?? null);
        $this->assertSame('5_0_1', $xref['xref']['1_0'] ?? null);
        $this->assertSame('1_0', $xref['trailer']['root']);
        $this->assertSame(2, $xref['trailer']['size']);
        $this->assertSame(['AA', 'BB'], $xref['trailer']['id']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamMapsMultiRangeIndexSections(): void
    {
        $stream_data = \pack(
            'C*',
            0,
            1,
            0,
            0,
            30,
            0,
            1,
            0,
            0,
            31,
            0,
            1,
            0,
            0,
            32,
            0,
            1,
            0,
            0,
            33,
            0,
            1,
            0,
            0,
            34,
            0,
            1,
            0,
            0,
            35,
            0,
            1,
            0,
            0,
            36,
        );

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '5_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '3', 0], ['numeric', '0', 0]], 0],
                    ['/', 'Index', 0],
                    [
                        '[',
                        [
                            ['numeric', '3', 0],
                            ['numeric', '1', 0],
                            ['numeric', '15', 0],
                            ['numeric', '1', 0],
                            ['numeric', '17', 0],
                            ['numeric', '2', 0],
                            ['numeric', '41', 0],
                            ['numeric', '3', 0],
                        ],
                        0,
                    ],
                    ['/', 'Size', 0],
                    ['numeric', '44', 0],
                    ['/', 'DecodeParms', 0],
                    ['<<', [['/', 'Columns', 0], ['numeric', '4', 0], ['/', 'Predictor', 0], ['numeric', '10', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $xref = $parser->decodeXrefStreamPublic(100, ['xref' => []]);

        $this->assertSame(30, $xref['xref']['3_0'] ?? null);
        $this->assertSame(31, $xref['xref']['15_0'] ?? null);
        $this->assertSame(32, $xref['xref']['17_0'] ?? null);
        $this->assertSame(33, $xref['xref']['18_0'] ?? null);
        $this->assertSame(34, $xref['xref']['41_0'] ?? null);
        $this->assertSame(35, $xref['xref']['42_0'] ?? null);
        $this->assertSame(36, $xref['xref']['43_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamFallsBackToSizeWhenIndexIsMissing(): void
    {
        $stream_data = \pack('C*', 0, 1, 0, 0, 20, 0, 1, 0, 0, 21);

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '5_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '3', 0], ['numeric', '0', 0]], 0],
                    ['/', 'Size', 0],
                    ['numeric', '2', 0],
                    ['/', 'DecodeParms', 0],
                    ['<<', [['/', 'Columns', 0], ['numeric', '4', 0], ['/', 'Predictor', 0], ['numeric', '10', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $xref = $parser->decodeXrefStreamPublic(100, ['xref' => []]);

        $this->assertSame(20, $xref['xref']['0_0'] ?? null);
        $this->assertSame(21, $xref['xref']['1_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamRejectsOddIndexArrayLength(): void
    {
        $stream_data = \pack('C*', 0, 1, 0, 0, 20);

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '5_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '3', 0], ['numeric', '0', 0]], 0],
                    ['/', 'Index', 0],
                    ['[', [['numeric', '3', 0], ['numeric', '1', 0], ['numeric', '15', 0]], 0],
                    ['/', 'Size', 0],
                    ['numeric', '16', 0],
                    ['/', 'DecodeParms', 0],
                    ['<<', [['/', 'Columns', 0], ['numeric', '4', 0], ['/', 'Predictor', 0], ['numeric', '10', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Invalid xref stream Index array: expected even number of values');
        $parser->decodeXrefStreamPublic(100, ['xref' => []]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamRejectsRowCountMismatchAgainstIndexCoverage(): void
    {
        $stream_data = \pack('C*', 0, 1, 0, 0, 20, 0, 1, 0, 0, 21);

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '5_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '3', 0], ['numeric', '0', 0]], 0],
                    ['/', 'Index', 0],
                    ['[', [['numeric', '3', 0], ['numeric', '3', 0]], 0],
                    ['/', 'Size', 0],
                    ['numeric', '20', 0],
                    ['/', 'DecodeParms', 0],
                    ['<<', [['/', 'Columns', 0], ['numeric', '4', 0], ['/', 'Predictor', 0], ['numeric', '10', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Invalid xref stream row count: expected 3 rows from Index, got 2');
        $parser->decodeXrefStreamPublic(100, ['xref' => []]);
    }

    public function testProcessDdataUsesDefaultTypeWhenFirstFieldWidthIsZero(): void
    {
        $parser = new XrefHarness();
        $sdata = [];
        $parser->processDdataPublic($sdata, [[9, 3]], [0, 1, 1]);

        $this->assertSame([1, 9, 3], $sdata[0] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetXrefDataThrowsWhenStartxrefIsMissing(): void
    {
        $parser = new XrefHarness();
        $parser->setPdfDataPublic("%PDF-1.7\nNo xref markers here");

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Unable to find startxref (1)');
        $parser->getXrefDataPublic();
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testProcessObjIndexesIgnoresUnknownEntryTypes(): void
    {
        $xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => [],
        ];
        $obj_num = 9;

        $parser = new XrefStreamHarness();
        $parser->processObjIndexesPublic($xref, $obj_num, [[9, 1, 2]]);

        $this->assertSame(10, $obj_num);
        $this->assertSame([], $xref['xref']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testPngUnpredictorCoversAveragePredictor(): void
    {
        $parser = new XrefStreamHarness();
        $decoded = [];

        $parser->pngUnpredictorPublic([[3, 4]], $decoded, 1, [2]);

        if (!isset($decoded[0][0])) {
            $this->fail('Decoded first row value is missing.');
        }

        $this->assertSame(5, $decoded[0][0]);
    }

    public function testMinDistanceCoversAllOutcomeBranches(): void
    {
        $parser = new XrefStreamHarness();
        $ddata = [[0]];

        $parser->minDistancePublic($ddata, 0, 5, 0, [7, 2, 0]);
        $this->assertSame(12, $ddata[0][0] ?? null);

        $parser->minDistancePublic($ddata, 0, 5, 0, [10, 3, 9]);
        $this->assertSame(8, $ddata[0][0] ?? null);

        $parser->minDistancePublic($ddata, 0, 5, 0, [0, 10, 4]);
        $this->assertSame(9, $ddata[0][0] ?? null);
    }

    public function testProcessXrefPrevAndDecodeParmsHandleInvalidInput(): void
    {
        $parser = new XrefStreamHarness();

        $prevxref = null;
        $parser->processXrefPrevPublic(['name', 'x', 0], $prevxref);
        $this->assertNull($prevxref);

        $parser->processXrefPrevPublic(['numeric', '17', 0], $prevxref);
        $this->assertSame(17, $prevxref);

        $columns = 9;
        $predictor = 3;
        $parser->processXrefDecodeParmsPublic(['name', 'x', 0], $columns, $predictor);
        $this->assertSame(9, $columns);
        $this->assertSame(3, $predictor);

        $parser->processXrefDecodeParmsPublic(
            ['<<', [['/', 'Columns', 0], ['numeric', '-5', 0], ['/', 'Predictor', 0], ['numeric', '12', 0]], 0],
            $columns,
            $predictor,
        );
        $this->assertSame(0, $columns);
        $this->assertSame(12, $predictor);
    }

    public function testProcessXrefTypeFtAndObjrefCoverTrailerBranches(): void
    {
        $parser = new XrefStreamHarness();
        $xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => [],
        ];

        $sarr = [
            ['/', 'Root', 0],
            ['objref', '1_0', 0],
            ['/', 'Info', 0],
            ['objref', '2_0', 0],
            ['/', 'Encrypt', 0],
            ['objref', '3_0', 0],
            ['/', 'Size', 0],
            ['numeric', '4', 0],
            ['/', 'ID', 0],
            ['[', [['hex', 'AA', 0], ['hex', 'BB', 0]], 0],
            ['/', 'ID', 0],
            ['[', [['hex', '', 0], ['hex', 'BB', 0]], 0],
        ];

        $parser->processXrefTypeFtPublic('Root', $sarr, 0, $xref, true);
        $parser->processXrefTypeFtPublic('Info', $sarr, 2, $xref, true);
        $parser->processXrefTypeFtPublic('Encrypt', $sarr, 4, $xref, true);
        $parser->processXrefTypeFtPublic('Size', $sarr, 6, $xref, true);
        $parser->processXrefTypeFtPublic('ID', $sarr, 8, $xref, true);
        $parser->processXrefTypeFtPublic('ID', $sarr, 10, $xref, true);

        $this->assertSame('1_0', $xref['trailer']['root']);
        $this->assertSame('2_0', $xref['trailer']['info']);
        $this->assertSame('3_0', $xref['trailer']['encrypt'] ?? null);
        $this->assertSame(4, $xref['trailer']['size']);
        $this->assertSame(['AA', 'BB'], $xref['trailer']['id']);

        $before = $xref;
        $parser->processXrefTypeFtPublic('Size', $sarr, 6, $xref, false);
        $this->assertSame($before, $xref);

        $objref = $parser->processXrefObjrefPublic('Root', [['/', 'Root', 0], ['name', 'x', 0]], 0, $xref);
        $this->assertSame($xref, $objref);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefThrowsWhenTrailerIsMissing(): void
    {
        $parser = new XrefHarness();
        $parser->setPdfDataPublic("xref\r\n0 1\r\n0000000000 65535 f\r\n");

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Unable to find trailer');
        $parser->decodeXrefPublic(0, ['xref' => []]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetXrefDataDetectsRepeatedOffsetLoop(): void
    {
        $parser = new XrefHarness();
        $parser->setPdfDataPublic("%PDF-1.7\ninvalid\n");

        try {
            $parser->getXrefDataPublic(5, ['xref' => []]);
        } catch (PPException $exception) {
            $this->assertContains($exception->getMessage(), [
                'Unable to find startxref (3)',
                'Unable to find xref (4)',
            ]);
        }

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('LOOP: this XRef offset has been already processed');
        $parser->getXrefDataPublic(5, ['xref' => []]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetXrefDataFindsObjectStreamStartxrefFromOffset(): void
    {
        $stream_data = \pack('C*', 0, 1, 9, 0);

        $parser = new XrefHarness();
        $parser->setPdfDataPublic('AAAAA12 0 obj .... xref');
        $parser->setStubRawObject(['objref', '12_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '1', 0], ['numeric', '1', 0]], 0],
                    ['/', 'Index', 0],
                    ['[', [['numeric', '0', 0], ['numeric', '1', 0]], 0],
                    ['/', 'Size', 0],
                    ['numeric', '1', 0],
                    ['/', 'DecodeParms', 0],
                    ['<<', [['/', 'Columns', 0], ['numeric', '3', 0], ['/', 'Predictor', 0], ['numeric', '10', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $xref = $parser->getXrefDataPublic(5, ['xref' => []]);

        $this->assertSame(9, $xref['xref']['0_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetXrefDataFindsStartxrefMarkerWhenOffsetIsNonZero(): void
    {
        $body = 'xxxxxxxxxxxxxxxxxxxx';
        $pdf =
            $body
            . "xref\r\n"
            . "0 1\r\n"
            . "0000000001 00000 n\r\n"
            . "trailer << /Size 1 >>\r\n"
            . "startxref\n20\n%%EOF";

        $parser = new XrefHarness();
        $parser->setPdfDataPublic($pdf);

        $start = (int) \strpos($pdf, 'startxref');
        $xref = $parser->getXrefDataPublic($start - 1, ['xref' => []]);

        $this->assertSame(1, $xref['xref']['0_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetXrefDataThrowsStartxref3ForNonZeroOffsetWithoutMarkers(): void
    {
        $parser = new XrefHarness();
        $parser->setPdfDataPublic('%PDF-1.7\nbody without xref hints');

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Unable to find startxref (3)');
        $parser->getXrefDataPublic(5, ['xref' => []]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefCallsGetXrefDataWhenTrailerContainsPrev(): void
    {
        $parser = new class() extends XrefHarness {
            public int $capturedOffset = -1;

            protected function getXrefData(int $offset = 0, array $xref = []): array
            {
                $this->capturedOffset = $offset;
                $xref['xref']['prev_0'] = $offset;

                return [
                    'trailer' => $xref['trailer'] ?? ['id' => [], 'info' => '', 'root' => '', 'size' => 0],
                    'xref' => $xref['xref'],
                ];
            }
        };

        $parser->setPdfDataPublic("xref\r\n0 1\r\n0000000001 00000 n\r\ntrailer << /Size 1 /Prev 11 >>\r\n");

        $xref = $parser->decodeXrefPublic(0, ['xref' => []]);

        $this->assertSame(11, $parser->capturedOffset);
        $this->assertSame(11, $xref['xref']['prev_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamThrowsWhenRawObjectValueIsNotString(): void
    {
        $parser = new XrefHarness();
        $parser->setStubRawObject(['numeric', [], 0]);

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Unable to find xref stream');
        $parser->decodeXrefStreamPublic(0, ['xref' => []]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamHandlesNonArrayDictionaryPayload(): void
    {
        $stream_data = \pack('C*', 0, 1, 9, 0);

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '1_0', 0]);
        $parser->setStubIndirectObject([
            ['<<', 'not-array', 0],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $xref = $parser->decodeXrefStreamPublic(0, ['xref' => ['9_0' => 99]]);
        $this->assertSame(99, $xref['xref']['9_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamCallsGetXrefDataWhenPrevIsPresent(): void
    {
        $stream_data = \pack('C*', 0, 1, 9, 0);

        $parser = new class() extends XrefHarness {
            public int $capturedOffset = -1;

            protected function getXrefData(int $offset = 0, array $xref = []): array
            {
                $this->capturedOffset = $offset;
                $xref['xref']['prev_0'] = $offset;

                return [
                    'trailer' => $xref['trailer'] ?? ['id' => [], 'info' => '', 'root' => '', 'size' => 0],
                    'xref' => $xref['xref'],
                ];
            }
        };

        $parser->setStubRawObject(['objref', '1_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '1', 0], ['numeric', '1', 0]], 0],
                    ['/', 'Index', 0],
                    ['[', [['numeric', '0', 0], ['numeric', '1', 0]], 0],
                    ['/', 'Size', 0],
                    ['numeric', '1', 0],
                    ['/', 'Prev', 0],
                    ['numeric', '77', 0],
                    ['/', 'DecodeParms', 0],
                    ['<<', [['/', 'Columns', 0], ['numeric', '3', 0], ['/', 'Predictor', 0], ['numeric', '10', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $xref = $parser->decodeXrefStreamPublic(0, ['xref' => []]);

        $this->assertSame(77, $parser->capturedOffset);
        $this->assertSame(77, $xref['xref']['prev_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testProcessObjIndexesMapRejectsSparseRowLookup(): void
    {
        $parser = new XrefStreamHarness();
        $xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => [],
        ];

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Invalid xref stream row at index 2');
        $parser->processObjIndexesMapPublic($xref, [2 => 10], [[1, 20, 0]]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testProcessSingleObjIndexRejectsMissingMandatoryFields(): void
    {
        $parser = new XrefStreamHarness();
        $xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => [],
        ];

        try {
            $parser->processSingleObjIndexPublic($xref, 8, [1]);
            $this->fail('Expected missing offset exception for in-use entry.');
        } catch (PPException $exception) {
            $this->assertSame(
                'Invalid xref stream entry for object 8: missing offset for in-use entry',
                $exception->getMessage(),
            );
        }

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains(
            'Invalid xref stream entry for object 9: missing object stream reference for compressed entry',
        );
        $parser->processSingleObjIndexPublic($xref, 9, [2, 55]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseXrefIndexSectionsCoversNullAndValidationErrors(): void
    {
        $parser = new XrefStreamHarness();

        $this->assertNull($parser->parseXrefIndexSectionsPublic(null));
        $this->assertNull($parser->parseXrefIndexSectionsPublic(['[', 'oops', 0]));

        try {
            $parser->parseXrefIndexSectionsPublic(['[', [['name', 'x', 0], ['numeric', '1', 0]], 0]);
            $this->fail('Expected invalid numeric token exception.');
        } catch (PPException $exception) {
            $this->assertSame('Invalid xref stream Index array: expected numeric values', $exception->getMessage());
        }

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Invalid xref stream Index array: values must be non-negative');
        $parser->parseXrefIndexSectionsPublic(['[', [['numeric', '-1', 0], ['numeric', '1', 0]], 0]);
    }

    public function testBuildXrefObjectNumbersExpandsSections(): void
    {
        $parser = new XrefStreamHarness();

        $this->assertSame([3, 4, 5, 10], $parser->buildXrefObjectNumbersPublic([[3, 3], [10, 1]]));
        $this->assertSame([], $parser->buildXrefObjectNumbersPublic([]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamThrowsWhenIndexAndSizeAreMissing(): void
    {
        $stream_data = \pack('C*', 1, 9, 0);

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '5_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '1', 0], ['numeric', '1', 0]], 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Unable to determine xref stream Index coverage: missing Index and Size');
        $parser->decodeXrefStreamPublic(100, ['xref' => []]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefStreamWithoutPredictorUsesRawRows(): void
    {
        $stream_data = \pack('C*', 1, 9, 0);

        $parser = new XrefHarness();
        $parser->setStubRawObject(['objref', '5_0', 0]);
        $parser->setStubIndirectObject([
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'XRef', 0],
                    ['/', 'W', 0],
                    ['[', [['numeric', '1', 0], ['numeric', '1', 0], ['numeric', '1', 0]], 0],
                    ['/', 'Index', 0],
                    ['[', [['numeric', '0', 0], ['numeric', '1', 0]], 0],
                    ['/', 'Size', 0],
                    ['numeric', '1', 0],
                ],
                0,
            ],
            ['stream', $stream_data, 0, [$stream_data, []]],
        ]);

        $xref = $parser->decodeXrefStreamPublic(100, ['xref' => []]);
        $this->assertSame(9, $xref['xref']['0_0'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeXrefBreaksWhenEntryPatternStartsAtDifferentOffset(): void
    {
        $pdf = "xref\r\nabc\r\n0 1 n\r\ntrailer << /Size 1 >>\r\n";

        $parser = new XrefHarness();
        $parser->setPdfDataPublic($pdf);
        $xref = $parser->decodeXrefPublic(0, ['xref' => ['9_0' => 99]]);

        $this->assertSame(99, $xref['xref']['9_0'] ?? null);
        $this->assertSame(1, $xref['trailer']['size']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseXrefIndexSectionsRejectsNonArrayTokens(): void
    {
        $parser = new XrefStreamHarness();

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Invalid xref stream Index array: expected numeric values');
        $parser->parseXrefIndexSectionsPublic(['[', [1 => ['numeric', '1', 0], 2 => ['numeric', '2', 0]], 0]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testProcessXrefTypeSkipsSlashTokenWithNonStringName(): void
    {
        $parser = new XrefStreamHarness();
        $xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => [],
        ];
        $wbt = [0, 0, 0];
        $state = [
            'index_sections' => null,
            'prevxref' => null,
            'predictor' => 0,
            'columns' => 0,
            'size' => null,
            'valid_crs' => false,
        ];

        $parser->processXrefTypePublic(
            [
                ['/', [['numeric', '1', 0]], 0],
                ['numeric', '7', 0],
            ],
            $xref,
            $wbt,
            $state,
            true,
        );

        $this->assertSame([], $xref['xref']);
        $this->assertFalse($state['valid_crs']);
    }

    public function testProcessXrefObjrefReturnsWhenObjrefValueIsNotString(): void
    {
        $parser = new XrefStreamHarness();
        $xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => [],
        ];

        $result = $parser->processXrefObjrefPublic(
            'Root',
            [
                ['/', 'Root', 0],
                ['objref', [['numeric', '1', 0]], 0],
            ],
            0,
            $xref,
        );

        $this->assertSame($xref, $result);
    }
}
