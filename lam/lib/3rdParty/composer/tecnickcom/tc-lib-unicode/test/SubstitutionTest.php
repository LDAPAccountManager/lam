<?php

/**
 * SubstitutionTest.php
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

namespace Test;

use Com\Tecnick\Unicode\Substitution;
use PHPUnit\Framework\Attributes\DataProvider;

/**
 * Substitution dispatcher test
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class SubstitutionTest extends TestUtil
{
    protected function getTestObject(): Substitution
    {
        return new Substitution();
    }

    /**
     * @param array<int, int> $input
     * @param array<int, int> $expected
     */
    #[DataProvider('replaceCharsDataProvider')]
    public function testReplaceChars(array $input, array $expected): void
    {
        $sub = $this->getTestObject();
        $this->assertSame($expected, $sub->replaceChars($input));
    }

    /**
     * @return array<string, array{0: array<int, int>, 1: array<int, int>}>
     */
    public static function replaceCharsDataProvider(): array
    {
        return [
            // Empty input
            'empty' => [
                [],
                [],
            ],

            // Pure ASCII: Thai script not detected, returned unchanged
            'ascii_only' => [
                [0x41, 0x42, 0x43],
                [0x41, 0x42, 0x43],
            ],

            // Non-Thai non-Devanagari Unicode (Bengali): not yet handled, pass through
            // U+0985 BENGALI LETTER A
            'bengali_passthrough' => [
                [0x0985, 0x0986],
                [0x0985, 0x0986],
            ],

            // Thai only consonants (Thai detected but no leading vowels):
            // handler runs but nothing is repositioned
            // U+0E01 KO KAI, U+0E02 KHO KHAI
            'thai_consonants_only' => [
                [0x0E01, 0x0E02],
                [0x0E01, 0x0E02],
            ],

            // Thai with multiple codepoints: exercises the detectScripts
            // short-circuit where the second Thai codepoint skips the check
            // U+0E01, U+0E02, U+0E03
            'thai_multiple_consonants' => [
                [0x0E01, 0x0E02, 0x0E03],
                [0x0E01, 0x0E02, 0x0E03],
            ],

            // Thai with a leading vowel: dispatcher delegates to ThaiHandler
            // U+0E40 (SARA E) + U+0E01 (KO KAI) → U+0E01, U+0E40
            'thai_vowel_reposition' => [
                [0x0E40, 0x0E01],
                [0x0E01, 0x0E40],
            ],

            // Mixed script: Thai cluster plus ASCII; only Thai part transformed
            // 0x41, U+0E40, U+0E01, 0x42 → 0x41, U+0E01, U+0E40, 0x42
            'mixed_ascii_thai' => [
                [0x41, 0x0E40, 0x0E01, 0x42],
                [0x41, 0x0E01, 0x0E40, 0x42],
            ],

            // Devanagari consonant without matra: handler runs, nothing moved
            // U+0915 KA, U+0916 KHA
            'devanagari_consonants_only' => [
                [0x0915, 0x0916],
                [0x0915, 0x0916],
            ],

            // Devanagari: detectScripts short-circuit — second Devanagari
            // codepoint skips the range check once already detected
            // U+0915, U+0916, U+0917
            'devanagari_multiple_consonants' => [
                [0x0915, 0x0916, 0x0917],
                [0x0915, 0x0916, 0x0917],
            ],

            // Devanagari left matra reposition via dispatcher
            // U+0915 KA + U+093F → U+093F, U+0915
            'devanagari_matra_reposition' => [
                [0x0915, 0x093F],
                [0x093F, 0x0915],
            ],

            // Mixed ASCII + Devanagari cluster
            // 0x41, U+0915, U+093F, 0x42 → 0x41, U+093F, U+0915, 0x42
            'mixed_ascii_devanagari' => [
                [0x41, 0x0915, 0x093F, 0x42],
                [0x41, 0x093F, 0x0915, 0x42],
            ],

            // Devanagari codepoint in block but outside consonant range:
            // U+0900 INVERTED CANDRABINDU (combining mark) — no matra reorder
            'devanagari_non_consonant' => [
                [0x0900, 0x093F],
                [0x0900, 0x093F],
            ],

            // Hangul Jamo leading consonant + vowel: dispatcher delegates
            // U+1100 + U+1161 → U+AC00 가 (GA)
            'hangul_lv_composition' => [
                [0x1100, 0x1161],
                [0xAC00],
            ],

            // Hangul Jamo detected via extended-A range (U+A960): triggers hangul handler;
            // U+A960 is not in the standard L range so it passes through unchanged but
            // ensures isHangulJamo covers HANGUL_JAMO_EXT_A
            'hangul_ext_a_passthrough' => [
                [0xA960],
                [0xA960],
            ],

            // Hangul Jamo detected via extended-B range (U+D7B0): same as above for EXT_B
            'hangul_ext_b_passthrough' => [
                [0xD7B0],
                [0xD7B0],
            ],

            // Hangul: detectScripts short-circuit — second Hangul Jamo codepoint skips check
            // U+1100, U+1102 — both L, no V, so no composition
            'hangul_multiple_leading_consonants' => [
                [0x1100, 0x1102],
                [0x1100, 0x1102],
            ],

            // Hangul L + V + T full composition via dispatcher
            // U+1100, U+1161, U+11A8 → U+AC01
            'hangul_lvt_composition' => [
                [0x1100, 0x1161, 0x11A8],
                [0xAC01],
            ],
        ];
    }
}
