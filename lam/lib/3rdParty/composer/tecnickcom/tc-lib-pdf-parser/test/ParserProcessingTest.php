<?php

/**
 * ParserProcessingTest.php
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

/**
 * @phpstan-import-type RawObjectArray from \Com\Tecnick\Pdf\Parser\Process\RawObject
 */
class ParserProcessingTest extends TestCase
{
    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseRejectsEmptyAndInvalidData(): void
    {
        $parser = new ParserHarness();

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Empty PDF data.');
        $parser->parse('');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseRejectsDataWithoutPdfHeader(): void
    {
        $parser = new ParserHarness();

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Invalid PDF data: missing %PDF header.');
        $parser->parse('not a pdf');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseLoadsPositiveOffsetsOnlyAndClearsPdfData(): void
    {
        $parser = new ParserHarness();
        $parser->setStubXrefData([
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '1_0',
                'size' => 3,
            ],
            'xref' => [
                '1_0' => 10,
                '2_0' => 0,
                '3_0' => -1,
            ],
        ]);
        $parser->setStubIndirectReturn([['numeric', '42', 0]]);

        $parsed = $parser->parse("junk%PDF-1.7\n");

        $calls = $parser->getIndirectCalls();
        $firstCall = $calls[0] ?? null;
        $this->assertCount(1, $calls);
        $this->assertSame(['1_0', 10, true], $firstCall);
        $this->assertSame('', $parser->getPdfDataPublic());
        $this->assertArrayHasKey('1_0', $parsed[1]);
        $this->assertArrayNotHasKey('2_0', $parsed[1]);
        $this->assertArrayNotHasKey('3_0', $parsed[1]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseUsesDecodeStreamsConfigForDirectObjects(): void
    {
        $parser = new ParserHarness();
        $parser->setStubXrefData([
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '1_0',
                'size' => 2,
            ],
            'xref' => [
                '1_0' => 10,
            ],
        ]);
        $parser->setStubIndirectReturn([['numeric', '42', 0]]);

        $parser->parse("%PDF-1.7\n");
        $calls = $parser->getIndirectCalls();
        $this->assertSame(['1_0', 10, true], $calls[0] ?? null);

        $lazy = new ParserHarness(['decode_streams' => false]);
        $lazy->setStubXrefData([
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '1_0',
                'size' => 2,
            ],
            'xref' => [
                '1_0' => 10,
            ],
        ]);
        $lazy->setStubIndirectReturn([['numeric', '42', 0]]);

        $lazy->parse("%PDF-1.7\n");
        $lazyCalls = $lazy->getIndirectCalls();
        $this->assertSame(['1_0', 10, false], $lazyCalls[0] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParentIndirectObjectRejectsInvalidReference(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic("%PDF-1.7\n1 0 obj\nendobj\n");

        $this->expectException(PPException::class);
        $this->expectExceptionMessageContains('Invalid object reference:');
        $parser->callParentGetIndirectObject('invalid', 0, true);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParentIndirectObjectReturnsEmptyResultWhenTargetIsMissing(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic("%PDF-1.7\n");

        $obj = $parser->callParentGetIndirectObject('1_0', 0, true);

        $this->assertSame([], $obj);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testRawIndirectObjectDecodesStreamWhenDictionaryIsPresent(): void
    {
        $parser = new ParserHarness();
        $parser->setRawObjectQueue([
            ['<<', [['/', 'Length', 0], ['numeric', '3', 0]], 5],
            ['stream', 'abcdef', 12],
            ['endobj', 'endobj', 18],
        ]);

        $objdata = $parser->getRawIndirectObjectPublic(0, true);

        $this->assertCount(2, $objdata);
        $entry = $objdata[1] ?? null;
        if (!\is_array($entry)) {
            $this->fail('Missing decoded stream entry at index 1.');
        }

        $this->assertSame('stream', $entry[0]);
        $this->assertSame('abcdef', $entry[1]);

        $decoded = $entry[3] ?? null;
        if (!\is_array($decoded)) {
            $this->fail('Decoded stream payload not available.');
        }

        $this->assertSame('abc', $decoded[0]);
        $this->assertSame([], $decoded[1]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetFiltersParsesSingleAndArraySyntax(): void
    {
        $parser = new ParserHarness();

        $single = [
            ['/', 'Filter',      0],
            ['/', 'FlateDecode', 0],
        ];
        $filters = $parser->getFiltersPublic([], $single, 0);
        $this->assertSame(['FlateDecode'], $filters);

        $list = [
            ['/', 'Filter', 0],
            [
                '[',
                [
                    ['/',       'FlateDecode',    0],
                    ['numeric', '1',              0],
                    ['/',       'ASCIIHexDecode', 0],
                ],
                0,
            ],
        ];
        $filters = $parser->getFiltersPublic([], $list, 0);
        $this->assertSame(['FlateDecode', 'ASCIIHexDecode'], $filters);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetObjectValResolvesCachedAndMappedObjectReferences(): void
    {
        $parser = new ParserHarness();
        $parser->setObjectsPublic([
            '3_0' => [['string', 'cached', 0]],
        ]);
        $cached = $parser->getObjectValPublic(['objref', '3_0', 0]);
        $this->assertSame(['string', 'cached', 0], $cached);

        $parser = new ParserHarness();
        $parser->setXrefMapPublic(['4_0' => 99]);
        $parser->setStubIndirectReturn([['string', 'loaded', 0]]);
        $loaded = $parser->getObjectValPublic(['objref', '4_0', 0]);
        $this->assertSame(['string', 'loaded', 0], $loaded);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetDecodedStreamTracksErrorsWhenConfiguredToIgnore(): void
    {
        $parser = new ParserHarness(['ignore_filter_errors' => true]);

        $result = $parser->getDecodedStreamPublic(['UnknownFilter'], 'sample-data');

        $this->assertSame('sample-data', $result[0]);
        $this->assertSame(['UnknownFilter'], $result[1]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetDecodedStreamThrowsWhenFilterErrorsAreNotIgnored(): void
    {
        $parser = new ParserHarness();

        $this->expectException(PPException::class);
        $parser->getDecodedStreamPublic(['UnknownFilter'], 'sample-data');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParentIndirectObjectFindsObjectAfterOffsetShift(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic("%PDF-1.7\nX1 0 obj\nendobj\n");
        $parser->setRawObjectQueue([
            ['numeric', '7',      16],
            ['endobj',  'endobj', 22],
        ]);

        $obj = $parser->callParentGetIndirectObject('1_0', 9, true);

        $this->assertSame([['numeric', '7', 16]], $obj);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetFiltersHandlesMissingAndInvalidArrayPayload(): void
    {
        $parser = new ParserHarness();

        $filters = ['FlateDecode'];
        $this->assertSame($filters, $parser->getFiltersPublic($filters, [['/', 'Filter', 0]], 0));

        $invalid = [
            ['/', 'Filter', 0],
            ['[', 'invalid', 0],
        ];
        $this->assertSame([], $parser->getFiltersPublic([], $invalid, 0));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetDecodeParmsHandlesDictionaryArrayAndMissingValues(): void
    {
        $parser = new ParserHarness();

        $this->assertSame([], $parser->getDecodeParmsPublic([['/', 'DecodeParms', 0]], 0));

        $dict = [
            ['/', 'DecodeParms', 0],
            [
                '<<',
                [
                    ['/', 'Columns', 0],
                    ['numeric', '5', 0],
                    ['/', 'EarlyChange', 0],
                    ['true', 'true', 0],
                    ['/', 'FilterName', 0],
                    ['/', 'FlateDecode', 0],
                    ['/', 'Text', 0],
                    ['string', 'abc', 0],
                    ['/', 'Ignored', 0],
                    ['[', [], 0],
                ],
                0,
            ],
        ];

        $this->assertSame(
            [
                'Columns' => 5,
                'EarlyChange' => true,
                'FilterName' => 'FlateDecode',
                'Text' => 'abc',
            ],
            $parser->getDecodeParmsPublic($dict, 0),
        );

        $array = [
            ['/', 'DecodeParms', 0],
            [
                '[',
                [
                    ['null', 'null', 0],
                    [
                        '<<',
                        [
                            ['/', 'Rows', 0],
                            ['numeric', '2', 0],
                            ['/', 'Enabled', 0],
                            ['false', 'false', 0],
                        ],
                        0,
                    ],
                ],
                0,
            ],
        ];

        $this->assertSame(
            [
                'Rows' => 2,
                'Enabled' => false,
            ],
            $parser->getDecodeParmsPublic($array, 0),
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testDecodeStreamHandlesEmptyStreamAndDecodeParmsExtraction(): void
    {
        $parser = new ParserHarness(['ignore_filter_errors' => true]);

        $this->assertSame(['', []], $parser->decodeStreamPublic([], ''));

        $sdic = [
            ['/', 'Filter', 0],
            ['/', 'UnknownFilter', 0],
            ['/', 'DecodeParms', 0],
            [
                '<<',
                [
                    ['/', 'Columns', 0],
                    ['numeric', '3', 0],
                    ['/', 'Predictor', 0],
                    ['numeric', '12', 0],
                ],
                0,
            ],
        ];

        $result = $parser->decodeStreamPublic($sdic, 'abc');
        $this->assertSame('abc', $result[0]);
        $this->assertSame(['UnknownFilter'], $result[1]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParentIndirectObjectReturnsNullObjectWhenSearchMissesTwice(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic("%PDF-1.7\nno objects\n");

        $obj = $parser->callParentGetIndirectObject('1_0', 2, true);

        $this->assertSame([['null', 'null', 3]], $obj);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetFiltersSkipsSlashEntriesWithNonStringNames(): void
    {
        $parser = new ParserHarness();
        $sdic = [
            ['/', 'Filter', 0],
            [
                '[',
                [
                    ['/', [['numeric', '1', 0]], 0],
                    ['/', 'FlateDecode', 0],
                ],
                0,
            ],
        ];

        $filters = $parser->getFiltersPublic([], $sdic, 0);
        $this->assertSame(['FlateDecode'], $filters);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetDecodeParmsSkipsInvalidPairsAcrossLoopChecks(): void
    {
        $parser = new ParserHarness();
        $sdic = [
            ['/', 'DecodeParms', 0],
            [
                '<<',
                [
                    ['numeric', '0',            0],
                    ['numeric', '1',            0],
                    ['/',       'MissingValue', 0],
                    ['null',    'null',         0],
                    ['/',       'Valid',        0],
                    ['numeric', '7',            0],
                    ['/',       'DanglingKey',  0],
                ],
                0,
            ],
        ];

        $params = $parser->getDecodeParmsPublic($sdic, 0);
        $this->assertSame(['Valid' => 7], $params);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseResolvesCompressedObjectFromObjectStream(): void
    {
        $parser = new ParserHarness();
        $parser->setStubXrefData([
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '1_0',
                'size' => 3,
            ],
            'xref' => [
                '1_0' => 10,
                '2_0' => '1_0_0',
            ],
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(1, 4, '2 0 (A)'));

        $parsed = $parser->parse("%PDF-1.7\n");

        $this->assertArrayHasKey('2_0', $parsed[1]);
        $obj = $parsed[1]['2_0'] ?? null;
        $this->assertIsArray($obj);
        $this->assertNotEmpty($obj);
        $this->assertCount(1, $parser->getIndirectCalls());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseWithLazyStreamsReparsesObjectStreamForDecodedPayload(): void
    {
        $parser = new class(['decode_streams' => false]) extends ParserHarness {
            /** @var array<int, array{0:string,1:int,2:bool}> */
            private array $calls = [];

            /**
             * @return array<int, RawObjectArray>
             */
            protected function getIndirectObject(string $obj_ref, int $offset = 0, bool $decoding = true): array
            {
                $this->calls[] = [$obj_ref, $offset, $decoding];

                if ($obj_ref !== '1_0') {
                    return [['null', 'null', 0]];
                }

                if ($decoding) {
                    return [
                        [
                            '<<',
                            [
                                ['/', 'Type', 0],
                                ['/', 'ObjStm', 0],
                                ['/', 'N', 0],
                                ['numeric', '1', 0],
                                ['/', 'First', 0],
                                ['numeric', '4', 0],
                            ],
                            0,
                        ],
                        ['stream', 'raw', 0, ['2 0 (A)', []]],
                    ];
                }

                return [
                    [
                        '<<',
                        [
                            ['/', 'Type', 0],
                            ['/', 'ObjStm', 0],
                            ['/', 'N', 0],
                            ['numeric', '1', 0],
                            ['/', 'First', 0],
                            ['numeric', '4', 0],
                        ],
                        0,
                    ],
                    ['stream', 'raw', 0],
                ];
            }

            /** @return array<int, array{0:string,1:int,2:bool}> */
            public function getCalls(): array
            {
                return $this->calls;
            }
        };

        $parser->setStubXrefData([
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '1_0',
                'size' => 3,
            ],
            'xref' => [
                '1_0' => 10,
                '2_0' => '1_0_0',
            ],
        ]);

        $parsed = $parser->parse("%PDF-1.7\n");

        $this->assertArrayHasKey('2_0', $parsed[1]);
        $calls = $parser->getCalls();
        $this->assertSame(['1_0', 10, false], $calls[0] ?? null);
        $this->assertSame(['1_0', 10, true], $calls[1] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseSkipsObjectAlreadyInjectedDuringIteration(): void
    {
        $parser = new class() extends ParserHarness {
            protected function getIndirectObject(string $obj_ref, int $offset = 0, bool $decoding = true): array
            {
                $obj = parent::getIndirectObject($obj_ref, $offset, $decoding);
                if ($obj_ref === '1_0') {
                    $this->objects['2_0'] = [['string', 'prefilled', 0]];
                }

                return $obj;
            }
        };

        $parser->setStubXrefData([
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '1_0',
                'size' => 3,
            ],
            'xref' => [
                '1_0' => 10,
                '2_0' => 20,
            ],
        ]);
        $parser->setStubIndirectReturn([['numeric', '9', 0]]);

        $parsed = $parser->parse("%PDF-1.7\n");

        $calls = $parser->getIndirectCalls();
        $this->assertCount(1, $calls);
        $first = $calls[0] ?? null;
        $this->assertIsArray($first);
        $this->assertSame('1_0', $first[0]);
        $this->assertSame('prefilled', $parsed[1]['2_0'][0][1] ?? '');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseSkipsCompressedObjectWhenStreamEnvelopeIsInvalid(): void
    {
        $parser = new ParserHarness();
        $parser->setStubXrefData([
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '1_0',
                'size' => 3,
            ],
            'xref' => [
                '1_0' => 10,
                '2_0' => '1_0_0',
            ],
        ]);
        $parser->setStubIndirectReturn([
            ['<<', [['/', 'N', 0], ['numeric', '1', 0], ['/', 'First', 0], ['numeric', '4', 0]], 0],
            ['stream', 'raw', 0],
        ]);

        $parsed = $parser->parse("%PDF-1.7\n");

        $this->assertArrayNotHasKey('2_0', $parsed[1]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetObjectValReturnsOriginalWhenCompressedLookupFails(): void
    {
        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 0,
        ]);

        $obj = ['objref', '2_0', 0];
        $result = $parser->getObjectValPublic($obj);

        $this->assertSame($obj, $result);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetObjectValReturnsOriginalForNonPositiveOffsetAndInvalidLocator(): void
    {
        $parser = new ParserHarness();
        $parser->setXrefMapPublic(['2_0' => 0]);
        $obj = ['objref', '2_0', 0];
        $this->assertSame($obj, $parser->getObjectValPublic($obj));

        $parser = new ParserHarness();
        $parser->setXrefMapPublic(['2_0' => 'bad_locator']);
        $this->assertSame($obj, $parser->getObjectValPublic($obj));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetObjectValReturnsOriginalWhenCachedObjectStreamNeedsMissingReparseOffset(): void
    {
        $parser = new ParserHarness();
        $parser->setObjectsPublic([
            '1_0' => [
                [
                    '<<',
                    [
                        ['/', 'Type', 0],
                        ['/', 'ObjStm', 0],
                        ['/', 'N', 0],
                        ['numeric', '1', 0],
                        ['/', 'First', 0],
                        ['numeric', '4', 0],
                    ],
                    0,
                ],
                ['stream', 'raw', 0],
            ],
        ]);
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
        ]);

        $obj = ['objref', '2_0', 0];
        $this->assertSame($obj, $parser->getObjectValPublic($obj));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetObjectValReturnsOriginalWhenIndirectObjectParsesEmpty(): void
    {
        $parser = new ParserHarness();
        $parser->setXrefMapPublic(['2_0' => 10]);
        $parser->setStubIndirectReturn([]);

        $obj = ['objref', '2_0', 0];
        $this->assertSame($obj, $parser->getObjectValPublic($obj));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetObjectValReturnsFirstElementForResolvedCompressedObject(): void
    {
        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 12,
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(1, 4, '2 0 (A)'));

        $resolved = $parser->getObjectValPublic(['objref', '2_0', 0]);

        $this->assertNotSame('objref', $resolved[0]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetObjectValHandlesObjectStreamIndexAndBodyValidation(): void
    {
        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 12,
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(1, 4, 'x y (A)'));

        $obj = ['objref', '2_0', 0];
        $this->assertSame($obj, $parser->getObjectValPublic($obj));

        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 12,
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(1, 4, '0 0 ()'));
        $this->assertSame($obj, $parser->getObjectValPublic($obj));

        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 12,
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(0, 4, '2 0 (A)'));
        $this->assertSame($obj, $parser->getObjectValPublic($obj));

        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 12,
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(1, 0, ''));
        $this->assertSame($obj, $parser->getObjectValPublic($obj));

        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 12,
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(1, 6, '2 -10 '));
        $this->assertSame($obj, $parser->getObjectValPublic($obj));

        $parser = new ParserHarness();
        $parser->setXrefMapPublic([
            '2_0' => '1_0_0',
            '1_0' => 12,
        ]);
        $parser->setStubIndirectReturn($this->buildObjectStreamObject(1, 4, '2 0    '));
        $this->assertSame($obj, $parser->getObjectValPublic($obj));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testGetDecodeParmsHandlesSparseDictionaryIndexes(): void
    {
        $parser = new ParserHarness();
        $sdic = [
            ['/', 'DecodeParms', 0],
            [
                '<<',
                [
                    2 => ['/', 'Columns', 0],
                    3 => ['numeric', '5', 0],
                ],
                0,
            ],
        ];

        $this->assertSame([], $parser->getDecodeParmsPublic($sdic, 0));
    }

    /**
     * @return array<int, RawObjectArray>
     */
    private function buildObjectStreamObject(int $n, int $first, string $decodedData): array
    {
        return [
            [
                '<<',
                [
                    ['/', 'Type', 0],
                    ['/', 'ObjStm', 0],
                    ['/', 'N', 0],
                    ['numeric', (string) $n, 0],
                    ['/', 'First', 0],
                    ['numeric', (string) $first, 0],
                ],
                0,
            ],
            ['stream', 'raw', 0, [$decodedData, []]],
        ];
    }

    /**
     * Regression: processAngular() must bail out when getRawObject() fails
     * to advance $offset, rather than spinning forever and exhausting PHP
     * memory. Without the guard, the inner do-while loop accumulates
     * identical zero-length tokens at the same offset until OOM.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testProcessAngularBailsOnNonAdvancingByte(): void
    {
        // `<<` then `~` — a byte that processDefault() cannot consume — and
        // no `>>` terminator. Before the fix this hangs / OOMs in
        // RawObject::processAngular() at the inner do-while loop.
        $parser = new ParserHarness();
        $parser->setPdfDataPublic('<<~');

        $element = $parser->callParentGetRawObject(0);

        $this->assertIsArray($element);
        $this->assertSame('<<', $element[0]);
        $this->assertIsArray($element[1]);
        // The non-advancing guard must short-circuit within a single
        // iteration; the trailing array_pop then leaves $objval empty.
        $this->assertLessThan(5, \count($element[1]));
    }

    /**
     * Regression: processBracket() must bail out on a non-advancing parse,
     * for the same reasons as the dictionary loop above.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testProcessBracketBailsOnNonAdvancingByte(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic('[~');

        $element = $parser->callParentGetRawObject(0);

        $this->assertIsArray($element);
        $this->assertSame('[', $element[0]);
        $this->assertIsArray($element[1]);
        $this->assertLessThan(5, \count($element[1]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParentRawObjectParsesBooleanKeywords(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic('true false');

        $first = $parser->callParentGetRawObject(0);
        $second = $parser->callParentGetRawObject(5);

        $this->assertSame('boolean', $first[0]);
        $this->assertSame('true', $first[1]);
        $this->assertSame('boolean', $second[0]);
        $this->assertSame('false', $second[1]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParentRawObjectParsesNestedParentheses(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic('((ab))');

        $element = $parser->callParentGetRawObject(0);

        $this->assertSame('(', $element[0]);
        $this->assertSame('(ab)', $element[1]);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParentRawObjectParsesHexStringAndMalformedHexFallback(): void
    {
        $parser = new ParserHarness();
        $parser->setPdfDataPublic('< 4A 4B >');

        $hex = $parser->callParentGetRawObject(0);
        $this->assertSame('<', $hex[0]);
        $this->assertSame(' 4A 4B ', $hex[1]);

        $parser = new ParserHarness();
        $parser->setPdfDataPublic('<GG>');
        $fallback = $parser->callParentGetRawObject(0);

        $this->assertSame('<', $fallback[0]);
        $this->assertSame('', $fallback[1]);
        $this->assertSame(4, $fallback[2]);
    }
}
