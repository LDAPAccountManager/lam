<?php

declare(strict_types=1);

/**
 * StepL.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Com\Tecnick\Unicode\Bidi;

use Com\Tecnick\Unicode\Data\Constant as UniConstant;
use Com\Tecnick\Unicode\Data\Mirror as UniMirror;

/**
 * Com\Tecnick\Unicode\Bidi\StepL
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * @phpstan-import-type CharData from \Com\Tecnick\Unicode\Bidi\Shaping\Arabic
 */
class StepL
{
    /**
     * Array of characters data to return
     *
     * @var array<int, CharData>
     */
    protected array $chardata = [];

    /**
     * Number of characters in $this->chardata
     */
    protected int $numchars;

    /**
     * L steps
     *
     * @param array<int, CharData> $chardata Array of characters data
     * @param int   $pel      Paragraph embedding level
     * @param int   $maxlevel Maximum level
     */
    public function __construct(
        array $chardata,
        /**
         * Paragraph embedding level
         */
        protected int $pel,
        /**
         * Maximum level
         */
        protected int $maxlevel,
    ) {
        // reorder chars by their original position
        \usort($chardata, static fn($apos, $bpos): int => $apos['pos'] - $bpos['pos']);
        $this->chardata = $chardata;
        $this->numchars = \count($this->chardata);
        $this->processL1();
        $this->processL2();
        $this->processL4();
    }

    /**
     * Returns the processed array
     *
     * @return array<int, CharData>
     */
    public function getChrData(): array
    {
        return $this->chardata;
    }

    /**
     * Reset the embedding level of the character at the given index to the paragraph embedding level.
     *
     * @param int $idx Character index
     */
    private function resetLevel(int $idx): void
    {
        $item = $this->chardata[$idx] ?? null;
        assert($item !== null, 'Expected StepL character data at the index to reset');
        $item['level'] = $this->pel;
        $this->chardata[$idx] = $item;
    }

    /**
     * Returns true when the codepoint is an isolate formatting character (FSI, LRI, RLI, or PDI).
     *
     * @param int $char Codepoint
     */
    private function isIsolateFormat(int $char): bool
    {
        return $char >= UniConstant::LRI && $char <= UniConstant::PDI;
    }

    /**
     * L1. On each line, reset the embedding level of the following characters to the paragraph embedding level:
     *     1. Segment separators,
     *     2. Paragraph separators,
     *     3. Any sequence of whitespace characters and/or isolate formatting characters (FSI, LRI, RLI, and PDI)
     *        preceding a segment separator or paragraph separator, and
     *     4. Any sequence of whitespace characters and/or isolate formatting characters (FSI, LRI, RLI, and PDI)
     *        at the end of the line.
     *
     * This rule is applied using the original character types, not the resolved ones.
     */
    protected function processL1(): void
    {
        // Indexes of the current pending run of whitespace and/or isolate formatting characters.
        $pending = [];
        for ($idx = 0; $idx < $this->numchars; ++$idx) {
            $item = $this->chardata[$idx] ?? null;
            assert($item !== null, 'Expected StepL character data at the current index');

            // L1.1 / L1.2: reset segment and paragraph separators and, with them (L1.3),
            // any immediately preceding run of whitespace / isolate formatting characters.
            if ($item['otype'] === 'S' || $item['otype'] === 'B') {
                foreach ($pending as $pidx) {
                    $this->resetLevel($pidx);
                }

                $this->resetLevel($idx);
                $pending = [];
                continue;
            }

            // Accumulate whitespace and isolate formatting characters as a candidate run.
            if ($item['otype'] === 'WS' || $this->isIsolateFormat($item['char'])) {
                $pending[] = $idx;
                continue;
            }

            // Any other character breaks (and discards) the pending run.
            $pending = [];
        }

        // L1.4: reset a run of whitespace / isolate formatting characters at the end of the line.
        foreach ($pending as $pidx) {
            $this->resetLevel($pidx);
        }
    }

    /**
     * L2. From the highest level found in the text to the lowest odd level on each line,
     *     including intermediate levels not actually present in the text,
     *     reverse any contiguous sequence of characters that are at that level or higher.
     *     This rule reverses a progressively larger series of substrings.
     */
    protected function processL2(): void
    {
        for ($level = $this->maxlevel; $level > 0; --$level) {
            $ordered = [];
            $reversed = [];
            foreach ($this->chardata as $chardatum) {
                if ($chardatum['level'] >= $level) {
                    $reversed[] = $chardatum;

                    continue;
                }

                if ($reversed !== []) {
                    $ordered = \array_merge($ordered, \array_reverse($reversed));
                    $reversed = [];
                }

                $ordered[] = $chardatum;
            }

            if ($reversed !== []) {
                $ordered = \array_merge($ordered, \array_reverse($reversed));
            }

            $this->chardata = $ordered;
        }
    }

    /**
     * L4. A character is depicted by a mirrored glyph if and only if
     *     (a) the resolved directionality of that character is R, and
     *     (b) the Bidi_Mirrored property value of that character is true.
     *
     * The resolved directionality is R exactly when the embedding level is odd, which also covers
     * neutral mirrored characters (brackets, guillemets) resolved into an RTL run — not only
     * strong-R types. Each eligible character is mirrored exactly once, after reordering (L2).
     */
    protected function processL4(): void
    {
        foreach ($this->chardata as $idx => $chardatum) {
            if (($chardatum['level'] % 2) !== 1) {
                continue;
            }

            $mirror = UniMirror::UNI[$chardatum['char']] ?? null;
            if ($mirror === null) {
                continue;
            }

            $chardatum['char'] = $mirror;
            $this->chardata[$idx] = $chardatum;
        }
    }
}
