<?php

declare(strict_types=1);

/**
 * Load.php
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

namespace Com\Tecnick\Pdf\Font;

use Com\Tecnick\File\Dir;
use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\Load
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-type TFontDataCidInfo array{
 *            'Ordering': string,
 *            'Registry': string,
 *            'Supplement': int,
 *            'uni2cid': array<int, int>,
 *        }
 *
 * @phpstan-type TFontDataDesc array{
 *            'Ascent': int,
 *            'AvgWidth': int,
 *            'CapHeight': int,
 *            'Descent': int,
 *            'Flags': int,
 *            'FontBBox': string,
 *            'ItalicAngle': int,
 *            'Leading': int,
 *            'MaxWidth': int,
 *            'MissingWidth': int,
 *            'StemH': int,
 *            'StemV': int,
 *            'XHeight': int,
 *        }
 *
 * @phpstan-type TFontDataEncTable array{
 *            'encodingID': int,
 *            'offset': int,
 *            'platformID': int,
 *        }
 *
 * @phpstan-type TFontDataMode array{
 *            'bold': bool,
 *            'italic': bool,
 *            'linethrough': bool,
 *            'overline': bool,
 *            'underline': bool,
 *        }
 *
 * @phpstan-type TFontDataTableItem array{
 *            'checkSum': int,
 *            'data': string,
 *            'length': int,
 *            'offset': int,
 *        }
 *
 * @phpstan-type TFontData array{
 *        'Ascender': int,
 *        'Ascent': int,
 *        'AvgWidth': float,
 *        'CapHeight': int,
 *        'CharacterSet': string,
 *        'Descender': int,
 *        'Descent': int,
 *        'EncodingScheme': string,
 *        'FamilyName': string,
 *        'Flags': int,
 *        'FontBBox': array<int>,
 *        'FontName': string,
 *        'FullName': string,
 *        'IsFixedPitch': bool,
 *        'ItalicAngle': int,
 *        'Leading': int,
 *        'MaxWidth': int,
 *        'MissingWidth': int|null,
 *        'StdHW': int,
 *        'StdVW': int,
 *        'StemH': int,
 *        'StemV': int,
 *        'UnderlinePosition': int,
 *        'UnderlineThickness': int,
 *        'Version': string,
 *        'Weight': string,
 *        'XHeight': int,
 *        'bbox': string,
 *        'cbbox': array<int, array<int, int>>,
 *        'cidinfo': TFontDataCidInfo,
 *        'compress': bool,
 *        'ctg': string,
 *        'ctgdata': array<int, int>,
 *        'cw':  array<int, int>,
 *        'cwu': array<int, int>,
 *        'datafile': string,
 *        'desc': TFontDataDesc,
 *        'diff': string,
 *        'diff_n': int,
 *        'diffid'?: int,
 *        'dir': string,
 *        'dw': int,
 *        'enc': string,
 *        'enc_map': array<int, string>,
 *        'encodingTables': array<int, TFontDataEncTable>,
 *        'encoding_id': int,
 *        'encrypted': string,
 *        'fakestyle': bool,
 *        'family': string,
 *        'file': string,
 *        'file_n': int,
 *        'file_name': string,
 *        'i': int,
 *        'ifile': string,
 *        'indexToLoc': array<int, int>,
 *        'input_file': string,
 *        'isUnicode': bool,
 *        'italicAngle': float,
 *        'key': string,
 *        'lenIV': int,
 *        'length1': int,
 *        'length2': int,
 *        'linked': bool,
 *        'mode': TFontDataMode,
 *        'n': int,
 *        'name': string,
 *        'numGlyphs': int,
 *        'numHMetrics': int,
 *        'originalsize': int,
 *        'pdfa': bool,
 *        'platform_id': int,
 *        'settype': string,
 *        'short_offset': bool,
 *        'size1': int,
 *        'size2': int,
 *        'style': string,
 *        'subset': bool,
 *        'subsetchars': array<int, bool>,
 *        'table': array<string, TFontDataTableItem>,
 *        'tot_num_glyphs': int,
 *        'type': string,
 *        'underlinePosition': int,
 *        'underlineThickness': int,
 *        'unicode': bool,
 *        'unitsPerEm': int,
 *        'up': int,
 *        'urk': float,
 *        'ut': int,
 *        'weight': string,
 *    }
 */
abstract class Load
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
     * Valid Font types
     *
     * @var array<string, bool> Font types
     */
    protected const FONTTYPES = [
        'Core' => true,
        'TrueType' => true,
        'TrueTypeUnicode' => true,
        'Type1' => true,
        'cidfont0' => true,
    ];

    /**
     * Font data
     *
     * @var TFontData
     */
    protected array $data = [
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
     * @param ObjFile|null $fileHelper Optional file helper for font loading.
     */
    public function __construct(?ObjFile $fileHelper = null)
    {
        $this->ownsFileHelper = $fileHelper === null;
        $this->fileHelper = $fileHelper ?? new ObjFile(allowedPaths: $this->buildAllowedPaths());
    }

    /**
     * Load the font data
     *
     * @throws FontException in case of error
     */
    public function load(): void
    {
        $this->getFontInfo();
        $this->checkType();
        $this->setName();
        $this->setDefaultWidth();
        if ($this->data['fakestyle']) {
            $this->setArtificialStyles();
        }

        $this->setFileData();
    }

    /**
     * Load the font data
     *
     * @throws FontException in case of error
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getFontInfo(): void
    {
        $this->findFontFile();

        if ($this->ownsFileHelper) {
            $this->fileHelper->setAllowedPaths($this->buildAllowedPaths());
        }

        // read the font definition file
        $fdt = $this->fileHelper->getFileData($this->data['ifile']);
        if ($fdt === false) {
            throw new FontException('unable to read file: ' . $this->data['ifile']);
        }

        /** @var array<string, mixed>|null $fdtdata */
        $fdtdata = \json_decode($fdt, true, 5, JSON_OBJECT_AS_ARRAY);
        if ($fdtdata === null) {
            throw new FontException('JSON decoding error [' . \json_last_error() . ']');
        }

        if (!isset($fdtdata['type'])) {
            throw new FontException('The font definition file has a bad format: ' . $this->data['ifile']);
        }

        $merged = \array_replace_recursive($this->data, $fdtdata);
        /** @var TFontData $merged */
        $this->data = $merged;
    }

    /**
     * Returns a list of font directories
     *
     * @return array<string> Font directories
     */
    protected function findFontDirectories(): array
    {
        $dir = new Dir();
        $dirs = [];
        if (\defined('K_PATH_FONTS')) {
            $kpathfonts = (string) \constant('K_PATH_FONTS');
            if ($kpathfonts !== '') {
                $dirs[] = $kpathfonts;
                $glb = \glob($kpathfonts . DIRECTORY_SEPARATOR . '*', GLOB_ONLYDIR);
                if ($glb !== false) {
                    $dirs = [...$dirs, ...$glb];
                }
            }
        }

        $parent_font_dir = $dir->findParentDir('fonts', __DIR__);
        if ($parent_font_dir !== '' && $parent_font_dir !== '/') {
            $dirs[] = $parent_font_dir;
            $glb = \glob($parent_font_dir . DIRECTORY_SEPARATOR . '*', GLOB_ONLYDIR);
            if ($glb !== false) {
                $dirs = \array_merge($dirs, $glb);
            }
        }

        return \array_unique($dirs);
    }

    /**
     * Build trusted roots for local font definition loading.
     *
     * @return array<string>
     */
    protected function buildAllowedPaths(): array
    {
        return FontPaths::buildAllowedPaths();
    }

    /**
     * Load the font data
     *
     * @throws FontException in case of error
     */
    protected function findFontFile(): void
    {
        if ($this->data['ifile'] !== '') {
            $this->data['dir'] = \dirname($this->data['ifile']);
            return;
        }

        $this->data['ifile'] = \strtolower($this->data['key']) . '.json';

        // find font definition file names
        $files = \array_unique([
            \strtolower($this->data['key']) . '.json',
            \strtolower($this->data['family']) . '.json',
        ]);

        // directories where to search for the font definition file
        $dirs = $this->findFontDirectories();

        foreach ($files as $file) {
            foreach ($dirs as $dir) {
                if (!\is_readable($dir . DIRECTORY_SEPARATOR . $file)) {
                    continue;
                }

                $this->data['ifile'] = $dir . DIRECTORY_SEPARATOR . $file;
                $this->data['dir'] = $dir;
                break 2;
            }

            // we have not found the version with style variations
            $this->data['fakestyle'] = true;
        }
    }

    protected function setDefaultWidth(): void
    {
        if ($this->data['dw'] !== 0) {
            return;
        }

        if ($this->data['desc']['MissingWidth'] > 0) {
            $this->data['dw'] = $this->data['desc']['MissingWidth'];
        } elseif (isset($this->data['cw'][32]) && $this->data['cw'][32] !== 0) {
            $this->data['dw'] = $this->data['cw'][32];
        } else {
            $this->data['dw'] = 600;
        }
    }

    /**
     * Check Font Type
     *
     * @throws FontException on unknown font type
     */
    protected function checkType(): void
    {
        if (isset(self::FONTTYPES[$this->data['type']])) {
            return;
        }

        throw new FontException('Unknown font type: ' . $this->data['type']);
    }

    /**
     * @return void
     *
     * @throws FontException on using a CID0 font in a pdfa
     */
    protected function setName(): void
    {
        if ($this->data['type'] === 'Core') {
            $this->data['name'] = Core::FONT[$this->data['key']] ?? $this->data['key'];
            $this->data['subset'] = false;
        } elseif ($this->data['type'] === 'Type1' || $this->data['type'] === 'TrueType') {
            $this->data['subset'] = false;
        } elseif ($this->data['type'] === 'TrueTypeUnicode') {
            $this->data['enc'] = 'Identity-H';
        } elseif ($this->data['type'] === 'cidfont0' && $this->data['pdfa']) {
            throw new FontException('CID0 fonts are not supported, all fonts must be embedded in PDF/A mode!');
        }

        if ($this->data['name'] === '') {
            $this->data['name'] = $this->data['key'];
        }
    }

    /**
     * Set artificial styles if the font variation file is missing
     */
    protected function setArtificialStyles(): void
    {
        // artificial bold
        if ($this->data['mode']['bold']) {
            $this->data['name'] .= 'Bold';
            $this->data['desc']['StemV'] = $this->data['desc']['StemV'] === 0
                ? 123
                : (int) \round($this->data['desc']['StemV'] * 1.75);
        }

        // artificial italic
        if ($this->data['mode']['italic']) {
            $this->data['name'] .= 'Italic';
            if ($this->data['desc']['ItalicAngle'] !== 0) {
                $this->data['desc']['ItalicAngle'] -= 11;
            } else {
                $this->data['desc']['ItalicAngle'] = -11;
            }

            if ($this->data['desc']['Flags'] !== 0) {
                $this->data['desc']['Flags'] |= 64; //bit 7
            } else {
                $this->data['desc']['Flags'] = 64;
            }
        }
    }

    public function setFileData(): void
    {
        if ($this->data['file'] === '') {
            return;
        }

        if (\str_contains($this->data['type'], 'TrueType')) {
            $this->data['length1'] = $this->data['originalsize'];
            $this->data['length2'] = 0;
        } elseif ($this->data['type'] !== 'Core') {
            $this->data['length1'] = $this->data['size1'];
            $this->data['length2'] = $this->data['size2'];
        }
    }
}
