<?php

/**
 * ParserHarness.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Pdfparser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Parser\Parser;

/**
 * @phpstan-import-type RawObjectArray from \Com\Tecnick\Pdf\Parser\Process\RawObject
 */
class ParserHarness extends Parser
{
    /**
     * @var array{
     *        'trailer': array{
     *            'encrypt'?: string,
     *            'id': array<int, string>,
     *            'info': string,
     *            'root': string,
     *            'size': int,
     *        },
     *        'xref': array<string, int|string>,
     *    }
     */
    private array $stubXrefData = [
        'trailer' => [
            'id' => [],
            'info' => '',
            'root' => '',
            'size' => 0,
        ],
        'xref' => [],
    ];

    /** @var array<int, array{0:string,1:int,2:bool}> */
    private array $indirectCalls = [];

    /** @var array<int, RawObjectArray> */
    private array $stubIndirectReturn = [['null', 'null', 0]];

    /** @var array<int, RawObjectArray> */
    private array $rawObjectQueue = [];

    private bool $useParentIndirect = false;

    private bool $useParentRawObject = false;

    /**
     * @param array{
     *        'trailer': array{
     *            'encrypt'?: string,
     *            'id': array<int, string>,
     *            'info': string,
     *            'root': string,
     *            'size': int,
     *        },
     *        'xref': array<string, int|string>,
     *    } $xref
     */
    public function setStubXrefData(array $xref): void
    {
        $this->stubXrefData = $xref;
    }

    /** @return array<int, array{0:string,1:int,2:bool}> */
    public function getIndirectCalls(): array
    {
        return $this->indirectCalls;
    }

    /**
     * @param array<int, RawObjectArray> $obj
     */
    public function setStubIndirectReturn(array $obj): void
    {
        $this->stubIndirectReturn = $obj;
    }

    /**
     * @param array<int, RawObjectArray> $queue
     */
    public function setRawObjectQueue(array $queue): void
    {
        $this->rawObjectQueue = $queue;
    }

    public function setPdfDataPublic(string $data): void
    {
        $this->pdfdata = $data;
    }

    public function getPdfDataPublic(): string
    {
        return $this->pdfdata;
    }

    /**
     * @param array<string, array<int, RawObjectArray>> $objects
     */
    public function setObjectsPublic(array $objects): void
    {
        $this->objects = $objects;
    }

    /**
     * @param array<string, int|string> $xref
     */
    public function setXrefMapPublic(array $xref): void
    {
        $this->xref = [
            'trailer' => [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ],
            'xref' => $xref,
        ];
    }

    /**
     * @param RawObjectArray $obj
     *
     * @return RawObjectArray
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function getObjectValPublic(array $obj): array
    {
        return $this->getObjectVal($obj);
    }

    /**
     * @param array<string>             $filters
     * @param array<int, RawObjectArray> $sdic
     *
     * @return array<string>
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function getFiltersPublic(array $filters, array $sdic, int $key): array
    {
        return $this->getFilters($filters, $sdic, $key);
    }

    /**
     * @param array<int, RawObjectArray> $sdic
     *
     * @return array<string, mixed>
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function getDecodeParmsPublic(array $sdic, int $key): array
    {
        return $this->getDecodeParms($sdic, $key);
    }

    /**
     * @param array<string> $filters
     * @param array<string, mixed> $params
     *
     * @return array{0:string,1:array<string>}
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function getDecodedStreamPublic(array $filters, string $stream, array $params = []): array
    {
        return $this->getDecodedStream($filters, $stream, $params);
    }

    /**
     * @param array<int, RawObjectArray> $sdic
     *
     * @return array{0:string,1:array<string>}
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function decodeStreamPublic(array $sdic, string $stream): array
    {
        return $this->decodeStream($sdic, $stream);
    }

    /**
     * @return array<int, RawObjectArray>
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function getRawIndirectObjectPublic(int $offset, bool $decoding): array
    {
        return $this->getRawIndirectObject($offset, $decoding);
    }

    /**
     * @return array<int, RawObjectArray>
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function callParentGetIndirectObject(string $obj_ref, int $offset = 0, bool $decoding = true): array
    {
        $this->useParentIndirect = true;
        try {
            return $this->getIndirectObject($obj_ref, $offset, $decoding);
        } finally {
            $this->useParentIndirect = false;
        }
    }

    /**
     * Test-only: bypass the queue override below and run the real inherited
     * getRawObject against `$this->pdfdata`. Lets tests exercise the real
     * processAngular / processBracket loops with arbitrary byte input.
     *
     * @return RawObjectArray
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function callParentGetRawObject(int $offset = 0): array
    {
        $this->useParentRawObject = true;
        try {
            return $this->getRawObject($offset);
        } finally {
            $this->useParentRawObject = false;
        }
    }

    /**
     * @param array{
     *        'trailer'?: array{
     *            'encrypt'?: string,
     *            'id': array<int, string>,
     *            'info': string,
     *            'root': string,
     *            'size': int,
     *        },
     *        'xref'?: array<string, int|string>,
     *    } $xref
     *
     * @return array{
     *        'trailer': array{
     *            'encrypt'?: string,
     *            'id': array<int, string>,
     *            'info': string,
     *            'root': string,
     *            'size': int,
     *        },
     *        'xref': array<string, int|string>,
     *    }
     */
    protected function getXrefData(int $offset = 0, array $xref = []): array
    {
        $unused = [$offset, $xref];
        unset($unused);

        return $this->stubXrefData;
    }

    /**
     * @return array<int, RawObjectArray>
     */
    protected function getIndirectObject(string $obj_ref, int $offset = 0, bool $decoding = true): array
    {
        if ($this->useParentIndirect) {
            return parent::getIndirectObject($obj_ref, $offset, $decoding);
        }

        $this->indirectCalls[] = [$obj_ref, $offset, $decoding];

        return $this->stubIndirectReturn;
    }

    /**
     * @return RawObjectArray
     */
    protected function getRawObject(int $offset = 0): array
    {
        if ($this->useParentRawObject) {
            return parent::getRawObject($offset);
        }

        if (empty($this->rawObjectQueue)) {
            return ['endobj', 'endobj', $offset];
        }

        return \array_shift($this->rawObjectQueue);
    }
}
