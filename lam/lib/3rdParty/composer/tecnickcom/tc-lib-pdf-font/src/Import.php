<?php

declare(strict_types=1);

/**
 * Import.php
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
use Com\Tecnick\File\Dir;
use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\Import\Core;
use Com\Tecnick\Pdf\Font\Import\ProcessorInterface;
use Com\Tecnick\Pdf\Font\Import\TrueType;
use Com\Tecnick\Pdf\Font\Import\TypeOne;
use Com\Tecnick\Unicode\Data\Encoding;

/**
 * Com\Tecnick\Pdf\Font\Import
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
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
class Import
{
    /**
     * File helper used to load font definition files.
     */
    protected ObjFile $fileHelper;

    /**
     * True when the file helper is created internally by this class.
     */
    protected bool $ownsFileHelper = false;

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
     * Import the specified font and create output files.
     *
     * @param string $file        Font file to process
     * @param string $output_path Output path for generated font files (must be writeable by the web server).
     *                            Leave empty for the default font folder.
     * @param string|FontType $type Font type (or FontType enum case). Leave empty for autodetect mode.
     *                            Valid values are:
     *                            Core (AFM - Adobe Font Metrics) TrueTypeUnicode TrueType
     *                            Type1 CID0JP (CID-0 Japanese) CID0KR (CID-0 Korean) CID0CS
     *                            (CID-0 Chinese Simplified) CID0CT (CID-0 Chinese Traditional)
     * @param string $encoding    Name of the encoding table to use. Leave empty for default mode.
     *                            Omit this parameter for TrueType Unicode and symbolic fonts like
     *                            Symbol or ZapfDingBats.
     * @param int    $flags       Unsigned 32-bit integer containing flags specifying various characteristics
     *                            of the font as described in "PDF32000:2008 - 9.8.2 Font Descriptor Flags":
     *                            +1 for fixed width font +4 for symbol or +32 for non-symbol +64 for italic
     *                            Note: Fixed and Italic mode are generally autodetected, so you have to set
     *                            it to 32 = non-symbolic font (default) or 4 = symbolic font.
     * @param int    $platform_id Platform ID for CMAP table to extract.
     *                            For a Unicode font for Windows this
     *                            value should be 3, for Macintosh
     *                            should be 1.
     * @param int    $encoding_id Encoding ID for CMAP table to extract.
     *                            For a Unicode font for Windows this
     *                            value should be 1, for Macintosh
     *                            should be 0. When Platform ID is 3,
     *                            legal values for Encoding ID are: 0 =
     *                            Symbol, 1 = Unicode, 2 = ShiftJIS, 3 =
     *                            PRC, 4 = Big5, 5 = Wansung, 6 = Johab,
     *                            7 = Reserved, 8 = Reserved, 9 =
     *                            Reserved, 10 = UCS-4.
     * @param bool   $linked      If true, links the font file to system font instead of copying the font data
     *                            (not transportable). This option is unsupported for Type1 fonts.
     * @param ObjFile|null $fileHelper Optional file helper for font loading.
     *
     * @throws FileException in case of error
     * @throws FontException in case of error
     */
    public function __construct(
        string $file,
        string $output_path = '',
        string|FontType $type = '',
        string $encoding = '',
        int $flags = 32,
        int $platform_id = 3,
        int $encoding_id = 1,
        bool $linked = false,
        ?ObjFile $fileHelper = null,
    ) {
        if ($flags < 0 || $flags > 0xFFFF_FFFF) {
            // the descriptor flags are an unsigned 32-bit integer (ISO 32000-1 Table 122)
            throw new FontException('the font descriptor flags must fit 32 unsigned bits: ' . $flags);
        }

        $this->ownsFileHelper = $fileHelper === null;
        $this->fileHelper = $fileHelper ?? new ObjFile();
        $file = $this->fileHelper->resolveLocalPath($file);
        if ($this->ownsFileHelper) {
            $this->fileHelper->setAllowedPaths(self::buildAllowedPaths($file));
        }

        if (!$this->fileHelper->isAllowedFile($file)) {
            throw new FontException('Invalid font file name: ' . $file);
        }

        $this->fdt['input_file'] = $file;
        $this->fdt['file_name'] = $this->makeFontName($file);
        if ($this->fdt['file_name'] === '') {
            throw new FontException('the font name is empty');
        }

        $this->fdt['dir'] = $this->findOutputPath($output_path);
        if ($this->ownsFileHelper) {
            $this->fileHelper->setAllowedPaths(self::buildAllowedPaths($file, $this->fdt['dir']));
        }

        $this->fdt['datafile'] = $this->fdt['dir'] . $this->fdt['file_name'] . '.json';
        if (\file_exists($this->fdt['datafile'])) {
            throw new FontException('this font has been already imported: ' . $this->fdt['datafile']);
        }

        // get font data
        if (!is_file($file)) {
            throw new FontException('invalid font file: ' . $file);
        }

        if (($font = $this->fileHelper->getLocalFileData($file)) === false) {
            throw new FontException('unable to read the input font file: ' . $file);
        }

        $this->font = $font;

        $this->fbyte = new Byte($this->font);

        if ($type instanceof FontType) {
            $type = $type->value;
        }

        $this->fdt['settype'] = $type;
        $this->fdt['type'] = $this->getFontType($type);
        $this->fdt['isUnicode'] = $this->isUnicodeType($this->fdt['type']);
        $this->fdt['Flags'] = $flags;
        $this->initFlags();
        $this->fdt['enc'] = $this->getEncodingTable($encoding);
        // 'diff' is derived in saveFontData(), where the type is final
        $this->fdt['originalsize'] = \strlen($this->font);
        $this->fdt['ctg'] = $this->fdt['file_name'] . '.ctg.z';
        $this->fdt['platform_id'] = $platform_id;
        $this->fdt['encoding_id'] = $encoding_id;
        $this->fdt['linked'] = $linked;

        // the artifacts this import creates are removed when it does not complete
        $leftovers = $this->getArtifactPaths();

        try {
            try {
                /** @var ProcessorInterface $processor */
                $processor = match ($this->fdt['type']) {
                    'Core' => new Core(font: $this->font, fdt: $this->fdt, fileHelper: $this->fileHelper),
                    'Type1' => new TypeOne(font: $this->font, fdt: $this->fdt, fileHelper: $this->fileHelper),
                    default => new TrueType(
                        font: $this->font,
                        fdt: $this->fdt,
                        fileHelper: $this->fileHelper,
                        fbyte: $this->fbyte,
                    ),
                };
            } catch (\RangeException $exc) {
                // the byte reader ran past the end of a truncated or corrupt font program
                throw new FontException('Malformed font program: ' . $exc->getMessage(), 0, $exc);
            }

            $this->fdt = $processor->getFontMetrics();

            $this->saveFontData();
        } catch (\Throwable $exc) {
            self::removeArtifacts($leftovers);
            throw $exc;
        }
    }

    /**
     * Returns the artifacts of this import that are not on disk yet, indexed by full path.
     *
     * Files that already exist are excluded, since linked mode reuses an existing symbolic
     * link (see TrueType::setFontFile).
     *
     * @return array<string, true>
     */
    private function getArtifactPaths(): array
    {
        $names = [
            $this->fdt['file_name'] . '.z', // compressed font program
            $this->fdt['ctg'], // compressed CIDToGIDMap
            self::linkedFileName($this->fdt['file_name'], $this->fdt['input_file']), // linked font
            \basename($this->fdt['datafile']), // definition file
        ];

        $paths = [];
        foreach ($names as $name) {
            $path = $this->fdt['dir'] . $name;
            if (!\file_exists($path) && !\is_link($path)) {
                $paths[$path] = true;
            }
        }

        return $paths;
    }

    /**
     * Returns the name of the symbolic link that linked mode creates for a font program.
     *
     * The extension of the input file is carried over, reduced to lowercase alphanumeric
     * characters.
     *
     * @param string $fileName  Sanitized font name stem.
     * @param string $inputFile Path of the input font file.
     */
    public static function linkedFileName(string $fileName, string $inputFile): string
    {
        $ext = \preg_replace('/[^a-z0-9]/', '', \strtolower(\pathinfo($inputFile, PATHINFO_EXTENSION)));

        return $ext === null || $ext === '' ? $fileName : $fileName . '.' . $ext;
    }

    /**
     * Remove the artifacts of a failed import.
     *
     * @param array<string, true> $paths Full paths of the artifacts to remove.
     */
    private static function removeArtifacts(array $paths): void
    {
        foreach (\array_keys($paths) as $path) {
            // is_link() also answers for a link whose target is gone
            if (!\file_exists($path) && !\is_link($path)) {
                continue;
            }

            \unlink($path);
        }
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
     * Get the output font name
     */
    public function getFontName(): string
    {
        return $this->fdt['file_name'];
    }

    /**
     * Initialize font flags from font name
     */
    protected function initFlags(): void
    {
        $filename = \strtolower(\basename($this->fdt['input_file']));

        if (
            \str_contains($filename, 'mono')
            || \str_contains($filename, 'courier')
            || \str_contains($filename, 'fixed')
        ) {
            $this->fdt['Flags'] |= 1;
        }

        if (\str_contains($filename, 'symbol') || \str_contains($filename, 'zapfdingbats')) {
            $this->fdt['Flags'] |= 4;
        }

        if (\str_contains($filename, 'italic') || \str_contains($filename, 'oblique')) {
            $this->fdt['Flags'] |= 64;
        }

        // ISO 32000-1 Table 123: Symbolic (bit 3, 4) and Nonsymbolic (bit 6, 32) shall not
        // both be set nor both be clear; the symbolic one wins
        $this->fdt['Flags'] = ($this->fdt['Flags'] & 4) !== 0 ? $this->fdt['Flags'] & ~32 : $this->fdt['Flags'] | 32;
    }

    /**
     * Returns true if the path contains a stream wrapper or a parent directory reference.
     */
    private static function hasUnsafePath(string $path): bool
    {
        return (
            $path !== ''
            && (
                \str_contains($path, '://')
                || \str_contains(\str_ireplace('%2E', '.', \html_entity_decode($path, ENT_QUOTES, 'UTF-8')), '..')
            )
        );
    }

    /**
     * Build trusted roots for local file validation.
     *
     * The roots are the input font directory and, when available, the output directory.
     * Each root is listed both as given and, when resolvable, as its canonical realpath.
     *
     * The input root is derived from the caller-supplied font path, so this allowlist
     * confines the import to the font's own directory but is not a sandbox against a hostile
     * path: never pass a path taken from user input.
     *
     * @return array<string>
     */
    private static function buildAllowedPaths(string $fontFile, string $outputDir = ''): array
    {
        $roots = [];

        $fontDir = \dirname($fontFile);
        if ($fontDir !== '' && $fontDir !== '.') {
            $roots[] = $fontDir;
        }

        if ($outputDir !== '') {
            $roots[] = $outputDir;
        }

        $allowed = [];
        foreach ($roots as $root) {
            $normalized = \rtrim($root, '/\\');
            if ($normalized === '') {
                continue;
            }

            $allowed[] = $normalized;

            $resolved = \realpath($normalized);
            if ($resolved !== false) {
                $allowed[] = \rtrim($resolved, '/\\');
            }
        }

        return \array_values(\array_unique($allowed));
    }

    /**
     * Save the exported metadata font file
     *
     * @throws FileException
     * @throws FontException
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     */
    protected function saveFontData(): void
    {
        // re-derived here because a processor may downgrade 'TrueTypeUnicode' to 'TrueType'
        $this->fdt['isUnicode'] = $this->isUnicodeType($this->fdt['type']);
        $this->fdt['diff'] = $this->getEncodingDiff();

        $missingWidth = $this->fdt['MissingWidth'];
        $pfile =
            '{"type":"'
            . $this->fdt['type']
            . '"'
            . ',"name":"'
            . $this->fdt['name']
            . '"'
            . ',"up":'
            . $this->fdt['underlinePosition']
            . ',"ut":'
            . $this->fdt['underlineThickness']
            . ',"dw":'
            . ($missingWidth !== null && $missingWidth > 0 ? $missingWidth : $this->fdt['AvgWidth'])
            . ',"diff":"'
            . $this->fdt['diff']
            . '"'
            . ',"platform_id":'
            . $this->fdt['platform_id']
            . ',"encoding_id":'
            . $this->fdt['encoding_id'];

        if ($this->fdt['type'] === 'Core') {
            // Core
            $pfile .= ',"enc":""';
        } elseif ($this->fdt['type'] === 'Type1') {
            // Type 1
            $pfile .=
                ',"enc":"'
                . $this->fdt['enc']
                . '"'
                . ',"file":"'
                . $this->fdt['file']
                . '"'
                . ',"size1":'
                . $this->fdt['size1']
                . ',"size2":'
                . $this->fdt['size2'];
        } else {
            $pfile .= ',"originalsize":' . $this->fdt['originalsize'];
            if ($this->fdt['type'] === 'cidfont0') {
                // getFontType() only accepts a settype that is a key of this map
                $pfile .= ',' . UniToCid::TYPE[$this->fdt['settype']];
            } else {
                // TrueType
                $pfile .= ',"enc":"' . $this->fdt['enc'] . '","file":"' . $this->fdt['file'] . '"';
                $pfile .= $this->saveCIDToGIDMap();
            }
        }

        if ($this->fdt['isUnicode']) {
            $pfile .= ',"isUnicode":true';
        } else {
            $pfile .= ',"isUnicode":false';
        }

        $pfile .=
            ',"desc":{"Flags":'
            . $this->fdt['Flags']
            . ',"FontBBox":"['
            . $this->fdt['bbox']
            . ']"'
            . ',"ItalicAngle":'
            . $this->fdt['italicAngle']
            . ',"Ascent":'
            . $this->fdt['Ascent']
            . ',"Descent":'
            . $this->fdt['Descent']
            . ',"Leading":'
            . $this->fdt['Leading']
            . ',"CapHeight":'
            . $this->fdt['CapHeight']
            . ',"XHeight":'
            . $this->fdt['XHeight']
            . ',"StemV":'
            . $this->fdt['StemV']
            . ',"StemH":'
            . $this->fdt['StemH']
            . ',"AvgWidth":'
            . $this->fdt['AvgWidth']
            . ',"MaxWidth":'
            . $this->fdt['MaxWidth']
            . ',"MissingWidth":'
            . (string) ($missingWidth ?? 0)
            . '}';
        $pfile .= self::getBBoxMapJson('cbbox', $this->fdt['cbbox']);
        // the same boxes keyed by codepoint, as 'cwu' does for 'cw'
        $pfile .= self::getBBoxMapJson('cbboxu', $this->fdt['cbboxu']);
        $pfile .= self::getWidthMapJson('cw', $this->fdt['cw']);
        $pfile .= self::getWidthMapJson('cwu', $this->fdt['cwu']);

        $pfile .= '}' . "\n";

        // store file
        FileWriter::write($this->fileHelper, $this->fdt['datafile'], $pfile);
    }

    /**
     * Store the CIDToGIDMap artifact of a TrueType font and return the definition file
     * members that describe it.
     *
     * The artifact is written only for a 'TrueTypeUnicode' font; for any other type 'ctg'
     * is cleared.
     *
     * @throws FileException
     * @throws FontException
     */
    private function saveCIDToGIDMap(): string
    {
        if ($this->fdt['type'] !== 'TrueTypeUnicode') {
            $this->fdt['ctg'] = '';
            return '';
        }

        $out = ',"ctg":"' . $this->fdt['ctg'] . '"';
        $cidtogidmap = \str_pad('', 131_072, "\x00"); // (256 * 256 * 2) = 131072
        $ctgustr = '';
        foreach ($this->fdt['ctgdata'] as $cid => $gid) {
            if ($cid > 0xFFFF) {
                // codepoints above the BMP do not fit the 16-bit CIDToGIDMap table
                // and are stored in the definition file
                $ctgustr .= ',"' . $cid . '":' . (int) $gid;
                continue;
            }

            $this->updateCIDtoGIDmap($cidtogidmap, (int) $cid, (int) $gid);
        }

        if ($ctgustr !== '') {
            $out .= ',"ctgu":{' . \substr($ctgustr, 1) . '}';
        }

        // store compressed CIDToGIDMap
        FileWriter::write(
            $this->fileHelper,
            $this->fdt['dir'] . $this->fdt['ctg'],
            Zlib::compress($cidtogidmap, 'unable to compress CIDToGIDMap'),
        );

        return $out;
    }

    /**
     * Returns the definition file member of a glyph bounding box map, or an empty string
     * when the map holds nothing.
     *
     * @param string                      $key Name of the member.
     * @param array<int, array<int, int>> $map Bounding boxes indexed by character code.
     */
    private static function getBBoxMapJson(string $key, array $map): string
    {
        if ($map === []) {
            return '';
        }

        $out = '';
        foreach ($map as $cid => $bbox) {
            $box = \array_pad(\array_values($bbox), 4, 0);
            $out .= ',"' . $cid . '":[' . $box[0] . ',' . $box[1] . ',' . $box[2] . ',' . $box[3] . ']';
        }

        return ',"' . $key . '":{' . \substr($out, 1) . '}';
    }

    /**
     * Returns the definition file member of a character width map, or an empty string
     * when the map holds nothing.
     *
     * @param string          $key Name of the member.
     * @param array<int, int> $map Widths indexed by character code.
     */
    private static function getWidthMapJson(string $key, array $map): string
    {
        if ($map === []) {
            return '';
        }

        $out = '';
        foreach ($map as $cid => $width) {
            $out .= ',"' . $cid . '":' . $width;
        }

        return ',"' . $key . '":{' . \substr($out, 1) . '}';
    }

    /**
     * Make the output font name
     *
     * @param string $font_file Input font file
     *
     * @throws FontException
     */
    protected function makeFontName(string $font_file): string
    {
        $font_path_parts = \pathinfo($font_file);
        if ($font_path_parts['filename'] === '') {
            throw new FontException('Invalid font file name: ' . $font_file);
        }

        $fname = \preg_replace('/[^a-z0-9_]/', '', \strtolower($font_path_parts['filename']));
        if ($fname === null) {
            throw new FontException('Invalid font file name: ' . $font_file);
        }

        return \str_replace(['bold', 'oblique', 'italic', 'regular'], ['b', 'i', 'i', ''], $fname);
    }

    /**
     * Find the path where to store the processed font.
     *
     * @param string $output_path Output path for generated font files (must be writeable by the web server).
     *                            Leave empty for the default font folder (K_PATH_FONTS).
     *
     * @throws FontException if an explicit output path cannot be written to.
     */
    protected function findOutputPath(string $output_path = ''): string
    {
        if ($output_path !== '') {
            // is_writable() is also true for a writable regular file, hence the is_dir() check
            if (self::hasUnsafePath($output_path) || !\is_dir($output_path) || !\is_writable($output_path)) {
                throw new FontException('The output path is not a writable directory: ' . $output_path);
            }

            return self::withTrailingSlash($output_path);
        }

        if (\defined('K_PATH_FONTS')) {
            $kpathfonts = (string) \constant('K_PATH_FONTS');
            if ($kpathfonts !== '' && \is_writable($kpathfonts)) {
                return self::withTrailingSlash($kpathfonts);
            }
        }

        $dirobj = new Dir();
        $dir = $dirobj->findParentDir('fonts', __DIR__);
        if ($dir === '') {
            $dir = \sys_get_temp_dir();
        }

        return self::withTrailingSlash($dir);
    }

    /**
     * Normalize a directory so it can be concatenated with a bare file name.
     *
     * @param string $dir Directory path.
     */
    private static function withTrailingSlash(string $dir): string
    {
        return \str_ends_with($dir, '/') ? $dir : $dir . '/';
    }

    /**
     * Get the font type
     *
     * @param string $font_type Font type. Leave empty for autodetect mode.
     *
     * @throws FontException
     */
    protected function getFontType(string $font_type): string
    {
        // autodetect font type
        if ($font_type === '') {
            if (\str_starts_with($this->font, 'StartFontMetrics')) {
                // AFM type, used only for the 14 Core fonts
                return 'Core';
            }

            // formats carrying a signature this library cannot read
            $unsupported = [
                'OTTO' => 'OpenType with CFF data',
                'ttcf' => 'TrueType Collection',
                'wOFF' => 'WOFF',
                'wOF2' => 'WOFF2',
                'true' => 'legacy Macintosh sfnt',
                'typ1' => 'sfnt-housed Type1',
                '%!PS' => 'Type1 in ASCII (PFA) form, only the binary (PFB) form is read',
            ];
            foreach ($unsupported as $signature => $description) {
                if (\str_starts_with($this->font, $signature)) {
                    throw new FontException('Unsupported font format: ' . $description);
                }
            }

            if (\strlen($this->font) < 4) {
                // too short to carry a sfnt version
                throw new FontException('Unable to detect the font type: the file is too short');
            }

            if ($this->fbyte->getULong(0) === 0x1_0000) {
                return 'TrueTypeUnicode';
            }

            return 'Type1';
        }

        if (\str_starts_with($font_type, 'CID0')) {
            // only the known collections have a CIDSystemInfo block to emit
            if (!isset(UniToCid::TYPE[$font_type])) {
                throw new FontException('unknown or unsupported CID-0 font type: ' . $font_type);
            }

            return 'cidfont0';
        }

        return FontType::fromLoose($font_type)->value;
    }

    /**
     * Get the encoding table
     *
     * @param string $encoding Name of the encoding table to use. Leave empty for default mode.
     *                         Omit this parameter for TrueType Unicode and symbolic fonts like
     *                         Symbol or ZapfDingBats.
     *
     * @throws FontException
     */
    protected function getEncodingTable(string $encoding = ''): string
    {
        if ($encoding === '') {
            if ($this->fdt['type'] === 'Type1' && ($this->fdt['Flags'] & 4) === 0) {
                return 'cp1252';
            }

            return '';
        }

        $enc = \preg_replace('/[^A-Za-z0-9_\-]/', '', $encoding);
        if ($enc === null) {
            throw new FontException('Invalid encoding name: ' . $encoding);
        }

        if (!isset(Encoding::MAP[$enc])) {
            throw new FontException('Unknown encoding name: ' . $encoding);
        }

        return $enc;
    }

    /**
     * If required, get differences between the reference encoding (cp1252) and the current encoding
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getEncodingDiff(): string
    {
        $diff = '';
        if (
            ($this->fdt['type'] === 'TrueType' || $this->fdt['type'] === 'Type1')
            && ($this->fdt['enc'] !== '' && $this->fdt['enc'] !== 'cp1252')
        ) {
            // build differences from reference encoding
            $enc_ref = Encoding::MAP['cp1252'];
            $enc_target = Encoding::MAP[$this->fdt['enc']];
            $last = 0;
            for ($idx = 32; $idx <= 255; ++$idx) {
                $target = $enc_target[$idx] ?? '';
                $ref = $enc_ref[$idx] ?? '';
                if ($target === $ref) {
                    continue;
                }

                if ($idx !== ($last + 1)) {
                    $diff .= $idx . ' ';
                }

                $last = $idx;
                $diff .= '/' . $target . ' ';
            }
        }

        return $diff;
    }

    /**
     * Update the CIDToGIDMap string with a new value
     *
     * The CIDToGIDMap is made up of 16-bit big-endian values mapping a zero-based
     * Character Identifier index to its zero-based glyph id index.
     *
     * @param string $map CIDToGIDMap (binary), modified in place.
     * @param int    $cid CID value.
     * @param int    $gid GID value.
     */
    protected function updateCIDtoGIDmap(string &$map, int $cid, int $gid): void
    {
        // entries outside the 0..0xFFFF range are left as 0 (notdef)
        if ($cid >= 0 && $cid <= 0xFFFF && $gid >= 0 && $gid <= 0xFFFF) {
            $map[$cid * 2] = \chr($gid >> 8);
            $map[($cid * 2) + 1] = \chr($gid & 0xFF);
        }
    }

    /**
     * Returns true when the given resolved font type addresses glyphs by Unicode codepoint.
     *
     * @param string $type Resolved font type.
     */
    private function isUnicodeType(string $type): bool
    {
        return $type === 'TrueTypeUnicode' || $type === 'cidfont0';
    }
}
