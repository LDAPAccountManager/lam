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
 */

namespace Com\Tecnick\Unicode\Substitution;

use Com\Tecnick\Unicode\Data\Thai as ThaiData;

/**
 * Com\Tecnick\Unicode\Substitution\Thai
 *
 * Repositions Thai leading vowels (sara E, sara AE, etc.) to follow their
 * base consonant, converting Unicode logical order to the visual order
 * expected by PDF glyph streams.
 *
 * In Unicode logical order a leading vowel precedes its base consonant.
 * For PDF rendering the glyph stream must present the base consonant first,
 * so each leading vowel is moved to immediately after the consonant it
 * modifies.
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
     * Iterates over the codepoint array and repositions any leading-vowel
     * clusters that are immediately followed by a base consonant.
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

            if ($this->isLeadingVowel($cp)) {
                $idx = $this->processVowelCluster($idx, $len, $result);
                continue;
            }

            $result[] = $cp;
            ++$idx;
        }

        $this->ordarr = array_values($result);
    }

    /**
     * Collects consecutive leading vowels starting at $idx, then appends the
     * following base consonant (if present) before the vowels; otherwise
     * appends the vowels unchanged.
     *
     * @param int             $idx    Current index in $this->ordarr.
     * @param int             $len    Length of $this->ordarr.
     * @param list<int> $result Result accumulator (passed by reference).
     *
     * @return int Updated index after all consumed codepoints.
     */
    private function processVowelCluster(int $idx, int $len, array &$result): int
    {
        $vowels = $this->collectLeadingVowels($idx, $len);
        $nextIdx = $idx + count($vowels);

        if ($nextIdx < $len) {
            $nextCp = $this->ordarr[$nextIdx] ?? null;
            if ($nextCp !== null && $this->isBaseConsonant($nextCp)) {
                $result[] = $nextCp;
                foreach ($vowels as $vowel) {
                    $result[] = $vowel;
                }

                return $nextIdx + 1;
            }
        }

        foreach ($vowels as $vowel) {
            $result[] = $vowel;
        }

        return $nextIdx;
    }

    /**
     * Returns all consecutive leading vowels starting at $idx.
     *
     * @param int $idx Starting index.
     * @param int $len Length of $this->ordarr.
     *
     * @return list<int> Collected vowel codepoints.
     */
    private function collectLeadingVowels(int $idx, int $len): array
    {
        $vowels = [];
        while ($idx < $len) {
            $cp = $this->ordarr[$idx] ?? null;
            if ($cp === null || !$this->isLeadingVowel($cp)) {
                break;
            }

            $vowels[] = $cp;
            ++$idx;
        }

        return $vowels;
    }

    /**
     * Returns true when $cp is a Thai leading vowel.
     */
    private function isLeadingVowel(int $codepoint): bool
    {
        return array_key_exists($codepoint, ThaiData::LEADING_VOWELS);
    }

    /**
     * Returns true when $codepoint is a Thai base consonant.
     */
    private function isBaseConsonant(int $codepoint): bool
    {
        return $codepoint >= ThaiData::BASE_CONSONANT_FIRST && $codepoint <= ThaiData::BASE_CONSONANT_LAST;
    }
}
