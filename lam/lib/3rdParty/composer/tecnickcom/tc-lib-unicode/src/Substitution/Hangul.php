<?php

declare(strict_types=1);

/**
 * Hangul.php
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

namespace Com\Tecnick\Unicode\Substitution;

use Com\Tecnick\Unicode\Data\Hangul as HangulData;

/**
 * Com\Tecnick\Unicode\Substitution\Hangul
 *
 * Composes Hangul Jamo sequences into precomposed Hangul syllables per the
 * Unicode Standard, version 15.1, section 3.12 "Conjoining Jamo Behavior".
 *
 * Two composition rules are applied left-to-right in a single pass:
 *
 *   Rule 1 — L + V → LV syllable
 *     When a leading consonant (choseong, U+1100–U+1112) is immediately
 *     followed by a vowel (jungseong, U+1161–U+1175) the pair is replaced
 *     by the corresponding precomposed syllable:
 *       S = SBase + (L − LBase) × NCount + (V − VBase) × TCount
 *
 *   Rule 2 — LV + T → LVT syllable
 *     When the syllable produced by Rule 1 (or an existing LV precomposed
 *     syllable already in the input) is immediately followed by a trailing
 *     consonant (jongseong, U+11A8–U+11C2) the pair is merged:
 *       S = LV + (T − TBase)
 *
 * Codepoints that do not participate in either rule are passed through
 * unchanged.
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class Hangul
{
    /**
     * Transformed codepoint array.
     *
     * @var list<int>
     */
    private array $ordarr;

    /**
     * @param array<int, int> $ordarr Array of Unicode codepoints.
     */
    public function __construct(array $ordarr)
    {
        $this->ordarr = array_values($ordarr);
        $this->process();
    }

    /**
     * Returns the transformed codepoint array.
     *
     * @return list<int>
     */
    public function getOrdarr(): array
    {
        return $this->ordarr;
    }

    /**
     * Iterates over the codepoint array applying L+V and LV+T composition.
     */
    private function process(): void
    {
        $len = count($this->ordarr);
        $result = [];
        $idx = 0;
        while ($idx < $len) {
            $codepoint = $this->ordarr[$idx] ?? null;
            if ($codepoint === null) {
                ++$idx;
                continue;
            }

            if ($this->isLeadingConsonant($codepoint) && ($idx + 1) < $len) {
                $idx = $this->composeLV($idx, $len, $result);
                continue;
            }

            $result[] = $codepoint;
            ++$idx;
        }

        $this->ordarr = array_values($result);
    }

    /**
     * Attempts L+V composition at $idx. On success also attempts LV+T.
     * Falls back to emitting the leading consonant unchanged if no vowel
     * follows.
     *
     * @param int             $idx    Current index (L position).
     * @param int             $len    Length of $this->ordarr.
     * @param list<int> $result Result accumulator (passed by reference).
     *
     * @return int Updated index after all consumed codepoints.
     */
    private function composeLV(int $idx, int $len, array &$result): int
    {
        $lChar = $this->ordarr[$idx] ?? null;
        $vChar = $this->ordarr[$idx + 1] ?? null;

        if ($lChar === null) {
            return $idx + 1;
        }

        if ($vChar === null || !$this->isVowel($vChar)) {
            $result[] = $lChar;
            return $idx + 1;
        }

        $lvSyllable = $this->buildLVSyllable($lChar, $vChar);
        $nextIdx = $idx + 2;

        if ($nextIdx < $len) {
            $tChar = $this->ordarr[$nextIdx] ?? null;
            if ($tChar !== null && $this->isTrailingConsonant($tChar)) {
                $result[] = $lvSyllable + ($tChar - HangulData::TBASE);
                return $nextIdx + 1;
            }
        }

        $result[] = $lvSyllable;
        return $nextIdx;
    }

    /**
     * Computes the LV precomposed syllable from a leading consonant and vowel.
     *
     * @param int $lChar Leading consonant codepoint.
     * @param int $vChar Vowel codepoint.
     */
    private function buildLVSyllable(int $lChar, int $vChar): int
    {
        $lIndex = $lChar - HangulData::LBASE;
        $vIndex = $vChar - HangulData::VBASE;
        return HangulData::SBASE + ($lIndex * HangulData::NCOUNT) + ($vIndex * HangulData::TCOUNT);
    }

    /**
     * Returns true when $codepoint is a Hangul leading consonant (choseong).
     *
     * Range: U+1100–U+1112 (LCount = 19 entries).
     */
    private function isLeadingConsonant(int $codepoint): bool
    {
        return $codepoint >= HangulData::LBASE && $codepoint < (HangulData::LBASE + HangulData::LCOUNT);
    }

    /**
     * Returns true when $codepoint is a Hangul vowel (jungseong).
     *
     * Range: U+1161–U+1175 (VCount = 21 entries).
     */
    private function isVowel(int $codepoint): bool
    {
        return $codepoint >= HangulData::VBASE && $codepoint < (HangulData::VBASE + HangulData::VCOUNT);
    }

    /**
     * Returns true when $codepoint is a Hangul trailing consonant (jongseong).
     *
     * Range: U+11A8–U+11C2 (TCount − 1 = 27 entries; TBase = U+11A7 is
     * not itself a valid trailing consonant).
     */
    private function isTrailingConsonant(int $codepoint): bool
    {
        return $codepoint > HangulData::TBASE && $codepoint < (HangulData::TBASE + HangulData::TCOUNT);
    }
}
