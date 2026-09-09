<?php

declare(strict_types=1);

/**
 * XrefStream.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfParser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

namespace Com\Tecnick\Pdf\Parser\Process;

use Com\Tecnick\Pdf\Parser\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Parser\Process\XrefStream
 *
 * Process the cross-reference stream entries
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfParser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * @phpstan-import-type RawObjectArray from \Com\Tecnick\Pdf\Parser\Process\RawObject
 *
 * @phpstan-type XrefData array{
 *                 'trailer': array{
 *                     'encrypt'?: string,
 *                     'id': array<int, string>,
 *                     'info': string,
 *                     'root': string,
 *                     'size': int,
 *                 },
 *                 'xref': array<string, int|string>,
 *             }
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class XrefStream extends \Com\Tecnick\Pdf\Parser\Process\RawObject
{
    /**
     * Object numbers already decided by a newer section of the /Prev chain,
     * used as a set keyed by object number.
     *
     * @var array<int, bool>
     */
    protected array $xrefdone = [];

    /**
     * Store a cross-reference entry, unless a newer section already decided the object.
     *
     * A section overrides the sections it chains to via /Prev (PDF 32000-1 7.5.6) and the
     * chain is walked newest first, so the first section that mentions an object number
     * wins whatever the entry type.
     *
     * @param array<string, int|string> $xrefmap Map of the entries decoded so far.
     * @param int                       $objNum  Object number.
     * @param string                    $index   Entry key "[object number]_[generation number]".
     * @param int|string                $value   Entry value.
     */
    protected function storeXrefEntry(array &$xrefmap, int $objNum, string $index, int|string $value): void
    {
        if (!isset($this->xrefdone[$objNum]) && !\array_key_exists($index, $xrefmap)) {
            $xrefmap[$index] = $value;
        }

        $this->xrefdone[$objNum] = true;
    }

    /**
     * Mark an object number as decided by the current section without storing an entry.
     *
     * @param int $objNum Object number.
     */
    protected function freeXrefEntry(int $objNum): void
    {
        $this->xrefdone[$objNum] = true;
    }

    /**
     * Process the xref stream rows, numbering them sequentially from $obj_num.
     *
     * @param XrefData                    $xref    XREF data.
     * @param int                         $obj_num Object number of the first row.
     * @param array<int, array<int, int>> $sdata   Decoded entry values.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function processObjIndexes(array &$xref, int &$obj_num, array $sdata): void
    {
        foreach ($sdata as $sdatum) {
            $this->processSingleObjIndex($xref, $obj_num, $sdatum);
            ++$obj_num;
        }
    }

    /**
     * Process object indexes using explicit object numbers.
     *
     * @param XrefData                    $xref       XREF data.
     * @param array<int, int>             $objNumbers Object number of each row.
     * @param array<int, array<int, int>> $sdata      Decoded entry values.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function processObjIndexesMap(array &$xref, array $objNumbers, array $sdata): void
    {
        $objCount = \count($objNumbers);
        $rowCount = \count($sdata);
        if ($rowCount !== $objCount) {
            throw new PPException(
                'Invalid xref stream row count: expected ' . $objCount . ' rows from Index, got ' . $rowCount,
            );
        }

        foreach ($objNumbers as $idx => $objNum) {
            $row = $sdata[$idx] ?? null;
            if (!\is_array($row)) {
                throw new PPException('Invalid xref stream row at index ' . $idx);
            }

            $this->processSingleObjIndex($xref, $objNum, $row);
        }
    }

    /**
     * Process a single xref stream row.
     *
     * @param XrefData        $xref   XREF data.
     * @param int             $objNum Object number of the row.
     * @param array<int, int> $sdatum Decoded entry values of the row.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function processSingleObjIndex(array &$xref, int $objNum, array $sdatum): void
    {
        $entryType = (int) ($sdatum[0] ?? 0);
        switch ($entryType) {
            case 0:
                // (f) linked list of free objects
                $this->freeXrefEntry($objNum);
                break;
            case 1:
                if (!\array_key_exists(1, $sdatum)) {
                    throw new PPException(
                        'Invalid xref stream entry for object ' . $objNum . ': missing offset for in-use entry',
                    );
                }

                // (n) objects that are in use but are not compressed
                // create unique object index: [object number]_[generation number]
                $index = $objNum . '_' . (int) ($sdatum[2] ?? 0);
                // store object offset position
                $this->storeXrefEntry($xref['xref'], $objNum, $index, (int) $sdatum[1]);
                break;
            case 2:
                if (!\array_key_exists(1, $sdatum) || !\array_key_exists(2, $sdatum)) {
                    throw new PPException(
                        'Invalid xref stream entry for object '
                        . $objNum
                        . ': missing object stream reference for compressed entry',
                    );
                }

                // compressed objects
                // $row[1] = object number of the object stream in which this object is stored
                // $row[2] = index of this object within the object stream
                $this->storeXrefEntry(
                    $xref['xref'],
                    $objNum,
                    $objNum . '_0',
                    (int) $sdatum[1] . '_0_' . (int) $sdatum[2],
                );
                break;
            default:
                // any other type is a reference to the null object (PDF 32000-1 7.5.8.2)
                $this->freeXrefEntry($objNum);
                break;
        }
    }

    /**
     * Read the entries of a cross-reference stream dictionary into the parsing state.
     *
     * @param array<int, RawObjectArray> $sarr        Cross-reference stream dictionary.
     * @param XrefData                   $xref        XREF data.
     * @param array<int, int>            $wbt         Field widths in bytes.
     * @param array{
     *      index_sections: array<int, array{0:int, 1:int}>|null,
     *      prevxref: int|null,
     *      predictor: int,
     *      columns: int,
     *      colors: int,
     *      bits: int,
     *      size: int|null,
     *      valid_crs: bool
     * } $state Parsing state.
     * @param bool                       $filltrailer If true, fill the trailer data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function processXrefType(array $sarr, array &$xref, array &$wbt, array &$state, bool $filltrailer): void
    {
        // a dictionary is a flat list of key/value pairs: only even positions are keys,
        // otherwise a value that happens to be a name is taken for a key
        $count = \count($sarr);
        for ($key = 0; $key < $count; $key += 2) {
            $val = $sarr[$key] ?? null;
            if (!\is_array($val) || $val[0] !== '/') {
                continue;
            }

            if (!\is_string($val[1])) {
                continue;
            }

            $next = $sarr[$key + 1] ?? null;

            switch ($val[1]) {
                case 'Type':
                    $state['valid_crs'] = \is_array($next) && $next[0] === '/' && $next[1] === 'XRef';
                    break;
                case 'Index':
                    $state['index_sections'] = $this->parseXrefIndexSections($next);
                    break;
                case 'Prev':
                    $this->processXrefPrev($next, $state['prevxref']);
                    break;
                case 'Size':
                    if (\is_array($next) && $next[0] === 'numeric') {
                        $state['size'] = (int) $next[1];
                    }

                    break;
                case 'W':
                    // number of bytes (in the decoded stream) of the corresponding field
                    if (\is_array($next)) {
                        $wbt[0] = (int) ($next[1][0][1] ?? 0);
                        $wbt[1] = (int) ($next[1][1][1] ?? 0);
                        $wbt[2] = (int) ($next[1][2][1] ?? 0);
                    }
                    break;
                case 'DecodeParms':
                    $this->processXrefDecodeParms($next, $state);
                    break;
            }

            $this->processXrefTypeFt($val[1], $sarr, $key, $xref, $filltrailer);
        }
    }

    /**
     * Parse the xref stream Index array into normalized [startObj, count] pairs.
     *
     * @param RawObjectArray|null $indexObj Index object token.
     *
     * @return array<int, array{0:int, 1:int}>|null Sections, or null when Index is missing.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function parseXrefIndexSections(?array $indexObj): ?array
    {
        if (!\is_array($indexObj)) {
            return null;
        }

        $values = $indexObj[1];
        if (!\is_array($values)) {
            return null;
        }

        if ((\count($values) % 2) !== 0) {
            throw new PPException('Invalid xref stream Index array: expected even number of values');
        }

        $sections = [];
        for ($idx = 0, $max = \count($values); $idx < $max; $idx += 2) {
            $startToken = $values[$idx] ?? null;
            $countToken = $values[$idx + 1] ?? null;
            if (!\is_array($startToken) || !\is_array($countToken)) {
                throw new PPException('Invalid xref stream Index array: expected numeric values');
            }

            if ($startToken[0] !== 'numeric' || $countToken[0] !== 'numeric') {
                throw new PPException('Invalid xref stream Index array: expected numeric values');
            }

            $startObj = (int) $startToken[1];
            $count = (int) $countToken[1];
            if ($startObj < 0 || $count < 0) {
                throw new PPException('Invalid xref stream Index array: values must be non-negative');
            }

            $sections[] = [$startObj, $count];
        }

        return $sections;
    }

    /**
     * Build the full object-number map for decoded xref stream rows.
     *
     * @param array<int, array{0:int, 1:int}> $indexSections Normalized Index sections.
     *
     * @return array<int, int> Object number of each row.
     */
    protected function buildXrefObjectNumbers(array $indexSections): array
    {
        $objNumbers = [];
        foreach ($indexSections as $section) {
            $startObj = $section[0];
            $count = $section[1];
            $limit = $startObj + $count;
            for ($objNum = $startObj; $objNum < $limit; ++$objNum) {
                $objNumbers[] = $objNum;
            }
        }

        return $objNumbers;
    }

    /**
     * Read the /Prev offset of a cross-reference stream dictionary.
     *
     * @param RawObjectArray|null $next     Value token of the /Prev entry.
     * @param int|null            $prevxref Offset of the previous xref section.
     */
    protected function processXrefPrev(?array $next, ?int &$prevxref): void
    {
        if (\is_array($next) && $next[0] === 'numeric') {
            // get previous xref offset
            $prevxref = (int) $next[1];
        }
    }

    /**
     * Read the predictor geometry declared in /DecodeParms: /Predictor, /Columns,
     * /Colors and /BitsPerComponent.
     *
     * @param RawObjectArray|null $next  Value token of the /DecodeParms entry.
     * @param array{
     *      index_sections: array<int, array{0:int, 1:int}>|null,
     *      prevxref: int|null,
     *      predictor: int,
     *      columns: int,
     *      colors: int,
     *      bits: int,
     *      size: int|null,
     *      valid_crs: bool
     * } $state Parsing state.
     */
    protected function processXrefDecodeParms(?array $next, array &$state): void
    {
        $decpar = $next[1] ?? null;
        if (!\is_array($decpar)) {
            return;
        }

        // only even positions of the flat key/value list are dictionary keys
        $count = \count($decpar);
        for ($kdc = 0; $kdc < $count; $kdc += 2) {
            $vdc = $decpar[$kdc] ?? null;
            $nextDecpar = $decpar[$kdc + 1] ?? null;
            if (!\is_array($vdc) || !\is_array($nextDecpar) || $vdc[0] !== '/' || $nextDecpar[0] !== 'numeric') {
                continue;
            }

            $key = match ($vdc[1]) {
                'Columns' => 'columns',
                'Predictor' => 'predictor',
                'Colors' => 'colors',
                'BitsPerComponent' => 'bits',
                default => null,
            };
            if ($key !== null) {
                $state[$key] = (int) $nextDecpar[1];
            }
        }

        $state['columns'] = \max(0, $state['columns']);
        $state['predictor'] = \max(0, $state['predictor']);
    }

    /**
     * Fill the trailer data from a cross-reference stream dictionary entry.
     *
     * @param string                     $type        Dictionary key name.
     * @param array<int, RawObjectArray> $sarr        Cross-reference stream dictionary.
     * @param int                        $key         Index of the dictionary key.
     * @param XrefData                   $xref        XREF data.
     * @param bool                       $filltrailer If true, fill the trailer data.
     */
    protected function processXrefTypeFt(string $type, array $sarr, int $key, array &$xref, bool $filltrailer): void
    {
        if (!$filltrailer) {
            return;
        }

        $next = $sarr[$key + 1] ?? null;

        switch ($type) {
            case 'Size':
                if (\is_array($next) && $next[0] === 'numeric') {
                    $xref['trailer']['size'] = (int) $next[1];
                }

                break;
            case 'ID':
                $id0 = $next[1][0][1] ?? null;
                $id1 = $next[1][1][1] ?? null;
                if (!\is_array($next) || !\is_string($id0) || !\is_string($id1) || $id0 === '' || $id1 === '') {
                    break;
                }
                $xref['trailer']['id'] = [
                    $id0,
                    $id1,
                ];
                break;
            default:
                $xref = $this->processXrefObjref($type, $sarr, $key, $xref);
                break;
        }
    }

    /**
     * Fill the trailer references /Root, /Info and /Encrypt.
     *
     * @param string                     $type Dictionary key name.
     * @param array<int, RawObjectArray> $sarr Cross-reference stream dictionary.
     * @param int                        $key  Index of the dictionary key.
     * @param XrefData                   $xref XREF data.
     *
     * @return XrefData XREF data.
     */
    protected function processXrefObjref(string $type, array $sarr, int $key, array $xref): array
    {
        $next = $sarr[$key + 1] ?? null;
        if (!\is_array($next) || $next[0] !== 'objref') {
            return $xref;
        }

        $val = $next[1];
        if (!\is_string($val)) {
            return $xref;
        }

        switch ($type) {
            case 'Root':
                $xref['trailer']['root'] = $val;
                break;
            case 'Info':
                $xref['trailer']['info'] = $val;
                break;
            case 'Encrypt':
                $xref['trailer']['encrypt'] = $val;
                break;
        }

        return $xref;
    }
}
