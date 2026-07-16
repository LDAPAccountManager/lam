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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font\Import;

use Com\Tecnick\File\Byte;
use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\Import\TrueType
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 * @SuppressWarnings("PHPMD.ExcessiveClassLength")
 */
class TrueType
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
     * Highest valid Unicode code point (U+10FFFF).
     *
     * Used to clamp attacker-controlled character ranges in the uint32-based cmap
     * subtable formats (8, 12 and 13) so a malformed font cannot drive a multi-billion
     * iteration loop or unbounded memory growth.
     */
    private const MAX_UNICODE_CODEPOINT = 0x10_FFFF;

    /**
     * Maximum number of character codes a single cmap subtable may map.
     *
     * A valid (non-overlapping) Unicode cmap cannot map more code points than exist,
     * so this acts as a hard upper bound that rejects abusive subtables.
     */
    private const MAX_CMAP_ENTRIES = 0x11_0000;

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
     * Process TrueType font
     *
     * @param string           $font       Content of the input font file
     * @param TFontData        $fdt        Extracted font metrics
     * @param ObjFile          $fileHelper File helper for font loading.
     * @param Byte             $fbyte      Object used to read font bytes
     * @param array<int, bool> $subchars   Array containing subset chars
     * @param bool             $withCbbox  Whether to compute per-glyph bounding boxes.
     *                                     The subsetting path does not use them, so it can
     *                                     skip the per-glyph glyf reads by passing false.
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
    ) {
        $this->fileHelper = $fileHelper;
        \ksort($subchars);
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
        $this->checkMagickNumber();
        $this->offset += 2; // skip flags
        $this->getBbox();
        $this->getIndexToLoc();
        $this->getEncodingTables();
        $this->getOS2Metrics();
        $this->getFontName();
        $this->getPostData();
        $this->getHheaData();
        $this->getMaxpData();
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
        if ($this->fdt['desc']['MaxWidth'] !== 0) {
            // subsetting mode
            $this->fdt['Flags'] = $this->fdt['desc']['Flags'];
            return;
        }

        if ($this->fdt['type'] === 'cidfont0') {
            return;
        }

        if ($this->fdt['linked']) {
            // creates a symbolic link to the existing font
            \symlink($this->fdt['input_file'], $this->fdt['dir'] . $this->fdt['file_name']);
            return;
        }

        // store compressed font
        $this->fdt['file'] = $this->fdt['file_name'] . '.z';
        $fpt = $this->fileHelper->fopenLocal($this->fdt['dir'] . $this->fdt['file'], 'wb');

        $cmpr = \gzcompress($this->font);
        if ($cmpr === false) {
            throw new FontException('Error compressing font file.');
        }

        \fwrite($fpt, $cmpr);
        \fclose($fpt);
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

        // Skip the searchRange, entrySelector and rangeShift fields (3 * uint16)
        $this->offset += 6;

        // tables array
        $this->fdt['table'] = [];
        // ---------- get tables ----------
        for ($idx = 0; $idx < $numTables; ++$idx) {
            // get table info
            $tag = \substr($this->font, $this->offset, 4);
            $this->offset += 4;
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
     * Ensure every table the parser later dereferences is present in the table directory.
     *
     * A truncated or malformed font that omits one of these would otherwise trigger an
     * undefined-array-key warning followed by an uncaught TypeError; converting that into
     * a controlled FontException keeps the documented exception contract.
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
     *  6 - Fixed (32-bit)     fontRevision        Set by font manufacturer (Fixed = 4 bytes)
     * 10 - uint32             checksumAdjustment
     * 14 - uint32             magicNumber         Always 0x5F0F3CF5
     * 16 - uint16             flags               @Link https://learn.microsoft.com/en-us/typography/opentype/spec/head
     * 18 - uint16             unitsPerEm          Any value from 16 to 16384 (a power of 2 is recommended)
     * 26 - LONGDATETIME       created             64-bit number of seconds since 12:00 midnight 1904/01/01 in GMT/UTC time zone.
     * 34 - LONGDATETIME       modified            64-bit number of seconds since 12:00 midnight 1904/01/01 in GMT/UTC time zone.
     * 36 - int16              xMin                Minimum x coordinate across all glyph bounding boxes.
     * 38 - int16              yMin                Minimum y coordinate across all glyph bounding boxes.
     * 40 - int16              xMax                Maximum x coordinate across all glyph bounding boxes.
     * 42 - int16              yMax                Maximum y coordinate across all glyph bounding boxes.
     * 44 - uint16             macStyle            bits (0:Bold, 1:Italic, 2:Underline, 3:Outline, 4:Shadow, 5:Condensed, 6:Extended, 7-15:(0) Reserved)
     * 46 - uint16             lowestRecPPEM       Smallest readable size in pixels.
     * 48 - int16              fontDirectionHint   Deprecated -- Set to 2
     * 50 - int16              indexToLocFormat    0 for short offsets (Offset16), 1 for long (Offset32).
     * 52 - int16              glyphDataFormat     0 for current format.
     *
     *  @return void
     */
    protected function getBbox(): void
    {
        $this->fdt['unitsPerEm'] = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
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
        // PDF font flags
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
        if ($this->fdt['short_offset']) {
            // The loca table uses data type Offset16 (uint16-be)
            $this->fdt['tot_num_glyphs'] = (int) \floor($this->fdt['table']['loca']['length'] / 2); // numGlyphs + 1
            for ($idx = 0; $idx < $this->fdt['tot_num_glyphs']; ++$idx) {
                $this->fdt['indexToLoc'][$idx] = $this->fbyte->getUShort($this->offset) * 2;
                if (
                    isset($this->fdt['indexToLoc'][$idx - 1])
                    && $this->fdt['indexToLoc'][$idx] === $this->fdt['indexToLoc'][$idx - 1]
                ) {
                    // the last glyph didn't have an outline
                    unset($this->fdt['indexToLoc'][$idx - 1]);
                }

                $this->offset += 2;
            }
        } else {
            // The loca table uses data type Offset32 (uint32-be)
            $this->fdt['tot_num_glyphs'] = (int) \floor($this->fdt['table']['loca']['length'] / 4); // numGlyphs + 1
            for ($idx = 0; $idx < $this->fdt['tot_num_glyphs']; ++$idx) {
                $this->fdt['indexToLoc'][$idx] = $this->fbyte->getULong($this->offset);
                if (
                    isset($this->fdt['indexToLoc'][$idx - 1])
                    && $this->fdt['indexToLoc'][$idx] === $this->fdt['indexToLoc'][$idx - 1]
                ) {
                    // the last glyph didn't have an outline
                    unset($this->fdt['indexToLoc'][$idx - 1]);
                }

                $this->offset += 4;
            }
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
     *
     * @return void
     */
    protected function getEncodingTables(): void
    {
        $this->offset = $this->fdt['table']['cmap']['offset'] + 2;
        $numEncodingTables = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;
        $this->fdt['encodingTables'] = [];
        for ($idx = 0; $idx < $numEncodingTables; ++$idx) {
            $this->fdt['encodingTables'][$idx]['platformID'] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $this->fdt['encodingTables'][$idx]['encodingID'] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $this->fdt['encodingTables'][$idx]['offset'] = $this->fbyte->getULong($this->offset);
            $this->offset += 4;
        }
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
     * @return void
     *
     * @throws FontException
     */
    protected function getOS2Metrics(): void
    {
        // OS/2 is optional in some older or non-Windows TrueType fonts.
        if (!isset($this->fdt['table']['OS/2'])) {
            // No OS/2 table present: use conservative metric defaults.
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
        // xAvgCharWidth
        $this->fdt['AvgWidth'] = (int) \round($this->fbyte->getFWord($this->offset) * $this->fdt['urk']);
        $this->offset += 2;
        // usWeightClass
        $usWeightClass = \round($this->fbyte->getUFWord($this->offset) * $this->fdt['urk']);
        // estimate StemV and StemH (400 = usWeightClass for Normal - Regular font)
        $this->fdt['StemV'] = (int) \round((70 * $usWeightClass) / 400);
        $this->fdt['StemH'] = (int) \round((30 * $usWeightClass) / 400);
        $this->offset += 2;
        $this->offset += 2; // usWidthClass
        $fsType = $this->fbyte->getShort($this->offset);
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
        // Restricted-license: bit 1 set, no permissive override from bits 2 or 3.
        if (($fsType & 0x000E) === 0x0002) {
            throw new FontException(
                'This Font cannot be modified, embedded or exchanged in any manner'
                . ' without first obtaining permission of the legal owner.',
            );
        }

        // Bitmap-embedding-only: incompatible with vector PDF stream embedding.
        if (($fsType & 0x0200) !== 0) {
            throw new FontException('This font is licensed for bitmap embedding only'
            . ' and cannot be embedded in a vector PDF document.');
        }

        // No-subsetting: embedding is allowed but the font must not be subsetted.
        if (($fsType & 0x0100) !== 0) {
            $this->fdt['subset'] = false;
        }
    }

    /**
     * Convert string encoding based on the platformId and encodingId using the mb_convert_encoding
     * or iconv functions if they are available.
     *
     * @param string    $str           The encoded string from the TTF NameRecord to convert
     * @param int       $platformId    The platformId from the TTF NameRecord
     * @param int       $encodingId    The encodingId from the TTF NameRecord
     *
     * @return string   Returns the string converted to UTF-8 or the original string if conversion
     *                  fails or is not  available.
     */
    protected function convertStringEncoding(string $str, int $platformId, int $encodingId): string
    {
        $original = $str;

        if ($platformId === 1) {
            // Legacy Macintosh platform uses 'MacRoman' encoding which is not available in PHP mbstring.
            // Convert with iconv (macintosh = MacRoman) if available or mb_convert_encoding using
            // Windows-1252 (closest substitute for MacRoman) if available.
            if (\function_exists('\iconv')) {
                $str = \iconv('macintosh', 'UTF-8', $str);
            } elseif (\function_exists('\mb_convert_encoding')) {
                $str = \mb_convert_encoding($str, 'UTF-8', 'Windows-1252');
            }
        } elseif (\function_exists('\mb_convert_encoding')) {
            // All Unicode (platformId=0) strings are UTF-16BE
            $stringEncoding = 'UTF-16BE';

            // Windows platform (platformId=3) uses specific string encodings for encodingIds 3, 4, and 5
            if ($platformId === 3) {
                $stringEncoding = match ($encodingId) {
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
     * @return void
     *
     * @throws FontException
     */
    protected function getFontName(): void
    {
        $this->fdt['name'] = '';
        $this->offset = $this->fdt['table']['name']['offset'];
        $this->offset += 2; // skip Format selector (=0).
        // Number of NameRecords that follow n.
        $numNameRecords = $this->fbyte->getUShort($this->offset);
        $this->offset += 2;

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
            if ($nameID === 6) {
                // String length (in bytes).
                $stringLength = $this->fbyte->getUShort($this->offset);
                $this->offset += 2;
                // String offset from start of storage area (in bytes).
                $stringOffset = $this->fbyte->getUShort($this->offset);
                $this->offset += 2;

                $this->offset = $this->fdt['table']['name']['offset'] + $stringStorageOffset + $stringOffset;
                // TTF encoded name string
                $name = \substr($this->font, $this->offset, $stringLength);
                // Convert the string encoding if possible
                $name = $this->convertStringEncoding($name, $platformId, $encodingId);

                $name = \preg_replace('/[^a-zA-Z0-9_\-]/', '', $name);
                if ($name === null || $name === '') {
                    throw new FontException('Error getting font name.');
                }

                $this->fdt['name'] = $name;
                break;
            } else {
                $this->offset += 4; // skip String length, String offset
            }
        }
    }

    /**
     * Get the PostScript Table (TTF post table)
     *
     * @return void
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
        $this->offset += 2;
        if ($isFixedPitch) {
            $this->fdt['Flags'] |= 1;
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
     *
     * @return void
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
     *
     * @return void
     */
    protected function getMaxpData(): void
    {
        $this->offset = $this->fdt['table']['maxp']['offset'];

        // Skip the Table version number (Version16Dot16 = 4 bytes).
        $this->offset += 4;

        // get the the number of glyphs in the font.
        $this->fdt['numGlyphs'] = $this->fbyte->getUShort($this->offset);
    }

    /**
     * Get font heights
     *
     * @return void
     */
    protected function getHeights(): void
    {
        // get xHeight (height of x)
        $this->fdt['XHeight'] = $this->fdt['Ascent'] + $this->fdt['Descent'];
        if (isset($this->fdt['ctgdata'][120]) && $this->fdt['ctgdata'][120] !== 0) {
            $this->offset =
                $this->fdt['table']['glyf']['offset'] + $this->fdt['indexToLoc'][$this->fdt['ctgdata'][120]] + 4;
            $yMin = $this->fbyte->getFWord($this->offset);
            $this->offset += 4;
            $yMax = $this->fbyte->getFWord($this->offset);
            $this->offset += 2;
            $this->fdt['XHeight'] = (int) \round(($yMax - $yMin) * $this->fdt['urk']);
        }

        // get CapHeight (height of H)
        $this->fdt['CapHeight'] = (int) $this->fdt['Ascent'];
        if (isset($this->fdt['ctgdata'][72]) && $this->fdt['ctgdata'][72] !== 0) {
            $this->offset =
                $this->fdt['table']['glyf']['offset'] + $this->fdt['indexToLoc'][$this->fdt['ctgdata'][72]] + 4;
            $yMin = $this->fbyte->getFWord($this->offset);
            $this->offset += 4;
            $yMax = $this->fbyte->getFWord($this->offset);
            $this->offset += 2;
            $this->fdt['CapHeight'] = (int) \round(($yMax - $yMin) * $this->fdt['urk']);
        }
    }

    /**
     * Get font widths
     *
     * @return void
     */
    protected function getWidths(): void
    {
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
        // Iterate the actual CID->GID map instead of scanning the full 0..65535 code space,
        // so fonts with few glyphs skip the empty positions. ctgdata is built in ascending
        // CID order, so the resulting cw/cbbox ordering is unchanged.
        foreach ($this->fdt['ctgdata'] as $cid => $gid) {
            if (isset($chw[$gid])) {
                $this->fdt['cw'][$cid] = $chw[$gid];
            }

            if ($this->withCbbox && isset($this->fdt['indexToLoc'][$gid])) {
                $this->offset = $this->fdt['table']['glyf']['offset'] + $this->fdt['indexToLoc'][$gid];
                $xMin = (int) \round($this->fbyte->getFWord($this->offset + 2) * $this->fdt['urk']);
                $yMin = (int) \round($this->fbyte->getFWord($this->offset + 4) * $this->fdt['urk']);
                $xMax = (int) \round($this->fbyte->getFWord($this->offset + 6) * $this->fdt['urk']);
                $yMax = (int) \round($this->fbyte->getFWord($this->offset + 8) * $this->fdt['urk']);
                $this->fdt['cbbox'][$cid] = [$xMin, $yMin, $xMax, $yMax];
            }
        }
    }

    /**
     * Add CTG entry to map CID to GID
     *
     * @param int  $cid Character Identifier
     * @param int  $gid Glyph ID (zero-based index of the glyph in the font's glyph collection)
     *
     * @return void
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
     * @return void
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
            14 => $this->processFormat14(),
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
        for ($chr = 0; $chr < 256; ++$chr) {
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
        for ($chr = 0; $chr < 256; ++$chr) {
            // Array that maps high bytes to subHeaders: value is subHeader index * 8.
            $subHeaderKeys[$chr] = $this->fbyte->getUShort($this->offset) / 8;
            $this->offset += 2;
            if ($numSubHeaders < $subHeaderKeys[$chr]) {
                $numSubHeaders = $subHeaderKeys[$chr];
            }
        }

        // the number of subHeaders is equal to the max of subHeaderKeys + 1
        ++$numSubHeaders;
        // read subHeader structures
        $subHeaders = [];
        $numGlyphIndexArray = 0;
        for ($ish = 0; $ish < $numSubHeaders; ++$ish) {
            $subHeaders[$ish]['firstCode'] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $subHeaders[$ish]['entryCount'] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $subHeaders[$ish]['idDelta'] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $subHeaders[$ish]['idRangeOffset'] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
            $subHeaders[$ish]['idRangeOffset'] -= 2 + (($numSubHeaders - $ish - 1) * 8);
            $subHeaders[$ish]['idRangeOffset'] /= 2;
            $numGlyphIndexArray += $subHeaders[$ish]['entryCount'];
        }

        $glyphIndexArray = [
            0 => 0,
        ];
        for ($gid = 0; $gid < $numGlyphIndexArray; ++$gid) {
            $glyphIndexArray[$gid] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        for ($chr = 0; $chr < 256; ++$chr) {
            $shk = $subHeaderKeys[$chr];
            if ($shk === 0) {
                // one byte code
                $cdx = $chr;
                $gid = $glyphIndexArray[0];
                $this->addCtgItem($cdx, $gid);
            } else {
                // two bytes code
                $start_byte = $subHeaders[$shk]['firstCode'];
                $end_byte = $start_byte + $subHeaders[$shk]['entryCount'];
                for ($jdx = $start_byte; $jdx < $end_byte; ++$jdx) {
                    // combine high and low bytes
                    $cdx = ($chr << 8) + $jdx;
                    $idRangeOffset = $subHeaders[$shk]['idRangeOffset'] + $jdx - $subHeaders[$shk]['firstCode'];
                    $gid = \max(0, ($glyphIndexArray[$idRangeOffset] + $subHeaders[$shk]['idDelta']) % 65_536);
                    $this->addCtgItem($cdx, $gid);
                }
            }
        }
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
        $segCount = \floor($this->fbyte->getUShort($this->offset) / 2);
        $this->offset += 2;
        $this->offset += 6; // skip searchRange, entrySelector, rangeShift
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

        $gidlen = \floor($length / 2) - 8 - (4 * $segCount);
        $glyphIdArray = []; // glyph index array
        for ($kdx = 0; $kdx < $gidlen; ++$kdx) {
            $glyphIdArray[$kdx] = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
        }

        for ($kdx = 0; $kdx < $segCount; ++$kdx) {
            for ($chr = $startCount[$kdx]; $chr <= $endCount[$kdx]; ++$chr) {
                if ($idRangeOffset[$kdx] === 0) {
                    $gid = \max(0, ($idDelta[$kdx] + $chr) % 65_536);
                } else {
                    $gid = (int) (\floor($idRangeOffset[$kdx] / 2) + ($chr - $startCount[$kdx]) - ($segCount - $kdx));
                    $gid = \max(0, ($glyphIdArray[$gid] + $idDelta[$kdx]) % 65_536);
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
        for ($kdx = 0; $kdx < $entryCount; ++$kdx) {
            $chr = $kdx + $firstCode;
            $gid = $this->fbyte->getUShort($this->offset);
            $this->offset += 2;
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
     *  4      - uint32                startCharCode  Last character code in this group (high byte set to \0 if ia32=0)
     *  8      - uint32                startGlyphID   Glyph index corresponding to the starting character code
     */
    protected function processFormat8(): void
    {
        $this->offset += 10; // skip reserved, length and version/language
        $is32 = [];
        for ($kdx = 0; $kdx < 8192; ++$kdx) {
            $is32[$kdx] = $this->fbyte->getByte($this->offset);
            ++$this->offset;
        }

        $nGroups = $this->fbyte->getULong($this->offset);
        $this->offset += 4;
        $budget = self::MAX_CMAP_ENTRIES;
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
            $budget -= $endCharCode - $startCharCode + 1;
            if ($budget < 0) {
                throw new FontException('cmap format 8 subtable maps too many code points');
            }

            for ($cpw = $startCharCode; $cpw <= $endCharCode; ++$cpw) {
                $is32idx = (int) \floor($cpw / 8);
                if (isset($is32[$is32idx]) && ($is32[$is32idx] & (1 << (7 - ($cpw % 8)))) === 0) {
                    $chr = $cpw;
                } else {
                    // 32 bit format
                    // convert to decimal (http://www.unicode.org/faq//utf_bom.html#utf16-4)
                    //LEAD_OFFSET = (0xD800 - (0x10000 >> 10)) = 55232
                    //SURROGATE_OFFSET = (0x10000 - (0xD800 << 10) - 0xDC00) = -56613888
                    $chr = ((55_232 + ($cpw >> 10)) << 10) + (0xDC00 + ($cpw & 0x3FF)) - 56_613_888;
                }

                $this->addCtgItem($chr, $startGlyphID);
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
        for ($kdx = 0; $kdx < $numChars; ++$kdx) {
            $chr = $kdx + $startCharCode;
            $gid = $this->fbyte->getUShort($this->offset);
            $this->addCtgItem($chr, $gid);
            $this->offset += 2;
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
        $budget = self::MAX_CMAP_ENTRIES;
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
            $budget -= $endCharCode - $startCharCode + 1;
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
        $budget = self::MAX_CMAP_ENTRIES;
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
            $budget -= $endCharCode - $startCharCode + 1;
            if ($budget < 0) {
                throw new FontException('cmap format 13 subtable maps too many code points');
            }

            for ($chr = $startCharCode; $chr <= $endCharCode; ++$chr) {
                $this->addCtgItem($chr, $glyphID);
            }
        }
    }

    /**
     * Process Format 14: Unicode Variation Sequences
     *
     * 'cmap' Subtable Format 14 (format field already consumed, $this->offset points past it):
     *   0 - uint16                                  format                  (already consumed) Always 14
     *   2 - uint32                                  length                  Byte length of this subtable (incl. header)
     *   6 - uint32                                  numVarSelectorRecords   Number of VariationSelector records
     *  10 - VariationSelector[numVarSelectorRecords] varSelector            Array of VariationSelector records
     *
     * VariationSelector Record (11 bytes):
     *   0 - uint24     varSelector          Variation selector value
     *   3 - Offset32   defaultUVSOffset     Offset from subtable start to Default UVS table; 0 if absent
     *   7 - Offset32   nonDefaultUVSOffset  Offset from subtable start to Non-Default UVS table; 0 if absent
     *
     * NonDefaultUVS Table:
     *   0 - uint32        numUVSMappings  Number of UVS Mapping records
     *   4 - UVSMapping[]  uvsMappings     Array of UVSMapping records
     *
     * UVSMapping Record (5 bytes):
     *   0 - uint24   unicodeValue  Base Unicode code point of the variation sequence
     *   3 - uint16   glyphID       Glyph ID to use for this variation sequence
     *
     * Default UVS sequences reuse the glyph already mapped in the main cmap table; no action required.
     */
    protected function processFormat14(): void
    {
        // The format field (uint16) was consumed before this method was called,
        // so the subtable starts 2 bytes before the current offset.
        $subtableOffset = $this->offset - 2;

        $this->offset += 4; // skip length (uint32)

        $numVarSelectors = $this->fbyte->getULong($this->offset);
        $this->offset += 4;

        for ($idx = 0; $idx < $numVarSelectors; ++$idx) {
            $this->offset += 3; // skip varSelector (uint24)

            $this->offset += 4; // skip defaultUVSOffset - default sequences use the main cmap glyph

            $nonDefaultOffset = $this->fbyte->getULong($this->offset);
            $this->offset += 4;

            if ($nonDefaultOffset === 0) {
                continue;
            }

            // Process the Non-Default UVS table for this variation selector.
            $savedOffset = $this->offset;
            $this->offset = $subtableOffset + $nonDefaultOffset;

            $numUVSMappings = $this->fbyte->getULong($this->offset);
            $this->offset += 4;

            for ($jdx = 0; $jdx < $numUVSMappings; ++$jdx) {
                // unicodeValue: uint24 (3 bytes, big-endian)
                $unicodeValue =
                    ($this->fbyte->getByte($this->offset) << 16)
                    | ($this->fbyte->getByte($this->offset + 1) << 8)
                    | $this->fbyte->getByte($this->offset + 2);
                $this->offset += 3;

                $glyphID = $this->fbyte->getUShort($this->offset);
                $this->offset += 2;

                $this->addCtgItem($unicodeValue, $glyphID);
            }

            $this->offset = $savedOffset;
        }
    }
}
