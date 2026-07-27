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
     * Store font data
     *
     *  @throws FileException
     *  @throws FontException
     */
    protected function storeFontData(): void
    {
        // read first segment
        $dat = \unpack('Cmarker/Ctype/Vsize', \substr($this->font, 0, 6));
        if ($dat === false || $dat['marker'] !== 128) {
            throw new FontException('Font file is not a valid binary Type1');
        }

        $this->fdt['size1'] = $dat['size'];
        $fontlen = \strlen($this->font);
        // the first segment plus the 6-byte header of the second segment must fit in the file
        if ((6 + $this->fdt['size1'] + 6) > $fontlen) {
            throw new FontException('Type1 font segment 1 length exceeds the file size');
        }

        $data = \substr($this->font, 6, $this->fdt['size1']);
        // read second segment
        $dat = \unpack('Cmarker/Ctype/Vsize', \substr($this->font, 6 + $this->fdt['size1'], 6));
        if ($dat === false || $dat['marker'] !== 128) {
            throw new FontException('Font file is not a valid binary Type1');
        }

        $this->fdt['size2'] = $dat['size'];
        if ((12 + $this->fdt['size1'] + $this->fdt['size2']) > $fontlen) {
            throw new FontException('Type1 font segment 2 length exceeds the file size');
        }

        $this->fdt['encrypted'] = \substr($this->font, 12 + $this->fdt['size1'], $this->fdt['size2']);
        $data .= $this->fdt['encrypted'];
        // store compressed font
        $this->fdt['file'] = $this->fdt['file_name'] . '.z';
        $fpt = $this->fileHelper->fopenLocal($this->fdt['dir'] . $this->fdt['file'], 'wb');

        $cmpr = \gzcompress($data);
        if ($cmpr === false) {
            throw new FontException('Unable to compress font data');
        }

        \fwrite($fpt, $cmpr);
        \fclose($fpt);
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
            \preg_match('#/FontName[\s]*+\/([^\s]*+)#', $this->font, $matches) !== 1
            && \preg_match('#/FullName[\s]*+\(([^\)]*+)#', $this->font, $matches) !== 1
        ) {
            throw new FontException('Unable to extract font name');
        }

        $name = \preg_replace('/[^a-zA-Z0-9_\-]/', '', $matches[1]);
        if ($name === null) {
            throw new FontException('Unable to extract font name');
        }

        $this->fdt['name'] = $name;

        $bvl = [0, 0, 0, 0];
        if (\preg_match('#/FontBBox[\s]*+{([^}]*+)#', $this->font, $matches) === 1) {
            $rawbvl = \explode(' ', \trim($matches[1]));
            $bvl = [
                (int) $rawbvl[0],
                (int) ($rawbvl[1] ?? 0),
                (int) ($rawbvl[2] ?? 0),
                (int) ($rawbvl[3] ?? 0),
            ];
        }

        $this->fdt['bbox'] = \implode(' ', $bvl);
        $this->fdt['Ascent'] = $bvl[3];
        $this->fdt['Descent'] = $bvl[1];

        $this->fdt['italicAngle'] = \preg_match('#/ItalicAngle[\s]*+([0-9\+\-]*+)#', $this->font, $matches) === 1
            ? (int) $matches[1]
            : 0;

        if ($this->fdt['italicAngle'] !== 0) {
            $this->fdt['Flags'] |= 64;
        }

        $this->fdt['underlinePosition'] = \preg_match('#/UnderlinePosition[\s]*+([0-9\+\-]*+)#', $this->font, $matches)
        === 1
            ? (int) $matches[1]
            : 0;
        $this->fdt['underlineThickness'] = \preg_match(
            '#/UnderlineThickness[\s]*+([0-9\+\-]*+)#',
            $this->font,
            $matches,
        ) === 1
            ? (int) $matches[1]
            : 0;

        if (\preg_match('#/isFixedPitch[\s]*+([^\s]*+)#', $this->font, $matches) === 1 && $matches[1] === 'true') {
            $this->fdt['Flags'] = (int) $this->fdt['Flags'] | 1;
        }

        $this->fdt['weight'] = 'Book';
        if (\preg_match('#/Weight[\s]*+\(([^\)]*+)#', $this->font, $matches) === 1 && $matches[1] !== '') {
            $this->fdt['weight'] = \strtolower($matches[1]);
        }

        $this->fdt['Leading'] = 0;
    }

    /**
     * Extract Font information
     *
     * @return array<string, int>
     */
    protected function getInternalMap(): array
    {
        $imap = [];
        $fmap = [];
        $matches = \preg_match_all('#dup[\s]([0-9]+)[\s]*+/([^\s]*+)[\s]put#sU', $this->font, $fmap, PREG_SET_ORDER);
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
            $bvl = \explode(' ', $matches[1]);
            if (\count($bvl) >= 6) {
                $vl1 = (int) $bvl[2];
                $vl2 = (int) $bvl[4];
                $this->fdt['XHeight'] = \min($vl1, $vl2);
                $this->fdt['CapHeight'] = \max($vl1, $vl2);
            }
        }

        $this->getRandomBytes($eplain);
        return $this->getCharstringData($eplain);
    }

    /**
     * Extract eexec info
     *
     * @param string $eplain Decoded eexec encrypted part
     */
    protected function extractStem(string $eplain): void
    {
        $matches = [];
        if (\preg_match('#/StdVW[\s]*+\[([^\]]*+)#', $eplain, $matches) === 1) {
            $this->fdt['StemV'] = (int) $matches[1];
        } elseif ($this->fdt['weight'] === 'bold' || $this->fdt['weight'] === 'black') {
            $this->fdt['StemV'] = 123;
        } else {
            $this->fdt['StemV'] = 70;
        }

        $this->fdt['StemH'] = \preg_match('#/StdHW[\s]*+\[([^\]]*+)#', $eplain, $matches) === 1
            ? (int) $matches[1]
            : 30;

        if (\preg_match('#/Cap[X]?Height[\s]*+\[([^\]]*+)#', $eplain, $matches) === 1) {
            $this->fdt['CapHeight'] = (int) $matches[1];
        } else {
            $this->fdt['CapHeight'] = (int) $this->fdt['Ascent'];
        }

        $this->fdt['XHeight'] = (int) $this->fdt['Ascent'] + (int) $this->fdt['Descent'];
    }

    /**
     * Get the number of random bytes at the beginning of charstrings
     */
    protected function getRandomBytes(string $eplain): void
    {
        $this->fdt['lenIV'] = 4;
        $matches = [];
        if (\preg_match('#/lenIV[\s]*+([\d]*+)#', $eplain, $matches) === 1) {
            $this->fdt['lenIV'] = (int) $matches[1];
        }
    }

    /**
     * @return array<int, array<int, string>>
     */
    protected function getCharstringData(string $eplain): array
    {
        $this->fdt['enc_map'] = [];
        $charstringsPos = \strpos($eplain, '/CharStrings');
        if ($charstringsPos === false) {
            return [];
        }

        $eplain = \substr($eplain, $charstringsPos + 1);
        $matches = [];
        \preg_match_all('#/([A-Za-z0-9\.]*+)[\s][0-9]+[\s]RD[\s](.*)[\s]ND#sU', $eplain, $matches, PREG_SET_ORDER);
        /** @var array<int, array<int, string>> $matches */
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
     * get CID
     *
     * @param array<string, int> $imap
     * @param array<int, string> $val
     */
    protected function getCid(array $imap, array $val): int
    {
        if (isset($imap[$val[1]])) {
            return $imap[$val[1]];
        }

        $cid = \array_search($val[1], $this->fdt['enc_map'], true);

        if ($cid === false) {
            return 0;
        }

        if ($cid > 1000) {
            return 1000;
        }

        return (int) $cid;
    }

    /**
     * Decode number
     *
     * @param array<int, int> $ccom
     * @param array<int, int> $cdec
     * @param array<int, int> $cwidths
     *
     * @throws FontException
     */
    protected function decodeNumber(int $idx, int &$cck, int &$cid, array &$ccom, array &$cdec, array &$cwidths): int
    {
        if ($ccom[$idx] === 255) {
            if (!isset($ccom[$idx + 4])) {
                throw new FontException('Truncated Type1 charstring number operand');
            }

            $sval = \chr($ccom[$idx + 1]) . \chr($ccom[$idx + 2]) . \chr($ccom[$idx + 3]) . \chr($ccom[$idx + 4]);
            $vsval = \unpack('li', $sval);
            if ($vsval === false) {
                throw new FontException('Unable to unpack number');
            }

            $cdec[$cck] = (int) $vsval['i'];
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
        if ($cck <= 0) {
            return ++$idx;
        }

        if ($cdec[$cck] !== 13) {
            return ++$idx;
        }

        // hsbw command: update width
        $cwidths[$cid] = $cdec[$cck - 1];
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
        $cc1 = 52_845;
        $cc2 = 22_719;
        foreach ($matches as $match) {
            $cid = $this->getCid($imap, $match);
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
            while ($idx < $clen) {
                $idx = $this->decodeNumber($idx, $cck, $cid, $ccom, $cdec, $cwidths);
                ++$cck;
            }
        }

        $this->setCharWidths($cwidths);
    }
}
