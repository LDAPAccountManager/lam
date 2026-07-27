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
     * Namespace and schema-version prefix for subset cache keys.
     *
     * Bump the trailing version segment to invalidate previously cached
     * subsets whenever the subsetting algorithm or key format changes.
     */
    protected const SUBSET_CACHE_KEY_PREFIX = 'tc-lib-pdf-font:subset:v2:';

    /**
     * Array of character subsets for each font file
     *
     * @var array<string, array<int, bool>>
     */
    protected array $subchars = [];

    /**
     * PDF string block with the fonts definitions
     */
    protected string $out = '';

    /**
     * Initialize font data
     *
     * @param array<string, TFontData>     $fonts       Array of imported fonts data
     * @param int                          $pon         Current PDF Object Number
     * @param Encrypt                      $encrypt     Encrypt object
     * @param ObjFile                      $fileHelper  File helper for font loading.
     * @param FontSubsetCacheInterface     $subsetCache Optional cache for subset font programs.
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
        $this->fileHelper = $fileHelper ?? new ObjFile(allowedPaths: $this->buildAllowedPaths());

        $this->pon = $pon;
        $this->enc = $encrypt;

        $this->out = $this->getEncodingDiffs();
        $this->out .= $this->getFontFiles();
        $this->out .= $this->getFontDefinitions();
    }

    /**
     * Build trusted roots for local font file loading.
     *
     * @return array<string>
     */
    private function buildAllowedPaths(): array
    {
        return FontPaths::buildAllowedPaths();
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
     * Get the PDF output string for Font resources dictionary.
     *
     * @param array<string, TFontData|array{'i': int, 'n': int}> $data Font data.
     *
     * @return string
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
     * Get the PDF output string for Font resources dictionary.
     *
     * @return string
     */
    public function getOutFontDict(): string
    {
        return $this->getOutFontResources($this->fonts);
    }

    /**
     * Get the PDF output string for XOBject Font resources dictionary.
     *
     * @param array<string> $keys Array of font keys.
     *
     * @return string
     */
    public function getOutFontDictByKeys(array $keys): string
    {
        if ($keys === []) {
            return '';
        }

        $data = [];
        foreach ($keys as $key) {
            $data[$key] = [
                'i' => $this->fonts[$key]['i'],
                'n' => $this->fonts[$key]['n'],
            ];
        }

        return $this->getOutFontResources($data);
    }

    /**
     * Get the PDF output string for font encoding diffs
     *
     * @return string
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
                $file_key = \md5($font['file']);
                if (!isset($this->subchars[$file_key]) || $this->subchars[$file_key] === []) {
                    $this->subchars[$file_key] = $font['subsetchars'];
                } else {
                    foreach ($font['subsetchars'] as $cid => $enabled) {
                        if (!$enabled) {
                            continue;
                        }

                        $this->subchars[$file_key][(int) $cid] = true;
                    }
                }
            }
        }

        return $out;
    }

    /**
     * Get the PDF output string for font files
     *
     * @return string
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
                continue;
            }

            $dkey = \md5($font['file']);
            if (!isset($done[$dkey])) {
                $fontfile = $this->getFontFullPath($font['dir'], $font['file']);
                $font_data = $this->fileHelper->getLocalFileData($fontfile);
                if ($font_data === false) {
                    throw new FontException('Unable to read font file: ' . $fontfile);
                }

                if ($font['subset']) {
                    $font_data = \gzuncompress($font_data);
                    if ($font_data === false) {
                        throw new FontException('Unable to uncompress font file: ' . $fontfile);
                    }

                    $subchars = $this->subchars[$dkey];
                    // Only derive the cache key when a cache is configured: subsetCacheKey()
                    // hashes the whole (multi-MB) font program, which is pure waste otherwise.
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

                    $font_data = $subsetFont;
                    $font['length1'] = \strlen($font_data);
                    $font_data = \gzcompress($font_data);
                    if ($font_data === false) {
                        throw new FontException('Unable to compress font file: ' . $fontfile);
                    }
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
                    // Length2/Length3 are only valid for Type1 FontFile streams,
                    // not for TrueType (FontFile2) or CFF (FontFile3) programs.
                    $out .= ' /Length2 ' . $font['length2'] . ' /Length3 0';
                }

                $out .= ' >> stream' . "\n" . $stream . "\n" . 'endstream' . "\n" . 'endobj' . "\n";
                $done[$dkey] = $this->pon;
            }

            $this->fonts[$fkey]['file_n'] = $done[$dkey];
        }

        return $out;
    }

    /**
     * Build the cache key identifying a subset font program.
     *
     * The subset output is fully determined by the uncompressed font program
     * bytes, the cmap-selection metrics that drive glyph mapping
     * (platform_id, encoding_id, type) and the requested subset characters,
     * so the key combines all of them. The version prefix allows invalidating
     * stale entries if the subset algorithm changes.
     *
     * The font program (potentially several MB) is fingerprinted with xxh128:
     * this is a content hash purely for cache addressing, not a security
     * primitive, so a fast non-cryptographic 128-bit hash is sufficient and
     * keeps cache-hit lookups cheap.
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
     *
     * @return string
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
