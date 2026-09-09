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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 *
 * Source: https://www.unicode.org/Public/17.0.0/ucd/UnicodeData.txt
 *         https://www.unicode.org/Public/17.0.0/ucd/IndicPositionalCategory.txt
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
     * @var array<int, true>
     */
    public const LEFT_MATRAS = [
        0x093F => true, // DEVANAGARI VOWEL SIGN I
        0x094E => true, // DEVANAGARI VOWEL SIGN PRISHTHAMATRA E
    ];

    /**
     * Devanagari Virama (U+094D).
     *
     * Joins two consonants into a conjunct cluster. When scanning a consonant
     * cluster for pre-base matra reordering, consecutive (consonant + VIRAMA)
     * pairs extend the cluster.
     */
    public const VIRAMA = 0x094D;

    /**
     * Devanagari Nukta (U+093C).
     *
     * Combining dot that turns a base consonant into another consonant. It belongs to
     * the consonant it follows, so it is part of the cluster scanned for pre-base matra
     * reordering.
     */
    public const NUKTA = 0x093C;

    /**
     * First codepoint of the standard Devanagari consonant range.
     *
     * U+0915 DEVANAGARI LETTER KA
     */
    public const BASE_CONSONANT_FIRST = 0x0915;

    /**
     * Last codepoint of the standard Devanagari consonant range.
     *
     * U+0939 DEVANAGARI LETTER HA
     */
    public const BASE_CONSONANT_LAST = 0x0939;

    /**
     * First codepoint of the extended Devanagari consonant range
     * (consonants with nukta, deprecated precomposed forms).
     *
     * U+0958 DEVANAGARI LETTER QA
     */
    public const BASE_CONSONANT_EXT_FIRST = 0x0958;

    /**
     * Last codepoint of the extended Devanagari consonant range.
     *
     * U+095F DEVANAGARI LETTER YYA
     */
    public const BASE_CONSONANT_EXT_LAST = 0x095F;

    /**
     * First codepoint of the additional Devanagari consonant range.
     *
     * U+0978 DEVANAGARI LETTER MARWARI DDA
     */
    public const BASE_CONSONANT_ADD_FIRST = 0x0978;

    /**
     * Last codepoint of the additional Devanagari consonant range.
     *
     * U+097F DEVANAGARI LETTER BBA
     */
    public const BASE_CONSONANT_ADD_LAST = 0x097F;
}
