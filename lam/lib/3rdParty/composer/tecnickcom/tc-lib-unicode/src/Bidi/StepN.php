<?php

declare(strict_types=1);

/**
 * StepN.php
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

use Com\Tecnick\Unicode\Data\Bracket as UniBracket;
use Com\Tecnick\Unicode\Data\Type as UniType;

/**
 * Com\Tecnick\Unicode\Bidi\StepN
 *
 * N steps of the Bidirectional Algorithm: resolving neutral and isolate formatting
 * types (N0 to N2).
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class StepN extends \Com\Tecnick\Unicode\Bidi\StepBase
{
    /**
     * List or bracket pairs positions
     *
     * @var array<int, int>
     */
    protected array $brackets = [];

    /**
     * Maximum number of entries of the BD16 bracket stack
     */
    public const MAX_PAIRING_DEPTH = 63;

    /**
     * Stack used to store bracket positions
     *
     * @var array<int, array{int, int}>
     */
    protected array $bstack = [];

    /**
     * True when BD16 has been stopped for the remainder of the isolating run sequence
     * because the bracket stack overflowed.
     */
    protected bool $bstackFull = false;

    /**
     * Isolate formatting types per UAX #9. Together with the neutral types B, S, WS and
     * ON of UniType::NEUTRAL they form the NI types resolved by the N rules.
     * 'NI' itself is not a bidirectional character type: it is accepted as a synonym of
     * the whole set.
     *
     * @var array<string, string>
     */
    protected const ISOLATE_FORMATTING = [
        'FSI' => 'FSI',
        'LRI' => 'LRI',
        'RLI' => 'RLI',
        'PDI' => 'PDI',
        'NI' => 'NI',
    ];

    /**
     * Returns true when the character at the given index has a Neutral or Isolate formatting
     * (NI) type per UAX #9, i.e. it is subject to the N rules.
     */
    protected function isNI(int $idx): bool
    {
        $type = $this->getItem($idx)['type'];
        return isset(self::ISOLATE_FORMATTING[$type]) || isset(UniType::NEUTRAL[$type]);
    }

    /**
     * @return array{char: int, i: int, level: int, otype: string, pdimatch: int, pos: int, type: string, x: int}
     */
    private function getItem(int $idx): array
    {
        $item = $this->seq['item'][$idx] ?? null;
        assert($item !== null, 'Expected StepN sequence item at the requested index');

        return $item;
    }

    private function setItemType(int $idx, string $type): void
    {
        $item = $this->getItem($idx);
        $item['type'] = $type;
        $this->seq['item'][$idx] = $item;
    }

    /**
     * Process N steps
     * Resolving Neutral and Isolate Formatting Types
     *
     * Neutral and isolate formatting (i.e. NI) characters are resolved one isolating run sequence at a time.
     * Its results are that all NIs become either R or L. Generally, NIs take on the direction of the surrounding text.
     * In case of a conflict, they take on the embedding direction.
     * At isolating run sequence boundaries where the type of the character on the other side of the boundary
     * is required, the type assigned to sos or eos is used.
     *
     * Bracket pairs within an isolating run sequence are processed as units so that both the opening and the closing
     * paired bracket in a pair resolve to the same direction. Note that this rule is applied based on the current
     * bidirectional character type of each paired bracket and not the original type, as this could have changed under
     * X6. The current bidirectional character type may also have changed under a previous iteration of the for loop in
     * N0 in the case of nested bracket pairs.
     */
    protected function process(): void
    {
        $this->processStep($this->getBracketPairs(...));
        $this->processN0();
        $this->processStep($this->processN1(...));
        $this->processStep($this->processN2(...));
    }

    /**
     * BD16. Find all bracket pairs
     */
    protected function getBracketPairs(int $idx): void
    {
        if ($this->bstackFull) {
            return;
        }

        $item = $this->getItem($idx);
        // BD14 and BD15: only a bracket whose current bidirectional type is ON can be part
        // of a pair, so a bracket retyped by an override (X6) does not pair.
        if ($item['type'] !== 'ON') {
            return;
        }

        $char = $item['char'];
        if (array_key_exists($char, UniBracket::OPEN)) {
            // process open bracket
            if ($char === 0x3008) {
                $char = 0x2329;
            }

            // BD16 uses a fixed 63 element stack: when it overflows, stop processing BD16 for
            // the remainder of the isolating run sequence and keep the pairs found so far.
            if (\count($this->bstack) >= self::MAX_PAIRING_DEPTH) {
                $this->bstackFull = true;
                return;
            }

            $this->bstack[] = [$idx, (int) $char];
            return;
        }

        if (array_key_exists($char, UniBracket::CLOSE)) {
            // process closing bracket
            if ($char === 0x3009) {
                $char = 0x232A;
            }

            // BD16: a closing bracket pairs with the nearest matching opening bracket, so
            // the stack is scanned from the top and popped through the first match only.
            $tmpstack = $this->bstack;
            while ($tmpstack !== []) {
                $item = \array_pop($tmpstack);
                $openBracket = UniBracket::OPEN[$item[1]] ?? null;
                if ($openBracket !== null && $char === $openBracket) {
                    $this->brackets[$item[0]] = $idx;
                    $this->bstack = $tmpstack;
                    break;
                }
            }
        }
    }

    /**
     * Return the normalized char type for the N0 step
     * Within this scope, bidirectional types EN and AN are treated as R.
     *
     * @param string $type Char type
     */
    protected function getN0Type(string $type): string
    {
        return $type === 'AN' || $type === 'EN' ? 'R' : $type;
    }

    /**
     * N0. Process bracket pairs in an isolating run sequence sequentially in the logical order of the text positions
     *     of the opening paired brackets.
     */
    protected function processN0(): void
    {
        // Sort the list of bracket pairs in ascending order based on the text position
        // of the opening paired bracket (done once, after all pairs have been collected).
        \ksort($this->brackets);

        $odir = $this->seq['edir'] === 'L' ? 'R' : 'L';
        // For each bracket-pair element in the list of pairs of text positions
        foreach ($this->brackets as $open => $close) {
            if (!$this->processInsideBrackets($open, $close, $odir)) {
                continue;
            }

            for ($jdx = $open - 1; $jdx >= 0; --$jdx) {
                $btype = $this->getN0Type($this->getItem($jdx)['type']);
                if ($btype === $odir) {
                    // 1. If the preceding strong type is also opposite the embedding direction,
                    //    context is established, so set the type for both brackets in the pair to that direction.
                    $this->setBracketsType($open, $close, $odir);
                    break;
                }

                if ($btype === $this->seq['edir']) {
                    // 2. Otherwise set the type for both brackets in the pair to the embedding direction.
                    $this->setBracketsType($open, $close, $this->seq['edir']);
                    break;
                }
            }

            if ($jdx < 0) {
                $this->setBracketsType($open, $close, $this->seq['sos']);
            }

            // d. Otherwise, there are no strong types within the bracket pair. Therefore, do not set the type for that
            //    bracket pair. Note that if the enclosed text contains no strong types the bracket pairs will both
            //    resolve to the same level when resolved individually using rules N1 and N2.
        }
    }

    /**
     * Inspect the bidirectional types of the characters enclosed within the bracket pair.
     *
     * @param int    $open  Open bracket entry
     * @param int    $close Close bracket entry
     * @param string $odir  Opposite direction (L or R)
     *
     * @return bool True if type has not been found
     */
    protected function processInsideBrackets(int $open, int $close, string $odir): bool
    {
        $opposite = false;
        // a. Inspect the bidirectional types of the characters enclosed within the bracket pair.
        for ($jdx = $open + 1; $jdx < $close; ++$jdx) {
            $btype = $this->getN0Type($this->getItem($jdx)['type']);
            // b. If any strong type (either L or R) matching the embedding direction is found,
            // set the type for both brackets in the pair to match the embedding direction.
            if ($btype === $this->seq['edir']) {
                $this->setBracketsType($open, $close, $this->seq['edir']);
                break;
            }

            if ($btype === $odir) {
                // c. Otherwise, if there is a strong type it must be opposite the embedding direction.
                $opposite = true;
            }
        }

        // Therefore, test for an established context with a preceding strong type by checking backwards before
        // the opening paired bracket until the first strong type (L, R, or sos) is found.
        return $jdx === $close && $opposite;
    }

    /**
     * Set the brackets type
     *
     * @param int    $open  Open bracket entry
     * @param int    $close Close bracket entry
     * @param string $type  Type
     */
    protected function setBracketsType(int $open, int $close, string $type): void
    {
        $this->setItemType($open, $type);
        $this->setItemType($close, $type);

        // Any number of characters that had original bidirectional character type NSM
        // prior to the application of W1 that immediately follow a paired bracket which
        // changed to L or R under N0 should change to match the type of their preceding bracket.
        $next = $close + 1;
        while ($next < $this->seq['length']) {
            $item = $this->getItem($next);
            if ($item['otype'] !== 'NSM') {
                break;
            }

            $this->setItemType($next, $type);
            ++$next;
        }
    }

    /**
     * N1. A sequence of NIs takes the direction of the surrounding strong text if the text on both sides has the same
     *     direction. European and Arabic numbers act as if they were R in terms of their influence on NIs.
     *     The start-of-sequence (sos) and end-of-sequence (eos) types are used at isolating run sequence boundaries.
     *
     * @param int $idx Current character position
     */
    protected function processN1(int $idx): void
    {
        if (!$this->isNI($idx)) {
            return;
        }

        $prev = $this->processN1prev($idx - 1);
        if ($prev === '') {
            return;
        }

        $jdx = $this->getNextN1Char($idx);
        $next = $this->processN1next($jdx);
        if ($next === '') {
            return;
        }

        if ($next === $prev) {
            for ($bdx = $idx; $bdx < $jdx && $bdx < $this->seq['length']; ++$bdx) {
                $this->setItemType($bdx, $next);
            }
        }
    }

    /**
     * Get the previous direction
     *
     * @param int $bdx Position of the preceding character
     *
     * @return string Previous direction
     */
    protected function processN1prev(int $bdx): string
    {
        if ($bdx < 0) {
            return $this->seq['sos'];
        }

        $item = $this->getItem($bdx);
        if (\in_array($item['type'], ['R', 'AN', 'EN'], true)) {
            return 'R';
        }

        if ($item['type'] === 'L') {
            return 'L';
        }

        return '';
    }

    /**
     * Get the next direction
     *
     * @param int $jdx Position of the next character
     *
     * @return string Next direction
     */
    protected function processN1next(int $jdx): string
    {
        if ($jdx >= $this->seq['length']) {
            return $this->seq['eos'];
        }

        $item = $this->getItem($jdx);
        if (\in_array($item['type'], ['R', 'AN', 'EN'], true)) {
            return 'R';
        }

        if ($item['type'] === 'L') {
            return 'L';
        }

        return '';
    }

    /**
     * Return the index of the next valid char for N1
     *
     * @param int $idx Start index
     */
    protected function getNextN1Char(int $idx): int
    {
        $jdx = $idx + 1;
        while ($jdx < $this->seq['length'] && $this->isNI($jdx)) {
            ++$jdx;
        }

        return $jdx;
    }

    /**
     * N2. Any remaining NIs take the embedding direction.
     *
     * @param int $idx Current character position
     */
    protected function processN2(int $idx): void
    {
        if ($this->isNI($idx)) {
            $this->setItemType($idx, $this->seq['edir']);
        }
    }
}
