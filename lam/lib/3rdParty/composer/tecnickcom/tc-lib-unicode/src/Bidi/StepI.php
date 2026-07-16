<?php

declare(strict_types=1);

/**
 * StepI.php
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

/**
 * Com\Tecnick\Unicode\Bidi\StepI
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class StepI extends \Com\Tecnick\Unicode\Bidi\StepBase
{
    /**
     * Process I steps
     */
    protected function process(): void
    {
        $this->seq['maxlevel'] = 0;
        $this->processStep($this->processI(...));
    }

    /**
     * I1. For all characters with an even (left-to-right) embedding level, those of type R go up one level and those
     *     of type AN or EN go up two levels.
     * I2. For all characters with an odd (right-to-left) embedding level, those of type L, EN or AN go up one level.
     *
     * @param int $idx Current character position
     */
    protected function processI(int $idx): void
    {
        $item = $this->seq['item'][$idx] ?? null;
        assert($item !== null, 'Expected StepI sequence item at current index');

        $odd = $item['level'] % 2;
        if ($odd !== 0) {
            if ($item['type'] === 'L' || $item['type'] === 'EN' || $item['type'] === 'AN') {
                ++$item['level'];
            }

            $this->seq['item'][$idx] = $item;
            $this->seq['maxlevel'] = (int) \max($this->seq['maxlevel'], $item['level']);

            return;
        }

        if ($item['type'] === 'R') {
            ++$item['level'];
        }

        if ($item['type'] === 'AN' || $item['type'] === 'EN') {
            $item['level'] += 2;
        }

        $this->seq['item'][$idx] = $item;

        // update the maximum level
        $this->seq['maxlevel'] = (int) \max($this->seq['maxlevel'], $item['level']);
    }
}
