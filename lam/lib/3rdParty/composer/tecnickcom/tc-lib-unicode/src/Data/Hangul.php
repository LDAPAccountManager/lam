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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 *
 * Source: section 3.12 "Conjoining Jamo Behavior" of the Unicode standard
 */

namespace Com\Tecnick\Unicode\Data;

/**
 * Com\Tecnick\Unicode\Data\Hangul
 *
 * Algorithmic constants for the composition of Hangul Jamo into precomposed Hangul
 * syllables, as defined in section 3.12 of the Unicode standard.
 *
 * Precomposed syllables occupy the range U+AC00-U+D7A3 and are derived by:
 *
 *   S = SBase + (L - LBase) * NCount + (V - VBase) * TCount + (T - TBase)
 *
 * where T = TBase means "no trailing consonant" (TBase itself is not a
 * trailing consonant; the effective trailing index is 0 in that case).
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class Hangul
{
    /**
     * First precomposed Hangul syllable: U+AC00 HANGUL SYLLABLE GA.
     */
    public const SBASE = 0xAC00;

    /**
     * First Hangul leading consonant (choseong): U+1100 HANGUL CHOSEONG KIYEOK.
     */
    public const LBASE = 0x1100;

    /**
     * First Hangul vowel (jungseong): U+1161 HANGUL JUNGSEONG A.
     */
    public const VBASE = 0x1161;

    /**
     * Trailing consonant base value: U+11A7.
     *
     * The first actual trailing consonant (jongseong) is U+11A8; TBase is
     * one below that, so that (T - TBase) gives a 1-based index and a T of
     * TBase itself encodes "no trailing consonant" (index 0).
     */
    public const TBASE = 0x11A7;

    /**
     * Number of leading consonants (19).
     *
     * Covers U+1100-U+1112.
     */
    public const LCOUNT = 19;

    /**
     * Number of vowels (21).
     *
     * Covers U+1161-U+1175.
     */
    public const VCOUNT = 21;

    /**
     * Number of trailing consonant slots (28), including the "none" slot.
     *
     * Effective trailing consonants: U+11A8-U+11C2 (27 codepoints).
     * The 28th slot represents absence of a trailing consonant.
     */
    public const TCOUNT = 28;

    /**
     * Number of precomposed syllables per leading consonant.
     *
     * NCount = VCount * TCount = 21 * 28 = 588.
     */
    public const NCOUNT = self::VCOUNT * self::TCOUNT;

    /**
     * Total number of precomposed Hangul syllables.
     *
     * SCount = LCount * NCount = 19 * 588 = 11172.
     */
    public const SCOUNT = self::LCOUNT * self::NCOUNT;
}
