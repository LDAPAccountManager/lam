<?php

declare(strict_types=1);

/**
 * Buffer.php
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

use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\Buffer
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-type TFileOptions array{
 *   allowedHosts?: array<string>,
 *   maxRemoteSize?: int,
 *   curlopts?: array<int, bool|int|string>,
 *   defaultCurlOpts?: array<int, bool|int|string>,
 *   fixedCurlOpts?: array<int, bool|int|string>
 * }
 *
 * @phpstan-import-type TFontData from Load
 */
abstract class Buffer
{
    /**
     * Size in bytes of a CIDToGIDMap table: 65536 big-endian 16-bit glyph indices.
     */
    protected const CTG_TABLE_SIZE = 131_072; // (256 * 256 * 2)

    /**
     * Number of CIDToGIDMap tables kept in memory.
     *
     * Each table is CTG_TABLE_SIZE bytes, so the cache is bounded to one megabyte.
     */
    protected const CTG_CACHE_SIZE = 8;

    /**
     * Highest glyph index an sfnt program can address, as a 16-bit value.
     */
    protected const MAX_GID = 0xFFFF;

    /**
     * Array containing all fonts data
     *
     * @var array<string, TFontData>
     */
    protected array $font = [];

    /**
     * Font counter
     */
    protected int $numfonts = 0;

    /**
     * Resolved font key for each (font family, style) pair.
     *
     * @var array<string, array<string, string>>
     */
    protected array $fontKeyCache = [];

    /**
     * Array containing encoding differences
     *
     * @var array<int, string>
     */
    protected array $encdiff = [];

    /**
     * Index for Encoding differences
     */
    protected int $numdiffs = 0;

    /**
     * Optional file helper forwarded to font loaders.
     *
     * @var ObjFile|null
     */
    protected ?ObjFile $fileHelper;

    /**
     * Uncompressed CIDToGIDMap tables indexed by font directory and file name.
     *
     * Each table is a string of 65536 big-endian 16-bit glyph indices addressed by
     * Unicode codepoint, as stored in the '.ctg.z' artifact of the font.
     *
     * Ordered from the least to the most recently used.
     *
     * @var array<string, string>
     */
    protected array $ctgtable = [];

    /**
     * Initialize fonts buffer
     *
     * @param float   $kunit   Unit of measure conversion ratio.
     * @param bool    $subset  If true, embed only the characters used by the document.
     *                         Valid only for TrueTypeUnicode fonts.
     *                         Subsetting is computational and memory intensive.
     * @param bool    $unicode True if we are in Unicode mode, False otherwise.
     * @param bool    $pdfa    True if we are in PDF/A mode, False otherwise.
     * @param ObjFile|null $fileHelper Optional file helper for font loading.
     *
     * @throws FontException if the unit ratio is not a positive number.
     */
    public function __construct(
        protected float $kunit,
        protected bool $subset = false,
        protected bool $unicode = true,
        protected bool $pdfa = false,
        ?ObjFile $fileHelper = null,
    ) {
        if ($kunit <= 0 || !\is_finite($kunit)) {
            // every font metric is divided by this ratio
            throw new FontException('The unit of measure conversion ratio must be a finite number greater than zero');
        }

        $this->fileHelper = $fileHelper;
    }

    /**
     * Get the default subset mode
     */
    public function isSubsetMode(): bool
    {
        return $this->subset;
    }

    /**
     * Returns the fonts buffer
     *
     * @return array<string, TFontData>
     */
    public function getFonts(): array
    {
        return $this->font;
    }

    /**
     * Returns the encoding differences buffer
     *
     * @return array<int, string>
     */
    public function getEncDiffs(): array
    {
        return $this->encdiff;
    }

    /**
     * Returns true if the specified font key exist on buffer
     *
     * @param string $key Font key
     */
    public function isValidKey(string $key): bool
    {
        return isset($this->font[$key]);
    }

    /**
     * Get font by key
     *
     * @param string $key Font key
     *
     * @return TFontData Returns the fonts array.
     *
     * @throws FontException in case of error
     */
    public function getFont(string $key): array
    {
        if (!isset($this->font[$key])) {
            throw new FontException('The font ' . $key . ' has not been loaded');
        }

        return $this->font[$key];
    }

    /**
     * Add a character to the subset list
     *
     * @param string $key  The font key
     * @param int    $char The Unicode character value to add
     *
     * @throws FontException
     */
    public function addSubsetChar(string $key, int $char): void
    {
        if (!isset($this->font[$key])) {
            throw new FontException('The font ' . $key . ' has not been loaded');
        }

        $this->font[$key]['subsetchars'][$char] = true;
    }

    /**
     * Returns the CIDToGIDMap table of the given font.
     *
     * The table is read once per definition file and shared by every font
     * instance backed by it.
     *
     * @param TFontData $font Font data.
     *
     * @return string Table of 65536 big-endian 16-bit glyph indices.
     *
     * @throws FontException in case of error
     */
    protected function getCtgTable(array $font): string
    {
        $ctg = \strtolower($font['ctg']);
        // the directory is part of the key: two fonts may ship a different table under the same name
        $cachekey = $font['dir'] . '|' . $ctg;
        if (isset($this->ctgtable[$cachekey])) {
            $table = $this->ctgtable[$cachekey];
            // move the entry to the most recent end
            unset($this->ctgtable[$cachekey]);
            $this->ctgtable[$cachekey] = $table;
            return $table;
        }

        $ctgfile = FontPaths::findFontFile($font['dir'], $ctg);
        if ($ctgfile === '') {
            throw new FontException('Unable to locate the file: ' . $ctg);
        }

        $fileHelper = $this->fileHelper ?? new ObjFile(allowedPaths: FontPaths::buildAllowedPaths());
        $content = $fileHelper->getLocalFileData($ctgfile);
        if ($content === false) {
            throw new FontException('Unable to read font file: ' . $ctgfile);
        }

        if (\str_ends_with($ctgfile, '.z')) {
            $content = Zlib::uncompress($content, self::CTG_TABLE_SIZE);
            if ($content === false) {
                throw new FontException('Unable to uncompress font file: ' . $ctgfile);
            }
        }

        // a short artifact is padded with notdef entries and a long one is truncated
        $content = \strlen($content) < self::CTG_TABLE_SIZE
            ? \str_pad($content, self::CTG_TABLE_SIZE, "\x00")
            : \substr($content, 0, self::CTG_TABLE_SIZE);

        $this->ctgtable[$cachekey] = $content;
        $this->evictOldestCtgTables();
        return $content;
    }

    /**
     * Drop the least recently used CIDToGIDMap tables beyond the size of the cache.
     */
    protected function evictOldestCtgTables(): void
    {
        $excess = \count($this->ctgtable) - self::CTG_CACHE_SIZE;
        if ($excess <= 0) {
            return;
        }

        foreach (\array_slice(\array_keys($this->ctgtable), 0, $excess) as $oldest) {
            unset($this->ctgtable[$oldest]);
        }
    }

    /**
     * Record a glyph index used by the document and the codepoint it was encoded from.
     *
     * The first codepoint mapped to a glyph is the one stored for it.
     *
     * @param string $key The font key
     * @param int    $gid The glyph index, in the 0..65535 range
     * @param int    $ord The Unicode codepoint the glyph was selected for
     *
     * @throws FontException if the font is not loaded, or the glyph index is out of range
     */
    public function addUsedGid(string $key, int $gid, int $ord): void
    {
        if (!isset($this->font[$key])) {
            throw new FontException('The font ' . $key . ' has not been loaded');
        }

        if ($gid < 0 || $gid > self::MAX_GID) {
            throw new FontException('The glyph index ' . $gid . ' is outside the 0..65535 range');
        }

        if (!isset($this->font[$key]['usedgid'][$gid])) {
            $this->font[$key]['usedgid'][$gid] = $ord;
        }
    }

    /**
     * Add a new font to the fonts buffer
     *
     * The definition file (and the font file itself when embedding) must be present either in the current directory
     * or in the one indicated by K_PATH_FONTS if the constant is defined.
     *
     * @param int    $objnum Current PDF object number
     * @param string $font   Font family.
     *                       If it is a standard family name, it will override the corresponding font.
     * @param string $style  Font style.
     *                       Possible values are (case-insensitive):
     *                       regular (default)
     *                       B: bold
     *                       I: italic
     *                       U: underline
     *                       D: strikeout (linethrough)
     *                       O: overline
     * @param string $ifile  The font definition file (or empty for autodetect).
     *                       By default, the name is built from the family and style, in lower case with no spaces.
     * @param ?bool  $subset If true, embed only the characters used by the document.
     *                       Valid only for TrueTypeUnicode fonts.
     *                       Set to null to use the default value.
     *                       Subsetting is computational and memory intensive.
     *
     * @return string Font key
     *
     * @throws FontException in case of error
     */
    public function add(
        int &$objnum,
        string $font,
        string $style = '',
        string $ifile = '',
        ?bool $subset = null,
    ): string {
        if ($subset === null) {
            $subset = $this->subset;
        }

        // the font key depends only on (family, style, unicode, pdfa), so an already
        // resolved key is reused when the definition file is autodetected
        if ($ifile === '' && isset($this->fontKeyCache[$font][$style])) {
            $cachedKey = $this->fontKeyCache[$font][$style];
            if (isset($this->font[$cachedKey])) {
                $this->aggregateSubset($cachedKey, $subset);
                return $cachedKey;
            }
        }

        $fobj = new Font($font, $style, $ifile, $subset, $this->unicode, $this->pdfa, true, $this->fileHelper);
        $key = $fobj->getFontkey();
        if ($ifile === '') {
            $this->fontKeyCache[$font][$style] = $key;
        }

        if (isset($this->font[$key])) {
            $this->aggregateSubset($key, $subset);
            return $key;
        }

        $fobj->load();
        $this->font[$key] = $fobj->getFontData();

        $this->setFontDiff($key);

        $this->font[$key]['i'] = ++$this->numfonts;
        $this->font[$key]['n'] = ++$objnum;

        return $key;
    }

    /**
     * Record the subsetting mode requested for a font that is already in the buffer.
     *
     * The font is subset only when every request for it asked for a subset.
     *
     * @param string $key    Font key.
     * @param bool   $subset True if this request asked for a subset.
     */
    protected function aggregateSubset(string $key, bool $subset): void
    {
        $this->font[$key]['subset'] = $this->font[$key]['subset'] && $subset;
    }

    /**
     * Set font diff
     *
     * @param string $key Font key
     */
    protected function setFontDiff(string $key): void
    {
        if ($this->font[$key]['diff'] === '') {
            return;
        }

        $diffid = \array_search($this->font[$key]['diff'], $this->encdiff, true);
        if ($diffid === false) {
            $diffid = ++$this->numdiffs;
            $this->encdiff[$diffid] = $this->font[$key]['diff'];
        }

        $this->font[$key]['diffid'] = $diffid;
    }
}
