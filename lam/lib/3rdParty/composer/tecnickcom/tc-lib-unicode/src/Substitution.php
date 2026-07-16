<?php

declare(strict_types=1);

/**
 * Substitution.php
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Com\Tecnick\Unicode;

use Com\Tecnick\Unicode\Data\ScriptRanges;
use Com\Tecnick\Unicode\Substitution\Devanagari as DevanagariHandler;
use Com\Tecnick\Unicode\Substitution\Hangul as HangulHandler;
use Com\Tecnick\Unicode\Substitution\Thai as ThaiHandler;

/**
 * Com\Tecnick\Unicode\Substitution
 *
 * Top-level entry point for context-sensitive Unicode character substitution.
 *
 * Detects which scripts are present in the codepoint array in a single pass,
 * then applies the matching per-script handler(s) in sequence:
 * Thai → Devanagari → Hangul.
 *
 * Codepoints belonging to unsupported or unrecognised scripts are passed
 * through unmodified. The method never discards input it cannot classify.
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class Substitution
{
    /**
     * Applies script-specific character substitutions to the codepoint array.
     *
     * Performs one pass to detect active scripts, then one pass per active
     * handler. Empty input is returned as-is.
     *
     * @param array<int, int> $ordarr Array of Unicode codepoints.
     *
     * @return array<int, int> Transformed codepoint array.
     */
    public function replaceChars(array $ordarr): array
    {
        if ($ordarr === []) {
            return [];
        }

        $scripts = $this->detectScripts($ordarr);

        if ($scripts['thai']) {
            $subobj = new ThaiHandler($ordarr);
            $ordarr = $subobj->getOrdarr();
        }

        if ($scripts['devanagari']) {
            $subobj = new DevanagariHandler($ordarr);
            $ordarr = $subobj->getOrdarr();
        }

        if ($scripts['hangul']) {
            $subobj = new HangulHandler($ordarr);
            $ordarr = $subobj->getOrdarr();
        }

        return $ordarr;
    }

    /**
     * Scans $ordarr once and returns a map of which scripts are present.
     *
     * @param array<int, int> $ordarr
     *
     * @return array{thai: bool, devanagari: bool, hangul: bool}
     */
    private function detectScripts(array $ordarr): array
    {
        $scripts = ['thai' => false, 'devanagari' => false, 'hangul' => false];
        foreach ($ordarr as $codepoint) {
            if (!$scripts['thai'] && $this->isInRange($codepoint, ScriptRanges::THAI)) {
                $scripts['thai'] = true;
            }

            if (!$scripts['devanagari'] && $this->isInRange($codepoint, ScriptRanges::DEVANAGARI)) {
                $scripts['devanagari'] = true;
            }

            if (!$scripts['hangul'] && $this->isHangulJamo($codepoint)) {
                $scripts['hangul'] = true;
            }
        }

        return $scripts;
    }

    /**
     * Returns true when $codepoint is in any of the three Hangul Jamo ranges.
     */
    private function isHangulJamo(int $codepoint): bool
    {
        return (
            $this->isInRange($codepoint, ScriptRanges::HANGUL_JAMO)
            || $this->isInRange($codepoint, ScriptRanges::HANGUL_JAMO_EXT_A)
            || $this->isInRange($codepoint, ScriptRanges::HANGUL_JAMO_EXT_B)
        );
    }

    /**
     * Returns true when $codepoint falls within $range[0]..$range[1] inclusive.
     *
     * @param int             $codepoint
     * @param array{int, int} $range
     */
    private function isInRange(int $codepoint, array $range): bool
    {
        return $codepoint >= $range[0] && $codepoint <= $range[1];
    }
}
