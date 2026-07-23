<?php

declare(strict_types=1);

/**
 * Arabic.php
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

namespace Com\Tecnick\Unicode\Bidi\Shaping;

use Com\Tecnick\Unicode\Data\Arabic as UniArabic;

/**
 * Com\Tecnick\Unicode\Bidi\Shaping\Arabic
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * @phpstan-type CharData array{
 *     'char': int,
 *     'i': int,
 *     'level': int,
 *     'otype': string,
 *     'pdimatch': int,
 *     'pos': int,
 *     'type': string,
 *     'x': int,
 * }
 *
 * @phpstan-type SeqData array{
 *     'e': int,
 *     'edir': string,
 *     'end': int,
 *     'eos': string,
 *     'length': int,
 *     'maxlevel': int,
 *     'sos': string,
 *     'start': int,
 *     'item': array<int, CharData>,
 * }
 */
abstract class Arabic
{
    /**
     * Sequence to process and return
     *
     * @var SeqData
     */
    protected array $seq = [
        'e' => 0,
        'edir' => '',
        'end' => 0,
        'eos' => '',
        'length' => 0,
        'maxlevel' => 0,
        'sos' => '',
        'start' => 0,
        'item' => [],
    ];

    /**
     * Array of processed chars
     *
     * @var array<int, CharData>
     */
    protected array $newchardata = [];

    /**
     * Array of AL characters
     *
     * @var array<int, CharData>
     */
    protected array $alchars = [];

    /**
     * Number of AL characters
     */
    protected int $numalchars = 0;

    /**
     * @param array<int, array<int>> $arabicarr
     */
    private function getSubstitute(array $arabicarr, int $char, int $form): ?int
    {
        $forms = $arabicarr[$char] ?? null;
        if ($forms === null) {
            return null;
        }

        $substitute = $forms[$form] ?? null;
        if (!\is_int($substitute)) {
            return null;
        }

        return $substitute;
    }

    private function setNewChar(int $idx, int $char): void
    {
        $item = $this->newchardata[$idx] ?? null;
        assert($item !== null, 'Expected shaped character at the requested index');
        $item['char'] = $char;
        $this->newchardata[$idx] = $item;
    }

    private function getNewCharIndexBySourceIndex(int $sourceIndex): ?int
    {
        foreach ($this->newchardata as $idx => $item) {
            if ($item['i'] === $sourceIndex) {
                return $idx;
            }
        }

        return null;
    }

    /**
     * Check if it is a LAA LETTER
     *
     * @param ?CharData $prevchar Previous char
     * @param CharData  $thischar Current char
     */
    protected function isLaaLetter(?array $prevchar, array $thischar): bool
    {
        return $prevchar !== null
        && $prevchar['char'] === UniArabic::LAM
        && array_key_exists($thischar['char'], UniArabic::LAA);
    }

    /**
     * Check next char
     *
     * @param CharData  $thischar Current char
     * @param ?CharData $nextchar Next char
     */
    protected function hasNextChar(array $thischar, ?array $nextchar): bool
    {
        return (
            $nextchar !== null
            && ($nextchar['otype'] === 'AL' || $nextchar['otype'] === 'NSM')
            && $nextchar['type'] === $thischar['type']
            && $nextchar['char'] !== UniArabic::QUESTION_MARK
        );
    }

    /**
     * Check previous char
     *
     * @param ?CharData $prevchar Previous char
     * @param CharData  $thischar Current char
     */
    protected function hasPrevChar(?array $prevchar, array $thischar): bool
    {
        return (
            $prevchar !== null
            && ($prevchar['otype'] === 'AL' || $prevchar['otype'] === 'NSM')
            && $prevchar['type'] === $thischar['type']
        );
    }

    /**
     * Check if it is a middle character
     *
     * @param ?CharData $prevchar Previous char
     * @param CharData  $thischar Current char
     * @param ?CharData $nextchar Next char
     */
    protected function isMiddleChar(?array $prevchar, array $thischar, ?array $nextchar): bool
    {
        return $this->hasPrevChar($prevchar, $thischar) && $this->hasNextChar($thischar, $nextchar);
    }

    /**
     * Check if it is a final character
     *
     * @param ?CharData $prevchar Previous char
     * @param CharData  $thischar Current char
     * @param ?CharData $nextchar Next char
     */
    protected function isFinalChar(?array $prevchar, array $thischar, ?array $nextchar): bool
    {
        if ($this->hasPrevChar($prevchar, $thischar)) {
            return true;
        }

        return $nextchar !== null && $nextchar['char'] === UniArabic::QUESTION_MARK;
    }

    /**
     * Set initial or middle char
     *
     * @param int                    $idx       Current index
     * @param ?CharData              $prevchar  Previous char
     * @param CharData               $thischar  Current char
     * @param array<int, array<int>> $arabicarr Substitution array
     */
    protected function setMiddleChar(int $idx, ?array $prevchar, array $thischar, array $arabicarr): void
    {
        if ($prevchar !== null && \in_array($prevchar['char'], UniArabic::END, true)) {
            $substitute = $this->getSubstitute($arabicarr, $thischar['char'], 2);
            if ($substitute !== null) {
                // initial
                $this->setNewChar($idx, $substitute);
            }

            return;
        }

        $substitute = $this->getSubstitute($arabicarr, $thischar['char'], 3);
        if ($substitute !== null) {
            // medial
            $this->setNewChar($idx, $substitute);
        }
    }

    /**
     * Set initial char
     *
     * @param int                    $idx       Current index
     * @param CharData               $thischar  Current char
     * @param array<int, array<int>> $arabicarr Substitution array
     */
    protected function setInitialChar(int $idx, array $thischar, array $arabicarr): void
    {
        $substitute = $this->getSubstitute($arabicarr, $thischar['char'], 2);
        if ($substitute !== null) {
            $this->setNewChar($idx, $substitute);
        }
    }

    /**
     * Set final char
     *
     * @param int                    $idx       Current index
     * @param ?CharData              $prevchar  Previous char
     * @param CharData               $thischar  Current char
     * @param array<int, array<int>> $arabicarr Substitution array
     */
    protected function setFinalChar(int $idx, ?array $prevchar, array $thischar, array $arabicarr): void
    {
        $prevItem = $idx > 0 ? $this->seq['item'][$idx - 1] ?? null : null;
        $prevPrevItem = $idx > 1 ? $this->seq['item'][$idx - 2] ?? null : null;
        if (
            $idx > 1
            && $thischar['char'] === UniArabic::HEH
            && $prevItem !== null
            && $prevPrevItem !== null
            && $prevItem['char'] === UniArabic::LAM
            && $prevPrevItem['char'] === UniArabic::LAM
        ) {
            // Allah Word
            $this->setNewChar($idx - 2, -1);
            $this->setNewChar($idx - 1, -1);
            $this->setNewChar($idx, UniArabic::LIGATURE_ALLAH_ISOLATED_FORM);
            return;
        }

        if ($prevchar !== null && \in_array($prevchar['char'], UniArabic::END, true)) {
            $substitute = $this->getSubstitute($arabicarr, $thischar['char'], 0);
            if ($substitute !== null) {
                // isolated
                $this->setNewChar($idx, $substitute);
            }

            return;
        }

        $substitute = $this->getSubstitute($arabicarr, $thischar['char'], 1);
        if ($substitute !== null) {
            // final
            $this->setNewChar($idx, $substitute);
        }
    }

    /**
     * Process AL character
     *
     * @param int       $idx      Current index
     * @param int       $pos      Current char position
     * @param ?CharData $prevchar Previous char
     * @param CharData  $thischar Current char
     * @param ?CharData $nextchar Next char
     */
    protected function processAlChar(int $idx, int $pos, ?array $prevchar, array $thischar, ?array $nextchar): void
    {
        $laaletter = $this->isLaaLetter($prevchar, $thischar);
        $arabicarr = UniArabic::SUBSTITUTE;
        if ($laaletter) {
            $arabicarr = UniArabic::LAA;
            $prevchar = $pos > 1 ? $this->alchars[$pos - 2] ?? null : null;
        }

        $resolved = false;
        if ($this->isMiddleChar($prevchar, $thischar, $nextchar)) {
            $this->setMiddleChar($idx, $prevchar, $thischar, $arabicarr);
            $resolved = true;
        }

        if (!$resolved && $this->hasNextChar($thischar, $nextchar)) {
            $this->setInitialChar($idx, $thischar, $arabicarr);
            $resolved = true;
        }

        if (!$resolved && $this->isFinalChar($prevchar, $thischar, $nextchar)) {
            // final
            $this->setFinalChar($idx, $prevchar, $thischar, $arabicarr);
            $resolved = true;
        }

        if (!$resolved) {
            $substitute = $this->getSubstitute($arabicarr, $thischar['char'], 0);
            if ($substitute !== null) {
                // isolated
                $this->setNewChar($idx, $substitute);
            }
        }

        // if laa letter
        if ($laaletter) {
            // mark characters to delete
            $laaChar = $this->alchars[$pos - 1] ?? null;
            assert($laaChar !== null, 'Expected previous lam character while composing lam-alef ligature');
            $deleteIdx = $this->getNewCharIndexBySourceIndex($laaChar['i']);
            if ($deleteIdx === null) {
                assert(false, 'Expected shaped lam-alef source item before marking it for deletion');
                return;
            }

            $item = $this->newchardata[$deleteIdx] ?? null;
            assert($item !== null, 'Expected shaped lam-alef source item before marking it for deletion');
            $item['char'] = -1;
            $this->newchardata[$deleteIdx] = $item;
        }
    }
}
