<?php

declare(strict_types=1);

/**
 * Core.php
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

use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Unicode\Data\Encoding;

/**
 * Com\Tecnick\Pdf\Font\Import\Core
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
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
class Core implements ProcessorInterface
{
    /**
     * Adobe Glyph List subset mapping AFM glyph names to Unicode codepoints.
     *
     * It covers the glyph names of the Latin text Core14 fonts, and is only consulted for a
     * font whose codes are those of a text encoding (see isFontSpecific()).
     *
     * @var array<string, int>
     */
    private const GLYPH_UNICODE = [
        'A' => 0x0041,
        'AE' => 0x00C6,
        'Aacute' => 0x00C1,
        'Abreve' => 0x0102,
        'Acircumflex' => 0x00C2,
        'Adieresis' => 0x00C4,
        'Agrave' => 0x00C0,
        'Amacron' => 0x0100,
        'Aogonek' => 0x0104,
        'Aring' => 0x00C5,
        'Atilde' => 0x00C3,
        'B' => 0x0042,
        'C' => 0x0043,
        'Cacute' => 0x0106,
        'Ccaron' => 0x010C,
        'Ccedilla' => 0x00C7,
        'D' => 0x0044,
        'Dcaron' => 0x010E,
        'Dcroat' => 0x0110,
        'Delta' => 0x2206,
        'E' => 0x0045,
        'Eacute' => 0x00C9,
        'Ecaron' => 0x011A,
        'Ecircumflex' => 0x00CA,
        'Edieresis' => 0x00CB,
        'Edotaccent' => 0x0116,
        'Egrave' => 0x00C8,
        'Emacron' => 0x0112,
        'Eogonek' => 0x0118,
        'Eth' => 0x00D0,
        'Euro' => 0x20AC,
        'F' => 0x0046,
        'G' => 0x0047,
        'Gbreve' => 0x011E,
        'Gcommaaccent' => 0x0122,
        'H' => 0x0048,
        'I' => 0x0049,
        'Iacute' => 0x00CD,
        'Icircumflex' => 0x00CE,
        'Idieresis' => 0x00CF,
        'Idotaccent' => 0x0130,
        'Igrave' => 0x00CC,
        'Imacron' => 0x012A,
        'Iogonek' => 0x012E,
        'J' => 0x004A,
        'K' => 0x004B,
        'Kcommaaccent' => 0x0136,
        'L' => 0x004C,
        'Lacute' => 0x0139,
        'Lcaron' => 0x013D,
        'Lcommaaccent' => 0x013B,
        'Lslash' => 0x0141,
        'M' => 0x004D,
        'N' => 0x004E,
        'Nacute' => 0x0143,
        'Ncaron' => 0x0147,
        'Ncommaaccent' => 0x0145,
        'Ntilde' => 0x00D1,
        'O' => 0x004F,
        'OE' => 0x0152,
        'Oacute' => 0x00D3,
        'Ocircumflex' => 0x00D4,
        'Odieresis' => 0x00D6,
        'Ograve' => 0x00D2,
        'Ohungarumlaut' => 0x0150,
        'Omacron' => 0x014C,
        'Oslash' => 0x00D8,
        'Otilde' => 0x00D5,
        'P' => 0x0050,
        'Q' => 0x0051,
        'R' => 0x0052,
        'Racute' => 0x0154,
        'Rcaron' => 0x0158,
        'Rcommaaccent' => 0x0156,
        'S' => 0x0053,
        'Sacute' => 0x015A,
        'Scaron' => 0x0160,
        'Scedilla' => 0x015E,
        'Scommaaccent' => 0x0218,
        'T' => 0x0054,
        'Tcaron' => 0x0164,
        'Tcommaaccent' => 0x0162,
        'Thorn' => 0x00DE,
        'U' => 0x0055,
        'Uacute' => 0x00DA,
        'Ucircumflex' => 0x00DB,
        'Udieresis' => 0x00DC,
        'Ugrave' => 0x00D9,
        'Uhungarumlaut' => 0x0170,
        'Umacron' => 0x016A,
        'Uogonek' => 0x0172,
        'Uring' => 0x016E,
        'V' => 0x0056,
        'W' => 0x0057,
        'X' => 0x0058,
        'Y' => 0x0059,
        'Yacute' => 0x00DD,
        'Ydieresis' => 0x0178,
        'Z' => 0x005A,
        'Zacute' => 0x0179,
        'Zcaron' => 0x017D,
        'Zdotaccent' => 0x017B,
        'a' => 0x0061,
        'aacute' => 0x00E1,
        'abreve' => 0x0103,
        'acircumflex' => 0x00E2,
        'acute' => 0x00B4,
        'adieresis' => 0x00E4,
        'ae' => 0x00E6,
        'agrave' => 0x00E0,
        'amacron' => 0x0101,
        'ampersand' => 0x0026,
        'aogonek' => 0x0105,
        'aring' => 0x00E5,
        'asciicircum' => 0x005E,
        'asciitilde' => 0x007E,
        'asterisk' => 0x002A,
        'at' => 0x0040,
        'atilde' => 0x00E3,
        'b' => 0x0062,
        'backslash' => 0x005C,
        'bar' => 0x007C,
        'braceleft' => 0x007B,
        'braceright' => 0x007D,
        'bracketleft' => 0x005B,
        'bracketright' => 0x005D,
        'breve' => 0x02D8,
        'brokenbar' => 0x00A6,
        'bullet' => 0x2022,
        'c' => 0x0063,
        'cacute' => 0x0107,
        'caron' => 0x02C7,
        'ccaron' => 0x010D,
        'ccedilla' => 0x00E7,
        'cedilla' => 0x00B8,
        'cent' => 0x00A2,
        'circumflex' => 0x02C6,
        'colon' => 0x003A,
        'comma' => 0x002C,
        'commaaccent' => 0x0326,
        'copyright' => 0x00A9,
        'currency' => 0x00A4,
        'd' => 0x0064,
        'dagger' => 0x2020,
        'daggerdbl' => 0x2021,
        'dcaron' => 0x010F,
        'dcroat' => 0x0111,
        'degree' => 0x00B0,
        'dieresis' => 0x00A8,
        'divide' => 0x00F7,
        'dollar' => 0x0024,
        'dotaccent' => 0x02D9,
        'dotlessi' => 0x0131,
        'e' => 0x0065,
        'eacute' => 0x00E9,
        'ecaron' => 0x011B,
        'ecircumflex' => 0x00EA,
        'edieresis' => 0x00EB,
        'edotaccent' => 0x0117,
        'egrave' => 0x00E8,
        'eight' => 0x0038,
        'ellipsis' => 0x2026,
        'emacron' => 0x0113,
        'emdash' => 0x2014,
        'endash' => 0x2013,
        'eogonek' => 0x0119,
        'equal' => 0x003D,
        'eth' => 0x00F0,
        'exclam' => 0x0021,
        'exclamdown' => 0x00A1,
        'f' => 0x0066,
        'fi' => 0xFB01,
        'five' => 0x0035,
        'fl' => 0xFB02,
        'florin' => 0x0192,
        'four' => 0x0034,
        'fraction' => 0x2044,
        'g' => 0x0067,
        'gbreve' => 0x011F,
        'gcommaaccent' => 0x0123,
        'germandbls' => 0x00DF,
        'grave' => 0x0060,
        'greater' => 0x003E,
        'greaterequal' => 0x2265,
        'guillemotleft' => 0x00AB,
        'guillemotright' => 0x00BB,
        'guilsinglleft' => 0x2039,
        'guilsinglright' => 0x203A,
        'h' => 0x0068,
        'hungarumlaut' => 0x02DD,
        'hyphen' => 0x002D,
        'i' => 0x0069,
        'iacute' => 0x00ED,
        'icircumflex' => 0x00EE,
        'idieresis' => 0x00EF,
        'igrave' => 0x00EC,
        'imacron' => 0x012B,
        'iogonek' => 0x012F,
        'j' => 0x006A,
        'k' => 0x006B,
        'kcommaaccent' => 0x0137,
        'l' => 0x006C,
        'lacute' => 0x013A,
        'lcaron' => 0x013E,
        'lcommaaccent' => 0x013C,
        'less' => 0x003C,
        'lessequal' => 0x2264,
        'logicalnot' => 0x00AC,
        'lozenge' => 0x25CA,
        'lslash' => 0x0142,
        'm' => 0x006D,
        'macron' => 0x00AF,
        'minus' => 0x2212,
        'mu' => 0x00B5,
        'multiply' => 0x00D7,
        'n' => 0x006E,
        'nacute' => 0x0144,
        'ncaron' => 0x0148,
        'ncommaaccent' => 0x0146,
        'nine' => 0x0039,
        'notequal' => 0x2260,
        'ntilde' => 0x00F1,
        'numbersign' => 0x0023,
        'o' => 0x006F,
        'oacute' => 0x00F3,
        'ocircumflex' => 0x00F4,
        'odieresis' => 0x00F6,
        'oe' => 0x0153,
        'ogonek' => 0x02DB,
        'ograve' => 0x00F2,
        'ohungarumlaut' => 0x0151,
        'omacron' => 0x014D,
        'one' => 0x0031,
        'onehalf' => 0x00BD,
        'onequarter' => 0x00BC,
        'onesuperior' => 0x00B9,
        'ordfeminine' => 0x00AA,
        'ordmasculine' => 0x00BA,
        'oslash' => 0x00F8,
        'otilde' => 0x00F5,
        'p' => 0x0070,
        'paragraph' => 0x00B6,
        'parenleft' => 0x0028,
        'parenright' => 0x0029,
        'partialdiff' => 0x2202,
        'percent' => 0x0025,
        'period' => 0x002E,
        'periodcentered' => 0x00B7,
        'perthousand' => 0x2030,
        'plus' => 0x002B,
        'plusminus' => 0x00B1,
        'q' => 0x0071,
        'question' => 0x003F,
        'questiondown' => 0x00BF,
        'quotedbl' => 0x0022,
        'quotedblbase' => 0x201E,
        'quotedblleft' => 0x201C,
        'quotedblright' => 0x201D,
        'quoteleft' => 0x2018,
        'quoteright' => 0x2019,
        'quotesinglbase' => 0x201A,
        'quotesingle' => 0x0027,
        'r' => 0x0072,
        'racute' => 0x0155,
        'radical' => 0x221A,
        'rcaron' => 0x0159,
        'rcommaaccent' => 0x0157,
        'registered' => 0x00AE,
        'ring' => 0x02DA,
        's' => 0x0073,
        'sacute' => 0x015B,
        'scaron' => 0x0161,
        'scedilla' => 0x015F,
        'scommaaccent' => 0x0219,
        'section' => 0x00A7,
        'semicolon' => 0x003B,
        'seven' => 0x0037,
        'six' => 0x0036,
        'slash' => 0x002F,
        'space' => 0x0020,
        'sterling' => 0x00A3,
        'summation' => 0x2211,
        't' => 0x0074,
        'tcaron' => 0x0165,
        'tcommaaccent' => 0x0163,
        'thorn' => 0x00FE,
        'three' => 0x0033,
        'threequarters' => 0x00BE,
        'threesuperior' => 0x00B3,
        'tilde' => 0x02DC,
        'trademark' => 0x2122,
        'two' => 0x0032,
        'twosuperior' => 0x00B2,
        'u' => 0x0075,
        'uacute' => 0x00FA,
        'ucircumflex' => 0x00FB,
        'udieresis' => 0x00FC,
        'ugrave' => 0x00F9,
        'uhungarumlaut' => 0x0171,
        'umacron' => 0x016B,
        'underscore' => 0x005F,
        'uogonek' => 0x0173,
        'uring' => 0x016F,
        'v' => 0x0076,
        'w' => 0x0077,
        'x' => 0x0078,
        'y' => 0x0079,
        'yacute' => 0x00FD,
        'ydieresis' => 0x00FF,
        'yen' => 0x00A5,
        'z' => 0x007A,
        'zacute' => 0x017A,
        'zcaron' => 0x017E,
        'zdotaccent' => 0x017C,
        'zero' => 0x0030,
    ];

    /**
     * Lower-cased AFM 'EncodingScheme' values whose character codes are those of a Latin
     * text encoding, so that the metrics are addressed by glyph name through WinAnsi.
     *
     * Anything else is taken to be the font's own encoding (see isFontSpecific()). The empty
     * string is listed because the row is optional.
     *
     * @var array<string, bool>
     */
    private const TEXT_ENCODING_SCHEMES = [
        '' => true,
        'adobestandardencoding' => true,
        'extendedroman' => true,
        'isolatin1encoding' => true,
        'macromanencoding' => true,
        'macroman' => true,
        'pdfdocencoding' => true,
        'standardencoding' => true,
        'winansiencoding' => true,
        'winroman' => true,
    ];

    /**
     * WinAnsi (cp1252) glyph name to byte indexes, the inverse of Encoding::MAP['cp1252'].
     * Built once on first use.
     *
     * A name may be assigned more than one byte (ISO 32000-1 Annex D.2), so every byte it is
     * assigned is listed.
     *
     * @var array<string, array<int, int>>|null
     */
    private static ?array $winAnsiByName = null;

    /**
     * Unicode-keyed widths accumulated during AFM parsing.
     *
     * @var array<int, int>
     */
    private array $cwu = [];

    /**
     * Unicode-keyed glyph bounding boxes accumulated during AFM parsing.
     *
     * @var array<int, array<int, int>>
     */
    private array $cbboxu = [];

    /**
     * File helper used to load font definition files.
     */
    protected ObjFile $fileHelper;

    /**
     * @param string    $font       Content of the input font file
     * @param TFontData $fdt        Extracted font metrics
     * @param ObjFile   $fileHelper File helper for font loading.
     *
     * @throws FontException in case of error
     */
    public function __construct(
        protected string $font,
        protected array $fdt,
        ObjFile $fileHelper,
    ) {
        $this->fileHelper = $fileHelper;
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
     * Returns true when the character codes of the AFM are the font's own, rather than
     * those of a Latin text encoding.
     *
     * The two Core14 symbol fonts are recognised by name; every other font is classified by
     * its declared EncodingScheme. The flags passed to the importer do not take part.
     */
    private function isFontSpecific(): bool
    {
        if ($this->fdt['FontName'] === 'Symbol' || $this->fdt['FontName'] === 'ZapfDingbats') {
            return true;
        }

        return !isset(self::TEXT_ENCODING_SCHEMES[\strtolower($this->fdt['EncodingScheme'])]);
    }

    /**
     * Set the PDF font descriptor flags
     */
    protected function setFlags(): void
    {
        // ISO 32000-1 Table 123: bit 3 (Symbolic, 4) and bit 6 (Nonsymbolic, 32) shall not
        // both be set nor both be clear, so each branch clears the bit it contradicts
        if ($this->isFontSpecific()) {
            $this->fdt['Flags'] = ($this->fdt['Flags'] & ~32) | 4;
        } else {
            $this->fdt['Flags'] = ($this->fdt['Flags'] & ~4) | 32;
        }

        if ($this->fdt['IsFixedPitch']) {
            $this->fdt['Flags'] = (int) $this->fdt['Flags'] | 1;
        }

        if ((int) $this->fdt['ItalicAngle'] !== 0) {
            $this->fdt['Flags'] = (int) $this->fdt['Flags'] | 64;
        }
    }

    /**
     * Returns the Unicode codepoint of a glyph name, or null when it is not a name of the
     * Adobe Glyph List subset this class carries.
     *
     * @param string $name Glyph name.
     */
    protected static function getGlyphUnicode(string $name): ?int
    {
        return self::GLYPH_UNICODE[$name] ?? null;
    }

    /**
     * Returns the vertical stem width to assume for a font that declares no StdVW.
     *
     * It is estimated from the declared weight: 'Weight' carries the AFM row and 'weight'
     * the Type1 /Weight entry of the FontInfo dictionary.
     */
    protected function getDefaultStemV(): int
    {
        $weight = \strtolower($this->fdt['weight'] === '' ? $this->fdt['Weight'] : $this->fdt['weight']);
        return $weight === 'bold' || $weight === 'black' ? 123 : 70;
    }

    /**
     * Set Char widths
     *
     * @param array<int, int> $cwidths Extracted widths
     */
    protected function setCharWidths(array $cwidths): void
    {
        $this->fdt['MissingWidth'] = 600;
        if (isset($cwidths[32]) && $cwidths[32] !== 0) {
            $this->fdt['MissingWidth'] = $cwidths[32];
        }

        $this->fdt['MaxWidth'] = (int) $this->fdt['MissingWidth'];
        $this->fdt['AvgWidth'] = 0;
        $this->fdt['cw'] = [];
        // only the widths of the emitted single-byte range are averaged
        $numWidths = 0;
        for ($cid = 0; $cid <= 255; ++$cid) {
            if (!isset($cwidths[$cid])) {
                // only the codes the font defines are recorded
                continue;
            }

            if ($cwidths[$cid] > $this->fdt['MaxWidth']) {
                $this->fdt['MaxWidth'] = $cwidths[$cid];
            }

            $this->fdt['AvgWidth'] += $cwidths[$cid];
            $this->fdt['cw'][$cid] = $cwidths[$cid];
            ++$numWidths;
        }

        $this->fdt['AvgWidth'] = $numWidths > 0 ? (int) \round($this->fdt['AvgWidth'] / $numWidths) : 0;
    }

    /**
     * Returns the bytes a PDF content stream may use to select a glyph, in ascending
     * order, or an empty array when the glyph is not reachable through a single-byte code.
     *
     * A font-specific font is emitted without an /Encoding entry, so the AFM 'C' column
     * holds the code. Every other font is emitted as WinAnsiEncoding, where the glyph name
     * selects the byte.
     *
     * @param string $name    Glyph name (the AFM 'N' value).
     * @param string $rawcode Character code declared by the AFM 'C' column ('-1' when the
     *                        glyph is not encoded).
     *
     * @return array<int, int>
     */
    private function getMetricCodes(string $name, string $rawcode): array
    {
        if ($this->isFontSpecific()) {
            // a value that is not a number declares no code rather than the code zero
            if (\preg_match('/^[+-]?[0-9]++$/', $rawcode) !== 1) {
                return [];
            }

            $code = (int) $rawcode;
            return $code >= 0 && $code <= 255 ? [$code] : [];
        }

        return self::getWinAnsiByName()[$name] ?? [];
    }

    /**
     * Build (once) the WinAnsi glyph name to byte indexes reverse map.
     *
     * @return array<string, array<int, int>>
     */
    private static function getWinAnsiByName(): array
    {
        if (self::$winAnsiByName === null) {
            self::$winAnsiByName = [];
            // every byte the encoding assigns to the name is listed, in ascending order
            for ($cid = 0; $cid <= 255; ++$cid) {
                $name = Encoding::MAP['cp1252'][$cid];
                if ($name !== '.notdef') {
                    self::$winAnsiByName[$name][] = $cid;
                }
            }
        }

        return self::$winAnsiByName;
    }

    /**
     * Extract Metrics
     */
    protected function extractMetrics(): void
    {
        $cwd = [];
        $this->cwu = [];
        $this->cbboxu = [];
        $this->fdt['cbbox'] = [];
        $lines = \explode("\n", \str_replace("\r", '', $this->font));
        // the code space of the metrics is read first, so that it applies to every row
        // whatever its position in the file
        $this->readCodeSpaceFields($lines);
        // process each row
        foreach ($lines as $line) {
            // the ';' terminating each key/value group needs no surrounding whitespace, so
            // it is isolated into its own token before splitting on any run of whitespace
            $col = \preg_split('/\s+/', \trim(\str_replace(';', ' ; ', $line)), -1, PREG_SPLIT_NO_EMPTY);
            if ($col !== false && \count($col) > 1) {
                $this->processMetricRow($col, $cwd);
            }
        }

        $this->fdt['Leading'] = 0;
        $this->fdt['cwu'] = $this->cwu;
        $this->fdt['cbboxu'] = $this->cbboxu;
        $this->setCharWidths($cwd);
    }

    /**
     * Read the fields that decide the code space of the character metrics.
     *
     * @param array<int, string> $lines Rows of the AFM file.
     */
    private function readCodeSpaceFields(array $lines): void
    {
        foreach ($lines as $line) {
            $col = \preg_split('/\s+/', \trim($line), -1, PREG_SPLIT_NO_EMPTY);
            if ($col === false || \count($col) < 2) {
                continue;
            }

            if ($col[0] === 'FontName' || $col[0] === 'EncodingScheme') {
                // the value may hold several words, as in the row loop
                $this->fdt[$col[0]] = \implode(' ', \array_slice($col, 1));
            }
        }
    }

    /**
     * Split an AFM CharMetrics row into its key/value pairs.
     *
     * A row is a list of ';'-terminated groups, each starting with the key name
     * ('C', 'WX', 'N', 'B', 'L', ...) followed by its values.
     *
     * @param array<int, string> $col Row tokens, already stripped of empty ones.
     *
     * @return array<string, array<int, string>> Values indexed by key name.
     */
    private static function splitCharMetrics(array $col): array
    {
        $fields = [];
        $field = '';
        $values = [];
        foreach ($col as $item) {
            if ($item === ';') {
                if ($field !== '') {
                    $fields[$field] = $values;
                }

                $field = '';
                $values = [];
                continue;
            }

            if ($field === '') {
                $field = $item;
                continue;
            }

            $values[] = $item;
        }

        if ($field !== '') {
            $fields[$field] = $values;
        }

        return $fields;
    }

    /**
     * Returns the character code a CharMetrics row declares, as a decimal string.
     *
     * 'C' carries a decimal value and 'CH' a hexadecimal one between angle brackets
     * ('CH <20> ; WX 250 ; N space ;'). An unparsable value is reported as no value.
     *
     * @param array<string, array<int, string>> $fields Key/value groups of the row.
     */
    private static function rawCharCode(array $fields): string
    {
        $decimal = $fields['C'][0] ?? null;
        if ($decimal !== null) {
            return $decimal;
        }

        $hex = \trim($fields['CH'][0] ?? '', '<>');
        return \preg_match('/^[0-9a-fA-F]++$/', $hex) === 1 ? (string) \hexdec($hex) : '';
    }

    /**
     * Process a single AFM metric row
     *
     * @param array<int, string> $col Array containing row elements to process
     * @param array<int, int>    $cwd Array containing cid widths
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function processMetricRow(array $col, array &$cwd): void
    {
        switch ($col[0]) {
            case 'IsFixedPitch':
                $this->fdt['IsFixedPitch'] = $col[1] === 'true';
                break;
            case 'FontBBox':
                // the four corners feed the derived Ascender and Descender
                if (!isset($col[1], $col[2], $col[3], $col[4])) {
                    throw new FontException('the AFM FontBBox row must declare four values');
                }

                $this->fdt['FontBBox'] = [(int) $col[1], (int) $col[2], (int) $col[3], (int) $col[4]];
                break;
            case 'C':
            case 'CH':
                // the ';'-separated groups of a CharMetrics row may come in any order, so
                // they are read by key instead of by column position
                $fields = self::splitCharMetrics($col);
                $name = $fields['N'][0] ?? '';
                if ($name === '' || $name === '.notdef') {
                    break;
                }

                $width = (int) ($fields['WX'][0] ?? 0);
                $rawbbox = $fields['B'] ?? [];
                $bbox = \count($rawbbox) === 4
                    ? [(int) $rawbbox[0], (int) $rawbbox[1], (int) $rawbbox[2], (int) $rawbbox[3]]
                    : null;
                foreach ($this->getMetricCodes($name, self::rawCharCode($fields)) as $code) {
                    $cwd[$code] = $width;
                    if ($bbox !== null) {
                        $this->fdt['cbbox'][$code] = $bbox;
                    }
                }

                // the metrics are also stored under the Unicode codepoint, as the encoding
                // byte of a glyph is not its codepoint; the codes of a font-specific font
                // are its own, so no codepoint is derived for it
                $unicode = $this->isFontSpecific() ? null : self::getGlyphUnicode($name);
                if ($unicode !== null) {
                    $this->cwu[$unicode] = $width;
                    if ($bbox !== null) {
                        $this->cbboxu[$unicode] = $bbox;
                    }
                }

                break;
            case 'FontName':
            case 'FullName':
            case 'FamilyName':
            case 'Weight':
            case 'CharacterSet':
            case 'Version':
            case 'EncodingScheme':
                // these values may hold several words (e.g. 'FullName Helvetica Bold')
                $this->fdt[$col[0]] = \implode(' ', \array_slice($col, 1));
                break;
            case 'ItalicAngle':
                $this->fdt['ItalicAngle'] = self::roundedValue($col[1]);
                break;
            case 'UnderlinePosition':
            case 'UnderlineThickness':
            case 'CapHeight':
            case 'XHeight':
            case 'Ascender':
            case 'Descender':
            case 'StdHW':
            case 'StdVW':
                $this->fdt[$col[0]] = (int) $col[1];
                break;
        }
    }

    /**
     * Returns a real number of a font file rounded to the nearest integer, half away from
     * zero.
     *
     * @param string $value Raw value read from the font file.
     */
    protected static function roundedValue(string $value): int
    {
        return (int) \round(\is_numeric($value) ? (float) $value : 0.0);
    }

    /**
     * Map values to the correct key name
     *
     * @throws FontException
     */
    protected function remapValues(): void
    {
        // rename properties
        // FontName is the PostScript name ('Helvetica-Bold'), the one a PDF /BaseFont
        // expects; FullName is a human-readable variant ('Helvetica Bold') used as fallback
        $this->fdt['name'] = $this->fdt['FontName'] === '' ? $this->fdt['FullName'] : $this->fdt['FontName'];
        $this->fdt['underlinePosition'] = $this->fdt['UnderlinePosition'];
        $this->fdt['underlineThickness'] = $this->fdt['UnderlineThickness'];
        $this->fdt['italicAngle'] = $this->fdt['ItalicAngle'];
        $this->fdt['Ascent'] = $this->fdt['Ascender'];
        $this->fdt['Descent'] = $this->fdt['Descender'];
        // /StemV is a required font descriptor entry (ISO 32000-1 Table 122)
        $this->fdt['StemV'] = $this->fdt['StdVW'] !== 0 ? $this->fdt['StdVW'] : $this->getDefaultStemV();
        $this->fdt['StemH'] = $this->fdt['StdHW'] !== 0 ? $this->fdt['StdHW'] : 30;

        $name = \preg_replace('/[^a-zA-Z0-9_\-]/', '', $this->fdt['name']);
        if ($name === null) {
            throw new FontException('Invalid font name');
        }

        if ($name === '') {
            throw new FontException('Error getting font name.');
        }

        $this->fdt['name'] = $name;
        $this->fdt['bbox'] = \implode(' ', $this->fdt['FontBBox']);
    }

    /**
     * Derive the metrics that are not listed in the AFM file
     */
    protected function setMissingValues(): void
    {
        // Ascender and Descender are derived from the bounding box
        if (\count($this->fdt['FontBBox']) !== 4) {
            throw new FontException('the AFM file must declare a FontBBox with four values');
        }

        // the bounding box only applies when the AFM declares no value
        if ($this->fdt['Descender'] === 0) {
            $this->fdt['Descender'] = $this->fdt['FontBBox'][1];
        }

        if ($this->fdt['Ascender'] === 0) {
            $this->fdt['Ascender'] = $this->fdt['FontBBox'][3];
        }

        if ($this->fdt['CapHeight'] === 0) {
            $this->fdt['CapHeight'] = $this->fdt['Ascender'];
        }
    }

    /**
     * Process Core font
     *
     * @throws FontException
     */
    protected function process(): void
    {
        $this->extractMetrics();
        $this->setFlags();
        $this->setMissingValues();
        $this->remapValues();
    }
}
