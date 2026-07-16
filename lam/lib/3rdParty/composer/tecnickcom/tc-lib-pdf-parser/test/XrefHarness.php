<?php

/**
 * XrefHarness.php
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

use Com\Tecnick\Pdf\Parser\Process\Xref;

/**
 * @phpstan-import-type RawObjectArray from \Com\Tecnick\Pdf\Parser\Process\RawObject
 */
class XrefHarness extends Xref
{
    /** @var array<int, RawObjectArray> */
    private array $stubIndirectObject = [];

    /** @var RawObjectArray|null */
    private ?array $stubRawObject = null;

    public function setPdfDataPublic(string $pdfdata): void
    {
        $this->pdfdata = $pdfdata;
    }

    /**
     * @param array<int, RawObjectArray> $indirectObject
     */
    public function setStubIndirectObject(array $indirectObject): void
    {
        $this->stubIndirectObject = $indirectObject;
    }

    /**
     * @param RawObjectArray $rawObject
     */
    public function setStubRawObject(array $rawObject): void
    {
        $this->stubRawObject = $rawObject;
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
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function getXrefDataPublic(int $offset = 0, array $xref = []): array
    {
        return $this->getXrefData($offset, $xref);
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
     *        'xref': array<string, int|string>,
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
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function decodeXrefPublic(int $startxref, array $xref): array
    {
        return $this->decodeXref($startxref, $xref);
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
     *        'xref': array<string, int|string>,
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
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function decodeXrefStreamPublic(int $startxref, array $xref): array
    {
        return $this->decodeXrefStream($startxref, $xref);
    }

    /**
     * @param array<int, array<int, int>> $sdata
     * @param array<int, array<int, int>> $ddata
     * @param array<int, int>             $wbt
     */
    public function processDdataPublic(array &$sdata, array $ddata, array $wbt): void
    {
        $this->processDdata($sdata, $ddata, $wbt);
    }

    /**
     * @return array<int, RawObjectArray>
     */
    protected function getIndirectObject(string $obj_ref, int $offset = 0, bool $decoding = true): array
    {
        $unused = [$obj_ref, $offset, $decoding];
        unset($unused);

        return $this->stubIndirectObject;
    }

    /**
     * @return RawObjectArray
     */
    protected function getRawObject(int $offset = 0): array
    {
        if ($this->stubRawObject !== null) {
            return $this->stubRawObject;
        }

        return parent::getRawObject($offset);
    }
}
