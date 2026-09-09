<?php

declare(strict_types=1);

/**
 * Xref.php
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
 * Com\Tecnick\Pdf\Parser\Process\Xref
 *
 * Process the cross-reference sections and the trailer
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
 * @phpstan-import-type XrefData from \Com\Tecnick\Pdf\Parser\Process\XrefStream
 *
 * @phpstan-type XrefDataPartial array{
 *                 'trailer'?: array{
 *                     'encrypt'?: string,
 *                     'id': array<int, string>,
 *                     'info': string,
 *                     'root': string,
 *                     'size': int,
 *                 },
 *                 'xref': array<string, int|string>,
 *             }
 *
 * @phpstan-type XrefDataInput array{
 *                 'trailer'?: array{
 *                     'encrypt'?: string,
 *                     'id': array<int, string>,
 *                     'info': string,
 *                     'root': string,
 *                     'size': int,
 *                 },
 *                 'xref'?: array<string, int|string>,
 *             }
 */
abstract class Xref extends \Com\Tecnick\Pdf\Parser\Process\XrefStream
{
    /**
     * Default empty XREF data.
     *
     * @var XrefData
     */
    protected const XREF_EMPTY = [
        'trailer' => [
            'id' => [],
            'info' => '',
            'root' => '',
            'size' => 0,
        ],
        'xref' => [],
    ];

    /**
     * Maximum number of chained xref sections followed via /Prev.
     */
    protected const MAX_XREF_CHAIN = 1024;

    /**
     * Number of xref stream rows decoded per batch.
     */
    protected const XREF_ROW_BATCH = 4096;

    /**
     * Largest /Colors value the filter layer accepts.
     */
    protected const MAX_PREDICTOR_COLORS = 0xFFFF;

    /**
     * Largest /Columns value the filter layer accepts.
     */
    protected const MAX_PREDICTOR_COLUMNS = 0x7FFF_FFFF;

    /**
     * /BitsPerComponent values the filter layer accepts.
     *
     * @var array<int, int>
     */
    protected const PREDICTOR_BITS = [1, 2, 4, 8, 16];

    /**
     * XREF data.
     *
     * @var XrefData
     */
    protected array $xref = self::XREF_EMPTY;

    /**
     * Store the processed offsets, used as a set keyed by offset.
     *
     * @var array<int, bool>
     */
    protected array $mrkoff = [];

    /**
     * Get content of indirect object.
     *
     * @param string $obj_ref  Object number and generation number separated by underscore character.
     * @param int    $offset   Object offset.
     * @param bool   $decoding If true decode streams.
     *
     * @return array<int, RawObjectArray> Object data.
     */
    abstract protected function getIndirectObject(string $obj_ref, int $offset = 0, bool $decoding = true): array;

    /**
     * Get Cross-Reference (xref) table and trailer data from PDF document data.
     *
     * @param int           $offset Xref offset (if known).
     * @param XrefDataInput $xref   Previous xref array (if any).
     *
     * @return XrefData Xref and trailer data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getXrefData(int $offset = 0, array $xref = []): array
    {
        if (isset($this->mrkoff[$offset])) {
            throw new PPException('LOOP: this XRef offset has been already processed');
        }

        if (\count($this->mrkoff) >= static::MAX_XREF_CHAIN) {
            throw new PPException('Maximum number of chained XRef sections exceeded: ' . static::MAX_XREF_CHAIN);
        }

        if ($offset < 0 || $offset > \strlen($this->pdfdata)) {
            throw new PPException('Invalid XRef offset: ' . $offset);
        }

        $this->mrkoff[$offset] = true;
        $matches = [];
        if ($offset === 0) {
            // find last startxref
            $matchCount = \preg_match_all(
                '/[\r\n]startxref[\s]*[\r\n]+([0-9]+)[\s]*[\r\n]+%%EOF/i',
                $this->pdfdata,
                $matches,
                PREG_SET_ORDER,
                $offset,
            );
            if ($matchCount === false || $matchCount === 0) {
                throw new PPException('Unable to find startxref (1)');
            }

            $matches = \array_pop($matches);
            $startxref = (int) ($matches[1] ?? 0);
        } elseif (($pos = \strpos($this->pdfdata, 'xref', $offset)) !== false && $pos <= ($offset + 4)) {
            // Already pointing at the xref table
            $startxref = $pos;
        } elseif (\preg_match('/([0-9]+[\s][0-9]+[\s]obj)/i', $this->pdfdata, $matches, PREG_OFFSET_CAPTURE, $offset)) {
            // Cross-Reference Stream object
            $startxref = (int) $offset;
        } elseif (\preg_match(
            '/[\r\n]startxref[\s]*[\r\n]+([0-9]+)[\s]*[\r\n]+%%EOF/i',
            $this->pdfdata,
            $matches,
            PREG_OFFSET_CAPTURE,
            $offset,
        )) {
            // startxref found
            $startxref = (int) ($matches[1][0] ?? 0);
        } else {
            throw new PPException('Unable to find startxref (3)');
        }

        // the startxref value is read from the document and may point outside of it
        if ($startxref < 0 || $startxref > \strlen($this->pdfdata)) {
            throw new PPException('Invalid startxref offset: ' . $startxref);
        }

        $xref['xref'] ??= [];

        // check xref position (allow leading whitespace before the xref keyword)
        $xrefPos = \strpos($this->pdfdata, 'xref', $startxref);
        $hasPlainXref = false;
        if ($xrefPos !== false && $xrefPos >= $startxref) {
            $prefixLen = $xrefPos - $startxref;
            $prefix = $prefixLen > 0 ? \substr($this->pdfdata, $startxref, $prefixLen) : '';
            if (\trim($prefix, "\x00\x09\x0a\x0c\x0d\x20") === '') {
                $hasPlainXref = true;
            }
        }

        if ($hasPlainXref) {
            // Cross-Reference
            $xrefStart = \is_int($xrefPos) ? $xrefPos : $startxref;
            $xref = $this->decodeXref($xrefStart, $xref);
        } else {
            // Cross-Reference Stream
            $xref = $this->decodeXrefStream($startxref, $xref);
        }

        if ($xref['xref'] === []) {
            throw new PPException('Unable to find xref (4)');
        }

        return $xref;
    }

    /**
     * Decode the Cross-Reference section.
     *
     * @param int             $startxref Offset of the 'xref' keyword.
     * @param XrefDataPartial $xref      Previous xref array (if any).
     *
     * @return XrefData Xref and trailer data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function decodeXref(int $startxref, array $xref): array
    {
        $startxref = \min($startxref + 4, \strlen($this->pdfdata)); // 4 is the length of the word 'xref'
        // skip initial white space chars:
        // \x00 null (NUL)
        // \x09 horizontal tab (HT)
        // \x0A line feed (LF)
        // \x0C form feed (FF)
        // \x0D carriage return (CR)
        // \x20 space (SP)
        $offset = $startxref + \strspn($this->pdfdata, "\x00\x09\x0a\x0c\x0d\x20", $startxref);
        // initialize object number
        $obj_num = 0;
        $matches = [];
        // search for cross-reference entries or subsection
        while (
            \preg_match(
                '/(\d+)[\x20](\d+)[\x20]?([nf]?)(\r\n|[\x20]?[\r\n])/',
                $this->pdfdata,
                $matches,
                PREG_OFFSET_CAPTURE,
                $offset,
            ) === 1
        ) {
            $matchOffset = (int) ($matches[0][1] ?? -1);
            if ($matchOffset !== $offset) {
                // we are on another section
                break;
            }

            $offset += \strlen($matches[0][0] ?? '');
            $flag = $matches[3][0] ?? '';
            if ($flag === 'n') {
                // create unique object index: [object number]_[generation number]
                $index = $obj_num . '_' . (int) ($matches[2][0] ?? 0);
                // store object offset position
                $this->storeXrefEntry($xref['xref'], $obj_num, $index, (int) ($matches[1][0] ?? 0));
                ++$obj_num;
                continue;
            }

            if ($flag === 'f') {
                // a free object hides the entry an older section holds for the same number
                $this->freeXrefEntry($obj_num);
                ++$obj_num;
                continue;
            }

            // object number (index) - handle any other character (free object)
            $obj_num = (int) ($matches[1][0] ?? 0);
        }

        // get trailer data
        $trailerData = $this->extractTrailerDict($offset);
        if ($trailerData === null) {
            throw new PPException('Unable to find trailer');
        }

        return $this->getTrailerData($xref, $trailerData);
    }

    /**
     * Extract the content of the trailer dictionary, honouring nested "<<...>>" pairs.
     *
     * @param int $offset Offset from which to look for the 'trailer' keyword.
     *
     * @return string|null The dictionary content (without the outer delimiters), or null if not found.
     */
    protected function extractTrailerDict(int $offset): ?string
    {
        $search = \max(0, \min($offset, \strlen($this->pdfdata)));
        while (($trpos = \strpos($this->pdfdata, 'trailer', $search)) !== false) {
            $search = $trpos + 7; // 7 is the length of the word 'trailer'
            // only whitespace is allowed between the 'trailer' keyword and the '<<' delimiter
            $open = $search + \strspn($this->pdfdata, "\x00\x09\x0a\x0c\x0d\x20", $search);
            if (\substr($this->pdfdata, $open, 2) !== '<<') {
                continue;
            }

            $content = $this->extractBalancedDict($open);
            if ($content !== null) {
                return $content;
            }
        }

        return null;
    }

    /**
     * Return the content between the outermost balanced "<<" and ">>" starting at $open.
     *
     * Literal strings, hexadecimal strings and comments are skipped, so a ">>" inside
     * them does not close the dictionary.
     *
     * @param int $open Offset of the opening '<<'.
     *
     * @return string|null Inner content, or null if the dictionary is not balanced.
     */
    protected function extractBalancedDict(int $open): ?string
    {
        $len = \strlen($this->pdfdata);
        $depth = 0;
        $contentStart = $open + 2;
        for ($pos = $open; $pos < $len; ++$pos) {
            $char = $this->pdfdata[$pos];
            if ($char === '(') {
                // literal string
                $pos = $this->skipLiteralString($pos);
                continue;
            }

            if ($char === '%') {
                // comment up to the end of the line
                $pos += \strcspn($this->pdfdata, "\r\n", $pos);
                continue;
            }

            if ($char !== '<' && $char !== '>') {
                continue;
            }

            $next = $this->pdfdata[$pos + 1] ?? '';
            if ($char === '<' && $next !== '<') {
                // hexadecimal string
                $end = \strpos($this->pdfdata, '>', $pos + 1);
                if ($end === false) {
                    return null;
                }

                $pos = $end;
                continue;
            }

            if ($next !== $char) {
                continue;
            }

            if ($char === '<') {
                ++$depth;
                ++$pos;
                continue;
            }

            --$depth;
            if ($depth === 0) {
                return \substr($this->pdfdata, $contentStart, $pos - $contentStart);
            }

            ++$pos;
        }

        return null;
    }

    /**
     * Return the offset of the closing ')' of the literal string starting at $open.
     *
     * @param int $open Offset of the opening '('.
     *
     * @return int Offset of the closing delimiter, or the end of the data if unterminated.
     */
    protected function skipLiteralString(int $open): int
    {
        $len = \strlen($this->pdfdata);
        $depth = 1;
        for ($pos = $open + 1; $pos < $len; ++$pos) {
            $char = $this->pdfdata[$pos];
            if ($char === '\\') {
                // the escaped character cannot close the string
                ++$pos;
                continue;
            }

            if ($char === '(') {
                ++$depth;
                continue;
            }

            if ($char === ')') {
                --$depth;
                if ($depth === 0) {
                    return $pos;
                }
            }
        }

        return $len;
    }

    /**
     * Check that the row count the Index declares matches what the stream can hold.
     *
     * @param string                          $streamData    Decoded stream payload.
     * @param int                             $rowlen        Number of bytes per row.
     * @param array<int, array{0:int, 1:int}> $indexSections Normalized Index sections.
     * @param int                             $entryWidth    Number of bytes an entry needs.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function assertXrefStreamCoverage(
        string $streamData,
        int $rowlen,
        array $indexSections,
        int $entryWidth = 0,
    ): void {
        // a row that cannot hold a whole entry decodes to silently zero-filled fields
        if ($entryWidth > \max(1, $rowlen)) {
            throw new PPException(
                'Invalid xref stream row size: ' . $rowlen . ' cannot hold the ' . $entryWidth . ' bytes of an entry',
            );
        }

        $expected = 0;
        foreach ($indexSections as $section) {
            $expected += $section[1];
        }

        $rowlen = \max(1, $rowlen);
        $datalen = \strlen($streamData);
        // a partial trailing row would decode to a silently zero-padded entry
        if (($datalen % $rowlen) !== 0) {
            throw new PPException(
                'Invalid xref stream length: ' . $datalen . ' is not a multiple of the row size ' . $rowlen,
            );
        }

        $rowCount = \intdiv($datalen, $rowlen);
        if ($rowCount !== $expected) {
            throw new PPException(
                'Invalid xref stream row count: expected ' . $expected . ' rows from Index, got ' . $rowCount,
            );
        }
    }

    /**
     * Number of bytes of a row of a predicted stream, as the filter layer computes it.
     *
     * /Columns counts samples, so the byte width also depends on /Colors and
     * /BitsPerComponent. /Colors and /Columns are clamped up to 1 and values above the
     * bounds the filter accepts are rejected, matching the filter layer.
     *
     * @param int $colors  Colour components per sample.
     * @param int $bits    Bits per component.
     * @param int $columns Samples per row.
     *
     * @return int Bytes per row, or 0 when the filter rejects the declared geometry.
     */
    protected function predictedRowLength(int $colors, int $bits, int $columns): int
    {
        $colors = \max(1, $colors);
        if ($colors > static::MAX_PREDICTOR_COLORS) {
            return 0;
        }

        $columns = \max(1, $columns);
        if ($columns > static::MAX_PREDICTOR_COLUMNS) {
            return 0;
        }

        if (!\in_array($bits, static::PREDICTOR_BITS, true)) {
            return 0;
        }

        return \max(1, \intdiv(($colors * $bits * $columns) + 7, 8));
    }

    /**
     * Decode the un-predicted xref stream rows into xref entry values.
     *
     * @param string          $streamData Decoded stream payload.
     * @param int             $rowlen     Number of bytes per row.
     * @param array<int, int> $wbt        Field widths in bytes.
     *
     * @return array<int, array<int, int>> Decoded entry values, one row each.
     */
    protected function decodeXrefStreamRows(string $streamData, int $rowlen, array $wbt): array
    {
        $rowlen = \max(1, $rowlen);
        $datalen = \strlen($streamData);
        $batchlen = $rowlen * static::XREF_ROW_BATCH;

        // decode in batches to bound the memory used by unpack()
        $sdata = [];
        for ($start = 0; $start < $datalen; $start += $batchlen) {
            /** @var array<int, int> $bytes */
            $bytes = (array) \unpack('C*', \substr($streamData, $start, $batchlen));
            // split the rows
            $ddata = \array_chunk($bytes, $rowlen, false);
            // complete decoding
            $batch = [];
            $this->processDdata($batch, $ddata, $wbt);
            foreach ($batch as $row) {
                $sdata[] = $row;
            }
        }

        return $sdata;
    }

    /**
     * Decode the PDF trailer dictionary data.
     *
     * @param XrefDataPartial $xref         Previous xref array (if any).
     * @param string          $trailer_data Trailer content string.
     *
     * @return XrefData Xref and trailer data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    protected function getTrailerData(array $xref, string $trailer_data): array
    {
        $matches = [];
        if (($xref['trailer'] ?? []) === []) {
            // get only the last updated version
            $xref['trailer'] = [
                'id' => [],
                'info' => '',
                'root' => '',
                'size' => 0,
            ];

            // parse trailer_data
            if (\preg_match('/Size[\s]+([0-9]+)/i', $trailer_data, $matches) === 1) {
                $xref['trailer']['size'] = (int) ($matches[1] ?? 0);
            }

            if (\preg_match('/Root[\s]+([0-9]+)[\s]+([0-9]+)[\s]+R/i', $trailer_data, $matches) === 1) {
                $xref['trailer']['root'] = (int) ($matches[1] ?? 0) . '_' . (int) ($matches[2] ?? 0);
            }

            if (\preg_match('/Encrypt[\s]+([0-9]+)[\s]+([0-9]+)[\s]+R/i', $trailer_data, $matches) === 1) {
                $xref['trailer']['encrypt'] = (int) ($matches[1] ?? 0) . '_' . (int) ($matches[2] ?? 0);
            }

            if (\preg_match('/Info[\s]+([0-9]+)[\s]+([0-9]+)[\s]+R/i', $trailer_data, $matches) === 1) {
                $xref['trailer']['info'] = (int) ($matches[1] ?? 0) . '_' . (int) ($matches[2] ?? 0);
            }

            if (\preg_match('/ID[\s]*+[\[][\s]*+[<]([^>]*+)[>][\s]*+[<]([^>]*+)[>]/i', $trailer_data, $matches) === 1) {
                /** @var array<int, string> $id_array */
                $id_array = [$matches[1] ?? '', $matches[2] ?? ''];
                $xref['trailer']['id'] = $id_array;
            }
        }

        if (\preg_match('/Prev[\s]+([0-9]+)/i', $trailer_data, $matches) === 1) {
            // get previous xref
            return $this->getXrefData((int) ($matches[1] ?? 0), $xref);
        }

        $xref['trailer'] ??= self::XREF_EMPTY['trailer'];

        return $xref;
    }

    /**
     * Decode the Cross-Reference Stream section.
     *
     * @param int             $startxref Offset at which the xref section starts.
     * @param XrefDataPartial $xref      Previous xref array (if any).
     *
     * @return XrefData Xref and trailer data.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function decodeXrefStream(int $startxref, array $xref): array
    {
        // try to read Cross-Reference Stream
        $xrefobj = $this->getRawObject($startxref);
        if (!\is_string($xrefobj[1])) {
            throw new PPException('Unable to find xref stream');
        }

        $xrefcrs = $this->getIndirectObject($xrefobj[1], $startxref, true);

        $filltrailer = ($xref['trailer'] ?? []) === [];
        if ($filltrailer) {
            $xref['trailer'] = self::XREF_EMPTY['trailer'];
        }

        $valid_crs = false;
        $sarr = $xrefcrs[0][1] ?? [];
        /** @var array<int, array>|string $sarr */
        if (!\is_array($sarr)) {
            $sarr = [];
        }

        /** @var array<int, RawObjectArray> $sarr */
        /** @var XrefData $xref */

        $wbt = [0, 0, 0];
        $state = [
            'index_sections' => null,
            'prevxref' => null,
            'predictor' => 0,
            'columns' => 0,
            'colors' => 1,
            'bits' => 8,
            'size' => null,
            'valid_crs' => $valid_crs,
        ];
        $this->processXrefType($sarr, $xref, $wbt, $state, $filltrailer);
        $index_sections = $state['index_sections'];
        $size = $state['size'];
        $valid_crs = $state['valid_crs'];
        // decode data
        $streamData = $xrefcrs[1][3][0] ?? null;
        if ($valid_crs && \is_string($streamData)) {
            $entryWidth = \max(1, (int) (($wbt[0] ?? 0) + ($wbt[1] ?? 0) + ($wbt[2] ?? 0)));
            // the filter layer already reversed the predictor: a predicted stream
            // decodes to rows of the geometry declared in DecodeParms
            $predicted = (int) $state['predictor'] > 1
                ? $this->predictedRowLength($state['colors'], $state['bits'], $state['columns'])
                : 0;
            $rowlen = \max(1, $predicted > 0 ? $predicted : $entryWidth);
            if ($index_sections === null) {
                if ($size === null) {
                    throw new PPException('Unable to determine xref stream Index coverage: missing Index and Size');
                }

                $index_sections = [[0, $size]];
            }

            // reject a declared coverage the stream cannot hold before allocating for it
            $this->assertXrefStreamCoverage($streamData, $rowlen, $index_sections, $entryWidth);
            $sdata = $this->decodeXrefStreamRows($streamData, $rowlen, $wbt);
            $objNumbers = $this->buildXrefObjectNumbers($index_sections);
            $this->processObjIndexesMap($xref, $objNumbers, $sdata);
        }

        // end decoding data
        $prevxref = $state['prevxref'];
        if ($prevxref === null) {
            return $xref;
        }

        // get previous xref
        return $this->getXrefData($prevxref, $xref);
    }

    /**
     * Decode the fields of each xref stream row into its entry values.
     *
     * @param array<int, array<int, int>> $sdata Decoded entry values.
     * @param array<int, array<int, int>> $ddata Rows of the un-predicted stream.
     * @param array<int, int>             $wbt   Field widths in bytes.
     */
    protected function processDdata(array &$sdata, array $ddata, array $wbt): void
    {
        // for every row
        foreach ($ddata as $key => $row) {
            // initialize new row
            $sdata[$key] = [0, 0, 0];
            if (($wbt[0] ?? 0) === 0) {
                // default type field
                $sdata[$key][0] = 1;
            }

            $idx = 0; // count bytes in the row
            // rows come from array_chunk(), so their keys are contiguous from zero
            $rowlen = \count($row);
            // for every column
            for ($col = 0; $col < 3; ++$col) {
                // for every byte on the column
                $colWidth = (int) ($wbt[$col] ?? 0);
                for ($byte = 0; $byte < $colWidth; ++$byte) {
                    if ($idx >= $rowlen || !\array_key_exists($idx, $row)) {
                        // the declared field width is wider than the row: no byte is left to
                        // read, and neither is one for any later column
                        break 2;
                    }

                    $rowValue = (int) $row[$idx];
                    $currentValue = (int) ($sdata[$key][$col] ?? 0);
                    $shift = ($colWidth - 1 - $byte) * 8;
                    $sdata[$key][$col] = $currentValue + ($rowValue << $shift);
                    ++$idx;
                }
            }
        }
    }
}
