<?php

declare(strict_types=1);

/**
 * TrueType.php
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

namespace Com\Tecnick\Pdf\Font\Import;

use Com\Tecnick\File\Byte;
use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\FileWriter;
use Com\Tecnick\Pdf\Font\Import as FontImport;
use Com\Tecnick\Pdf\Font\Zlib;

/**
 * Com\Tecnick\Pdf\Font\Import\TrueType
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 * @SuppressWarnings("PHPMD.ExcessiveClassLength")
 */
class TrueType implements ProcessorInterface
{
    /**
     * File helper used to load font definition files.
     */
    protected ObjFile $fileHelper;

    /**
     * Minimum byte length of the OS/2 table needed to read through fsType.
     */
    private const OS2_MIN_LENGTH = 10;

    /**
     * Minimum byte length of the tables whose fixed-size header is read by this class.
     *
     * @var array<string, int>
     */
    private const MIN_TABLE_LENGTH = [
        'head' => 54, // fixed size of the table
        'hhea' => 36, // fixed size of the table
        'maxp' => 6, // through numGlyphs
        'post' => 16, // through isFixedPitch
        'cmap' => 4, // through numTables
        'name' => 6, // through stringOffset
        'hmtx' => 4, // one longHorMetric
    ];

    /**
     * Highest valid Unicode code point (U+10FFFF).
     *
     * Bounds the character ranges of the uint32-based cmap subtable formats (8, 12 and 13).
     */
    private const MAX_UNICODE_CODEPOINT = 0x10_FFFF;

    /**
     * Maximum number of character codes a single cmap subtable may map.
     *
     * A non-overlapping Unicode cmap cannot map more code points than exist.
     */
    private const MAX_CMAP_ENTRIES = 0x11_0000;

    /**
     * Maximum number of character codes a cmap subtable may map for each glyph of the font.
     *
     * The budget follows the number of glyphs the loca table accounts for, so that the
     * extracted metrics stay proportional to the font program.
     */
    private const MAX_CMAP_ENTRIES_PER_GLYPH = 8;

    /**
     * Smallest number of character codes a cmap subtable may map, whatever the glyph count.
     *
     * This is the size of the byte encoded subtable formats, which map the whole code space
     * they can address regardless of how many glyphs the font carries.
     */
    private const MIN_CMAP_ENTRIES = 256;

    /**
     * Name records usable as the font name, by decreasing preference.
     *
     * nameID 6 (PostScript name) is the one a PDF /BaseFont expects; the full name and
     * the family name are only used when the font does not declare it.
     *
     * @var array<int, int>
     */
    private const NAME_ID_PRIORITY = [
        6 => 3, // PostScript name
        4 => 2, // Full font name
        1 => 1, // Font family name
    ];

    /**
     * Priority-ordered (platformID, encodingID) fallback pairs for cmap subtable selection.
     * Used when no subtable matching the caller-requested pair is found.
     *
     * @var array<int, array{0: int, 1: int}>
     */
    private const CMAP_FALLBACK_PRIORITY = [
        [3, 10], // Windows UCS-4 (full Unicode, format 12)
        [3, 1], // Windows Unicode BMP
        [0, 6], // Unicode platform - full repertoire
        [0, 4], // Unicode platform - 2.0+, BMP + supplementary
        [0, 3], // Unicode platform - 2.0+, BMP only
        [0, 2], // Unicode platform - 1.1
        [0, 1], // Unicode platform - 1.1 (deprecated)
        [0, 0], // Unicode platform - 1.0 (deprecated)
        [1, 0], // Macintosh Roman (legacy)
        // Windows Symbol, last because its codes are not code points but the
        // 0xF000..0xF0FF private use range, which is kept as it is read
        [3, 0],
    ];

    /**
     * Array containing subset chars
     *
     * @var array<int, bool>
     */
    protected array $subchars = [];

    /**
     * Array containing subset glyphs indexes of chars from cmap table
     *
     * @var array<int, bool>
     */
    protected array $subglyphs = [
        0 => true,
    ];

    /**
     * Pointer position on the original font data
     */
    protected int $offset = 0;

    /**
     * True while the licensing bits of this font program allow subsetting.
     *
     * A font without an OS/2 table states no restriction, so the default is permissive.
     */
    protected bool $subsettingAllowed = true;

    /**
     * Process TrueType font
     *
     * @param string           $font       Content of the input font file
     * @param TFontData        $fdt        Extracted font metrics
     * @param ObjFile          $fileHelper File helper for font loading.
     * @param Byte             $fbyte      Object used to read font bytes
     * @param array<int, bool> $subchars   Array containing subset chars
     * @param bool             $withCbbox  If true, compute the per-glyph bounding boxes.
     * @param bool             $subsetting If true, the font program is being subset rather
     *                                     than imported, so the original file is neither
     *                                     copied nor linked again.
     *
     * @throws FileException
     * @throws FontException
     */
    public function __construct(
        protected string $font,
        protected array $fdt,
        ObjFile $fileHelper,
        protected Byte $fbyte,
        array $subchars = [],
        protected bool $withCbbox = true,
        protected bool $subsetting = false,
    ) {
        $this->fileHelper = $fileHelper;
        $this->subchars = $subchars;
        $this->process();
    }

    /**
     * Get all the extracted font metrics
     *
     * @return TFontData
     */
    public function getFontMetrics(): array
    {
        return $this->fdt;
    }

    /**
     * Get glyphs in the subset
     *
     * @return array<int, bool>
     */
    public function getSubGlyphs(): array
    {
        return $this->subglyphs;
    }

    /**
     * Returns true when the OS/2 licensing bits of this font program allow subsetting.
     *
     * This reports what the program declares, unlike the 'subset' metric, which carries
     * what the document asked for.
     */
    public function isSubsettingAllowed(): bool
    {
        return $this->subsettingAllowed;
    }

    /**
     * Process TrueType font
     *
     * @throws FileException
     * @throws FontException
     */
    protected function process(): void
    {
        $this->isValidType();
        $this->setFontFile();
        $this->getTables();
        $this->checkRequiredTables();
        $this->checkTableBounds();
        $this->checkMagickNumber();
        $this->offset += 2; // skip flags
        $this->getBbox();
        // read before the loca table so that its entry count can be bounded by numGlyphs
        $this->getMaxpData();
        $this->getIndexToLoc();
        $this->getEncodingTables();
        $this->getOS2Metrics();
        $this->getFontName();
        $this->getPostData();
        $this->getHheaData();
        $this->getCIDToGIDMap();
        $this->getHeights();
        $this->getWidths();
    }

    /**
     * Check if the font has a valid sfnt version header
     *
     * Valid TTF 1.0 files begin with 1.0 in Version16Dot16 format
     *
     * @throws FontException if the font is invalid
     */
    protected function isValidType(): void
    {
        if ($this->fbyte->getULong($this->offset) !== 0x0001_0000) {
            throw new FontException('sfnt version must be 0x00010000 for TrueType version 1.0.');
        }

        $this->offset += 4;
    }

    /**
     * Copy or link the original font file
     *
     * @throws FileException
     * @throws FontException
     */
    protected function setFontFile(): void
    {
        if ($this->subsetting) {
            // the artifacts of an already imported font are not written again
            $this->fdt['Flags'] = $this->fdt['desc']['Flags'];
            return;
        }

        if ($this->fdt['type'] === 'cidfont0') {
            return;
        }

        if ($this->fdt['linked']) {
            // Link to the existing font instead of copying it. The link keeps the original
            // extension, so the stored program is the raw (uncompressed) font file.
            //
            // The link resolves to the original font directory, which is outside the roots
            // FontPaths::buildAllowedPaths() trusts at render time: embedding a linked font
            // requires either K_PATH_FONTS to cover that directory, or a file helper passed
            // to Output whose allowlist includes it.
            $link = FontImport::linkedFileName($this->fdt['file_name'], $this->fdt['input_file']);
            $target = $this->fdt['dir'] . $link;
            // a relative input path would produce a dangling symlink
            $source = \realpath($this->fdt['input_file']);
            if ($source === false) {
                throw new FontException('unable to resolve the font file: ' . $this->fdt['input_file']);
            }

            // an existing link is reused; anything else occupying the name is an error
            if (!\is_link($target)) {
                if (\file_exists($target)) {
                    throw new FontException('unable to create the symbolic link: ' . $target);
                }

                $this->createLink($source, $target);
            }

            // no '.z' suffix, as the stored program is raw
            $this->fdt['file'] = $link;
            return;
        }

        // store compressed font
        $this->fdt['file'] = $this->fdt['file_name'] . '.z';
        FileWriter::write(
            $this->fileHelper,
            $this->fdt['dir'] . $this->fdt['file'],
            Zlib::compress($this->font, 'Error compressing font file.'),
        );
    }

    /**
     * Create the symbolic link of a linked font.
     *
     * The E_WARNING symlink() emits on failure is captured and carried by the exception
     * message.
     *
     * @param string $source Resolved font file to link to.
     * @param string $target Link to create.
     *
     * @throws FontException if the link cannot be created.
     */
    protected function createLink(string $source, string $target): void
    {
        $reason = '';
        \set_error_handler(static function (int $level, string $message) use (&$reason): bool {
            if ($level === E_WARNING) {
                $reason = ' (' . $message . ')';
            }

            return true;
        }, E_WARNING);

        try {
            $linked = \symlink($source, $target);
        } finally {
            \restore_error_handler();
        }

        if (!$linked) {
            throw new FontException('unable to create the symbolic link: ' . $target . $reason);
        }
    }

    /**
     * Get the font tables
     *
     *  TableDirectory:
     *   0 - uint32  sfntVersion    Either 0x00010000 (For TTF font) or 0x4F54544F (which spells OTTO)
     *   4 - uint16  numTables      Number of tables in font file
     *   6 - uint16  searchRange    pow(2, floor(log2(numTables))) * 16 OR 1 << (entrySelector+4)
     *   8 - uint16  entrySelector  floor(log2(numTables))
     *  10 - uint16  rangeShift     numTables * 16 - searchRange
     *
     *  TableRecord (starts at byte-offset 12):
     *    - uint8[4] tag           4 * ascii characters (range from 0x20 tp 0x7E) right padded with 0x20 (space) if len < 4
     *    - uint32   checksum      The checksum for this table
     *    - Offset32 offset        The table offset in bytes from the beginning of the font file
     *    - uint32   length        The size of a table in bytes (excluding padding bytes)
     */
    protected function getTables(): void
    {
        // get number of tables
        $numTables = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        // a record is 16 bytes and the directory starts after the 12-byte header, so the
        // count is bounded by the number of records the file can hold
        $numTables = \min($numTables, \intdiv(\max(0, \strlen($this->font) - 12), 16));

        // Skip the searchRange, entrySelector and rangeShift fields (3 * uint16)
        $this->offset += 6;

        // tables array
        $this->fdt['table'] = [];
        // ---------- get tables ----------
        for ($idx = 0; $idx < $numTables; ++$idx) {
            // get table info
            $tag = \substr($this->font, $this->offset, 4);
            $this->offset += 4;
            if ($tag === (string) (int) $tag) {
                // PHP would turn a numeric tag into an integer array key, breaking the shape
                // of the directory; no table this library reads carries one
                $this->offset += 12;
                continue;
            }

            $this->fdt['table'][$tag] = [
                'checkSum' => 0,
                'data' => '',
                'length' => 0,
                'offset' => 0,
            ];
            $this->fdt['table'][$tag]['checkSum'] = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $this->fdt['table'][$tag]['offset'] = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $this->fdt['table'][$tag]['length'] = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
        }
    }

    /**
     * Ensure every table dereferenced by this parser is present in the table directory.
     *
     * @throws FontException if a mandatory table is missing
     */
    protected function checkRequiredTables(): void
    {
        foreach (['head', 'loca', 'cmap', 'name', 'post', 'hhea', 'maxp', 'hmtx', 'glyf'] as $tag) {
            if (!isset($this->fdt['table'][$tag])) {
                throw new FontException('Missing required font table: ' . $tag);
            }
        }
    }

    /**
     * Ensure every table of the directory lies inside the font file.
     *
     * Records are also rejected when shorter than the fixed-size header this class reads
     * from them (see MIN_TABLE_LENGTH).
     *
     * @throws FontException if a table record points outside the font file
     */
    protected function checkTableBounds(): void
    {
        $fontlen = \strlen($this->font);
        foreach ($this->fdt['table'] as $tag => $record) {
            if ($record['offset'] > $fontlen || $record['length'] > ($fontlen - $record['offset'])) {
                throw new FontException('Font table out of bounds: ' . $tag);
            }

            if ($record['length'] < (self::MIN_TABLE_LENGTH[$tag] ?? 0)) {
                throw new FontException('Font table too short: ' . $tag);
            }
        }
    }

    /**
     * Verify the font file includes the mandatory magicNumber field
     *
     * Valid TTF 1.0 files have the magic number 0x5f0f3cf5 in
     * the "head" table offset 12 bytes from the start of the table.
     *
     * @throws FontException if the font is invalid
     */
    protected function checkMagickNumber(): void
    {
        $this->offset = $this->fdt['table']['head']['offset'] + 12;
        if ($this->fbyte->getULong($this->offset) !== 0x5f0f_3cf5) {
            // magicNumber must be 0x5f0f3cf5
            throw new FontException('magicNumber must be 0x5f0f3cf5');
        }

        $this->offset += 4;
    }

    /**
     *  Parse Font Header Table (head) for BBox, units and flags
     *
     *  0 - uint16             majorVersion        Major version of font header table (always 1)
     *  2 - uint16             minorVersion        Major version of font header table (always 0)
     *  4 - Fixed (32-bit)     fontRevision        Set by font manufacturer (Fixed = 4 bytes)
     *  8 - uint32             checksumAdjustment
     * 12 - uint32             magicNumber         Always 0x5F0F3CF5
     * 16 - uint16             flags               @Link https://learn.microsoft.com/en-us/typography/opentype/spec/head
     * 18 - uint16             unitsPerEm          Any value from 16 to 16384 (a power of 2 is recommended)
     * 20 - LONGDATETIME       created             64-bit number of seconds since 12:00 midnight 1904/01/01 in GMT/UTC time zone.
     * 28 - LONGDATETIME       modified            64-bit number of seconds since 12:00 midnight 1904/01/01 in GMT/UTC time zone.
     * 36 - int16              xMin                Minimum x coordinate across all glyph bounding boxes.
     * 38 - int16              yMin                Minimum y coordinate across all glyph bounding boxes.
     * 40 - int16              xMax                Maximum x coordinate across all glyph bounding boxes.
     * 42 - int16              yMax                Maximum y coordinate across all glyph bounding boxes.
     * 44 - uint16             macStyle            bits (0:Bold, 1:Italic, 2:Underline, 3:Outline, 4:Shadow, 5:Condensed, 6:Extended, 7-15:(0) Reserved)
     * 46 - uint16             lowestRecPPEM       Smallest readable size in pixels.
     * 48 - int16              fontDirectionHint   Deprecated -- Set to 2
     * 50 - int16              indexToLocFormat    0 for short offsets (Offset16), 1 for long (Offset32).
     * 52 - int16              glyphDataFormat     0 for current format.
     */
    protected function getBbox(): void
    {
        $this->fdt['unitsPerEm'] = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        if ($this->fdt['unitsPerEm'] < 16 || $this->fdt['unitsPerEm'] > 16_384) {
            throw new FontException('unitsPerEm must be between 16 and 16384, got: ' . $this->fdt['unitsPerEm']);
        }

        // units ratio constant
        $this->fdt['urk'] = 1000 / $this->fdt['unitsPerEm'];
        // skip field: created: (LONGDATETIME int64)
        // skip field: modified: (LONGDATETIME int64)
        $this->offset += 16;
        $xMin = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        $yMin = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        $xMax = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        $yMax = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        $this->fdt['bbox'] = $xMin . ' ' . $yMin . ' ' . $xMax . ' ' . $yMax;
        $macStyle = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        // the 'head' table settles the italic bit either way, overriding the guess
        // Import::initFlags() makes from the file name
        $this->fdt['Flags'] &= ~64;
        if (($macStyle & 2) === 2) {
            // italic flag
            $this->fdt['Flags'] |= 64;
        }
    }

    /**
     * Map glyph indexes to their corresponding byte-offset in the glyf table data
     *
     * The loca table is an array of values mapping each glyph id to the glyph's symbol in the TTF glyf table.
     * These offsets will be stored using uint16-be values if the indexToLocFormat flag in the header table is 0 and
     * uint32-be values otherwise.
     */
    protected function getIndexToLoc(): void
    {
        // indexToLocFormat flag in the head table (indexToLocFormat : 0 = short, 1 = long)
        $this->offset = $this->fdt['table']['head']['offset'] + 50;
        $this->fdt['short_offset'] = $this->fbyte->getShort($this->offset) === 0;
        $this->offset += 2;
        // get the offsets to the locations of the glyphs in the font, relative to the beginning of the glyphData table
        $this->fdt['indexToLoc'] = [];
        $this->offset = $this->fdt['table']['loca']['offset'];
        // the loca table holds exactly numGlyphs + 1 entries, anything past that is padding
        $maxEntries = $this->fdt['numGlyphs'] + 1;
        // offsets are relative to the start of the glyf table, and one pointing past its end
        // is clamped to the end, which yields an empty glyph
        $glyfLength = $this->fdt['table']['glyf']['length'];
        // the short format stores halved offsets, so it can only address even ones
        if ($this->fdt['short_offset']) {
            $glyfLength &= ~1;
        }

        // Offset16 entries are stored halved, so the short format reads two bytes and
        // doubles them where the long one reads four and takes them as they are
        $stride = $this->fdt['short_offset'] ? 2 : 4;
        $this->fdt['tot_num_glyphs'] = (int) \min($maxEntries, \intdiv($this->fdt['table']['loca']['length'], $stride)); // numGlyphs + 1
        for ($idx = 0; $idx < $this->fdt['tot_num_glyphs']; ++$idx) {
            $entry = $this->fdt['short_offset']
                ? $this->fbyte->getUShort($this->offset) * 2
                : $this->fbyte->getULong($this->offset);
            $this->fdt['indexToLoc'][$idx] = \min($entry, $glyfLength);
            if (
                isset($this->fdt['indexToLoc'][$idx - 1])
                && $this->fdt['indexToLoc'][$idx] === $this->fdt['indexToLoc'][$idx - 1]
            ) {
                // the last glyph didn't have an outline
                unset($this->fdt['indexToLoc'][$idx - 1]);
            }

            $this->offset += $stride;
        }
    }

    /**
     * Map character encoding ids to the index of the matching glyph (TTF cmap table)
     *
     * cmap table header:
     *   - uint16   version            Table version number (Always 0)
     *   - uint16   numTables          Number of encoding tables
     *
     * EncodingRecord :
     *   - uint16   platformId         Platform ID
     *   - uint16   encodingId         Platform-specific encoding ID
     *   - Offset32 subtableOffset     Byte offset from beginning of cmap table to the encoding subtable
     */
    protected function getEncodingTables(): void
    {
        $this->offset = $this->fdt['table']['cmap']['offset'] + 2;
        $numEncodingTables = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        // bounded by the declared length of the cmap table: the records are 8 bytes each
        // and start after the 4-byte header
        $numEncodingTables = \min($numEncodingTables, \intdiv(\max(0, $this->fdt['table']['cmap']['length'] - 4), 8));
        $this->fdt['encodingTables'] = [];
        // every subtable format this class handles reads at least the 4 bytes of the format
        // and the length that follows it, so a record pointing past that is dropped
        $maxSubtableOffset = \max(0, $this->fdt['table']['cmap']['length'] - 4);
        $idx = 0;
        for ($rec = 0; $rec < $numEncodingTables; ++$rec) {
            $platformID = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $encodingID = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $subtableOffset = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            if ($subtableOffset > $maxSubtableOffset) {
                continue;
            }

            $this->fdt['encodingTables'][$idx] = [
                'platformID' => $platformID,
                'encodingID' => $encodingID,
                'offset' => $subtableOffset,
            ];
            ++$idx;
        }
    }

    /**
     * Returns the offset of the first byte past the cmap table, or past the font file when
     * the declared length of the table runs beyond it.
     *
     * Every subtable is addressed within the cmap table, so its entry count is bounded by
     * this offset.
     */
    protected function getCmapEnd(): int
    {
        return \min(
            $this->fdt['table']['cmap']['offset'] + $this->fdt['table']['cmap']['length'],
            \strlen($this->font),
        );
    }

    /**
     * Returns how many records of the given size still fit in the cmap table.
     *
     * The arrays of every subtable format are clamped with this.
     *
     * @param int $recordSize Size in bytes of one record of the array being read.
     */
    protected function getCmapCapacity(int $recordSize): int
    {
        return \intdiv(\max(0, $this->getCmapEnd() - $this->offset), $recordSize);
    }

    /**
     * Returns how many character codes a cmap subtable is allowed to map.
     *
     * The budget is derived from the glyph count of the loca table, never below
     * MIN_CMAP_ENTRIES and never above the number of Unicode code points.
     */
    protected function getCmapEntryBudget(): int
    {
        return \min(self::MAX_CMAP_ENTRIES, \max(
            self::MIN_CMAP_ENTRIES,
            self::MAX_CMAP_ENTRIES_PER_GLYPH * $this->fdt['tot_num_glyphs'],
        ));
    }

    /**
     * Get OS/2 and Windows Metrics Table (TTF OS/2 table)
     *
     * The OS/2 table consists of a set of metrics and other data that are required in OpenType fonts
     *
     * Six versions of the OS/2 table have been defined: versions 0 to 5. All versions are supported,
     * but use of version 4 or later is strongly recommended.
     * @link https://learn.microsoft.com/en-us/typography/opentype/spec/os2
     *
     * OS/2 Table Version 0 (FWORD is an int16 in font design units):
     *   0 - uint16       version         OS/2 table version (0-5)
     *   2 - FWORD        xAvgCharWidth
     *   4 - uint16       usWeightClass
     *   6 - uint16       usWidthClass
     *   8 - uint16       fsType
     *  10 - FWORD        ySubscriptXSize
     *  12 - FWORD        ySubscriptYSize
     *  14 - FWORD        ySubscriptXOffset
     *  16 - FWORD        ySubscriptYOffset
     *  18 - FWORD        ySuperscriptXSize
     *  20 - FWORD        ySuperscriptYSize
     *  22 - FWORD        ySuperscriptXOffset
     *  24 - FWORD        ySuperscriptYOffset
     *  26 - FWORD        yStrikeoutSize
     *  28 - FWORD        yStrikeoutPosition
     *  30 - int16        sFamilyClass
     *  32 - uint8[10]    panose              (@Link https://learn.microsoft.com/en-us/typography/opentype/spec/os2#pan)
     *  34 - uint32       ulUnicodeRange1     Unicode Character Range 1
     *  38 - uint32       ulUnicodeRange2     Unicode Character Range 2
     *  42 - uint32       ulUnicodeRange3     Unicode Character Range 3
     *  46 - uint32       ulUnicodeRange4     Unicode Character Range 4
     *  50 - uint8[4]     tag                 4 * ascii (range from 0x20 tp 0x7E) right padded with 0x20 (space) if len < 4
     *  54 - uint16       fsSelection
     *  56 - uint16       usFirstCharIndex
     *  58 - uint16       usLastCharIndex
     *  60 - FWORD        sTypoAscender
     *  62 - FWORD        sTypoDescender
     *  64 - FWORD        sTypoLineGap
     *  66 - UFWORD       usWinAscent
     *  68 - UFWORD       usWinDescent
     *
     * @throws FontException
     */
    protected function getOS2Metrics(): void
    {
        // the OS/2 table is optional, and its metrics have a default
        if (!isset($this->fdt['table']['OS/2'])) {
            $this->fdt['AvgWidth'] = 0;
            $this->fdt['StemV'] = 70;
            $this->fdt['StemH'] = 30;
            return;
        }

        if ($this->fdt['table']['OS/2']['length'] < self::OS2_MIN_LENGTH) {
            throw new FontException(
                'OS/2 table is too short: expected at least '
                . self::OS2_MIN_LENGTH
                . ' bytes, got '
                . $this->fdt['table']['OS/2']['length']
                . ' bytes.',
            );
        }

        $this->offset = $this->fdt['table']['OS/2']['offset'];
        $this->offset += 2; // skip version
        // xAvgCharWidth, read as signed and floored at zero
        $this->fdt['AvgWidth'] = \max(0, (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']));
        $this->offset += 2;
        // usWeightClass is a weight class in the 1..1000 range, not a value in font design
        // units, so it is not scaled by the units ratio
        $usWeightClass = \min(1000, \max(1, $this->fbyte->getUShort($this->offset)));
        // estimate StemV and StemH, floored at one (400 = usWeightClass of a Regular font)
        $this->fdt['StemV'] = \max(1, (int) \round((70 * $usWeightClass) / 400));
        $this->fdt['StemH'] = \max(1, (int) \round((30 * $usWeightClass) / 400));
        $this->offset += 2;
        $this->offset += 2; // usWidthClass
        $fsType = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        $this->applyEmbeddingPolicy($fsType);
    }

    /**
     * Apply OS/2 fsType embedding-restrictions policy.
     *
     * fsType bits (OpenType spec §OS/2.fsType):
     *   bit 1  (0x0002) = Restricted License embedding - cannot embed in any PDF.
     *   bit 2  (0x0004) = Preview & Print embedding    - allowed.
     *   bit 3  (0x0008) = Editable embedding           - allowed.
     *   bit 8  (0x0100) = No Subsetting                - embedding allowed, subsetting must be off.
     *   bit 9  (0x0200) = Bitmap Embedding Only        - vector PDF embedding not permitted.
     *
     * When only the Restricted-License bit is set among bits 1-3 the font cannot be embedded.
     * A permissive bit (0x0004 or 0x0008) alongside 0x0002 takes precedence (spec §5.8.1).
     *
     * @throws FontException if the font's license does not permit embedding.
     */
    protected function applyEmbeddingPolicy(int $fsType): void
    {
        // restricted license: bit 1 set, no permissive override from bits 2 or 3
        if (($fsType & 0x000E) === 0x0002) {
            throw new FontException(
                'This Font cannot be modified, embedded or exchanged in any manner'
                . ' without first obtaining permission of the legal owner.',
            );
        }

        // bitmap embedding only: incompatible with vector PDF stream embedding
        if (($fsType & 0x0200) !== 0) {
            throw new FontException('This font is licensed for bitmap embedding only'
            . ' and cannot be embedded in a vector PDF document.');
        }

        // no subsetting: embedding is allowed but the whole program must be embedded
        if (($fsType & 0x0100) !== 0) {
            $this->subsettingAllowed = false;
            $this->fdt['subset'] = false;
        }
    }

    /**
     * Convert string encoding based on the platformId and encodingId using the mb_convert_encoding
     * or iconv functions if they are available.
     *
     * @param string $str        The encoded string from the TTF NameRecord to convert
     * @param int    $platformId The platformId from the TTF NameRecord
     * @param int    $encodingId The encodingId from the TTF NameRecord
     *
     * @return string The string converted to UTF-8, or the original string when the
     *                conversion fails or is not available.
     */
    protected function convertStringEncoding(string $str, int $platformId, int $encodingId): string
    {
        $original = $str;

        if ($platformId === 1) {
            // Legacy Macintosh platform uses 'MacRoman' encoding which is not available in PHP mbstring.
            // Convert with iconv (macintosh = MacRoman) if available or mb_convert_encoding using
            // Windows-1252 (closest substitute for MacRoman) if available.
            if ($this->hasIconv()) {
                // the E_WARNING a build without the 'macintosh' charset emits is silenced,
                // and the false it returns falls back to the original string below
                \set_error_handler(static fn(): bool => true, E_WARNING);

                try {
                    $str = \iconv('macintosh', 'UTF-8', $str);
                } finally {
                    \restore_error_handler();
                }
            } elseif ($this->hasMbstring()) {
                $str = \mb_convert_encoding($str, 'UTF-8', 'Windows-1252');
            }
        } elseif ($this->hasMbstring()) {
            // All Unicode (platformId=0) strings are UTF-16BE
            $stringEncoding = 'UTF-16BE';

            if ($platformId === 2) {
                // the deprecated ISO platform stores single-byte ASCII/ISO 8859-1 strings
                $stringEncoding = 'ISO-8859-1';
            } elseif ($platformId === 3) {
                // Windows platform uses specific string encodings for encodingIds 2, 3, 4 and 5
                // (encodingId 6, Johab, has no mbstring counterpart and stays UTF-16BE)
                $stringEncoding = match ($encodingId) {
                    2 => 'CP932',
                    3 => 'CP936',
                    4 => 'CP950',
                    5 => 'CP949',
                    default => 'UTF-16BE',
                };
            }

            $str = \mb_convert_encoding($str, 'UTF-8', $stringEncoding);
        }

        return is_string($str) ? $str : $original;
    }

    /**
     * Returns true when the iconv extension is available.
     */
    protected function hasIconv(): bool
    {
        return \function_exists('iconv');
    }

    /**
     * Returns true when the mbstring extension is available.
     */
    protected function hasMbstring(): bool
    {
        return \function_exists('mb_convert_encoding');
    }

    /**
     * Get the font name (TTF name table)
     *
     * NameTable Version 0:
     *  0 - uint16            version            Table version number (0; would be 1 for Version 1)
     *  2 - uint16            count              Number of name records
     *  4 - Offset16          storageOffset      Offset to start of string storage (from start of name table)
     *  6 - NameRecord[count] nameRecords        The NameRecords
     *
     * NameRecord (12 bytes):
     *  0 - uint16   platformId         Platform ID
     *  2 - uint16   encodingId         Platform-specific encoding ID
     *  4 - uint16   languageId         Language ID
     *  6 - uint16   nameId             Name ID (See list below in function body)
     *  8 - uint16   length             String length (in bytes)
     * 10 - Offset16 stringOffset       String offset from start of storage area (in bytes)
     *
     * @throws FontException
     */
    protected function getFontName(): void
    {
        $this->fdt['name'] = '';
        $priority = 0;
        $this->offset = $this->fdt['table']['name']['offset'];
        $this->offset += 2; // skip Format selector (=0).
        // Number of NameRecords that follow n.
        $numNameRecords = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        // bounded by the declared length of the table: the records are 12 bytes each and
        // start after the 6-byte header
        $numNameRecords = \min($numNameRecords, \intdiv(\max(0, $this->fdt['table']['name']['length'] - 6), 12));

        // Offset to start of string storage (from start of table).
        $stringStorageOffset = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        for ($idx = 0; $idx < $numNameRecords; ++$idx) {
            $platformId = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $encodingId = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $this->offset += 2; // Skip languageId.

            /**
             * List of standard Name IDs:
             *  -  0: Copyright notice
             *  -  1: Font Family Name
             *  -  2: Font Subfamily Name
             *  -  3: Unique font identifier
             *  -  4: Full font name reflecting all family and relevant subfamily descriptors
             *  -  5: Version string beginning with "Version <number>.<number>" case-insensitive
             *  -  6: Postscript name for the font.
             *  -  7: Trademark
             *  -  8: Manufacturer Name
             *  -  9: Designer Name
             *  - 10: Description
             *  - 11: URL of Vendor
             *  - 12: URL of Designer
             *  - 13: License Description (can be very long and will be dropped in subsetting)
             *  - 14: License Info URL
             *  ...
             *  - 25: Variations PostScript Name Prefix
             */
            $nameID = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            if (!isset(self::NAME_ID_PRIORITY[$nameID])) {
                $this->offset += 4; // skip String length, String offset
                continue;
            }

            // String length (in bytes).
            $stringLength = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            // String offset from start of storage area (in bytes).
            $stringOffset = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $recordOffset = $this->offset;

            $this->offset = $this->fdt['table']['name']['offset'] + $stringStorageOffset + $stringOffset;
            // a record whose string is not entirely inside the name table describes no name
            $name = ($stringStorageOffset + $stringOffset + $stringLength) > $this->fdt['table']['name']['length']
                ? ''
                : \substr($this->font, $this->offset, $stringLength);
            // Convert the string encoding if possible
            $name = $this->convertStringEncoding($name, $platformId, $encodingId);
            $name = (string) \preg_replace('/[^a-zA-Z0-9_\-]/', '', $name);

            // nameID 6 is the PostScript name and wins outright
            if ($nameID === 6 && $name !== '') {
                $this->fdt['name'] = $name;
                return;
            }

            // records are ordered by nameID, so a better fallback may still follow
            if ($name !== '' && self::NAME_ID_PRIORITY[$nameID] > $priority) {
                $priority = self::NAME_ID_PRIORITY[$nameID];
                $this->fdt['name'] = $name;
            }

            $this->offset = $recordOffset;
        }

        if ($this->fdt['name'] === '') {
            throw new FontException('Error getting font name.');
        }
    }

    /**
     * Get the PostScript Table (TTF post table)
     */
    protected function getPostData(): void
    {
        $this->offset = $this->fdt['table']['post']['offset'];
        $this->offset += 4; // skip Format Type
        $this->fdt['italicAngle'] = $this->fbyte->getFixed($this->offset);
        $this->offset += 4;
        $this->fdt['underlinePosition'] = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        $this->fdt['underlineThickness'] = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        $isFixedPitch = $this->fbyte->getULong($this->offset) !== 0;
        $this->offset += 4; // isFixedPitch is a uint32
        // the 'post' table settles the fixed pitch bit either way, overriding the guess
        // Import::initFlags() makes from the file name
        $this->fdt['Flags'] &= ~1;
        if ($isFixedPitch) {
            $this->fdt['Flags'] |= 1;
        }

        if ($this->fdt['italicAngle'] !== 0.0) {
            // a slanted font is italic whatever 'head.macStyle' claims
            $this->fdt['Flags'] |= 64;
        }
    }

    /**
     * Get the Horizontal Header Table (TTF hhea table)
     *
     *  0 - uint16      majorVersion                     hhea Major version
     *  2 - uint16      minorVersion                     hhea Minor version
     *  4 - FWORD       ascender
     *  6 - FWORD       descender
     *  8 - FWORD       lineGap
     * 10 - UFWORD      advanceWidthMax
     * 12 - FWORD       minLeftSideBearing
     * 14 - FWORD       minRightSideBearing
     * 16 - FWORD       xMaxExtent
     * 18 - int16       caretSlopeRise
     * 20 - int16       caretSlopeRun
     * 22 - int16       caretOffset
     * 24 - int16       reserved (set to 0)
     * 26 - int16       reserved (set to 0)
     * 28 - int16       reserved (set to 0)
     * 30 - int16       reserved (set to 0)
     * 32 - int16       metricDataFormat (set to 0)
     * 34 - uint16      numberOfHMetrics (in hmtx table)
     */
    protected function getHheaData(): void
    {
        // ---------- get hhea data ----------
        $this->offset = $this->fdt['table']['hhea']['offset'];
        $this->offset += 4; // skip Table version number
        // Ascender
        $this->fdt['Ascent'] = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        // Descender
        $this->fdt['Descent'] = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        // LineGap
        $this->fdt['Leading'] = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        // advanceWidthMax
        $this->fdt['MaxWidth'] = (int) \round($this->fbyte->getUFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;

        // skip several fields...
        $this->offset += 22;

        // get the number of hMetric entries in hmtx table
        $this->fdt['numHMetrics'] = $this->fbyte->getUShort($this->offset);
    }

    /**
     * Get the Maximum Profile Table (TTF maxp table)
     */
    protected function getMaxpData(): void
    {
        $this->offset = $this->fdt['table']['maxp']['offset'];

        // Skip the Table version number (Version16Dot16 = 4 bytes).
        $this->offset += 4;

        // get the number of glyphs in the font.
        $this->fdt['numGlyphs'] = $this->fbyte->getUShort($this->offset);
    }

    /**
     * Get font heights
     */
    protected function getHeights(): void
    {
        // get xHeight (height of x)
        $this->fdt['XHeight'] = $this->fdt['Ascent'] + $this->fdt['Descent'];
        $xheight = $this->getGlyphHeight(120);
        if ($xheight !== null) {
            $this->fdt['XHeight'] = $xheight;
        }

        // get CapHeight (height of H)
        $this->fdt['CapHeight'] = (int) $this->fdt['Ascent'];
        $capheight = $this->getGlyphHeight(72);
        if ($capheight !== null) {
            $this->fdt['CapHeight'] = $capheight;
        }
    }

    /**
     * Returns the height of the glyph of a character, or null when it has no outline.
     *
     * @param int $cid Character identifier ('x' or 'H').
     */
    private function getGlyphHeight(int $cid): ?int
    {
        $gid = $this->fdt['ctgdata'][$cid] ?? 0;
        if ($gid === 0) {
            return null;
        }

        // glyphs without an outline are removed from indexToLoc (see getIndexToLoc)
        $offset = $this->getGlyphHeaderOffset($gid);
        if ($offset === null) {
            return null;
        }

        // ISO 32000-1 Table 122 defines /XHeight and /CapHeight as the height of the glyph
        // above the baseline, so the height is yMax alone
        $yMax = $this->fbyte->getFWord($offset + 8);
        return (int) \round($yMax * $this->fdt['urk']);
    }

    /**
     * Returns the absolute offset of a glyph header, or null when it is not readable.
     *
     * The 10-byte header (numberOfContours and the four bounding box values) must lie
     * inside the glyf table.
     *
     * @param int $gid Glyph index.
     */
    private function getGlyphHeaderOffset(int $gid): ?int
    {
        $loc = $this->fdt['indexToLoc'][$gid] ?? null;
        if ($loc === null || ($loc + 10) > $this->fdt['table']['glyf']['length']) {
            return null;
        }

        return $this->fdt['table']['glyf']['offset'] + $loc;
    }

    /**
     * Get font widths
     */
    protected function getWidths(): void
    {
        if ($this->fdt['numHMetrics'] < 1) {
            throw new FontException('hhea.numberOfHMetrics must be greater than zero');
        }

        // bounded by the declared length of the hmtx table: each metric is 4 bytes
        $this->fdt['numHMetrics'] = \min($this->fdt['numHMetrics'], \intdiv($this->fdt['table']['hmtx']['length'], 4));
        if ($this->fdt['numHMetrics'] < 1) {
            throw new FontException('the hmtx table is too short to hold a single metric');
        }

        // create widths array
        $chw = [];
        $this->offset = $this->fdt['table']['hmtx']['offset'];
        for ($i = 0; $i < $this->fdt['numHMetrics']; ++$i) {
            $chw[$i] = (int) \round($this->fbyte->getUFWord($this->offset) * $this->fdt['urk']);
            $this->offset += 4; // skip lsb
        }

        if ($this->fdt['numHMetrics'] < $this->fdt['numGlyphs']) {
            // fill missing widths with the last value
            $chw = \array_pad($chw, $this->fdt['numGlyphs'], $chw[$this->fdt['numHMetrics'] - 1]);
        }

        $this->fdt['MissingWidth'] = $chw[0] ?? 0;
        $this->fdt['cw'] = [];
        $this->fdt['cbbox'] = [];
        // ctgdata is built in ascending CID order, so cw and cbbox are filled in the same order
        foreach ($this->fdt['ctgdata'] as $cid => $gid) {
            if ($gid === 0) {
                // a codepoint mapped to .notdef gets no width; the ctgdata entry is kept
                // as a faithful copy of the cmap
                continue;
            }

            if (isset($chw[$gid])) {
                $this->fdt['cw'][$cid] = $chw[$gid];
            }

            $offset = $this->withCbbox ? $this->getGlyphHeaderOffset($gid) : null;
            if ($offset !== null) {
                $xMin = (int) \round($this->fbyte->getFWord($offset + 2) * $this->fdt['urk']);
                $yMin = (int) \round($this->fbyte->getFWord($offset + 4) * $this->fdt['urk']);
                $xMax = (int) \round($this->fbyte->getFWord($offset + 6) * $this->fdt['urk']);
                $yMax = (int) \round($this->fbyte->getFWord($offset + 8) * $this->fdt['urk']);
                $this->fdt['cbbox'][$cid] = [$xMin, $yMin, $xMax, $yMax];
            }
        }
    }

    /**
     * Add CTG entry to map CID to GID
     *
     * @param int  $cid Character Identifier
     * @param int  $gid Glyph ID (zero-based index of the glyph in the font's glyph collection)
     */
    protected function addCtgItem(int $cid, int $gid): void
    {
        $this->fdt['ctgdata'][$cid] = $gid;
        if (isset($this->subchars[$cid])) {
            $this->subglyphs[$gid] = true;
        }
    }

    /**
     * Find the first encoding table record matching the given platformID and encodingID.
     *
     * @return array{platformID: int, encodingID: int, offset: int}|null
     */
    private function findTableEntry(int $platformID, int $encodingID): ?array
    {
        foreach ($this->fdt['encodingTables'] as $enctable) {
            if ($enctable['platformID'] === $platformID && $enctable['encodingID'] === $encodingID) {
                return $enctable;
            }
        }

        return null;
    }

    /**
     * Select the best available cmap encoding subtable.
     *
     * Selection priority:
     *   1. Exact match for the caller-requested (platform_id, encoding_id) pair.
     *   2. Fallback pairs in CMAP_FALLBACK_PRIORITY order.
     *
     * @return array{platformID: int, encodingID: int, offset: int}|null
     *         The chosen encoding-table record, or null when no usable subtable exists.
     */
    protected function selectEncodingTable(): ?array
    {
        // 1. Exact match
        $match = $this->findTableEntry($this->fdt['platform_id'], $this->fdt['encoding_id']);
        if ($match !== null) {
            return $match;
        }

        // 2. Priority fallbacks
        foreach (self::CMAP_FALLBACK_PRIORITY as [$pid, $eid]) {
            if ($pid === $this->fdt['platform_id'] && $eid === $this->fdt['encoding_id']) {
                continue; // already tried as exact match above
            }

            $match = $this->findTableEntry($pid, $eid);
            if ($match !== null) {
                return $match;
            }
        }

        return null;
    }

    /**
     * Process the font's cmap encoding table
     *
     * A Character Identifier (CID) is an integer matching the character code from a particular encoding.
     * @link https://www.php.net/mb_ord
     *
     * The Glyph ID (GID) is the zero-based index of the glyph in the font's glyph collection.
     *
     * @throws FontException
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getCIDToGIDMap(): void
    {
        $this->fdt['ctgdata'] = [];

        $enctable = $this->selectEncodingTable();
        if ($enctable === null) {
            throw new FontException(
                'No usable cmap subtable found for this font.'
                . ' Requested platformID='
                . $this->fdt['platform_id']
                . ' encodingID='
                . $this->fdt['encoding_id']
                . '. Available tables: '
                . \implode(', ', \array_map(
                    static fn(array $tbl): string => $tbl['platformID'] . '/' . $tbl['encodingID'],
                    $this->fdt['encodingTables'],
                ))
                . '.',
            );
        }

        $this->offset = $this->fdt['table']['cmap']['offset'] + $enctable['offset'];
        $format = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        match ($format) {
            0 => $this->processFormat0(),
            2 => $this->processFormat2(),
            4 => $this->processFormat4(),
            6 => $this->processFormat6(),
            8 => $this->processFormat8(),
            10 => $this->processFormat10(),
            12 => $this->processFormat12(),
            13 => $this->processFormat13(),
            14 => throw new FontException(
                'cmap format 14 is a supplementary Unicode Variation Sequences subtable'
                . ' and cannot be used as the character map of the font',
            ),
            default => throw new FontException('Unsupported cmap format: ' . $format),
        };

        // Glyph 0 is the .notdef glyph used when the font does not contain a glyph for a character
        if (!isset($this->fdt['ctgdata'][0])) {
            $this->fdt['ctgdata'][0] = 0;
        }

        if ($this->fdt['type'] !== 'TrueTypeUnicode') {
            return;
        }

        if (\count($this->fdt['ctgdata']) !== 256) {
            return;
        }

        // a byte encoded font is one whose character codes all fit a single byte
        if (\max(\array_keys($this->fdt['ctgdata'])) > 0xFF) {
            return;
        }

        $this->fdt['type'] = 'TrueType';
    }

    /**
     * Process Format 0: Byte encoding table
     *  0 - uint16      format              (unused) Always 0 for subtable format 0
     *  2 - uint16      length              (unused) The length of the subtable in bytes
     *  4 - uint16      language            (unused)
     *  6 - unit8[256]  glyphIdArray        An array that maps character codes to glyph index values.
     */
    protected function processFormat0(): void
    {
        $this->offset += 4; // skip length and version/language
        // a subtable holding fewer than the 256 documented bytes stops where it ends
        $entries = \min(256, $this->getCmapCapacity(1));
        for ($chr = 0; $chr < $entries; ++$chr) {
            $gid = $this->fbyte->getByte($this->offset);
            $this->addCtgItem($chr, $gid);
            ++$this->offset;
        }
    }

    /**
     * Process Format 2: High-byte mapping through table
     *   0 - uint16          format         (unused) Always 2 for subtable format 2
     *   2 - uint16          length         (unused) The length of the subtable in bytes
     *   4 - uint16          language       (unused)
     *   6 - uint16[256]     subHeaderKeys  Array mapping high bytes into the subHeaders array: value is subHeaders index × 8
     * 518 - SubHeader[]     subHeaders     Array of SubHeader records
     *     - unit16[]        glyphIdArray   Array containing sub-arrays used for mapping the low byte of 2-byte character
     *
     * SubHeader Record (8 bytes):
     *   0 - uint16          firstCode      First valid low byte for this SubHeader
     *   2 - uint16          entryCount     Number of valid low bytes for this SubHeader
     *   4 - int16           idDelta
     *   6 - unit16          idRangeOffset
     */
    protected function processFormat2(): void
    {
        $this->offset += 4; // skip length and version/language
        $subHeaderKeys = [];
        $numSubHeaders = 0;
        // the array of high byte keys is a fixed 256 entries, and stops at the end of the
        // cmap table like every other array of this subtable
        $numKeys = \min(256, $this->getCmapCapacity(2));
        for ($chr = 0; $chr < $numKeys; ++$chr) {
            // the stored value is the subHeader index * 8
            $subHeaderKeys[$chr] = \intdiv($this->fbyte->getUShort($this->offset), 8);
            $this->offset += 2;
            if ($numSubHeaders < $subHeaderKeys[$chr]) {
                $numSubHeaders = $subHeaderKeys[$chr];
            }
        }

        // the number of subHeaders is equal to the max of subHeaderKeys + 1
        ++$numSubHeaders;
        // each record is 8 bytes, and the array stops at the end of the cmap table; the high
        // bytes keyed to a sub-header the table has no room for map no glyph
        $numSubHeaders = \min($numSubHeaders, $this->getCmapCapacity(8));
        // keyed by index rather than as a list, as the lookup below indexes it with a value
        // read from the font file
        /** @var array<int, array{firstCode: int, entryCount: int, idDelta: int, idRangeOffset: int}> $subHeaders */
        $subHeaders = [];
        $numGlyphIndexArray = 0;
        for ($ish = 0; $ish < $numSubHeaders; ++$ish) {
            $firstCode = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $entryCount = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $idDelta = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $idRangeOffset = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            // the stored offset is relative to its own field, so rebase it on the start of
            // the glyph index array and convert it from bytes to entries
            $idRangeOffset -= 2 + (($numSubHeaders - $ish - 1) * 8);
            $subHeaders[$ish] = [
                'firstCode' => $firstCode,
                'entryCount' => $entryCount,
                'idDelta' => $idDelta,
                'idRangeOffset' => \intdiv($idRangeOffset, 2),
            ];
            $numGlyphIndexArray += $entryCount;
        }

        // this is the size of the shared glyph index array, not a number of mapped codes, so
        // it is bounded by the absolute ceiling rather than by the entry budget of the font
        if ($numGlyphIndexArray > self::MAX_CMAP_ENTRIES) {
            throw new FontException('cmap format 2 subtable declares too many glyph indexes');
        }

        // sub-headers may address overlapping ranges of the shared glyph index array, so the
        // sum of their entry counts over-estimates its real size
        $numGlyphIndexArray = \min($numGlyphIndexArray, $this->getCmapCapacity(2));

        $glyphIndexArray = [];
        for ($gid = 0; $gid < $numGlyphIndexArray; ++$gid) {
            $glyphIndexArray[$gid] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        for ($chr = 0; $chr < $numKeys; ++$chr) {
            $shk = $subHeaderKeys[$chr];
            $subHeader = $subHeaders[$shk] ?? null;
            if ($subHeader === null) {
                continue;
            }

            if ($shk === 0) {
                // one byte code: subHeaders[0] describes the single-byte range and the
                // code is its own low byte
                $this->addCtgItem($chr, $this->getFormat2Glyph($subHeader, $chr, $glyphIndexArray));
                continue;
            }

            // two bytes code: the range is a run of low bytes, so it stops at 256
            $start_byte = \min($subHeader['firstCode'], 256);
            $end_byte = \min($start_byte + $subHeader['entryCount'], 256);
            for ($jdx = $start_byte; $jdx < $end_byte; ++$jdx) {
                // combine high and low bytes
                $cdx = ($chr << 8) + $jdx;
                $this->addCtgItem($cdx, $this->getFormat2Glyph($subHeader, $jdx, $glyphIndexArray));
            }
        }
    }

    /**
     * Resolve the glyph index of a cmap format 2 low byte through its sub-header.
     *
     * @param array{firstCode: int, entryCount: int, idDelta: int, idRangeOffset: int} $subHeader Sub-header record.
     * @param int              $low             Low byte of the character code.
     * @param array<int, int>  $glyphIndexArray Glyph index array of the subtable.
     */
    private function getFormat2Glyph(array $subHeader, int $low, array $glyphIndexArray): int
    {
        $entry = $low - $subHeader['firstCode'];
        if ($entry < 0 || $entry >= $subHeader['entryCount']) {
            // the code is outside the range covered by this sub-header
            return 0;
        }

        // idRangeOffset comes from the font file, so an entry outside the glyph index
        // array falls back to notdef
        $glyphIndex = $glyphIndexArray[$subHeader['idRangeOffset'] + $entry] ?? 0;
        // a zero entry encodes missingGlyph and must not be shifted by idDelta
        return $glyphIndex === 0 ? 0 : ($glyphIndex + $subHeader['idDelta']) % 65_536;
    }

    /**
     * Process Format 4: Segment mapping to delta values
     *   0            - uint16              format         (unused) Always 4 for subtable format 4
     *   2            - uint16              length         The length of the subtable in bytes
     *   4            - uint16              language       (unused)
     *   6            - uint16              segCountX2     2 × segCount
     *   8            - uint16              searchRange    pow(2, floor(log2(segCount))) * 2 OR 1 << (entrySelector+1)
     *  10            - uint16              entrySelector  floor(log2(segCount)))
     *  12            - uint16              rangeShift     segCount * 2 - searchRange
     *  14            - unit16[segCount]    endCode        End characterCode for each segment; last segment = 0xFFFF
     *  14+2*segCount - uint16              reservedPad    Always 0
     *  16+2*segCount - uint16[segCount]    startCode      Start characterCode for each segment; last segment = 0xFFFF
     *  16+4*segCount - int16[segCount]     idDelta        Delta for all character codes in segment
     *  16+6*segCount - uint16[segCount]    idRangeOffset  Offsets into glyphIdArray or 0
     *  16+8*segCount - uint16[]            glyphIdArray   Glyph index array (arbitrary length)
     */
    protected function processFormat4(): void
    {
        $length = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        $this->offset += 2; // skip version/language
        $segCount = \intdiv($this->fbyte->getUShort($this->offset), 2);
        $this->offset += 2;
        $this->offset += 6; // skip searchRange, entrySelector, rangeShift
        // the four parallel arrays that follow hold one uint16 per segment each, plus the
        // reserved pad between the first two
        $segCount = \min($segCount, \intdiv(\max(0, $this->getCmapCapacity(2) - 1), 4));
        $endCount = []; // array of end character codes for each segment
        for ($kdx = 0; $kdx < $segCount; ++$kdx) {
            $endCount[$kdx] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        $this->offset += 2; // skip reservedPad
        $startCount = []; // array of start character codes for each segment
        for ($kdx = 0; $kdx < $segCount; ++$kdx) {
            $startCount[$kdx] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        $idDelta = []; // delta for all character codes in segment
        for ($kdx = 0; $kdx < $segCount; ++$kdx) {
            $idDelta[$kdx] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        $idRangeOffset = []; // Offsets into glyphIdArray or 0
        for ($kdx = 0; $kdx < $segCount; ++$kdx) {
            $idRangeOffset[$kdx] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        // the size is derived from the declared subtable length, and the array also stops at
        // the end of the cmap table; a segment indexing past it resolves to notdef
        $gidlen = \min(\max(0, \intdiv($length, 2) - 8 - (4 * $segCount)), $this->getCmapCapacity(2));
        $glyphIdArray = []; // glyph index array
        for ($kdx = 0; $kdx < $gidlen; ++$kdx) {
            $glyphIdArray[$kdx] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        $budget = $this->getCmapEntryBudget();
        for ($kdx = 0; $kdx < $segCount; ++$kdx) {
            // the mandatory 0xFFFF -> 0xFFFF segment closes the table and maps nothing,
            // so it is not charged to the budget
            if ($startCount[$kdx] !== 0xFFFF || $endCount[$kdx] !== 0xFFFF) {
                $budget -= \max(0, $endCount[$kdx] - $startCount[$kdx] + 1);
                if ($budget < 0) {
                    throw new FontException('cmap format 4 subtable maps too many code points');
                }
            }

            for ($chr = $startCount[$kdx]; $chr <= $endCount[$kdx]; ++$chr) {
                if ($idRangeOffset[$kdx] === 0) {
                    $gid = ($idDelta[$kdx] + $chr) % 65_536;
                } else {
                    $gid = \intdiv($idRangeOffset[$kdx], 2) + ($chr - $startCount[$kdx]) - ($segCount - $kdx);
                    // idRangeOffset comes from the font file, so an entry outside the glyph
                    // index array falls back to notdef
                    $glyphIndex = $glyphIdArray[$gid] ?? 0;
                    // a zero entry encodes missingGlyph and must not be shifted by idDelta
                    $gid = $glyphIndex === 0 ? 0 : ($glyphIndex + $idDelta[$kdx]) % 65_536;
                }

                if ($chr === 0xFFFF && $gid === 0) {
                    // the terminating segment closes the table, it does not map U+FFFF
                    continue;
                }

                $this->addCtgItem($chr, $gid);
            }
        }
    }

    /**
     * Process Format 6: Trimmed table mapping
     *   0 - uint16               format         (unused) Always 6 for subtable format 6
     *   2 - uint16               length         (unused) The length of the subtable in bytes
     *   4 - uint16               language       (unused)
     *   6 - uint16               firstCode      First character code of subrange
     *   8 - uint16               entryCount     Number of character codes in subrange
     *  10 - uint16[entryCount]   glyphIdArray   Array of glyph index values for character codes in the range
     */
    protected function processFormat6(): void
    {
        $this->offset += 4; // skip length and version/language
        $firstCode = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        $entryCount = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        // the subrange stops at the end of the cmap table
        $entryCount = \min($entryCount, $this->getCmapCapacity(2));
        for ($kdx = 0; $kdx < $entryCount; ++$kdx) {
            $chr = $kdx + $firstCode;
            $gid = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            if ($chr > 0xFFFF) {
                // format 6 addresses its subrange with uint16 codes
                continue;
            }

            $this->addCtgItem($chr, $gid);
        }
    }

    /**
     * Process Format 8: Mixed 16-bit and 32-bit coverage
     *  0      - uint16                format         (unused) Always 8 for subtable format 8
     *  2      - uint16                reserved       (unused) Always 0
     *  4      - uint32                length         (unused) The length of the subtable in bytes
     *  8      - uint32                language       (unused)
     * 12      - uint8[8192]           is32           Bit array indicating a value is the start of a 32-bit character code
     * 12+8192 - uint32                numGroups      Number of groupings which follow
     * 16+8192 - MapGroup[numGroups]   glyphIdArray   Array of glyph index values for character codes in the range
     *
     * SequentialMapGroup Record (12 bytes):
     *  0      - uint32                startCharCode  First character code in this group (high byte set to \0 if ia32=0)
     *  4      - uint32                endCharCode    Last character code in this group (high byte set to \0 if ia32=0)
     *  8      - uint32                startGlyphID   Glyph index corresponding to the starting character code
     */
    protected function processFormat8(): void
    {
        $this->offset += 10; // skip reserved, length and version/language
        // the is32 bit array only tells whether a group's bounds were written as 16 or 32 bit
        // values, which does not change them, so it is skipped; its last byte is read to
        // reject a subtable truncated inside it
        $this->fbyte->getByte($this->offset + 8191);
        $this->offset += 8192;

        $nGroups = $this->fbyte->getULong($this->offset);
        $this->offset += 4;
        // each record is 12 bytes, and the array stops at the end of the cmap table
        $nGroups = \min($nGroups, $this->getCmapCapacity(12));
        $budget = $this->getCmapEntryBudget();
        for ($idx = 0; $idx < $nGroups; ++$idx) {
            $startCharCode = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $endCharCode = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $startGlyphID = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            if ($startCharCode > self::MAX_UNICODE_CODEPOINT) {
                continue;
            }

            $endCharCode = \min($endCharCode, self::MAX_UNICODE_CODEPOINT);
            $budget -= \max(0, $endCharCode - $startCharCode + 1);
            if ($budget < 0) {
                throw new FontException('cmap format 8 subtable maps too many code points');
            }

            for ($cpw = $startCharCode; $cpw <= $endCharCode; ++$cpw) {
                $this->addCtgItem($cpw, $startGlyphID);
                ++$startGlyphID;
            }
        }
    }

    /**
     * Process Format 10: Trimmed array
     *   0 - uint16     format         (unused) Always 10 for subtable format 10
     *   2 - uint16     reserved       (unused) Always 0
     *   4 - uint32     length         (unused) The length of the subtable in bytes
     *   8 - uint32     language       (unused)
     *  12 - unit32     startCharCode  First character code covered
     *  16 - uint32     numChars       Number of character codes covered
     *  20 - uint16[]   glyphIdArray   Array of glyph index values for character codes in the range
     */
    protected function processFormat10(): void
    {
        $this->offset += 10; // skip reserved, length and version/language
        $startCharCode = $this->fbyte->getULong($this->offset);
        $this->offset += 4;
        $numChars = $this->fbyte->getULong($this->offset);
        $this->offset += 4;
        if ($numChars > $this->getCmapEntryBudget()) {
            throw new FontException('cmap format 10 subtable maps too many code points');
        }

        // the covered range stops at the end of the cmap table
        $numChars = \min($numChars, $this->getCmapCapacity(2));
        for ($kdx = 0; $kdx < $numChars; ++$kdx) {
            $chr = $kdx + $startCharCode;
            $gid = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            if ($chr > self::MAX_UNICODE_CODEPOINT) {
                continue;
            }

            $this->addCtgItem($chr, $gid);
        }
    }

    /**
     * Process Format 12: Segmented coverage
     *   0 - uint16                         format         (unused) Always 12 for subtable format 12
     *   2 - uint16                         reserved       (unused) Always 0
     *   4 - uint32                         length         (unused) The length of the subtable in bytes
     *   8 - uint32                         language       (unused)
     *  12 - uint32                         numGroups      Number of groupings which follow
     *  16 - SequentialMapGroup[numGroups]  groups         Array of SequentialMapGroup records
     *
     *  SequentialMapGroup Record (12 bytes):
     *   0 - uint32                         startCharCode  First character code in this group
     *   4 - uint32                         endCharCode    Last character code in this group
     *   8 - uint32                         startGlyphID   Glyph index corresponding to the starting character code
     */
    protected function processFormat12(): void
    {
        $this->offset += 10; // skip length and version/language
        $nGroups = $this->fbyte->getULong($this->offset);
        $this->offset += 4;
        // each record is 12 bytes, and the array stops at the end of the cmap table
        $nGroups = \min($nGroups, $this->getCmapCapacity(12));
        $budget = $this->getCmapEntryBudget();
        for ($kdx = 0; $kdx < $nGroups; ++$kdx) {
            $startCharCode = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $endCharCode = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $startGlyphCode = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            if ($startCharCode > self::MAX_UNICODE_CODEPOINT) {
                continue;
            }

            $endCharCode = \min($endCharCode, self::MAX_UNICODE_CODEPOINT);
            $budget -= \max(0, $endCharCode - $startCharCode + 1);
            if ($budget < 0) {
                throw new FontException('cmap format 12 subtable maps too many code points');
            }

            for ($chr = $startCharCode; $chr <= $endCharCode; ++$chr) {
                $this->addCtgItem($chr, $startGlyphCode);
                ++$startGlyphCode;
            }
        }
    }

    /**
     * Process Format 13: Many-to-one range mappings
     *   0 - uint16                        format         (unused) Always 13 for subtable format 13
     *   2 - uint16                        reserved       (unused) Always 0
     *   4 - uint32                        length         (unused) The length of the subtable in bytes
     *   8 - uint32                        language       (unused)
     *  12 - uint32                        numGroups      Number of groupings which follow
     *  16 - ConstantMapGroup[numGroups]   groups         Array of ConstantMapGroup records
     *
     *  ConstantMapGroup Record (12 bytes):
     *   0 - uint32                        startCharCode  First character code in this group
     *   4 - uint32                        endCharCode    Last character code in this group
     *   8 - uint32                        glyphID        Glyph index to be used for all characters in the group's range
     */
    protected function processFormat13(): void
    {
        $this->offset += 10; // skip reserved, length and language
        $nGroups = $this->fbyte->getULong($this->offset);
        $this->offset += 4;
        // each record is 12 bytes, and the array stops at the end of the cmap table
        $nGroups = \min($nGroups, $this->getCmapCapacity(12));
        $budget = $this->getCmapEntryBudget();
        for ($kdx = 0; $kdx < $nGroups; ++$kdx) {
            $startCharCode = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $endCharCode = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            $glyphID = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
            if ($startCharCode > self::MAX_UNICODE_CODEPOINT) {
                continue;
            }

            $endCharCode = \min($endCharCode, self::MAX_UNICODE_CODEPOINT);
            $budget -= \max(0, $endCharCode - $startCharCode + 1);
            if ($budget < 0) {
                throw new FontException('cmap format 13 subtable maps too many code points');
            }

            for ($chr = $startCharCode; $chr <= $endCharCode; ++$chr) {
                $this->addCtgItem($chr, $glyphID);
            }
        }
    }
}
