<?php

declare(strict_types=1);

/**
 * Devanagari.php
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
 * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
 *         https://unicode.org/Public/15.1.0/ucd/IndicPositionalCategory.txt
 * Unicode Standard version: 15.1
 */

namespace Com\Tecnick\Unicode\Data;

/**
 * Com\Tecnick\Unicode\Data\Devanagari
 *
 * Devanagari codepoint tables for character substitution and cluster
 * reordering.
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class Devanagari
{
    /**
     * Devanagari vowel signs with Indic Positional Category "Left".
     *
     * These matras are stored after their base consonant (or consonant
     * cluster) in Unicode logical order but must be rendered to the LEFT of
     * the base in a PDF glyph stream. They are therefore repositioned to
     * precede the consonant cluster during substitution.
     *
     * Codepoints:
     *   U+093F DEVANAGARI VOWEL SIGN I
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/IndicPositionalCategory.txt
     *
     * @var array<int, true>
     */
    public const LEFT_MATRAS = [
        0x093F => true,
    ];

    /**
     * Devanagari Virama (U+094D).
     *
     * Joins two consonants into a conjunct cluster. When scanning a consonant
     * cluster for pre-base matra reordering, consecutive (consonant + VIRAMA)
     * pairs extend the cluster.
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     */
    public const VIRAMA = 0x094D;

    /**
     * First codepoint of the standard Devanagari consonant range.
     *
     * U+0915 DEVANAGARI LETTER KA
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     */
    public const BASE_CONSONANT_FIRST = 0x0915;

    /**
     * Last codepoint of the standard Devanagari consonant range.
     *
     * U+0939 DEVANAGARI LETTER HA
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     */
    public const BASE_CONSONANT_LAST = 0x0939;

    /**
     * First codepoint of the extended Devanagari consonant range
     * (consonants with nukta — deprecated precomposed forms).
     *
     * U+0958 DEVANAGARI LETTER QA
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     */
    public const BASE_CONSONANT_EXT_FIRST = 0x0958;

    /**
     * Last codepoint of the extended Devanagari consonant range.
     *
     * U+095F DEVANAGARI LETTER YYA
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     */
    public const BASE_CONSONANT_EXT_LAST = 0x095F;
}
