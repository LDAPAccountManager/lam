<?php

/**
 * DevanagariTest.php
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

use Com\Tecnick\Unicode\Substitution\Devanagari;
use PHPUnit\Framework\Attributes\DataProvider;
use Test\TestUtil;

/**
 * Devanagari substitution test
 *
 * @since     2026-04-30
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class DevanagariTest extends TestUtil
{
    /**
     * @param array<int, int> $input
     * @param array<int, int> $expected
     */
    #[DataProvider('devanagariDataProvider')]
    public function testGetOrdarr(array $input, array $expected): void
    {
        $obj = new Devanagari($input);
        $this->assertSame($expected, $obj->getOrdarr());
    }

    /**
     * @return array<string, array{0: array<int, int>, 1: array<int, int>}>
     */
    public static function devanagariDataProvider(): array
    {
        // Codepoint reference:
        //   U+0915 KA   U+0916 KHA   U+0937 SHA   U+0939 HA
        //   U+094D VIRAMA   U+093F VOWEL SIGN I (left matra)
        //   U+0958 QA (first extended)   U+095F YYA (last extended)

        return [
            // Empty input returns empty output
            'empty' => [
                [],
                [],
            ],

            // Pure ASCII: no Devanagari codepoints, pass through unchanged
            'ascii_only' => [
                [0x41, 0x42, 0x43],
                [0x41, 0x42, 0x43],
            ],

            // Single consonant — no matra follows, pass through unchanged
            // U+0915 KA
            'consonant_only' => [
                [0x0915],
                [0x0915],
            ],

            // Last standard consonant — U+0939 HA
            'last_standard_consonant' => [
                [0x0939],
                [0x0939],
            ],

            // First extended consonant — U+0958 QA, unchanged (no matra)
            'first_extended_consonant' => [
                [0x0958],
                [0x0958],
            ],

            // Last extended consonant — U+095F YYA, unchanged (no matra)
            'last_extended_consonant' => [
                [0x095F],
                [0x095F],
            ],

            // Orphaned left matra at start (no preceding consonant): unchanged
            // U+093F alone
            'orphaned_left_matra' => [
                [0x093F],
                [0x093F],
            ],

            // Left matra followed by consonant (orphaned leading matra):
            // the matra is not recognised as following a cluster, unchanged
            // U+093F, U+0915
            'matra_then_consonant_no_reorder' => [
                [0x093F, 0x0915],
                [0x093F, 0x0915],
            ],

            // Simple reposition: U+0915 KA + U+093F → U+093F, U+0915
            'ka_with_i_vowel' => [
                [0x0915, 0x093F],
                [0x093F, 0x0915],
            ],

            // Last standard consonant + left matra: U+0939 HA + U+093F
            'ha_with_i_vowel' => [
                [0x0939, 0x093F],
                [0x093F, 0x0939],
            ],

            // Extended consonant + left matra: U+0958 QA + U+093F
            'extended_consonant_with_matra' => [
                [0x0958, 0x093F],
                [0x093F, 0x0958],
            ],

            // Conjunct cluster: U+0915 KA + U+094D VIRAMA + U+0916 KHA + U+093F
            // → U+093F, U+0915, U+094D, U+0916
            'conjunct_with_matra' => [
                [0x0915, 0x094D, 0x0916, 0x093F],
                [0x093F, 0x0915, 0x094D, 0x0916],
            ],

            // Longer conjunct: KA + VIRAMA + SHA + VIRAMA + HA + VOWEL SIGN I
            // → U+093F, KA, VIRAMA, SHA, VIRAMA, HA
            'three_consonant_conjunct_with_matra' => [
                [0x0915, 0x094D, 0x0937, 0x094D, 0x0939, 0x093F],
                [0x093F, 0x0915, 0x094D, 0x0937, 0x094D, 0x0939],
            ],

            // Conjunct where virama is not followed by a consonant: cluster
            // ends at the virama; the matra after virama is NOT moved
            // U+0915, U+094D, U+093F → U+0915, U+094D, U+093F
            // (U+094D followed by non-consonant ends the cluster at KA only,
            // but then U+094D is the next codepoint — not a left matra)
            'virama_then_matra_no_reorder' => [
                [0x0915, 0x094D, 0x093F],
                [0x0915, 0x094D, 0x093F],
            ],

            // Consonant followed by non-matra: pass through unchanged
            // U+0915, 0x41 (ASCII A)
            'consonant_then_ascii' => [
                [0x0915, 0x41],
                [0x0915, 0x41],
            ],

            // Two separate simple clusters
            // KA+I, KHA+I → I+KA, I+KHA
            'two_simple_clusters' => [
                [0x0915, 0x093F, 0x0916, 0x093F],
                [0x093F, 0x0915, 0x093F, 0x0916],
            ],

            // Mixed: ASCII + Devanagari cluster + ASCII
            // 0x41, U+0915, U+093F, 0x42 → 0x41, U+093F, U+0915, 0x42
            'mixed_ascii_devanagari' => [
                [0x41, 0x0915, 0x093F, 0x42],
                [0x41, 0x093F, 0x0915, 0x42],
            ],
        ];
    }

    public function testNormalizesSparseIndexes(): void
    {
        $obj = new Devanagari([5 => 0x0915, 9 => 0x093F]);
        $this->assertSame([0x093F, 0x0915], $obj->getOrdarr());
    }
}
