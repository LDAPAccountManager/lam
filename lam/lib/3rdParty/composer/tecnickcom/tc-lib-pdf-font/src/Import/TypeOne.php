<?php

declare(strict_types=1);

/**
 * TypeOne.php
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

use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\FileWriter;
use Com\Tecnick\Pdf\Font\Zlib;
use Com\Tecnick\Unicode\Data\Encoding;

/**
 * Com\Tecnick\Pdf\Font\Import\TypeOne
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
class TypeOne extends \Com\Tecnick\Pdf\Font\Import\Core
{
    /**
     * True when the Private dict of the font states '/CapHeight' itself.
     */
    private bool $hasDeclaredHeights = false;

    /**
     * Clear text portion of the font program, isolated by storeFontData().
     */
    private string $clear = '';

    /**
     * Returns the part of the font program the PostScript directives are read from.
     *
     * The clear text header holds the font info and the internal encoding map, so the eexec
     * encrypted portion is left out of the scan. The whole program is returned while the
     * segments have not been read yet.
     */
    private function clearText(): string
    {
        return $this->clear === '' ? $this->font : $this->clear;
    }

    /**
     * Store font data
     *
     *  @throws FileException
     *  @throws FontException
     */
    protected function storeFontData(): void
    {
        [$clear, $encrypted] = $this->readPfbSegments();

        // the ASCII header and the eexec encrypted portion are both required
        if ($clear === '' || $encrypted === '') {
            throw new FontException('Font file is not a valid binary Type1');
        }

        $this->clear = $clear;
        $this->fdt['size1'] = \strlen($clear);
        $this->fdt['size2'] = \strlen($encrypted);
        $this->fdt['encrypted'] = $encrypted;

        // store compressed font
        $this->fdt['file'] = $this->fdt['file_name'] . '.z';
        FileWriter::write(
            $this->fileHelper,
            $this->fdt['dir'] . $this->fdt['file'],
            Zlib::compress($clear . $encrypted, 'Unable to compress font data'),
        );
    }

    /**
     * Read the PFB segments and return the clear-text and the eexec encrypted portions.
     *
     * A PFB file is a sequence of [0x80, type, uint32 length] segments: type 1 is ASCII,
     * type 2 is binary and type 3 marks the end of the file. The eexec data may span several
     * type-2 segments, so every segment is read. The ASCII trailer that follows the binary
     * portion is dropped and emitted as '/Length3 0'.
     *
     * @return array{0: string, 1: string} Clear-text portion and encrypted portion.
     *
     * @throws FontException if the segment structure is not valid.
     */
    private function readPfbSegments(): array
    {
        $fontlen = \strlen($this->font);
        $clear = '';
        $encrypted = '';
        $pos = 0;
        while ($pos < $fontlen) {
            if (($pos + 2) > $fontlen || \ord($this->font[$pos]) !== 128) {
                throw new FontException('Font file is not a valid binary Type1');
            }

            $type = \ord($this->font[$pos + 1]);
            if ($type === 3) {
                break; // end of file marker: it carries no length field
            }

            if ($type !== 1 && $type !== 2 || ($pos + 6) > $fontlen) {
                throw new FontException('Font file is not a valid binary Type1');
            }

            /** @var array{'size': int} $dat */
            $dat = \unpack('Vsize', \substr($this->font, $pos + 2, 4));
            $size = $dat['size'];
            if (($pos + 6 + $size) > $fontlen) {
                throw new FontException('Type1 font segment ' . $type . ' length exceeds the file size');
            }

            if ($type === 2) {
                $encrypted .= \substr($this->font, $pos + 6, $size);
            } elseif ($encrypted === '') {
                $clear .= \substr($this->font, $pos + 6, $size);
            } else {
                break; // the ASCII trailer that closes the font is not embedded
            }

            $pos += 6 + $size;
        }

        return [$clear, $encrypted];
    }

    /**
     * Extract Font information
     *
     * @throws FontException
     */
    protected function extractFontInfo(): void
    {
        $matches = [];
        if (
            \preg_match('#/FontName[\s]*+\/([^\s]*+)#', $this->clearText(), $matches) !== 1
            && \preg_match('#/FullName[\s]*+\(([^\)]*+)#', $this->clearText(), $matches) !== 1
        ) {
            throw new FontException('Unable to extract font name');
        }

        $name = \preg_replace('/[^a-zA-Z0-9_\-]/', '', $matches[1]);
        if ($name === null || $name === '') {
            throw new FontException('Unable to extract font name');
        }

        $this->fdt['name'] = $name;

        $bvl = [0, 0, 0, 0];
        if (\preg_match('#/FontBBox[\s]*+{([^}]*+)#', $this->clearText(), $matches) === 1) {
            // the four values may be separated by any run of whitespace, newlines included
            $split = \preg_split('/\s+/', \trim($matches[1]), -1, PREG_SPLIT_NO_EMPTY);
            $rawbvl = \is_array($split) ? $split : [];
            $bvl = [
                (int) ($rawbvl[0] ?? 0),
                (int) ($rawbvl[1] ?? 0),
                (int) ($rawbvl[2] ?? 0),
                (int) ($rawbvl[3] ?? 0),
            ];
        }

        $this->fdt['bbox'] = \implode(' ', $bvl);
        $this->fdt['Ascent'] = $bvl[3];
        $this->fdt['Descent'] = $bvl[1];

        // the italic angle is a real number, so the decimal point is part of the pattern
        $this->fdt['italicAngle'] = \preg_match('#/ItalicAngle[\s]*+([0-9\+\-\.]*+)#', $this->clearText(), $matches)
        === 1
            ? self::roundedValue($matches[1])
            : 0;

        if ($this->fdt['italicAngle'] !== 0) {
            $this->fdt['Flags'] |= 64;
        }

        $this->fdt['underlinePosition'] = \preg_match(
            '#/UnderlinePosition[\s]*+([0-9\+\-]*+)#',
            $this->clearText(),
            $matches,
        ) === 1
            ? (int) $matches[1]
            : 0;
        $this->fdt['underlineThickness'] = \preg_match(
            '#/UnderlineThickness[\s]*+([0-9\+\-]*+)#',
            $this->clearText(),
            $matches,
        ) === 1
            ? (int) $matches[1]
            : 0;

        if (
            \preg_match('#/isFixedPitch[\s]*+([^\s]*+)#', $this->clearText(), $matches) === 1
            && $matches[1] === 'true'
        ) {
            $this->fdt['Flags'] = (int) $this->fdt['Flags'] | 1;
        }

        $this->fdt['weight'] = 'Book';
        if (\preg_match('#/Weight[\s]*+\(([^\)]*+)#', $this->clearText(), $matches) === 1 && $matches[1] !== '') {
            $this->fdt['weight'] = \strtolower($matches[1]);
        }

        $this->fdt['Leading'] = 0;
    }

    /**
     * Returns the internal encoding map (glyph name to character code)
     *
     * @return array<string, int>
     */
    protected function getInternalMap(): array
    {
        $imap = [];
        $fmap = [];
        $matches = \preg_match_all(
            // the separators are runs of whitespace: an entry written as 'dup  32 /space put'
            // is as valid as 'dup 32/space put'
            '#dup[\s]++([0-9]++)[\s]*+/([^\s]*+)[\s]++put#s',
            $this->clearText(),
            $fmap,
            PREG_SET_ORDER,
        );
        if ($matches !== false && $matches >= 1) {
            foreach ($fmap as $val) {
                $imap[$val[2]] = (int) $val[1];
            }
        }

        return $imap;
    }

    /**
     * Decrypt eexec encrypted part
     */
    protected function getEplain(): string
    {
        $csr = 55_665; // eexec encryption constant
        $cc1 = 52_845;
        $cc2 = 22_719;
        $elen = \strlen($this->fdt['encrypted']);
        $eplain = '';
        for ($idx = 0; $idx < $elen; ++$idx) {
            $chr = \ord($this->fdt['encrypted'][$idx]);
            $eplain .= \chr($chr ^ ($csr >> 8));
            $csr = ((($chr + $csr) * $cc1) + $cc2) % 65_536;
        }

        return $eplain;
    }

    /**
     * Extract eexec info
     *
     * @return array<int, array<int, string>>
     */
    protected function extractEplainInfo(): array
    {
        $eplain = $this->getEplain();
        $matches = [];
        if (\preg_match('#/ForceBold[\s]*+([^\s]*+)#', $eplain, $matches) === 1 && $matches[1] === 'true') {
            $this->fdt['Flags'] |= 0x4_0000;
        }

        $this->extractStem($eplain);
        if (\preg_match('#/BlueValues[\s]*+\[([^\]]*+)#', $eplain, $matches) === 1) {
            // the values may be separated by any run of whitespace
            $split = \preg_split('/\s+/', \trim($matches[1]), -1, PREG_SPLIT_NO_EMPTY);
            $bvl = \is_array($split) ? $split : [];
            // the blue zones only apply when the Private dict declares no height
            if (\count($bvl) >= 6 && !$this->hasDeclaredHeights) {
                $vl1 = (int) $bvl[2];
                $vl2 = (int) $bvl[4];
                $this->fdt['XHeight'] = \min($vl1, $vl2);
                $this->fdt['CapHeight'] = \max($vl1, $vl2);
            }
        }

        $this->readLenIV($eplain);
        return $this->getCharstringData($eplain);
    }

    /**
     * Extract the stem and height metrics
     *
     * @param string $eplain Decoded eexec encrypted part
     */
    protected function extractStem(string $eplain): void
    {
        $matches = [];
        $this->fdt['StemV'] = \preg_match('#/StdVW[\s]*+\[([^\]]*+)#', $eplain, $matches) === 1
            ? (int) $matches[1]
            : $this->getDefaultStemV();

        $this->fdt['StemH'] = \preg_match('#/StdHW[\s]*+\[([^\]]*+)#', $eplain, $matches) === 1
            ? (int) $matches[1]
            : 30;

        // '/CapHeight' is written as a plain number ('/CapHeight 700 def'), unlike the
        // '/StdVW' and '/StdHW' arrays above; both spellings are accepted here
        $this->hasDeclaredHeights = \preg_match('#/CapHeight[\s]*+\[?[\s]*+([-+]?[0-9]++)#', $eplain, $matches) === 1;
        $this->fdt['CapHeight'] = $this->hasDeclaredHeights ? (int) $matches[1] : (int) $this->fdt['Ascent'];

        $this->fdt['XHeight'] = (int) $this->fdt['Ascent'] + (int) $this->fdt['Descent'];
    }

    /**
     * Read the number of leading random bytes of each charstring (the '/lenIV' entry).
     *
     * The Type1 specification defines the default as 4 for a font that does not declare it.
     *
     * @param string $eplain Decoded eexec encrypted part
     */
    protected function readLenIV(string $eplain): void
    {
        $this->fdt['lenIV'] = 4;
        $matches = [];
        // an entry without a non-negative value keeps the default
        if (\preg_match('#/lenIV[\s]++([\d]++)#', $eplain, $matches) === 1) {
            $this->fdt['lenIV'] = (int) $matches[1];
        }
    }

    /**
     * Returns the charstring entries and set the encoding map
     *
     * @param string $eplain Decoded eexec encrypted part
     *
     * @return array<int, array<int, string>>
     *
     * @throws FontException if the charstrings cannot be scanned
     */
    protected function getCharstringData(string $eplain): array
    {
        $this->fdt['enc_map'] = [];
        $charstringsPos = \strpos($eplain, '/CharStrings');
        if ($charstringsPos === false) {
            return [];
        }

        $eplain = \substr($eplain, $charstringsPos + 1);
        $matches = $this->scanCharstrings($eplain);

        if ($this->fdt['enc'] === '') {
            return $matches;
        }

        if (!isset(Encoding::MAP[$this->fdt['enc']])) {
            return $matches;
        }

        $this->fdt['enc_map'] = Encoding::MAP[$this->fdt['enc']];
        return $matches;
    }

    /**
     * Scan the '/name length RD <binary> ND' entries of a CharStrings dictionary.
     *
     * 'RD' and 'ND' are locally defined procedure names, also spelled '-|' and '|-', so both
     * conventions are accepted. Only the entry header is matched, and the declared byte count
     * delimits the binary data.
     *
     * @param string $eplain Decoded eexec encrypted part, from the CharStrings dictionary on.
     *
     * @return array<int, array<int, string>> Entries as [full match, glyph name, charstring].
     *
     * @throws FontException if the charstrings cannot be scanned
     */
    private function scanCharstrings(string $eplain): array
    {
        $entries = [];
        $offset = 0;
        $found = [];
        while (true) {
            // a PostScript name is any run of characters other than whitespace and the
            // delimiters, '_' and '-' included (ligature names such as 'f_i')
            $res = \preg_match(
                '#/([^\s/{}\[\]()<>%]*+)[\s]([0-9]++)[\s](?:RD|-\|)[\s]#',
                $eplain,
                $found,
                PREG_OFFSET_CAPTURE,
                $offset,
            );
            if ($res === false) {
                throw new FontException('Unable to parse the Type1 charstrings');
            }

            if ($res !== 1) {
                return $entries;
            }

            $length = (int) $found[2][0];
            $start = (int) $found[0][1] + \strlen($found[0][0]);
            $charstring = \substr($eplain, $start, $length);
            if (\strlen($charstring) !== $length) {
                // the declared length runs past the end of a truncated dictionary
                return $entries;
            }

            $entries[] = [
                0 => '',
                1 => $found[1][0],
                2 => $charstring,
            ];
            $offset = $start + $length;
        }
    }

    /**
     * Returns every character code a charstring glyph name is encoded at.
     *
     * An encoding may give one name more than one code (ISO 32000-1 Annex D.2), so every
     * code is reported. The codes are those of the encoding the emitted font declares, and
     * those of the built-in encoding array of the program only when it declares none.
     *
     * @param array<string, int> $imap Internal encoding map
     * @param array<int, string> $val  Charstring match (name and encrypted data)
     *
     * @return array<int, int> The character codes, in ascending order, empty when the glyph
     *                         is not encoded.
     */
    protected function getCids(array $imap, array $val): array
    {
        if ($val[1] === '.notdef') {
            // '.notdef' names the fallback glyph, not a character
            return [];
        }

        // the declared encoding answers first, as the /Widths of the emitted font are
        // indexed by it; a Type1 font cannot address a code above the single-byte range
        $cids = [];
        foreach (\array_keys($this->fdt['enc_map'], $val[1], true) as $cid) {
            if ($cid < 0 || $cid > 255) {
                continue;
            }

            $cids[] = $cid;
        }

        if ($cids !== []) {
            \sort($cids);
            return $cids;
        }

        // the declared encoding does not name this glyph, so the built-in array of the
        // program answers; it is the only source when no encoding is declared
        if (isset($imap[$val[1]])) {
            $own = $imap[$val[1]];
            return $own >= 0 && $own <= 255 ? [$own] : [];
        }

        return [];
    }

    /**
     * Decode a charstring number operand
     *
     * @param int             $idx     Index of the current byte in the decrypted charstring
     * @param int             $cck     Index of the decoded value
     * @param int             $cid     Character code of the current charstring
     * @param array<int, int> $ccom    Decrypted charstring bytes
     * @param array<int, int> $cdec    Decoded charstring values
     * @param array<int, int> $cwidths Character widths indexed by character code
     *
     * @return int Index of the next byte to process
     *
     * @throws FontException
     */
    protected function decodeNumber(int $idx, int $cck, int $cid, array $ccom, array &$cdec, array &$cwidths): int
    {
        if ($ccom[$idx] === 255) {
            if (!isset($ccom[$idx + 4])) {
                throw new FontException('Truncated Type1 charstring number operand');
            }

            // a 255 operand is a 32-bit big-endian two's complement value
            $uval = ($ccom[$idx + 1] << 24) | ($ccom[$idx + 2] << 16) | ($ccom[$idx + 3] << 8) | $ccom[$idx + 4];
            $cdec[$cck] = $uval >= 0x8000_0000 ? $uval - 0x1_0000_0000 : $uval;
            return $idx + 5;
        }

        if ($ccom[$idx] >= 251) {
            if (!isset($ccom[$idx + 1])) {
                throw new FontException('Truncated Type1 charstring number operand');
            }

            $cdec[$cck] = (-($ccom[$idx] - 251) * 256) - $ccom[$idx + 1] - 108;
            return $idx + 2;
        }

        if ($ccom[$idx] >= 247) {
            if (!isset($ccom[$idx + 1])) {
                throw new FontException('Truncated Type1 charstring number operand');
            }

            $cdec[$cck] = (($ccom[$idx] - 247) * 256) + $ccom[$idx + 1] + 108;
            return $idx + 2;
        }

        if ($ccom[$idx] >= 32) {
            $cdec[$cck] = $ccom[$idx] - 139;
            return ++$idx;
        }

        $cdec[$cck] = $ccom[$idx];
        if ($ccom[$idx] === 12) {
            // an escaped command is the two byte sequence '12 n', both consumed here
            if (!isset($ccom[$idx + 1])) {
                throw new FontException('Truncated Type1 charstring escaped command');
            }

            if ($ccom[$idx + 1] === 7 && $cck >= 4) {
                // sbw command: 'sbx sby wx wy sbw', the horizontal width is the third
                // of the four operands
                $cwidths[$cid] = $cdec[$cck - 2];
            }

            return $idx + 2;
        }

        if ($ccom[$idx] !== 13) {
            return ++$idx;
        }

        if ($cck >= 2) {
            // hsbw command: 'sbx wx hsbw', the width is the second of the two operands
            $cwidths[$cid] = $cdec[$cck - 1];
        }

        return ++$idx;
    }

    /**
     * Process Type1 font
     *
     * @throws FileException
     * @throws FontException
     */
    protected function process(): void
    {
        $this->storeFontData();
        $this->extractFontInfo();
        $imap = $this->getInternalMap();
        $matches = $this->extractEplainInfo();
        $cwidths = [];
        $glyphNames = [];
        $cc1 = 52_845;
        $cc2 = 22_719;
        foreach ($matches as $match) {
            $cids = $this->getCids($imap, $match);
            if ($cids === []) {
                // the glyph has no character code, so it has no width to record either
                continue;
            }

            // the charstring is decoded once, under the lowest code the name is given
            $cid = $cids[0];
            $glyphNames[$cid] = $match[1];

            // decrypt charstring encrypted part
            $csr = 4330; // charstring encryption constant
            $ccd = $match[2];
            $clen = \strlen($ccd);
            $ccom = [];
            for ($idx = 0; $idx < $clen; ++$idx) {
                $chr = \ord($ccd[$idx]);
                $ccom[] = $chr ^ ($csr >> 8);
                $csr = ((($chr + $csr) * $cc1) + $cc2) % 65_536;
            }

            // decode numbers
            $cdec = [];
            $cck = 0;
            $idx = $this->fdt['lenIV'];
            try {
                while ($idx < $clen) {
                    $idx = $this->decodeNumber($idx, $cck, $cid, $ccom, $cdec, $cwidths);
                    ++$cck;
                }
            } catch (FontException $exc) {
                // a truncated operand ends this charstring only, keeping the width already
                // recorded for it
                unset($exc);
            }

            // the same outline is reached through every code the encoding gives the name
            if (isset($cwidths[$cid])) {
                foreach (\array_slice($cids, 1) as $alias) {
                    $cwidths[$alias] = $cwidths[$cid];
                }
            }
        }

        $this->setCharWidths($cwidths);
        $this->setUnicodeCharWidths($cwidths, $glyphNames);
    }

    /**
     * Record the glyph widths under the Unicode codepoint of their glyph name.
     *
     * A Type1 font is emitted with a single-byte encoding, where the character code of a
     * glyph is not its codepoint. The map is left empty for a font emitted without an
     * encoding, whose codes are its own.
     *
     * @param array<int, int>    $cwidths    Character widths indexed by character code.
     * @param array<int, string> $glyphNames Glyph names indexed by character code.
     */
    protected function setUnicodeCharWidths(array $cwidths, array $glyphNames): void
    {
        if ($this->fdt['enc'] === '') {
            return;
        }

        $cwu = [];
        foreach ($glyphNames as $cid => $name) {
            $unicode = self::getGlyphUnicode($name);
            if ($unicode === null || !isset($cwidths[$cid])) {
                continue;
            }

            $cwu[$unicode] = $cwidths[$cid];
        }

        \ksort($cwu);
        $this->fdt['cwu'] = $cwu;
    }
}
