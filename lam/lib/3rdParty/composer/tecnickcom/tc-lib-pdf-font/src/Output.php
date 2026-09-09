<?php

declare(strict_types=1);

/**
 * Output.php
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

use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Encrypt\Encrypt;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\Output
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
class Output extends \Com\Tecnick\Pdf\Font\OutFont
{
    /**
     * Namespace and schema version prefix for the subset cache keys.
     *
     * The trailing version segment invalidates the cached subsets when
     * the subsetting algorithm or the key format changes.
     */
    protected const SUBSET_CACHE_KEY_PREFIX = 'tc-lib-pdf-font:subset:v2:';

    /**
     * Array of character subsets for each font file
     *
     * @var array<string, array<int, bool>>
     */
    protected array $subchars = [];

    /**
     * Effective subset flag for each font file, keyed like $subchars.
     *
     * A font file shared by several fonts is emitted once, so it is subset only when every
     * font referencing it is subset.
     *
     * @var array<string, bool>
     */
    protected array $filesubset = [];

    /**
     * PDF string block with the fonts definitions
     */
    protected string $out = '';

    /**
     * Initialize font data
     *
     * @param array<string, TFontData>  $fonts       Array of imported fonts data
     * @param int                       $pon         Current PDF Object Number
     * @param Encrypt                   $encrypt     Encrypt object
     * @param ?ObjFile                  $fileHelper  Optional file helper for font loading.
     * @param ?FontSubsetCacheInterface $subsetCache Optional cache for subset font programs.
     *
     * @throws FileException
     * @throws FontException
     */
    public function __construct(
        protected array $fonts,
        int $pon,
        Encrypt $encrypt,
        ?ObjFile $fileHelper = null,
        protected ?FontSubsetCacheInterface $subsetCache = null,
    ) {
        $this->fileHelper = $fileHelper ?? new ObjFile(allowedPaths: FontPaths::buildAllowedPaths());

        $this->pon = $pon;
        $this->enc = $encrypt;

        $this->out = $this->getEncodingDiffs();
        $this->out .= $this->getFontFiles();
        $this->out .= $this->getFontDefinitions();
    }

    /**
     * Returns current PDF object number
     */
    public function getObjectNumber(): int
    {
        return $this->pon;
    }

    /**
     * Returns the PDF fonts block
     */
    public function getFontsBlock(): string
    {
        return $this->out;
    }

    /**
     * Get the PDF output string for the Font resources dictionary of the given fonts.
     *
     * @param array<string, TFontData|array{'i': int, 'n': int}> $data Font data.
     */
    private function getOutFontResources(array $data): string
    {
        if ($data === []) {
            return '';
        }

        $out = ' /Font <<';

        foreach ($data as $font) {
            $out .= ' /F' . (int) $font['i'] . ' ' . (int) $font['n'] . ' 0 R';
        }

        return $out . ' >>';
    }

    /**
     * Get the PDF output string for the Font resources dictionary of all the loaded fonts.
     */
    public function getOutFontDict(): string
    {
        return $this->getOutFontResources($this->fonts);
    }

    /**
     * Get the PDF output string for the Font resources dictionary of an XObject.
     *
     * @param array<string> $keys Array of font keys.
     *
     * @throws FontException if one of the keys is not a loaded font.
     */
    public function getOutFontDictByKeys(array $keys): string
    {
        if ($keys === []) {
            return '';
        }

        $data = [];
        foreach ($keys as $key) {
            if (!isset($this->fonts[$key])) {
                throw new FontException('The font ' . $key . ' has not been loaded');
            }

            $data[$key] = [
                'i' => $this->fonts[$key]['i'],
                'n' => $this->fonts[$key]['n'],
            ];
        }

        return $this->getOutFontResources($data);
    }

    /**
     * Get the PDF output string for font encoding diffs
     */
    protected function getEncodingDiffs(): string
    {
        $out = '';
        $done = []; // store processed items to avoid duplication
        foreach ($this->fonts as $fkey => $font) {
            if ($font['diff'] !== '') {
                $dkey = \md5($font['diff']);
                if (!isset($done[$dkey])) {
                    $out .=
                        ++$this->pon
                        . ' 0 obj'
                        . "\n"
                        . '<< /Type /Encoding /BaseEncoding /WinAnsiEncoding /Differences ['
                        . $font['diff']
                        . '] >>'
                        . "\n"
                        . 'endobj'
                        . "\n";
                    $done[$dkey] = $this->pon;
                }

                $this->fonts[$fkey]['diff_n'] = $done[$dkey];
            }

            // extract the character subset
            if ($font['file'] !== '') {
                $file_key = $this->fontFileKey($font);
                $this->filesubset[$file_key] = ($this->filesubset[$file_key] ?? true) && $font['subset'];
                // only the enabled entries are collected, as they are tested with isset()
                $this->subchars[$file_key] ??= [];
                foreach ($font['subsetchars'] as $cid => $enabled) {
                    if (!$enabled) {
                        continue;
                    }

                    $this->subchars[$file_key][(int) $cid] = true;
                }
            }
        }

        return $out;
    }

    /**
     * Returns the key identifying the font program file of a font.
     *
     * The directory is part of the key: two fonts may ship a different program under the
     * same file name.
     *
     * @param TFontData $font Font data.
     */
    protected function fontFileKey(array $font): string
    {
        return \md5($font['dir'] . '|' . $font['file']);
    }

    /**
     * Get the PDF output string for font files
     *
     * @throws FileException
     * @throws FontException
     */
    protected function getFontFiles(): string
    {
        $out = '';
        $done = []; // store processed items to avoid duplication
        foreach ($this->fonts as $fkey => $font) {
            if ($font['file'] === '') {
                // there is no embedded program to subset
                $this->fonts[$fkey]['subset'] = false;
                continue;
            }

            $dkey = $this->fontFileKey($font);
            if (!isset($done[$dkey])) {
                $fontfile = $this->getFontFullPath($font['dir'], $font['file']);
                $font_data = $this->fileHelper->getLocalFileData($fontfile);
                if ($font_data === false) {
                    throw new FontException('Unable to read font file: ' . $fontfile);
                }

                // a font stored by Import carries a '.z' suffix and is zlib compressed,
                // a linked font is the raw program
                $compressed = \str_ends_with($font['file'], '.z');

                // the file is shared, so the aggregated flag decides, not this font's own
                if ($this->filesubset[$dkey]) {
                    if ($compressed) {
                        // the expansion is bounded by the lengths the definition file records
                        // for the program, or unbounded when it records none
                        $font_data = Zlib::uncompress($font_data, $font['length1'] + $font['length2']);
                        if ($font_data === false) {
                            throw new FontException('Unable to uncompress font file: ' . $fontfile);
                        }
                    }

                    $subchars = $this->subchars[$dkey];
                    // the cache key is derived only when a cache is configured,
                    // as it hashes the whole font program
                    $cache = $this->subsetCache;
                    $cacheKey = '';
                    $subsetFont = null;
                    if ($cache !== null) {
                        $cacheKey = $this->subsetCacheKey($font_data, $font, $subchars);
                        $subsetFont = $cache->get($cacheKey);
                    }

                    if ($subsetFont === null) {
                        $sub = new Subset($font_data, $font, $this->fileHelper, $subchars);
                        $subsetFont = $sub->getSubsetFont();
                        $cache?->set($cacheKey, $subsetFont);
                    }

                    if ($subsetFont === $font_data) {
                        // the program was returned untouched, so the complete font is
                        // embedded and is not marked as a subset
                        $this->filesubset[$dkey] = false;
                    }

                    $font['length1'] = \strlen($subsetFont);
                    $font_data = $this->compressFontData($subsetFont, $fontfile);
                } elseif (!$compressed) {
                    if ($font['type'] !== 'Type1') {
                        // a Type1 stream keeps the /Length1 and /Length2 the import recorded
                        $font['length1'] = \strlen($font_data);
                    }

                    $font_data = $this->compressFontData($font_data, $fontfile);
                }

                ++$this->pon;
                $stream = $this->enc->encryptString($font_data, $this->pon);
                $out .=
                    $this->pon
                    . ' 0 obj'
                    . "\n"
                    . '<<'
                    . ' /Filter /FlateDecode'
                    . ' /Length '
                    . \strlen($stream)
                    . ' /Length1 '
                    . $font['length1'];
                if ($font['type'] === 'Type1') {
                    // Length2/Length3 are only valid for Type1 FontFile streams
                    $out .= ' /Length2 ' . $font['length2'] . ' /Length3 0';
                }

                $out .= ' >> stream' . "\n" . $stream . "\n" . 'endstream' . "\n" . 'endobj' . "\n";
                $done[$dkey] = $this->pon;
            }

            $this->fonts[$fkey]['file_n'] = $done[$dkey];
            // the program is emitted once for every font backed by it, so the aggregated
            // decision is recorded
            $this->fonts[$fkey]['subset'] = $this->filesubset[$dkey];
        }

        return $out;
    }

    /**
     * Compress a font program for a /FlateDecode stream.
     *
     * @param string $data     Font program to compress.
     * @param string $fontfile Font file name, used in the error message.
     *
     * @throws FontException if the data cannot be compressed.
     */
    protected function compressFontData(string $data, string $fontfile): string
    {
        return Zlib::compress($data, 'Unable to compress font file: ' . $fontfile);
    }

    /**
     * Build the cache key identifying a subset font program.
     *
     * The subset output is determined by the uncompressed font program bytes,
     * the cmap selection metrics (platform_id, encoding_id, type) and the
     * requested subset characters, so the key combines all of them.
     * The font program is fingerprinted with xxh128, a non-cryptographic hash
     * used for cache addressing only.
     *
     * @param string           $font_data Uncompressed font program bytes.
     * @param TFontData        $font      Extracted font metrics.
     * @param array<int, bool> $subchars  Subset characters (charcode => enabled).
     */
    protected function subsetCacheKey(string $font_data, array $font, array $subchars): string
    {
        \ksort($subchars);

        return (
            self::SUBSET_CACHE_KEY_PREFIX
            . \hash('xxh128', $font_data)
            . ':'
            . $font['platform_id']
            . ':'
            . $font['encoding_id']
            . ':'
            . $font['type']
            . ':'
            . \hash('xxh128', \implode(',', \array_keys(\array_filter($subchars))))
        );
    }

    /**
     * Get the PDF output string for fonts
     */
    protected function getFontDefinitions(): string
    {
        $out = '';
        foreach ($this->fonts as $font) {
            $out .= match (\strtolower($font['type'])) {
                'core' => $this->getCore($font),
                'cidfont0' => $this->getCid0($font),
                'type1' => $this->getTrueType($font),
                'truetype' => $this->getTrueType($font),
                'truetypeunicode' => $this->getTrueTypeUnicode($font),
                default => throw new FontException('Unsupported font type: ' . $font['type']),
            };
        }

        return $out;
    }
}
