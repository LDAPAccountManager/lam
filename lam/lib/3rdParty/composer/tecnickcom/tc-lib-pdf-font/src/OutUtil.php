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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

use Com\Tecnick\File\Dir;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\OutUtil
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
        $dirobj = new Dir();
        $kpathfonts = \defined('K_PATH_FONTS') ? (string) \constant('K_PATH_FONTS') : '';
        // directories where to search for the font definition file
        // ('.' searches the current working directory)
        $dirs = \array_unique([
            '.',
            $fontdir,
            $kpathfonts,
            $dirobj->findParentDir('fonts', __DIR__),
        ]);
        foreach ($dirs as $dir) {
            if (\is_readable($dir . DIRECTORY_SEPARATOR . $file)) {
                return $dir . DIRECTORY_SEPARATOR . $file;
            }
        }

        throw new FontException('Unable to locate the file: ' . $file);
    }

    /**
     * Outputs font widths
     *
     * @param TFontData $font Font to process
     * @param int    $cidoffset Offset for CID values
     *
     * @return string PDF command string for font widths
     */
    protected function getCharWidths(array $font, int $cidoffset = 0): string
    {
        \ksort($font['cw']);
        $range = $this->getWidthRanges($font, $cidoffset);
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
     * get width ranges of characters
     *
     * @param TFontData $font Font to process
     * @param int    $cidoffset Offset for CID values
     *
     * @return array<int, array<int, int>>
     */
    protected function getWidthRanges(array $font, int $cidoffset = 0): array
    {
        $range = [];
        $rangeid = 0;
        $prevcid = -2;
        $prevwidth = -1;
        $interval = false;
        // for each character
        foreach ($font['cw'] as $cid => $width) {
            $cid -= $cidoffset;
            if ($font['subset'] && !isset($font['subsetchars'][$cid])) {
                // ignore the unused characters (font subsetting)
                continue;
            }

            if ($width !== $font['dw']) {
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
