<?php

declare(strict_types=1);

/**
 * StepW.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Com\Tecnick\Unicode\Bidi;

use Com\Tecnick\Unicode\Data\Constant as UniConstant;

/**
 * Com\Tecnick\Unicode\Bidi\StepW
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class StepW extends \Com\Tecnick\Unicode\Bidi\StepBase
{
    /**
     * Returns the sequence item at the given index.
     *
     * @return array{char: int, i: int, level: int, otype: string, pdimatch: int, pos: int, type: string, x: int}
     */
    private function getItem(int $idx): array
    {
        $item = $this->seq['item'][$idx] ?? null;
        assert($item !== null, 'Expected StepW sequence item at the requested index');

        return $item;
    }

    private function setItemType(int $idx, string $type): void
    {
        $item = $this->getItem($idx);
        $item['type'] = $type;
        $this->seq['item'][$idx] = $item;
    }

    /**
     * Process W steps
     * Resolving Weak Types
     */
    protected function process(): void
    {
        $this->processStep($this->processW1(...));
        $this->processStep($this->processW2(...));
        $this->processStep($this->processW3(...));
        $this->processStep($this->processW4(...));
        $this->processStep($this->processW5(...));
        $this->processStep($this->processW6(...));
        $this->processStep($this->processW7(...));
    }

    /**
     * W1. Examine each nonspacing mark (NSM) in the isolating run sequence, and
     *     change the type of the NSM to Other Neutral if the previous character is an isolate initiator or PDI, and
     *     to the type of the previous character otherwise.
     *     If the NSM is at the start of the isolating run sequence, it will get the type of sos.
     *     (Note that in an isolating run sequence, an isolate initiator followed by an NSM or any type
     *     other than PDI must be an overflow isolate initiator.)
     *
     * @param int $idx Current character position
     */
    protected function processW1(int $idx): void
    {
        $item = $this->getItem($idx);
        if ($item['type'] !== 'NSM') {
            return;
        }

        $jdx = $idx - 1;
        if ($jdx < 0) {
            $this->setItemType($idx, $this->seq['sos']);
            return;
        }

        $prevItem = $this->getItem($jdx);
        if ($prevItem['char'] >= UniConstant::LRI && $prevItem['char'] <= UniConstant::PDI) {
            $this->setItemType($idx, 'ON');
            return;
        }

        $this->setItemType($idx, $prevItem['type']);
    }

    /**
     * W2. Search backward from each instance of a European number until the first strong type (R, L, AL, or sos)
     *     is found. If an AL is found, change the type of the European number to Arabic number.
     *
     * @param int $idx Current character position
     */
    protected function processW2(int $idx): void
    {
        $item = $this->getItem($idx);
        if ($item['type'] !== 'EN') {
            return;
        }

        $jdx = $idx - 1;
        while ($jdx >= 0) {
            $prevItem = $this->getItem($jdx);
            if ($prevItem['type'] === 'AL') {
                $this->setItemType($idx, 'AN');
                break;
            }

            if (\in_array($prevItem['type'], ['R', 'L'], true)) {
                break;
            }

            --$jdx;
        }
    }

    /**
     * W3. Change all ALs to R.
     *
     * @param int $idx Current character position
     */
    protected function processW3(int $idx): void
    {
        if ($this->getItem($idx)['type'] === 'AL') {
            $this->setItemType($idx, 'R');
        }
    }

    /**
     * W4. A single European separator between two European numbers changes to a European number.
     *     A single common separator between two numbers of the same type changes to that type.
     *
     * @param int $idx Current character position
     */
    protected function processW4(int $idx): void
    {
        $item = $this->getItem($idx);
        if (!\in_array($item['type'], ['ES', 'CS'], true)) {
            return;
        }

        $bdx = $idx - 1;
        $fdx = $idx + 1;
        if ($bdx < 0 || $fdx >= $this->seq['length']) {
            return;
        }

        $prevItem = $this->getItem($bdx);
        $nextItem = $this->getItem($fdx);
        if ($prevItem['type'] === $nextItem['type'] && \in_array($prevItem['type'], ['EN', 'AN'], true)) {
            $this->setItemType($idx, $prevItem['type']);
        }
    }

    /**
     * W5. A sequence of European terminators adjacent to European numbers changes to all European numbers.
     *
     * @param int $idx Current character position
     */
    protected function processW5(int $idx): void
    {
        if ($this->getItem($idx)['type'] !== 'ET') {
            return;
        }

        $this->processW5a($idx);
        $this->processW5b($idx);
    }

    /**
     * W5a
     *
     * @param int $idx Current character position
     */
    protected function processW5a(int $idx): void
    {
        for ($jdx = $idx - 1; $jdx >= 0; --$jdx) {
            if ($this->getItem($jdx)['type'] !== 'EN') {
                break;
            }

            $this->setItemType($idx, 'EN');
        }
    }

    /**
     * W5b
     *
     * @param int $idx Current character position
     */
    protected function processW5b(int $idx): void
    {
        if ($this->getItem($idx)['type'] !== 'ET') {
            return;
        }

        for ($jdx = $idx + 1; $jdx < $this->seq['length']; ++$jdx) {
            $nextItem = $this->getItem($jdx);
            if ($nextItem['type'] === 'EN') {
                $this->setItemType($idx, 'EN');
                continue;
            }

            if ($nextItem['type'] !== 'ET') {
                break;
            }
        }
    }

    /**
     * W6. Otherwise, separators and terminators change to Other Neutral.
     *
     * @param int $idx Current character position
     */
    protected function processW6(int $idx): void
    {
        if (\in_array($this->getItem($idx)['type'], ['ET', 'ES', 'CS', 'ON'], true)) {
            $this->setItemType($idx, 'ON');
        }
    }

    /**
     * W7. Search backward from each instance of a European number until the first strong type (R, L, or sos) is found.
     *     If an L is found, then change the type of the European number to L.
     *
     * @param int $idx Current character position
     */
    protected function processW7(int $idx): void
    {
        if ($this->getItem($idx)['type'] !== 'EN') {
            return;
        }

        for ($jdx = $idx - 1; $jdx >= 0; --$jdx) {
            $prevItem = $this->getItem($jdx);
            if ($prevItem['type'] === 'L') {
                $this->setItemType($idx, 'L');
                break;
            }

            if ($prevItem['type'] === 'R') {
                break;
            }
        }

        if ($this->seq['sos'] === 'L' && $jdx < 0) {
            $this->setItemType($idx, 'L');
        }
    }
}
