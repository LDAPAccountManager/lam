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
 */

namespace Com\Tecnick\Unicode\Substitution;

use Com\Tecnick\Unicode\Data\Devanagari as DevanagariData;

/**
 * Com\Tecnick\Unicode\Substitution\Devanagari
 *
 * Repositions Devanagari left-positional matras (vowel signs with Indic
 * Positional Category "Left", such as U+093F VOWEL SIGN I) to precede their
 * base consonant cluster in the codepoint array.
 *
 * In Unicode logical order a left matra is stored after the consonant (or
 * conjunct cluster) it modifies. For PDF glyph streams the matra glyph must
 * appear before the consonant, so each left matra is moved to immediately
 * before the cluster that precedes it.
 *
 * A consonant cluster is: base_consonant (VIRAMA base_consonant)*
 * Only base consonants in the range U+0915–U+0939 and U+0958–U+095F are
 * recognised as cluster heads. Orphaned matras (no preceding consonant) and
 * unknown codepoints are left unchanged.
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
     * Transformed codepoint array.
     *
     * @var list<int>
     */
    private array $ordarr;

    /**
     * @param array<int, int> $ordarr Array of Unicode codepoints.
     */
    public function __construct(array $ordarr)
    {
        $this->ordarr = array_values($ordarr);
        $this->process();
    }

    /**
     * Returns the transformed codepoint array.
     *
     * @return list<int>
     */
    public function getOrdarr(): array
    {
        return $this->ordarr;
    }

    /**
     * Iterates over the codepoint array and repositions left matras that
     * follow a consonant cluster.
     */
    private function process(): void
    {
        $len = count($this->ordarr);
        $result = [];
        $idx = 0;
        while ($idx < $len) {
            $cp = $this->ordarr[$idx] ?? null;
            if ($cp === null) {
                ++$idx;
                continue;
            }

            if ($this->isBaseConsonant($cp)) {
                $idx = $this->processConsonantCluster($idx, $len, $result);
                continue;
            }

            $result[] = $cp;
            ++$idx;
        }

        $this->ordarr = array_values($result);
    }

    /**
     * Collects the full conjunct cluster starting at $idx, then checks
     * whether the next codepoint is a left matra. If so, emits the matra
     * first and the cluster second; otherwise emits the cluster as-is.
     *
     * @param int             $idx    Current index in $this->ordarr.
     * @param int             $len    Length of $this->ordarr.
     * @param list<int> $result Result accumulator (passed by reference).
     *
     * @return int Updated index after all consumed codepoints.
     */
    private function processConsonantCluster(int $idx, int $len, array &$result): int
    {
        $cluster = $this->collectCluster($idx, $len);
        $endIdx = $idx + count($cluster);

        if ($endIdx < $len) {
            $matra = $this->ordarr[$endIdx] ?? null;
            if ($matra !== null && $this->isLeftMatra($matra)) {
                $result[] = $matra;
                foreach ($cluster as $codepoint) {
                    $result[] = $codepoint;
                }

                return $endIdx + 1;
            }
        }

        foreach ($cluster as $codepoint) {
            $result[] = $codepoint;
        }

        return $endIdx;
    }

    /**
     * Collects the full conjunct cluster: base_consonant (VIRAMA base_consonant)*
     *
     * @param int $idx Starting index of the cluster head.
     * @param int $len Length of $this->ordarr.
     *
     * @return list<int> Collected cluster codepoints.
     */
    private function collectCluster(int $idx, int $len): array
    {
        $first = $this->ordarr[$idx] ?? null;
        if ($first === null) {
            return [];
        }

        $cluster = [$first];
        $pos = $idx + 1;
        while (($pos + 1) < $len) {
            $virama = $this->ordarr[$pos] ?? null;
            $baseConsonant = $this->ordarr[$pos + 1] ?? null;
            assert($baseConsonant !== null, 'Expected Devanagari base consonant after virama candidate');
            if ($virama !== DevanagariData::VIRAMA || !$this->isBaseConsonant($baseConsonant)) {
                break;
            }

            $cluster[] = $virama;
            $cluster[] = $baseConsonant;
            $pos += 2;
        }

        return $cluster;
    }

    /**
     * Returns true when $codepoint is a Devanagari base consonant.
     */
    private function isBaseConsonant(int $codepoint): bool
    {
        return $this->isInStandardRange($codepoint) || $this->isInExtendedRange($codepoint);
    }

    /**
     * Returns true when $codepoint is in the standard consonant range
     * U+0915–U+0939.
     */
    private function isInStandardRange(int $codepoint): bool
    {
        return $codepoint >= DevanagariData::BASE_CONSONANT_FIRST && $codepoint <= DevanagariData::BASE_CONSONANT_LAST;
    }

    /**
     * Returns true when $codepoint is in the extended consonant range
     * U+0958–U+095F.
     */
    private function isInExtendedRange(int $codepoint): bool
    {
        return (
            $codepoint >= DevanagariData::BASE_CONSONANT_EXT_FIRST
            && $codepoint <= DevanagariData::BASE_CONSONANT_EXT_LAST
        );
    }

    /**
     * Returns true when $codepoint is a Devanagari left matra.
     */
    private function isLeftMatra(int $codepoint): bool
    {
        return array_key_exists($codepoint, DevanagariData::LEFT_MATRAS);
    }
}
