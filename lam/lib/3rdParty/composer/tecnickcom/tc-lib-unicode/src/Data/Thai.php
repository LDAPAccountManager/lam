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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 *
 * Source: https://www.unicode.org/Public/17.0.0/ucd/UnicodeData.txt
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class Thai
{
    /**
     * Thai preposed vowels: they are stored before the consonant they follow in
     * pronunciation and are displayed in that same position, so they need no
     * reordering in a left-to-right glyph stream.
     *
     * Codepoints:
     *   U+0E40 THAI CHARACTER SARA E
     *   U+0E41 THAI CHARACTER SARA AE
     *   U+0E42 THAI CHARACTER SARA O
     *   U+0E43 THAI CHARACTER SARA AI MAIMUAN
     *   U+0E44 THAI CHARACTER SARA AI MAIMALAI
     *
     * @var array<int, true>
     */
    public const LEADING_VOWELS = [
        0x0E40 => true,
        0x0E41 => true,
        0x0E42 => true,
        0x0E43 => true,
        0x0E44 => true,
    ];

    /**
     * Thai tone marks: combining marks that are not base consonants.
     *
     * Codepoints:
     *   U+0E48 THAI CHARACTER MAI EK
     *   U+0E49 THAI CHARACTER MAI THO
     *   U+0E4A THAI CHARACTER MAI TRI
     *   U+0E4B THAI CHARACTER MAI JATTAWA
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
     * Thai base consonant range: U+0E01-U+0E2E
     * (THAI CHARACTER KO KAI through THAI CHARACTER HO NOKHUK)
     */
    public const BASE_CONSONANT_FIRST = 0x0E01;

    /**
     * Thai base consonant range upper bound: U+0E2E
     */
    public const BASE_CONSONANT_LAST = 0x0E2E;
}
