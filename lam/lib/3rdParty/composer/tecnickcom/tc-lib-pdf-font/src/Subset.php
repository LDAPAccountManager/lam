<?php

declare(strict_types=1);

/**
 * Subset.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

use Com\Tecnick\File\Byte;
use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\Import\TrueType;

/**
 * Com\Tecnick\Pdf\Font\Subset
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-import-type TFontData from Load
 */
class Subset
{
    /**
     * array of table names to preserve (loca and glyf tables will be added later)
     * the cmap table is not needed and shall not be present,
     * since the mapping from character codes to glyph descriptions is provided separately
     *
     * @var array<string, bool>
     */
    protected const TABLENAMES = [
        'head' => true,
        'hhea' => true,
        'hmtx' => true,
        'maxp' => true,
        'cvt ' => true,
        'fpgm' => true,
        'prep' => true,
        'glyf' => true,
        'loca' => true,
    ];

    /**
     * Content of the input font file
     */
    protected string $font = '';

    /**
     * Object used to read font bytes
     */
    protected Byte $fbyte;

    /**
     * Extracted font metrics
     *
     * @var TFontData
     */
    protected array $fdt = [
        'Ascender' => 0,
        'Ascent' => 0,
        'AvgWidth' => 0.0,
        'CapHeight' => 0,
        'CharacterSet' => '',
        'Descender' => 0,
        'Descent' => 0,
        'EncodingScheme' => '',
        'FamilyName' => '',
        'Flags' => 0,
        'FontBBox' => [],
        'FontName' => '',
        'FullName' => '',
        'IsFixedPitch' => false,
        'ItalicAngle' => 0,
        'Leading' => 0,
        'MaxWidth' => 0,
        'MissingWidth' => 0,
        'StdHW' => 0,
        'StdVW' => 0,
        'StemH' => 0,
        'StemV' => 0,
        'UnderlinePosition' => 0,
        'UnderlineThickness' => 0,
        'Version' => '',
        'Weight' => '',
        'XHeight' => 0,
        'bbox' => '',
        'cbbox' => [],
        'cidinfo' => [
            'Ordering' => '',
            'Registry' => '',
            'Supplement' => 0,
            'uni2cid' => [],
        ],
        'compress' => false,
        'ctg' => '',
        'ctgdata' => [],
        'cw' => [],
        'cwu' => [],
        'datafile' => '',
        'desc' => [
            'Ascent' => 0,
            'AvgWidth' => 0,
            'CapHeight' => 0,
            'Descent' => 0,
            'Flags' => 0,
            'FontBBox' => '',
            'ItalicAngle' => 0,
            'Leading' => 0,
            'MaxWidth' => 0,
            'MissingWidth' => 0,
            'StemH' => 0,
            'StemV' => 0,
            'XHeight' => 0,
        ],
        'diff' => '',
        'diff_n' => 0,
        'dir' => '',
        'dw' => 0,
        'enc' => '',
        'enc_map' => [],
        'encodingTables' => [],
        'encoding_id' => 0,
        'encrypted' => '',
        'fakestyle' => false,
        'family' => '',
        'file' => '',
        'file_n' => 0,
        'file_name' => '',
        'i' => 0,
        'ifile' => '',
        'indexToLoc' => [],
        'input_file' => '',
        'isUnicode' => false,
        'italicAngle' => 0,
        'key' => '',
        'lenIV' => 0,
        'length1' => 0,
        'length2' => 0,
        'linked' => false,
        'mode' => [
            'bold' => false,
            'italic' => false,
            'linethrough' => false,
            'overline' => false,
            'underline' => false,
        ],
        'n' => 0,
        'name' => '',
        'numGlyphs' => 0,
        'numHMetrics' => 0,
        'originalsize' => 0,
        'pdfa' => false,
        'platform_id' => 0,
        'settype' => '',
        'short_offset' => false,
        'size1' => 0,
        'size2' => 0,
        'style' => '',
        'subset' => false,
        'subsetchars' => [],
        'table' => [],
        'tot_num_glyphs' => 0,
        'type' => '',
        'underlinePosition' => 0,
        'underlineThickness' => 0,
        'unicode' => false,
        'unitsPerEm' => 0,
        'up' => 0,
        'urk' => 0.0,
        'ut' => 0,
        'weight' => '',
    ];

    /**
     * Array containing subset glyphs indexes of chars from cmap table
     *
     * @var array<int, bool>
     */
    protected array $subglyphs = [];

    /**
     * Subset font
     */
    protected string $subfont = '';

    /**
     * Pointer position on the original font data
     */
    protected int $offset = 0;

    /**
     * File helper used to load font definition files.
     */
    protected ObjFile $fileHelper;

    /**
     * Process TrueType font
     *
     * @param string           $font       Content of the input font file
     * @param TFontData        $fdt        Extracted font metrics
     * @param ObjFile          $fileHelper Optional file helper for font loading.
     * @param array<int, bool> $subchars   Array containing subset chars
     *
     * @throws FontException in case of error
     */
    public function __construct(string $font, array $fdt, ObjFile $fileHelper, array $subchars = [])
    {
        $this->fileHelper = $fileHelper;
        $this->font = $font;
        $this->fbyte = new Byte($font);
        $trueType = new TrueType(
            font: $font,
            fdt: $fdt,
            fileHelper: $this->fileHelper,
            fbyte: $this->fbyte,
            subchars: $subchars,
            // Subsetting only needs the glyph program (loca/glyf); per-glyph bounding
            // boxes are never read here, so skip computing them.
            withCbbox: false,
        );
        $this->fdt = $trueType->getFontMetrics();
        $this->subglyphs = $trueType->getSubGlyphs();
        $this->addCompositeGlyphs();
        $this->addProcessedTables();
        $this->removeUnusedTables();
        $this->buildSubsetFont();
    }

    /**
     * Get all the extracted font metrics
     */
    public function getSubsetFont(): string
    {
        return $this->subfont;
    }

    /**
     * Returns the checksum of a TTF table.
     *
     * @param string $table  Table to check
     * @param int    $length Length of table in bytes
     *
     * @return int checksum
     *
     * @throws FontException
     */
    protected function getTableChecksum(string $table, int $length): int
    {
        $sum = 0;
        $tlen = (int) floor(($length + 3) / 4);
        $offset = 0;
        for ($idx = 0; $idx < $tlen; ++$idx) {
            $chunk = \substr($table, $offset, 4);
            if (\strlen($chunk) < 4) {
                // OpenType checksums use zero-padding for trailing partial words.
                $chunk = \str_pad($chunk, 4, "\0", STR_PAD_RIGHT);
            }

            $val = \unpack('Ni', $chunk);
            if ($val === false) {
                throw new FontException('Unable to unpack table data');
            }

            $sum += $val['i'];
            $offset += 4;
        }

        $sum = \unpack('Ni', \pack('N', $sum));
        if ($sum === false) {
            throw new FontException('Unable to unpack checksum');
        }

        return $sum['i'];
    }

    /**
     * Add composite glyphs
     */
    protected function addCompositeGlyphs(): void
    {
        $new_sga = $this->subglyphs;
        while ($new_sga !== []) {
            $sga = \array_keys($new_sga);
            $new_sga = [];
            foreach ($sga as $key) {
                $new_sga = $this->findCompositeGlyphs($new_sga, $key);
            }

            foreach ($new_sga as $gid => $enabled) {
                if (!$enabled) {
                    continue;
                }

                $this->subglyphs[$gid] = true;
            }
        }

        // sort glyphs by key
        \ksort($this->subglyphs);
    }

    /**
     * Find composite glyphs
     *
     * @param array<int, bool> $new_sga
     * @param int $key
     *
     * @return array<int, bool>
     */
    protected function findCompositeGlyphs(array $new_sga, int $key): array
    {
        if (isset($this->fdt['indexToLoc'][$key])) {
            /**
             * Glyph Header
             * - int16      numberOfContours    Normal glyph if >= 0 or composite glyph if negative (should be -1)
             * - int16      xMin                Minimum x for coordinate data.
             * - int16      yMin                Minimum y for coordinate data.
             * - int16      xMax                Maximum x for coordinate data.
             * - int16      yMax                Maximum y for coordinate data.
             */

            $this->offset = $this->fdt['table']['glyf']['offset'] + $this->fdt['indexToLoc'][$key];
            $numberOfContours = $this->fbyte->getShort($this->offset);
            $this->offset += 2;
            if ($numberOfContours < 0) { // composite glyph
                /**
                 * ComponentGlyph record
                 * - uint16            flags          Normal glyph if >= 0 or composite glyph if negative (should be -1)
                 * - uint16            glyphIndex     glyph index of component
                 * - u/int8|u/int16    argument1      x-offset for component or point number; type depends on bits 0 and 1 in component flags
                 * - u/int8|u/int16    argument2      y-offset for component or point number; type depends on bits 0 and 1 in component flags
                 * - [transform data]                 optional transform data
                 */
                $this->offset += 8; // skip xMin, yMin, xMax, yMax
                do {
                    $flags = $this->fbyte->getUShort($this->offset);
                    $this->offset += 2;
                    $glyphIndex = $this->fbyte->getUShort($this->offset);
                    $this->offset += 2;
                    if (!isset($this->subglyphs[$glyphIndex])) {
                        // add missing glyphs
                        $new_sga[$glyphIndex] = true;
                    }

                    // skip some bytes by case
                    // ARG_1_AND_2_ARE_WORDS (bit 0): [u]int32 if set and [u]int16 if not set
                    if (($flags & 1) !== 0) {
                        $this->offset += 4;
                    } else {
                        $this->offset += 2;
                    }

                    if (($flags & 8) !== 0) {
                        // WE_HAVE_A_SCALE (bit 3):            Adds 1 * F2DOT14 field
                        $this->offset += 2;
                    } elseif (($flags & 64) !== 0) {
                        // WE_HAVE_AN_X_AND_Y_SCALE (bit 6):   Adds 2 * F2DOT14 fields
                        $this->offset += 4;
                    } elseif (($flags & 128) !== 0) {
                        // WE_HAVE_A_TWO_BY_TWO (bit 7):       Adds 4 * F2DOT14 fields
                        $this->offset += 8;
                    }
                } while ($flags & 32); // MORE_COMPONENTS (bit 5)
            }
        }

        return $new_sga;
    }

    /**
     * Remove unused tables
     */
    protected function removeUnusedTables(): void
    {
        // get the tables to preserve
        $this->offset = 12;
        $tabname = \array_keys($this->fdt['table']);
        foreach ($tabname as $tag) {
            if (!isset(self::TABLENAMES[$tag])) {
                // remove the table
                unset($this->fdt['table'][$tag]);
                continue;
            }

            if (!isset($this->fdt['table'][$tag])) {
                $this->fdt['table'][$tag] = [
                    'checkSum' => 0,
                    'data' => '',
                    'length' => 0,
                    'offset' => 0,
                ];
            }

            $isSubsetTable = $tag === 'loca' || $tag === 'glyf';
            if (!$isSubsetTable) {
                $this->fdt['table'][$tag]['data'] = \substr(
                    $this->font,
                    $this->fdt['table'][$tag]['offset'],
                    $this->fdt['table'][$tag]['length'],
                );
                if ($tag === 'head') {
                    // set the checkSumAdjustment to 0
                    $this->fdt['table'][$tag]['data'] =
                        \substr($this->fdt['table'][$tag]['data'], 0, 8)
                        . "\x0\x0\x0\x0"
                        . \substr($this->fdt['table'][$tag]['data'], 12);
                }
            }

            $pad = 4 - ((int) $this->fdt['table'][$tag]['length'] % 4);
            if ($pad !== 4) {
                // the length of a table must be a multiple of four bytes
                $this->fdt['table'][$tag]['length'] += (int) $pad;
                $this->fdt['table'][$tag]['data'] .= \str_repeat("\x0", max(0, $pad));
            }

            $this->fdt['table'][$tag]['offset'] = $this->offset;
            $this->offset += $this->fdt['table'][$tag]['length'];

            // check sum is not changed
        }
    }

    /**
     * Add glyf and loca tables
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    protected function addProcessedTables(): void
    {
        // build new glyf and loca tables
        $glyf = '';
        $loca = '';
        $this->offset = 0;
        $glyf_offset = $this->fdt['table']['glyf']['offset'];
        for ($i = 0; $i < $this->fdt['tot_num_glyphs']; ++$i) {
            $nextidx = $this->getNextLocaIndex($i + 1);
            if (isset($this->subglyphs[$i], $this->fdt['indexToLoc'][$i]) && $nextidx !== null) {
                $length = $this->fdt['indexToLoc'][$nextidx] - $this->fdt['indexToLoc'][$i];
                $glyf .= \substr($this->font, $glyf_offset + $this->fdt['indexToLoc'][$i], $length);
            } else {
                $length = 0;
            }

            if ($this->fdt['short_offset']) {
                $loca .= \pack('n', \floor($this->offset / 2));
            } else {
                $loca .= \pack('N', $this->offset);
            }

            $this->offset += $length;
        }

        // add loca
        if (!isset($this->fdt['table']['loca'])) {
            $this->fdt['table']['loca'] = [
                'checkSum' => 0,
                'data' => '',
                'length' => 0,
                'offset' => 0,
            ];
        }

        $this->fdt['table']['loca']['data'] = $loca;
        $this->fdt['table']['loca']['length'] = \strlen($loca);
        $this->fdt['table']['loca']['offset'] = $this->offset;
        $pad = 4 - ($this->fdt['table']['loca']['length'] % 4);
        if ($pad !== 4) {
            // the length of a table must be a multiple of four bytes
            $this->fdt['table']['loca']['length'] += $pad;
            $this->fdt['table']['loca']['data'] .= \str_repeat("\x0", $pad);
        }

        $this->fdt['table']['loca']['checkSum'] = $this->getTableChecksum(
            $this->fdt['table']['loca']['data'],
            $this->fdt['table']['loca']['length'],
        );

        $this->offset += $this->fdt['table']['loca']['length'];

        // add glyf
        if (!isset($this->fdt['table']['glyf'])) {
            $this->fdt['table']['glyf'] = [
                'checkSum' => 0,
                'data' => '',
                'length' => 0,
                'offset' => 0,
            ];
        }

        $this->fdt['table']['glyf']['data'] = $glyf;
        $this->fdt['table']['glyf']['length'] = \strlen($glyf);
        $this->fdt['table']['glyf']['offset'] = $this->offset;
        $pad = 4 - ($this->fdt['table']['glyf']['length'] % 4);
        if ($pad !== 4) {
            // the length of a table must be a multiple of four bytes
            $this->fdt['table']['glyf']['length'] += $pad;
            $this->fdt['table']['glyf']['data'] .= \str_repeat("\x0", $pad);
        }

        $this->fdt['table']['glyf']['checkSum'] = $this->getTableChecksum(
            $this->fdt['table']['glyf']['data'],
            $this->fdt['table']['glyf']['length'],
        );
    }

    /**
     * Returns the first available loca index from $start, or null if none exists.
     */
    protected function getNextLocaIndex(int $start): ?int
    {
        for ($idx = $start; $idx <= $this->fdt['tot_num_glyphs']; ++$idx) {
            if (isset($this->fdt['indexToLoc'][$idx])) {
                return $idx;
            }
        }

        return null;
    }

    /**
     * build new subset font
     *
     * @throws FontException
     */
    protected function buildSubsetFont(): void
    {
        $this->subfont = '';
        $this->subfont .= \pack('N', 0x1_0000); // sfnt version
        $numTables = \count($this->fdt['table']);
        $this->subfont .= \pack('n', $numTables); // numTables
        $entrySelector = \floor(\log($numTables, 2));
        $searchRange = (2 ** $entrySelector) * 16;
        $rangeShift = ($numTables * 16) - $searchRange;
        $this->subfont .= \pack('n', $searchRange); // searchRange
        $this->subfont .= \pack('n', $entrySelector); // entrySelector
        $this->subfont .= \pack('n', $rangeShift); // rangeShift
        // Table offsets stored in $this->fdt start after the 12-byte sfnt header.
        // The full output adds the table directory immediately after that header,
        // so both directory offsets and in-buffer table positions must include this base.
        $tableDataBaseOffset = $numTables * 16;
        $this->offset = $tableDataBaseOffset;
        foreach ($this->fdt['table'] as $tag => $data) {
            $this->subfont .= $tag; // tag
            $this->subfont .= \pack('N', $data['checkSum']); // checkSum
            $this->subfont .= \pack('N', $data['offset'] + $this->offset); // offset
            $this->subfont .= \pack('N', $data['length']); // length
        }

        foreach ($this->fdt['table'] as $data) {
            $this->subfont .= $data['data'];
        }

        // set checkSumAdjustment on head table
        $checkSumAdjustment = 0xB1B0_AFBA - $this->getTableChecksum($this->subfont, \strlen($this->subfont));
        $headAdjustmentPos = $tableDataBaseOffset + $this->fdt['table']['head']['offset'] + 8;
        $this->subfont =
            \substr($this->subfont, 0, $headAdjustmentPos)
            . \pack('N', $checkSumAdjustment)
            . \substr($this->subfont, $headAdjustmentPos + 4);
    }
}
