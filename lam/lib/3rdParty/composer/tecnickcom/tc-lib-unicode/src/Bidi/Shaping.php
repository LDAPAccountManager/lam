<?php

declare(strict_types=1);

/**
 * Shaping.php
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

use Com\Tecnick\Unicode\Data\Arabic as UniArabic;
use Com\Tecnick\Unicode\Data\Constant as UniConstant;

/**
 * Com\Tecnick\Unicode\Bidi\Shaping
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
 */
class Shaping extends \Com\Tecnick\Unicode\Bidi\Shaping\Arabic
{
    /**
     * @return array{char: int, i: int, level: int, otype: string, pdimatch: int, pos: int, type: string, x: int}
     */
    private function getSeqItem(int $idx): array
    {
        $item = $this->seq['item'][$idx] ?? null;
        assert($item !== null, 'Expected shaping sequence item at the requested index');

        return $item;
    }

    /**
     * Shaping
     * Cursively connected scripts, such as Arabic or Syriac,
     * require the selection of positional character shapes that depend on adjacent characters.
     * Shaping is logically applied after the Bidirectional Algorithm is used and is limited to
     * characters within the same directional run.
     *
     * @param SeqData $seq isolated Sequence array
     */
    public function __construct(array $seq)
    {
        $this->seq = $seq;
        $this->newchardata = $seq['item'];
        $this->process();
    }

    /**
     * Returns the processed sequence
     *
     * @return SeqData
     */
    public function getSequence(): array
    {
        return $this->seq;
    }

    /**
     * Process
     */
    protected function process(): void
    {
        $this->setAlChars();
        for ($idx = 0; $idx < $this->seq['length']; ++$idx) {
            $thischar = $this->getSeqItem($idx);
            if ($thischar['otype'] !== 'AL') {
                continue;
            }

            $pos = $thischar['x'];
            $prevchar = $pos > 0 ? $this->alchars[$pos - 1] ?? null : null;
            $nextchar = ($pos + 1) < $this->numalchars ? $this->alchars[$pos + 1] ?? null : null;
            $this->processAlChar($idx, $pos, $prevchar, $thischar, $nextchar);
        }

        $this->combineShadda();
        $this->removeDeletedChars();
        $this->seq['item'] = \array_values($this->newchardata);
        // Keep 'length' consistent with 'item': removeDeletedChars() may have dropped entries.
        $this->seq['length'] = \count($this->seq['item']);
        $this->newchardata = []; // reset
    }

    /**
     * Set AL chars array
     */
    protected function setAlChars(): void
    {
        $this->numalchars = 0;
        for ($idx = 0; $idx < $this->seq['length']; ++$idx) {
            $item = $this->seq['item'][$idx] ?? null;
            assert($item !== null, 'Expected sequence item while building Arabic shaping cache');
            if (
                $item['otype'] === 'AL'
                || $item['char'] === UniConstant::SPACE
                || $item['char'] === UniConstant::ZERO_WIDTH_NON_JOINER
            ) {
                $this->alchars[$this->numalchars] = $item;
                $item['x'] = $this->numalchars;
                $this->seq['item'][$idx] = $item;
                ++$this->numalchars;
            }
        }
    }

    /**
     * Combine characters that can occur with Arabic Shadda (0651 HEX, 1617 DEC).
     * Putting the combining mark and shadda in the same glyph allows
     * to avoid the two marks overlapping each other in an illegible manner.
     */
    protected function combineShadda(): void
    {
        $last = $this->seq['length'] - 1;
        for ($idx = 0; $idx < $last; ++$idx) {
            $currentItem = $this->newchardata[$idx] ?? null;
            $nextItem = $this->newchardata[$idx + 1] ?? null;
            assert(
                $currentItem !== null && $nextItem !== null,
                'Expected adjacent chars while combining Arabic shadda',
            );

            $cur = $currentItem['char'];
            $nxt = $nextItem['char'];
            $diacritic = $nxt >= 0 ? UniArabic::DIACRITIC[$nxt] ?? null : null;
            if ($cur === UniArabic::SHADDA && $diacritic !== null) {
                $currentItem['char'] = -1;
                $nextItem['char'] = $diacritic;
                $this->newchardata[$idx] = $currentItem;
                $this->newchardata[$idx + 1] = $nextItem;
            }
        }
    }

    /**
     * Remove marked characters
     */
    protected function removeDeletedChars(): void
    {
        foreach ($this->newchardata as $key => $value) {
            if ($value['char'] >= 0) {
                continue;
            }

            unset($this->newchardata[$key]);
        }
    }
}
