<?php

/**
 * ThaiTest.php
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

namespace Test\Substitution;

use Com\Tecnick\Unicode\Substitution\Thai;
use PHPUnit\Framework\Attributes\DataProvider;
use Test\TestUtil;

/**
 * Thai substitution test
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class ThaiTest extends TestUtil
{
    /**
     * @param array<int, int> $input
     * @param array<int, int> $expected
     */
    #[DataProvider('thaiDataProvider')]
    public function testGetOrdarr(array $input, array $expected): void
    {
        $obj = new Thai($input);
        $this->assertSame($expected, $obj->getOrdarr());
    }

    /**
     * @return array<string, array{0: array<int, int>, 1: array<int, int>}>
     */
    public static function thaiDataProvider(): array
    {
        return [
            // Empty input returns empty output
            'empty' => [
                [],
                [],
            ],

            // Pure ASCII: no Thai codepoints, pass through unchanged
            'ascii_only' => [
                [0x41, 0x42, 0x43],
                [0x41, 0x42, 0x43],
            ],

            // Thai consonant only (no leading vowel): unchanged
            // U+0E01 THAI CHARACTER KO KAI
            'consonant_only' => [
                [0x0E01],
                [0x0E01],
            ],

            // Tone mark only: not a leading vowel, unchanged
            // U+0E48 THAI CHARACTER MAI EK
            'tone_mark_only' => [
                [0x0E48],
                [0x0E48],
            ],

            // Single leading vowel at end of array (orphaned): leave unchanged
            // U+0E40 THAI CHARACTER SARA E
            'orphaned_leading_vowel_end' => [
                [0x0E40],
                [0x0E40],
            ],

            // Leading vowel followed by a tone mark (not a base consonant):
            // leave unchanged — U+0E40, U+0E48
            'leading_vowel_then_tone_mark' => [
                [0x0E40, 0x0E48],
                [0x0E40, 0x0E48],
            ],

            // Leading vowel followed by an ASCII character (not a base
            // consonant): leave unchanged — U+0E40, 0x41
            'leading_vowel_then_ascii' => [
                [0x0E40, 0x41],
                [0x0E40, 0x41],
            ],

            // Simple reposition: U+0E40 (SARA E) + U+0E01 (KO KAI)
            // → U+0E01, U+0E40
            'sara_e_before_ko_kai' => [
                [0x0E40, 0x0E01],
                [0x0E01, 0x0E40],
            ],

            // U+0E41 (SARA AE) + U+0E02 (KHO KHAI)
            'sara_ae_before_kho_khai' => [
                [0x0E41, 0x0E02],
                [0x0E02, 0x0E41],
            ],

            // U+0E44 (SARA AI MAIMALAI) + last base consonant U+0E2E
            'sara_ai_before_ho_nokhuk' => [
                [0x0E44, 0x0E2E],
                [0x0E2E, 0x0E44],
            ],

            // Multiple consecutive leading vowels before one consonant
            // U+0E40, U+0E41, U+0E01 → U+0E01, U+0E40, U+0E41
            'two_leading_vowels_then_consonant' => [
                [0x0E40, 0x0E41, 0x0E01],
                [0x0E01, 0x0E40, 0x0E41],
            ],

            // Leading vowel + consonant + tone mark: only the vowel is moved;
            // the tone mark stays after the consonant
            // U+0E40, U+0E01, U+0E48 → U+0E01, U+0E40, U+0E48
            'vowel_consonant_tone' => [
                [0x0E40, 0x0E01, 0x0E48],
                [0x0E01, 0x0E40, 0x0E48],
            ],

            // Mixed: ASCII + Thai cluster + ASCII
            // 0x41, U+0E40, U+0E01, U+0E48, 0x42 → 0x41, U+0E01, U+0E40, U+0E48, 0x42
            'mixed_ascii_thai' => [
                [0x41, 0x0E40, 0x0E01, 0x0E48, 0x42],
                [0x41, 0x0E01, 0x0E40, 0x0E48, 0x42],
            ],

            // Two separate Thai clusters in one array
            // U+0E40, U+0E01, U+0E44, U+0E2E → U+0E01, U+0E40, U+0E2E, U+0E44
            'two_clusters' => [
                [0x0E40, 0x0E01, 0x0E44, 0x0E2E],
                [0x0E01, 0x0E40, 0x0E2E, 0x0E44],
            ],

            // Multiple consecutive leading vowels followed by non-consonant
            // (both left unchanged)
            'two_leading_vowels_no_consonant' => [
                [0x0E40, 0x0E41],
                [0x0E40, 0x0E41],
            ],
        ];
    }

    public function testNormalizesSparseIndexes(): void
    {
        $obj = new Thai([3 => 0x0E40, 7 => 0x0E01]);
        $this->assertSame([0x0E01, 0x0E40], $obj->getOrdarr());
    }
}
