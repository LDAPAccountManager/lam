<?php

declare(strict_types=1);

/**
 * Thai.php
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
 * Unicode Standard version: 15.1
 */

namespace Com\Tecnick\Unicode\Data;

/**
 * Com\Tecnick\Unicode\Data\Thai
 *
 * Thai codepoint tables for character substitution and reordering.
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class Thai
{
    /**
     * Thai leading vowels that must be repositioned after their base consonant
     * in a PDF glyph stream (visual order).
     *
     * These vowels visually precede the base consonant in text but are stored
     * before the consonant in Unicode logical order. For PDF rendering they
     * must appear after the consonant in the glyph array.
     *
     * Codepoints:
     *   U+0E40 THAI CHARACTER SARA E
     *   U+0E41 THAI CHARACTER SARA AE
     *   U+0E42 THAI CHARACTER SARA O
     *   U+0E43 THAI CHARACTER SARA AI MAIMUAN
     *   U+0E44 THAI CHARACTER SARA AI MAIMALAI
     *   U+0E4D THAI CHARACTER NIKHAHIT (leading form used with some vowels)
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     *
     * @var array<int, true>
     */
    public const LEADING_VOWELS = [
        0x0E40 => true,
        0x0E41 => true,
        0x0E42 => true,
        0x0E43 => true,
        0x0E44 => true,
        0x0E4D => true,
    ];

    /**
     * Thai tone marks (must not be treated as base consonants during cluster
     * scanning).
     *
     * Codepoints:
     *   U+0E48 THAI CHARACTER MAI EK
     *   U+0E49 THAI CHARACTER MAI THO
     *   U+0E4A THAI CHARACTER MAI TRI
     *   U+0E4B THAI CHARACTER MAI JATTAWA
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     *
     * @var array<int, true>
     */
    public const TONE_MARKS = [
        0x0E48 => true,
        0x0E49 => true,
        0x0E4A => true,
        0x0E4B => true,
    ];

    /**
     * Thai base consonant range: U+0E01–U+0E2E
     * (THAI CHARACTER KO KAI through THAI CHARACTER HO NOKHUK)
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     */
    public const BASE_CONSONANT_FIRST = 0x0E01;

    /**
     * Thai base consonant range upper bound: U+0E2E
     *
     * Source: https://unicode.org/Public/15.1.0/ucd/UnicodeData.txt
     */
    public const BASE_CONSONANT_LAST = 0x0E2E;
}
