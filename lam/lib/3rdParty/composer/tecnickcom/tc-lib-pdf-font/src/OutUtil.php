<?php

declare(strict_types=1);

/**
 * OutUtil.php
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

use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\OutUtil
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
abstract class OutUtil
{
    /**
     * Return font full path
     *
     * @param string $fontdir Original font directory
     * @param string $file    Font file name.
     *
     * @return string Font full path or empty string
     *
     * @throws FontException
     */
    protected function getFontFullPath(string $fontdir, string $file): string
    {
        $path = FontPaths::findFontFile($fontdir, $file);
        if ($path === '') {
            throw new FontException('Unable to locate the file: ' . $file);
        }

        return $path;
    }

    /**
     * Outputs font widths
     *
     * @param TFontData $font      Font to process
     * @param int       $cidoffset Offset for CID values
     *
     * @return string PDF command string for font widths
     */
    protected function getCharWidths(array $font, int $cidoffset = 0): string
    {
        \ksort($font['cw']);
        return $this->formatWidthRanges($this->getWidthRanges($font, $cidoffset));
    }

    /**
     * Outputs the font widths of a CID keyed font whose character codes are glyph indices.
     *
     * Only the glyphs used by the document are listed, as no other code can
     * appear in a content stream.
     *
     * @param TFontData $font Font to process
     *
     * @return string PDF command string for font widths
     */
    protected function getGidWidths(array $font): string
    {
        $widths = [];
        foreach ($font['usedgid'] as $gid => $ord) {
            $widths[(int) $gid] = self::normalizeWidth($font['cw'][$ord] ?? $font['dw']);
        }

        \ksort($widths);
        return $this->formatWidthRanges($this->buildWidthRanges($widths, self::normalizeWidth($font['dw'])));
    }

    /**
     * Returns a glyph width as the integer the /W and /Widths arrays are made of.
     *
     * @param mixed $width Raw width read from the font definition.
     */
    protected static function normalizeWidth(mixed $width): int
    {
        return (int) \round(\is_numeric($width) ? (float) $width : 0.0);
    }

    /**
     * Format the width ranges as a PDF /W array
     *
     * @param array<int, array<int, int>> $range Width ranges
     *
     * @return string PDF command string for font widths
     */
    protected function formatWidthRanges(array $range): string
    {
        // output data
        $wdt = '';
        foreach ($range as $kdx => $wds) {
            if (\count(\array_count_values($wds)) === 1) {
                // interval mode is more compact
                $wdt .= ' ' . $kdx . ' ' . ($kdx + \count($wds) - 1) . ' ' . $wds[0];
            } else {
                // range mode
                $wdt .= ' ' . $kdx . ' [ ' . \implode(' ', $wds) . ' ]';
            }
        }

        return '/W [' . $wdt . ' ]';
    }

    /**
     * Get the width ranges of the characters
     *
     * @param TFontData $font      Font to process
     * @param int       $cidoffset Offset for CID values
     *
     * @return array<int, array<int, int>>
     */
    protected function getWidthRanges(array $font, int $cidoffset = 0): array
    {
        $widths = [];
        foreach ($font['cw'] as $cid => $width) {
            $cid -= $cidoffset;
            if ($cid < 0) {
                // a character code below the offset has no CID
                continue;
            }

            if ($font['subset'] && !isset($font['subsetchars'][$cid])) {
                // ignore the unused characters (font subsetting)
                continue;
            }

            $widths[$cid] = self::normalizeWidth($width);
        }

        return $this->buildWidthRanges($widths, self::normalizeWidth($font['dw']));
    }

    /**
     * Build the width ranges of a CID to width map
     *
     * @param array<int, int> $widths Character widths indexed by CID.
     * @param int             $dwt    Default width.
     *
     * @return array<int, array<int, int>>
     */
    protected function buildWidthRanges(array $widths, int $dwt): array
    {
        $range = [];
        $rangeid = 0;
        $prevcid = -2;
        $prevwidth = -1;
        $interval = false;
        // for each character
        foreach ($widths as $cid => $width) {
            if ($width === $dwt) {
                // the default width applies
                continue;
            }

            if ($cid === ($prevcid + 1)) {
                // consecutive CID
                if ($width === $prevwidth) {
                    if ($width === $range[$rangeid][0]) {
                        $range[$rangeid][] = $width;
                    } else {
                        \array_pop($range[$rangeid]);
                        // new range
                        $rangeid = $prevcid;
                        $range[$rangeid] = [];
                        $range[$rangeid][] = $prevwidth;
                        $range[$rangeid][] = $width;
                    }

                    $interval = true;
                    $range[$rangeid][-1] = -1;
                } else {
                    if ($interval) {
                        // new range
                        $rangeid = $cid;
                        $range[$rangeid] = [];
                        $range[$rangeid][] = $width;
                    } else {
                        $range[$rangeid][] = $width;
                    }

                    $interval = false;
                }
            } else {
                // new range
                $rangeid = $cid;
                $range[$rangeid] = [];
                $range[$rangeid][] = $width;
                $interval = false;
            }

            $prevcid = $cid;
            $prevwidth = $width;
        }

        /** @var array<int, array<int, int>> $range */
        return $this->optimizeWidthRanges($range);
    }

    /**
     * Optimize width ranges
     *
     * @param array<int, array<int, int>> $range Width Ranges
     *
     * @return array<int, array<int, int>>
     */
    protected function optimizeWidthRanges(array $range): array
    {
        $prevk = -1;
        $nextk = -1;
        $prevint = false;
        foreach ($range as $kdx => $wds) {
            $cws = \count($wds);
            if ($kdx === $nextk && !$prevint && (!isset($wds[-1]) || $cws < 4)) {
                unset($range[$kdx][-1]);
                $range[$prevk] = [...$range[$prevk], ...$range[$kdx]];
                unset($range[$kdx]);
            } else {
                $prevk = $kdx;
            }

            $prevint = false;
            $nextk = $kdx + $cws;
            if (isset($wds[-1])) {
                unset($range[$kdx][-1]);
                $prevint = $cws > 3;
                --$nextk;
            }
        }

        return $range;
    }
}
