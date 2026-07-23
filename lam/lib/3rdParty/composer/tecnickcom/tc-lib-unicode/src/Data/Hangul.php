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
 *
 * Source: The Unicode Standard, version 15.1, section 3.12 "Conjoining Jamo Behavior"
 *         https://www.unicode.org/versions/Unicode15.1.0/
 *         https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
 * Unicode Standard version: 15.1
 */

namespace Com\Tecnick\Unicode\Data;

/**
 * Com\Tecnick\Unicode\Data\Hangul
 *
 * Algorithmic constants for Hangul Jamo → precomposed Hangul syllable
 * composition, as defined in section 3.12 of the Unicode Standard.
 *
 * Precomposed syllables occupy the range U+AC00–U+D7A3 and are derived by:
 *
 *   S = SBase + (L − LBase) × NCount + (V − VBase) × TCount + (T − TBase)
 *
 * where T = TBase means "no trailing consonant" (TBase itself is not a
 * trailing consonant; the effective trailing index is 0 in that case).
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
     * First precomposed Hangul syllable: U+AC00 HANGUL SYLLABLE GA.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const SBASE = 0xAC00;

    /**
     * First Hangul leading consonant (choseong): U+1100 HANGUL CHOSEONG KIYEOK.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const LBASE = 0x1100;

    /**
     * First Hangul vowel (jungseong): U+1161 HANGUL JUNGSEONG A.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const VBASE = 0x1161;

    /**
     * Trailing consonant base value: U+11A7.
     *
     * The first actual trailing consonant (jongseong) is U+11A8; TBase is
     * one below that, so that (T − TBase) gives a 1-based index and a T of
     * TBase itself encodes "no trailing consonant" (index 0).
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const TBASE = 0x11A7;

    /**
     * Number of leading consonants (19).
     *
     * Covers U+1100–U+1112.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const LCOUNT = 19;

    /**
     * Number of vowels (21).
     *
     * Covers U+1161–U+1175.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const VCOUNT = 21;

    /**
     * Number of trailing consonant slots (28), including the "none" slot.
     *
     * Effective trailing consonants: U+11A8–U+11C2 (27 codepoints).
     * The 28th slot represents absence of a trailing consonant.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const TCOUNT = 28;

    /**
     * Number of precomposed syllables per leading consonant.
     *
     * NCount = VCount × TCount = 21 × 28 = 588.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const NCOUNT = self::VCOUNT * self::TCOUNT;

    /**
     * Total number of precomposed Hangul syllables.
     *
     * SCount = LCount × NCount = 19 × 588 = 11172.
     *
     * Source: Unicode Standard 15.1, section 3.12
     */
    public const SCOUNT = self::LCOUNT * self::NCOUNT;
}
