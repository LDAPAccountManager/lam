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
        $name = $this->enc->encodeNameObject($fontname);
        $longname = $name;
        if ($fontenc !== '') {
            $longname .= '-' . $this->enc->encodeNameObject($fontenc);
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
            $out .= ' /Encoding /' . $this->enc->encodeNameObject($fontenc);
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
     * @param TFontData $font      Font to process
     * @param int       $cidoffset Offset for CID values
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

        if ($uni2cid === []) {
            // without a CID mapping the widths are already keyed as they are emitted
            return;
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

        // the map is replaced, not merged: the surviving keys are CIDs
        $font['cw'] = $chw;
    }

    /**
     * Get the PDF output string for a TrueTypeUnicode font.
     * Based on PDF Reference 1.3 (section 5)
     *
     * @param TFontData $font Font to process
     *
     * @throws EncException
     * @throws FontException
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
            // subset tag: exactly six uppercase letters followed by '+' (ISO 32000-1 9.6.4)
            $subtag = \sprintf('%06u', $fonti % 1_000_000);
            $subtag = \strtr($subtag, '0123456789', 'ABCDEFGHIJ');
            $fontname .= $subtag . '+';
        }

        $fontname .= $this->enc->encodeNameObject($fontnamebase);

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
            . $this->enc->encodeNameObject($fontenc)
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
        // a GID encoded font needs a specific map, as its character codes are glyph indices
        $cidhmap = $font['gidenc'] ? $this->getToUnicodeCMap($font) : Identity::CIDHMAP;
        if ($font['compress']) {
            $out .= ' /Filter /FlateDecode';
            $cidhmap = Zlib::compress($cidhmap, 'Unable to compress CIDHMAP');
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
            . ($font['gidenc'] ? $this->getGidWidths($font) : $this->getCharWidths($font, 0));
        if ($font['gidenc']) {
            // the character codes are the glyph indices themselves
            $out .= "\n" . '/CIDToGIDMap /Identity';
        } elseif ($fontctg !== '') {
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

        if ($fontctg !== '' && !$font['gidenc']) {
            $out .= ++$this->pon . ' 0 obj' . "\n";
            // embed the CIDToGIDMap: the mapping from CIDs to glyph indices
            $ctgfile = \strtolower($fontctg);
            $fontfile = $this->getFontFullPath($fontdir, $ctgfile);
            $content = $this->fileHelper->getLocalFileData($fontfile);
            if ($content === false) {
                throw new FontException('Unable to read font file: ' . $fontfile);
            }

            $stream = $this->enc->encryptString($content, $this->pon);
            $out .= '<< /Length ' . \strlen($stream) . '';
            if (\str_ends_with($fontfile, '.z')) {
                $out .= ' /Filter /FlateDecode';
            }

            $out .= ' >> stream' . "\n" . $stream . "\n" . 'endstream' . "\n" . 'endobj' . "\n";
        }

        return $out;
    }

    /**
     * Build the ToUnicode CMap of a font whose character codes are glyph indices.
     *
     * Only the glyphs used by the document are listed, as no other code can
     * appear in a content stream.
     *
     * @param TFontData $font Font to process
     */
    protected function getToUnicodeCMap(array $font): string
    {
        $usedgid = $font['usedgid'];
        \ksort($usedgid);

        $out =
            '/CIDInit /ProcSet findresource begin'
            . "\n"
            . '12 dict begin'
            . "\n"
            . 'begincmap'
            . "\n"
            . '/CIDSystemInfo << /Registry (Adobe) /Ordering (UCS) /Supplement 0 >> def'
            . "\n"
            . '/CMapName /Adobe-Identity-UCS def'
            . "\n"
            . '/CMapType 2 def'
            . "\n"
            . '/WMode 0 def'
            . "\n"
            . '1 begincodespacerange'
            . "\n"
            . '<0000> <FFFF>'
            . "\n"
            . 'endcodespacerange'
            . "\n";

        // a bfchar section holds at most 100 entries
        foreach (\array_chunk($usedgid, 100, true) as $chunk) {
            $out .= \count($chunk) . ' beginbfchar' . "\n";
            foreach ($chunk as $gid => $ord) {
                $out .= \sprintf("<%04x> <%s>\n", $gid, $this->getUtf16beHex((int) $ord));
            }

            $out .= 'endbfchar' . "\n";
        }

        return $out . 'endcmap' . "\n" . 'CMapName currentdict /CMap defineresource pop' . "\n" . 'end' . "\n" . 'end';
    }

    /**
     * Returns the hexadecimal UTF-16BE representation of a codepoint.
     *
     * Codepoints above the BMP are expanded to their surrogate pair.
     *
     * @param int $ord Unicode codepoint.
     */
    protected function getUtf16beHex(int $ord): string
    {
        if ($ord >= 0xD800 && $ord <= 0xDFFF) {
            // a lone surrogate code unit is not a codepoint
            return \sprintf('%04x', 0xFFFD);
        }

        if ($ord < 0x1_0000) {
            return \sprintf('%04x', \max(0, $ord));
        }

        // a value above the last Unicode codepoint has no UTF-16 form
        $ord = \min($ord, 0x10_FFFF) - 0x1_0000;
        return \sprintf('%04x%04x', 0xD800 + ($ord >> 10), 0xDC00 + ($ord & 0x3FF));
    }

    /**
     * Get the PDF output string for a Core font.
     *
     * @param TFontData $font Font to process
     */
    protected function getCore(array $font): string
    {
        $fontn = $font['n'];
        $fontname = $this->enc->encodeNameObject($font['name']);
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
     */
    protected function getTrueType(array $font): string
    {
        $fontname = $this->enc->encodeNameObject($font['name']);
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
        $defaultwidth = self::normalizeWidth($fontdw);
        for ($idx = 32; $idx < 256; ++$idx) {
            $out .= (isset($fontcw[$idx]) ? self::normalizeWidth($fontcw[$idx]) : $defaultwidth) . ' ';
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
     * A value that is not a number or a non empty string is dropped. The key is escaped as a
     * PDF name (ISO 32000-1 7.3.5), and is dropped when it escapes to an empty name.
     *
     * @param int|string $key Key name.
     * @param mixed      $val Value
     */
    protected function getKeyValOut(int|string $key, mixed $val): string
    {
        if (\is_float($val)) {
            $val = \sprintf('%F', $val);
        } elseif (\is_int($val)) {
            $val = (string) $val;
        } elseif (!\is_string($val) || $val === '') {
            return '';
        }

        $name = $this->enc->encodeNameObject((string) $key);
        if ($name === '') {
            return '';
        }

        return ' /' . $name . ' ' . $val;
    }
}
