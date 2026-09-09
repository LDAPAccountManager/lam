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
     * Largest glyph offset the short format of the loca table can address.
     *
     * The entries are stored halved as 16-bit values, so the last one they reach is
     * 0xFFFF * 2.
     */
    protected const MAX_SHORT_LOCA_OFFSET = 131_070;

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
    protected array $fdt = Load::DEFAULT_DATA;

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
     * Build the subset of a TrueType font
     *
     * @param string           $font       Content of the input font file
     * @param TFontData        $fdt        Extracted font metrics
     * @param ObjFile          $fileHelper File helper for font loading.
     * @param array<int, bool> $subchars   Array containing subset chars
     *
     * @throws FontException in case of error
     */
    public function __construct(string $font, array $fdt, ObjFile $fileHelper, array $subchars = [])
    {
        $this->fileHelper = $fileHelper;
        $this->font = $font;
        $this->fbyte = new Byte($font);

        try {
            $trueType = new TrueType(
                font: $font,
                fdt: $fdt,
                fileHelper: $this->fileHelper,
                fbyte: $this->fbyte,
                subchars: $subchars,
                // subsetting only needs the glyph program (loca and glyf tables)
                withCbbox: false,
                subsetting: true,
            );
            $this->fdt = $trueType->getFontMetrics();
            if (!$trueType->isSubsettingAllowed()) {
                // the OS/2 fsType carries the "No Subsetting" bit, so the whole program
                // is returned untouched
                $this->subfont = $font;
                return;
            }

            $this->subglyphs = $trueType->getSubGlyphs();
            $this->addCompositeGlyphs();
            $this->addProcessedTables();
            $this->removeUnusedTables();
            $this->buildSubsetFont();
        } catch (\RangeException $exc) {
            // the byte reader ran past the end of a truncated or corrupt font program
            throw new FontException('Malformed font program: ' . $exc->getMessage(), 0, $exc);
        }
    }

    /**
     * Returns the subset font program
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
     */
    protected function getTableChecksum(string $table, int $length): int
    {
        $sum = 0;
        $tlen = (int) \floor(($length + 3) / 4);
        $offset = 0;
        for ($idx = 0; $idx < $tlen; ++$idx) {
            // OpenType checksums use zero-padding for trailing partial words
            $chunk = \str_pad(\substr($table, $offset, 4), 4, "\0", STR_PAD_RIGHT);
            $sum += (\ord($chunk[0]) << 24) | (\ord($chunk[1]) << 16) | (\ord($chunk[2]) << 8) | \ord($chunk[3]);
            // the sum is truncated to 32 bits at every word
            $sum &= 0xFFFF_FFFF;
            $offset += 4;
        }

        return $sum;
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
     * Find the component glyphs of a composite glyph
     *
     * @param array<int, bool> $new_sga Glyph indexes to add to the subset
     * @param int              $key     Glyph index to inspect
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
            // the MORE_COMPONENTS chain is bounded by the end of this glyph
            $nextidx = $this->getNextLocaIndex($key + 1);
            $glyphEnd = $nextidx === null
                ? $this->fdt['table']['glyf']['offset'] + $this->fdt['table']['glyf']['length']
                : $this->fdt['table']['glyf']['offset'] + $this->fdt['indexToLoc'][$nextidx];
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
                    if (($this->offset + 4) > $glyphEnd) {
                        break; // the component record does not fit this glyph
                    }

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
     *
     * @throws FontException if a preserved table is truncated by the font file.
     */
    protected function removeUnusedTables(): void
    {
        // the sfnt specification requires the table directory to be sorted by tag in
        // ascending order, which is also the order buildSubsetFont() emits
        \ksort($this->fdt['table']);

        // get the tables to preserve
        $this->offset = 12;
        $tabname = \array_keys($this->fdt['table']);
        foreach ($tabname as $tag) {
            if (!isset(self::TABLENAMES[$tag])) {
                // remove the table
                unset($this->fdt['table'][$tag]);
                continue;
            }

            $isSubsetTable = $tag === 'loca' || $tag === 'glyf';
            if (!$isSubsetTable) {
                $this->fdt['table'][$tag]['data'] = \substr(
                    $this->font,
                    $this->fdt['table'][$tag]['offset'],
                    $this->fdt['table'][$tag]['length'],
                );
                // the emitted table directory advertises the declared length
                if (\strlen($this->fdt['table'][$tag]['data']) !== $this->fdt['table'][$tag]['length']) {
                    throw new FontException('truncated font table: ' . $tag);
                }

                if ($tag === 'head') {
                    // set the checkSumAdjustment to 0, replacing the four bytes in place
                    $this->fdt['table'][$tag]['data'] = \substr_replace(
                        $this->fdt['table'][$tag]['data'],
                        "\x0\x0\x0\x0",
                        8,
                        4,
                    );
                }

                // the checksum is computed from the bytes emitted, not copied from the
                // directory of the input font
                $this->fdt['table'][$tag]['checkSum'] = $this->getTableChecksum(
                    $this->fdt['table'][$tag]['data'],
                    $this->fdt['table'][$tag]['length'],
                );
            }

            $this->padTable($tag);
            $this->fdt['table'][$tag]['offset'] = $this->offset;
            $this->offset += \strlen($this->fdt['table'][$tag]['data']);
        }
    }

    /**
     * Pad the data of a table to the next four byte boundary.
     *
     * The gap to the next boundary is filled with zeroes. The table directory keeps
     * advertising the real length of the table.
     *
     * @param string $tag Tag of the table to pad.
     */
    protected function padTable(string $tag): void
    {
        $pad = (4 - (\strlen($this->fdt['table'][$tag]['data']) % 4)) % 4;
        if ($pad !== 0) {
            $this->fdt['table'][$tag]['data'] .= \str_repeat("\x0", $pad);
        }
    }

    /**
     * Add glyf and loca tables
     *
     * @throws FontException if the glyph data does not fit the format of the loca table.
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
            $length = 0;
            if (isset($this->subglyphs[$i], $this->fdt['indexToLoc'][$i]) && $nextidx !== null) {
                // a non-monotonic loca table would give a negative length
                $length = \max(0, $this->fdt['indexToLoc'][$nextidx] - $this->fdt['indexToLoc'][$i]);
                $glyf .= \substr($this->font, $glyf_offset + $this->fdt['indexToLoc'][$i], $length);
            }

            if ($this->fdt['short_offset']) {
                if ($this->offset > self::MAX_SHORT_LOCA_OFFSET) {
                    throw new FontException('The subset glyph data is too large for a short loca table');
                }

                $loca .= \pack('n', \intdiv($this->offset, 2));
            } else {
                $loca .= \pack('N', $this->offset);
            }

            $this->offset += $length;
        }

        // add loca and glyf; removeUnusedTables() assigns their offsets
        $this->fdt['table']['loca']['data'] = $loca;
        $this->fdt['table']['loca']['length'] = \strlen($loca);
        $this->padTable('loca');
        $this->fdt['table']['loca']['checkSum'] = $this->getTableChecksum(
            $this->fdt['table']['loca']['data'],
            \strlen($this->fdt['table']['loca']['data']),
        );

        $this->fdt['table']['glyf']['data'] = $glyf;
        $this->fdt['table']['glyf']['length'] = \strlen($glyf);
        $this->padTable('glyf');
        $this->fdt['table']['glyf']['checkSum'] = $this->getTableChecksum(
            $this->fdt['table']['glyf']['data'],
            \strlen($this->fdt['table']['glyf']['data']),
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
     * Build the subset font program
     */
    protected function buildSubsetFont(): void
    {
        $this->subfont = '';
        $this->subfont .= \pack('N', 0x1_0000); // sfnt version
        $numTables = \count($this->fdt['table']);
        $this->subfont .= \pack('n', $numTables); // numTables
        // entrySelector = floor(log2(numTables))
        $entrySelector = 0;
        while ((2 << $entrySelector) <= $numTables) {
            ++$entrySelector;
        }

        $searchRange = (1 << $entrySelector) * 16;
        $rangeShift = ($numTables * 16) - $searchRange;
        $this->subfont .= \pack('n', $searchRange); // searchRange
        $this->subfont .= \pack('n', $entrySelector); // entrySelector
        $this->subfont .= \pack('n', $rangeShift); // rangeShift
        // the offsets stored in $this->fdt start after the 12-byte sfnt header, so they are
        // shifted by the size of the table directory that follows it
        $tableDataBaseOffset = $numTables * 16;
        foreach ($this->fdt['table'] as $tag => $data) {
            $this->subfont .= $tag; // tag
            $this->subfont .= \pack('N', $data['checkSum']); // checkSum
            $this->subfont .= \pack('N', $data['offset'] + $tableDataBaseOffset); // offset
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
