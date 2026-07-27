<?php

declare(strict_types=1);

/**
 * OutFont.php
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
use Com\Tecnick\Pdf\Encrypt\Encrypt;
use Com\Tecnick\Pdf\Encrypt\Exception as EncException;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Unicode\Data\Identity;

/**
 * Com\Tecnick\Pdf\Font\OutFont
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
 * @phpstan-import-type TFontDataCidInfo from Load
 * @phpstan-import-type TFontDataDesc from Load
 */
abstract class OutFont extends \Com\Tecnick\Pdf\Font\OutUtil
{
    /**
     * Current PDF object number
     */
    protected int $pon;

    /**
     * Encrypt object
     */
    protected Encrypt $enc;

    /**
     * File helper used to load font files.
     */
    protected ObjFile $fileHelper;

    /**
     * Get the PDF output string for a CID-0 font.
     * A Type 0 CIDFont contains glyph descriptions based on the Adobe Type 1 font format
     *
     * @param TFontData $font Font to process
     *
     * @return string
     *
     * @throws EncException
     */
    protected function getCid0(array $font): string
    {
        $fontcw = $font['cw'];
        $fontname = $font['name'];
        $fontenc = $font['enc'];
        $fontn = $font['n'];
        $fonti = $font['i'];
        $fontdw = $font['dw'];
        $fontdesc = $font['desc'];
        $fontcidinfo = $font['cidinfo'];
        $cidregistry = $fontcidinfo['Registry'];
        $cidordering = $fontcidinfo['Ordering'];
        $cidsupplement = $fontcidinfo['Supplement'];

        $cidoffset = 0;
        if (!isset($fontcw[1])) {
            $cidoffset = 31;
        }

        $this->uniToCid($font, $cidoffset);
        $name = $fontname;
        $longname = $name;
        if ($fontenc !== '') {
            $longname .= '-' . $fontenc;
        }

        // obj 1
        $out =
            $fontn
            . ' 0 obj'
            . "\n"
            . '<</Type /Font'
            . ' /Subtype /Type0'
            . ' /BaseFont /'
            . $longname
            . ' /Name /F'
            . $fonti;
        if ($fontenc !== '') {
            $out .= ' /Encoding /' . $fontenc;
        }

        $out .= ' /DescendantFonts [' . ($this->pon + 1) . ' 0 R] >>' . "\n" . 'endobj' . "\n";

        // obj 2
        $out .= ++$this->pon . ' 0 obj' . "\n" . '<</Type /Font /Subtype /CIDFontType0 /BaseFont /' . $name;
        $cidinfo =
            '/Registry '
            . $this->enc->escapeDataString($cidregistry, $this->pon)
            . ' /Ordering '
            . $this->enc->escapeDataString($cidordering, $this->pon)
            . ' /Supplement '
            . $cidsupplement;
        $out .=
            ' /CIDSystemInfo <<'
            . $cidinfo
            . '>>'
            . ' /FontDescriptor '
            . ($this->pon + 1)
            . ' 0 R'
            . ' /DW '
            . $fontdw
            . "\n"
            . $this->getCharWidths($font, $cidoffset)
            . ' >>'
            . "\n"
            . 'endobj'
            . "\n";

        // obj 3
        $out .= ++$this->pon . ' 0 obj' . "\n" . '<</Type /FontDescriptor /FontName /' . $name;
        foreach ($fontdesc as $key => $val) {
            $out .= $this->getKeyValOut($key, $val);
        }

        return $out . ('>>' . "\n" . 'endobj' . "\n");
    }

    /**
     * Convert Unicode to CID
     *
     * @param TFontData $font Font to process
     * @param int    $cidoffset Offset for CID values
     */
    protected function uniToCid(array &$font, int $cidoffset): void
    {
        // convert unicode to cid.
        $fontcidinfo = $font['cidinfo'];
        $uni2cidraw = $fontcidinfo['uni2cid'];
        $uni2cid = [];
        foreach ($uni2cidraw as $uni => $cid) {
            $uni2cid[(int) $uni] = (int) $cid;
        }

        $fontcwraw = $font['cw'];
        $fontcw = [];
        foreach ($fontcwraw as $uni => $width) {
            $fontcw[(int) $uni] = (int) $width;
        }

        $chw = [];
        foreach ($fontcw as $uni => $width) {
            if (isset($uni2cid[$uni])) {
                $chw[$uni2cid[$uni] + $cidoffset] = $width;
            } elseif ($uni < 256) {
                $chw[$uni] = $width;
            } // else unknown character
        }

        foreach ($chw as $cid => $width) {
            $fontcw[$cid] = $width;
        }

        $font['cw'] = $fontcw;
    }

    /**
     * Get the PDF output string for a TrueTypeUnicode font.
     * Based on PDF Reference 1.3 (section 5)
     *
     * @param TFontData $font Font to process
     *
     * @return string
     *
     * @throws EncException
     * @throws FontException
     * @throws \RuntimeException
     *
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    protected function getTrueTypeUnicode(array $font): string
    {
        $fontsubset = $font['subset'];
        $fonti = $font['i'];
        $fontn = $font['n'];
        $fontenc = $font['enc'];
        $fontnamebase = $font['name'];
        $fontdw = $font['dw'];
        $fontctg = $font['ctg'];
        $fontdir = $font['dir'];
        $fontfilen = $font['file_n'];
        $fontdesc = $font['desc'];
        $fontcidinfo = $font['cidinfo'];

        $fontname = '';
        if ($fontsubset) {
            // change name for font subsetting
            $subtag = \sprintf('%06u', $fonti);
            $subtag = \strtr($subtag, '0123456789', 'ABCDEFGHIJ');
            $fontname .= $subtag . '+';
        }

        $fontname .= $fontnamebase;

        // Type0 Font
        // A composite font composed of other fonts, organized hierarchically

        // obj 1
        $out =
            $fontn
            . ' 0 obj'
            . "\n"
            . '<< /Type /Font'
            . ' /Subtype /Type0'
            . ' /BaseFont /'
            . $fontname
            . ' /Name /F'
            . $fonti
            . ' /Encoding /'
            . $fontenc
            . ' /ToUnicode '
            . ($this->pon + 1)
            . ' 0 R'
            . ' /DescendantFonts ['
            . ($this->pon + 2)
            . ' 0 R]'
            . ' >>'
            . "\n"
            . 'endobj'
            . "\n";

        // ToUnicode Object
        $out .= ++$this->pon . ' 0 obj' . "\n" . '<<';
        $cidhmap = Identity::CIDHMAP;
        if ($font['compress']) {
            $out .= ' /Filter /FlateDecode';
            $cidhmap = \gzcompress($cidhmap);
            if ($cidhmap === false) {
                throw new \RuntimeException('Unable to compress CIDHMAP');
            }
        }

        $stream = $this->enc->encryptString($cidhmap, $this->pon); // ToUnicode map for Identity-H
        $out .=
            ' /Length '
            . \strlen($stream)
            . ' >>'
            . ' stream'
            . "\n"
            . $stream
            . "\n"
            . 'endstream'
            . "\n"
            . 'endobj'
            . "\n";

        // CIDFontType2
        // A CIDFont whose glyph descriptions are based on TrueType font technology
        $out .= ++$this->pon . ' 0 obj' . "\n" . '<< /Type /Font /Subtype /CIDFontType2 /BaseFont /' . $fontname;
        // A dictionary containing entries that define the character collection of the CIDFont.
        $cidRegistry = $fontcidinfo['Registry'] === '' ? 'Adobe' : $fontcidinfo['Registry'];
        $cidOrdering = $fontcidinfo['Ordering'] === '' ? 'Identity' : $fontcidinfo['Ordering'];
        $cidinfo =
            '/Registry '
            . $this->enc->escapeDataString($cidRegistry, $this->pon)
            . ' /Ordering '
            . $this->enc->escapeDataString($cidOrdering, $this->pon)
            . ' /Supplement '
            . $fontcidinfo['Supplement'];
        $out .=
            ' /CIDSystemInfo << '
            . $cidinfo
            . ' >>'
            . ' /FontDescriptor '
            . ($this->pon + 1)
            . ' 0 R'
            . ' /DW '
            . $fontdw
            . "\n"
            . $this->getCharWidths($font, 0);
        if ($fontctg !== '') {
            $out .= "\n" . '/CIDToGIDMap ' . ($this->pon + 2) . ' 0 R';
        }

        $out .= ' >>' . "\n" . 'endobj' . "\n";

        // Font descriptor
        // A font descriptor describing the CIDFont default metrics other than its glyph widths
        $out .= ++$this->pon . ' 0 obj' . "\n" . '<< /Type /FontDescriptor /FontName /' . $fontname;
        foreach ($fontdesc as $key => $val) {
            $out .= $this->getKeyValOut($key, $val);
        }

        if ($fontfilen > 0) {
            // A stream containing a TrueType font
            $out .= ' /FontFile2 ' . $fontfilen . ' 0 R';
        }

        $out .= ' >>' . "\n" . 'endobj' . "\n";

        if ($fontctg !== '') {
            $out .= ++$this->pon . ' 0 obj' . "\n";
            // Embed CIDToGIDMap
            // A specification of the mapping from CIDs to glyph indices
            // search and get CTG font file to embed
            $ctgfile = \strtolower($fontctg);
            // search and get ctg font file to embed
            $fontfile = $this->getFontFullPath($fontdir, $ctgfile);
            $content = $this->fileHelper->getLocalFileData($fontfile);
            if ($content === false) {
                throw new FontException('Unable to read font file: ' . $fontfile);
            }

            $stream = $this->enc->encryptString($content, $this->pon);
            $out .= '<< /Length ' . \strlen($stream) . '';
            if (\str_ends_with($fontfile, '.z')) { // check file extension
                // Decompresses data encoded using the public-domain
                // zlib/deflate compression method, reproducing the
                // original text or binary data
                $out .= ' /Filter /FlateDecode';
            }

            $out .= ' >> stream' . "\n" . $stream . "\n" . 'endstream' . "\n" . 'endobj' . "\n";
        }

        return $out;
    }

    /**
     * Get the PDF output string for a Core font.
     *
     * @param TFontData $font Font to process
     *
     * @return string
     */
    protected function getCore(array $font): string
    {
        $fontn = $font['n'];
        $fontname = $font['name'];
        $fonti = $font['i'];
        $fontfamily = $font['family'];

        $out =
            $fontn
            . ' 0 obj'
            . "\n"
            . '<</Type /Font'
            . ' /Subtype /Type1'
            . ' /BaseFont /'
            . $fontname
            . ' /Name /F'
            . $fonti;
        if ($fontfamily !== 'symbol' && $fontfamily !== 'zapfdingbats') {
            $out .= ' /Encoding /WinAnsiEncoding';
        }

        return $out . (' >>' . "\n" . 'endobj' . "\n");
    }

    /**
     * Get the PDF output string for a TrueType font.
     *
     * @param TFontData $font Font to process
     *
     * @return string
     */
    protected function getTrueType(array $font): string
    {
        $fontname = $font['name'];
        $fonttype = $font['type'];
        $fonti = $font['i'];
        $fontn = $font['n'];
        $fontdw = $font['dw'];
        $fontfile = $font['file'];
        $fontfilen = $font['file_n'];
        $fontenc = $font['enc'];
        $fontdesc = $font['desc'];
        $fontcw = $font['cw'];

        // obj 1
        $out =
            $fontn
            . ' 0 obj'
            . "\n"
            . '<</Type /Font'
            . ' /Subtype /'
            . $fonttype
            . ' /BaseFont /'
            . $fontname
            . ' /Name /F'
            . $fonti
            . ' /FirstChar 32 /LastChar 255'
            . ' /Widths '
            . ($this->pon + 1)
            . ' 0 R'
            . ' /FontDescriptor '
            . ($this->pon + 2)
            . ' 0 R';
        if ($fontenc !== '') {
            if ($font['diff_n'] !== 0) {
                $out .= ' /Encoding ' . $font['diff_n'] . ' 0 R';
            } else {
                $out .= ' /Encoding /WinAnsiEncoding';
            }
        }

        $out .= ' >>' . "\n" . 'endobj' . "\n";

        // obj 2 - Widths
        $out .= ++$this->pon . ' 0 obj' . "\n" . '[';
        for ($idx = 32; $idx < 256; ++$idx) {
            if (isset($fontcw[$idx])) {
                $out .= (int) $fontcw[$idx] . ' ';
            } else {
                $out .= $fontdw . ' ';
            }
        }

        $out .= ']' . "\n" . 'endobj' . "\n";

        // obj 3 - Descriptor
        $out .= ++$this->pon . ' 0 obj' . "\n" . '<</Type /FontDescriptor /FontName /' . $fontname;
        foreach ($fontdesc as $fdk => $fdv) {
            $out .= $this->getKeyValOut($fdk, $fdv);
        }

        if ($fontfile !== '') {
            $out .= ' /FontFile' . ($fonttype === 'Type1' ? '' : '2') . ' ' . $fontfilen . ' 0 R';
        }

        return $out . ('>>' . "\n" . 'endobj' . "\n");
    }

    /**
     * Returns the formatted key/value PDF string
     *
     * @param string $key Key name
     * @param mixed  $val Value
     */
    protected function getKeyValOut(string $key, mixed $val): string
    {
        if (\is_float($val)) {
            $val = \sprintf('%F', $val);
        }

        return ' /' . $key . ' ' . (string) $val;
    }
}
