<?php

declare(strict_types=1);

/**
 * ScriptRanges.php
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
 * Source: https://unicode.org/Public/15.1.0/ucd/Blocks.txt
 * Unicode Standard version: 15.1
 */

namespace Com\Tecnick\Unicode\Data;

/**
 * Com\Tecnick\Unicode\Data\ScriptRanges
 *
 * Unicode codepoint ranges used to detect which scripts are present in a
 * codepoint array. Each entry is [firstCodepoint, lastCodepoint].
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
final class ScriptRanges
{
    /**
     * Thai block: U+0E00–U+0E7F
     * Source: https://unicode.org/Public/15.1.0/ucd/Blocks.txt
     *
     * @var array{int, int}
     */
    public const THAI = [0x0E00, 0x0E7F];

    /**
     * Devanagari block: U+0900–U+097F
     * Source: https://unicode.org/Public/15.1.0/ucd/Blocks.txt
     *
     * @var array{int, int}
     */
    public const DEVANAGARI = [0x0900, 0x097F];

    /**
     * Bengali block: U+0980–U+09FF
     * Source: https://unicode.org/Public/15.1.0/ucd/Blocks.txt
     *
     * @var array{int, int}
     */
    public const BENGALI = [0x0980, 0x09FF];

    /**
     * Hangul Jamo block: U+1100–U+11FF
     * Source: https://unicode.org/Public/15.1.0/ucd/Blocks.txt
     *
     * @var array{int, int}
     */
    public const HANGUL_JAMO = [0x1100, 0x11FF];

    /**
     * Hangul Jamo Extended-A block: U+A960–U+A97F
     * Source: https://unicode.org/Public/15.1.0/ucd/Blocks.txt
     *
     * @var array{int, int}
     */
    public const HANGUL_JAMO_EXT_A = [0xA960, 0xA97F];

    /**
     * Hangul Jamo Extended-B block: U+D7B0–U+D7FF
     * Source: https://unicode.org/Public/15.1.0/ucd/Blocks.txt
     *
     * @var array{int, int}
     */
    public const HANGUL_JAMO_EXT_B = [0xD7B0, 0xD7FF];
}
