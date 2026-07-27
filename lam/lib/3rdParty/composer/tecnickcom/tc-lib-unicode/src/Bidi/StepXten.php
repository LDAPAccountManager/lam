<?php

declare(strict_types=1);

/**
 * StepXten.php
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

/**
 * Com\Tecnick\Unicode\Bidi\StepXten
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * @phpstan-import-type SeqData from \Com\Tecnick\Unicode\Bidi\Shaping\Arabic
 * @phpstan-import-type CharData from \Com\Tecnick\Unicode\Bidi\Shaping\Arabic
 */
class StepXten
{
    /**
     * Number of characters
     */
    protected int $numchars;

    /**
     * Array of Level Run sequences
     *
     * @var array<int, array{'start': int, 'end': int, 'e': int}>
     */
    protected array $runseq = [];

    /**
     * Number of Level Run sequences
     */
    protected int $numrunseq = 0;

    /**
     * Array of Isolated Level Run sequences
     *
     * @var array<int, SeqData>
     */
    protected array $ilrs = [];

    /**
     * @return CharData
     */
    private function getCharData(int $idx): array
    {
        $charData = $this->chardata[$idx] ?? null;
        assert($charData !== null, 'Expected StepXten character data at the requested index');

        return $charData;
    }

    /**
     * @return array{start: int, end: int, e: int}
     */
    private function getRunSequence(int $idx): array
    {
        $runSequence = $this->runseq[$idx] ?? null;
        assert($runSequence !== null, 'Expected StepXten level run sequence at the requested index');

        return $runSequence;
    }

    /**
     * @param SeqData $isorun
     */
    private function findMatchingPdiStart(int $idx, array $isorun, int $numiso): int
    {
        $endItem = $isorun['item'][$isorun['length'] - 1] ?? null;
        assert($endItem !== null, 'Expected final StepXten isolate-run item');
        if (!$this->isIsolateInitiator($endItem['char'])) {
            return -1;
        }

        for ($kdx = $idx + 1; $kdx < $this->numrunseq; ++$kdx) {
            $runSequence = $this->getRunSequence($kdx);
            if ($runSequence['e'] !== $isorun['e']) {
                continue;
            }

            $startChar = $this->getCharData($runSequence['start']);
            if ($startChar['char'] !== UniConstant::PDI) {
                continue;
            }

            $pdimatch = $runSequence['start'];
            $this->chardata[$pdimatch]['pdimatch'] = $numiso;

            return $pdimatch;
        }

        return -1;
    }

    /**
     * @param SeqData $isorun
     */
    private function appendToParentRun(int $parent, array $isorun, int $pdimatch): void
    {
        $parentRun = $this->ilrs[$parent] ?? null;
        assert($parentRun !== null, 'Expected parent isolate-run sequence before appending');

        $parentRun['item'] = \array_merge($parentRun['item'], $isorun['item']);
        $parentRun['length'] += $isorun['length'];
        // The merged sequence now ends where the appended (child) run ends; 'end' is an
        // absolute position in the paragraph, so it is replaced, not accumulated.
        $parentRun['end'] = $isorun['end'];
        $this->ilrs[$parent] = $parentRun;

        if ($pdimatch >= 0) {
            $this->chardata[$pdimatch]['pdimatch'] = $parent;
        }
    }

    /**
     * X Steps for Bidirectional algorithm
     *
     * @param array<int, CharData> $chardata Array of UTF-8 codepoints
     * @param int   $pel      Paragraph Embedding Level
     */
    public function __construct(
        /**
         * Array of characters data to return
         */
        protected array $chardata,
        /**
         * Paragraph Embedding Level
         */
        protected int $pel,
    ) {
        $this->numchars = \count($chardata);
        $this->setIsolatedLevelRunSequences();
    }

    /**
     * Get the Isolated Run Sequences
     *
     * @return array<int, SeqData>
     */
    public function getIsolatedLevelRunSequences(): array
    {
        return $this->ilrs;
    }

    /**
     * Get the embedded direction (L or R)
     */
    protected function getEmbeddedDirection(int $level): string
    {
        return ($level % 2) === 0 ? 'L' : 'R';
    }

    protected function setLevelRunSequences(): void
    {
        $start = 0;
        while ($start < $this->numchars) {
            $level = $this->getCharData($start)['level'];
            $end = $start + 1;
            while ($end < $this->numchars) {
                $charData = $this->chardata[$end] ?? null;
                if ($charData === null || $charData['level'] !== $level) {
                    break;
                }

                ++$end;
            }

            --$end;
            $this->runseq[] = [
                'start' => $start,
                'end' => $end,
                'e' => $level,
            ];
            ++$this->numrunseq;
            $start = $end + 1;
        }
    }

    /**
     * returns true if the input char is an Isolate Initiator
     */
    protected function isIsolateInitiator(int $ord): bool
    {
        return $ord === UniConstant::RLI || $ord === UniConstant::LRI || $ord === UniConstant::FSI;
    }

    /**
     * Set level Isolated Level Run Sequences
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function setIsolatedLevelRunSequences(): void
    {
        $this->setLevelRunSequences();
        $numiso = 0;
        foreach ($this->runseq as $idx => $seq) {
            // Create a new level run sequence, and initialize it to contain just that level run
            $isorun = [
                'e' => $seq['e'],
                'edir' => $this->getEmbeddedDirection($seq['e']), // embedded direction
                'start' => $seq['start'], // position of the first char
                'end' => $seq['end'], // position of the last char
                'length' => $seq['end'] - $seq['start'] + 1,
                'sos' => '', // start-of-sequence
                'eos' => '', // end-of-sequence
                'maxlevel' => 0,
                'item' => [],
            ];
            for ($jdx = 0; $jdx < $isorun['length']; ++$jdx) {
                $isorun['item'][$jdx] = $this->getCharData($seq['start'] + $jdx);
            }

            // While the level run currently last in the sequence ends with an isolate initiator that has a
            // matching PDI, append the level run containing the matching PDI to the sequence.
            // (Note that this matching PDI must be the first character of its level run.)
            $pdimatch = $this->findMatchingPdiStart($idx, $isorun, $numiso);

            // For each level run in the paragraph whose first character is not a PDI,
            // or is a PDI that does not match any isolate initiator
            $parent = $this->getCharData($seq['start'])['pdimatch'];
            if ($parent >= 0 && array_key_exists($parent, $this->ilrs)) {
                $this->appendToParentRun($parent, $isorun, $pdimatch);
                continue;
            }

            $this->ilrs[$numiso] = $isorun;
            ++$numiso;
        }

        $this->setStartEndOfSequence();
    }

    /**
     * Determine the start-of-sequence (sos) and end-of-sequence (eos) types, either L or R,
     * for each isolating run sequence.
     */
    protected function setStartEndOfSequence(): void
    {
        foreach ($this->ilrs as $key => $seq) {
            // For sos, compare the level of the first character in the sequence with the level of the character
            // preceding it in the paragraph (not counting characters removed by X9), and if there is none,
            // with the paragraph embedding level.
            $firstChar = $seq['item'][0] ?? null;
            assert($firstChar !== null, 'Expected first character for StepXten isolate-run sequence');
            $lev = $firstChar['level'];
            $prev = $this->pel;
            if ($seq['start'] !== 0) {
                $prev = $this->getCharData($seq['start'] - 1)['level'];
            }

            $this->ilrs[$key]['sos'] = $this->getEmbeddedDirection((int) \max($prev, $lev));

            // For eos, compare the level of the last character in the sequence with the level of the character
            // following it in the paragraph (not counting characters removed by X9), and if there is none or the
            // last character of the sequence is an isolate initiator (lacking a matching PDI), with the paragraph
            // embedding level.
            $lastchr = \end($seq['item']);

            // A level run always contains at least one character, so end() is not false.
            assert($lastchr !== false, 'Expected final character for StepXten isolate-run sequence');

            $lev = $lastchr['level'];
            $next = $this->pel;
            $nextChar = $this->chardata[$seq['end'] + 1] ?? null;
            if ($nextChar !== null && !$this->isIsolateInitiator($lastchr['char'])) {
                $next = $nextChar['level'];
            }

            $this->ilrs[$key]['eos'] = $this->getEmbeddedDirection((int) \max($next, $lev));

            // If the higher level is odd, the sos or eos is R; otherwise, it is L.
        }
    }
}
