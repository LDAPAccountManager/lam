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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Com\Tecnick\Unicode\Bidi\Shaping;

use Com\Tecnick\Unicode\Data\Arabic as UniArabic;

/**
 * Com\Tecnick\Unicode\Bidi\Shaping\Arabic
 *
 * Joining context of the Arabic shaping: the form of a character is decided by the
 * Joining_Type of the nearest non-transparent characters around it, as defined by
 * ArabicShaping.txt and section 9.2 of the Unicode standard.
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
     * Index of the isolated form in the substitution rows.
     */
    protected const FORM_ISOLATED = 0;

    /**
     * Index of the final form in the substitution rows.
     */
    protected const FORM_FINAL = 1;

    /**
     * Index of the initial form in the substitution rows.
     */
    protected const FORM_INITIAL = 2;

    /**
     * Index of the medial form in the substitution rows.
     */
    protected const FORM_MEDIAL = 3;

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
     * Codepoints of the paragraph the sequence belongs to, in logical order.
     * The joining context is read from the paragraph and not from the sequence because
     * X9 removes the joining-relevant format characters (ZWJ and ZWNJ) from the latter.
     *
     * @var array<int, int>
     */
    protected array $paragraph = [];

    /**
     * Joining_Type of each paragraph position
     *
     * @var array<int, string>
     */
    protected array $joining = [];

    /**
     * Index of the sequence item of each paragraph position
     *
     * @var array<int, int>
     */
    protected array $seqindex = [];

    /**
     * Joining_Type of every position of the given paragraph.
     *
     * @param array<int, int> $paragraph Codepoints of the paragraph, in logical order
     *
     * @return array<int, string>
     */
    public static function getJoiningTypes(array $paragraph): array
    {
        $joining = [];
        foreach ($paragraph as $pos => $ord) {
            $joining[$pos] = UniArabic::getJoiningType($ord);
        }

        return $joining;
    }

    /**
     * Build the joining type of every paragraph position and the map from a paragraph
     * position to the sequence item that holds it. The joining types are kept when they
     * have been supplied by the caller: they depend only on the paragraph, which is
     * shared by all the isolating run sequences of that paragraph.
     */
    protected function setJoiningContext(): void
    {
        if ($this->joining === []) {
            $this->joining = self::getJoiningTypes($this->paragraph);
        }

        $this->seqindex = [];
        foreach ($this->seq['item'] as $idx => $item) {
            $this->seqindex[$item['pos']] = $idx;
        }
    }

    /**
     * Position of the nearest non-transparent character before the given one,
     * or null at the beginning of the paragraph.
     */
    protected function getPrevPosition(int $pos): ?int
    {
        for ($idx = $pos - 1; $idx >= 0; --$idx) {
            if (($this->joining[$idx] ?? 'U') !== 'T') {
                return $idx;
            }
        }

        return null;
    }

    /**
     * Position of the nearest non-transparent character after the given one,
     * or null at the end of the paragraph.
     */
    protected function getNextPosition(int $pos): ?int
    {
        $length = \count($this->paragraph);
        for ($idx = $pos + 1; $idx < $length; ++$idx) {
            if (($this->joining[$idx] ?? 'U') !== 'T') {
                return $idx;
            }
        }

        return null;
    }

    /**
     * Returns the Joining_Type of a paragraph position (Non_Joining outside the paragraph).
     */
    protected function getJoiningType(?int $pos): string
    {
        if ($pos === null) {
            return 'U';
        }

        return $this->joining[$pos] ?? 'U';
    }

    /**
     * True when a character of the given joining type connects to the character that
     * follows it: Dual_Joining, Join_Causing and Left_Joining do.
     */
    protected function linksForward(string $type): bool
    {
        return $type === 'D' || $type === 'C' || $type === 'L';
    }

    /**
     * True when a character of the given joining type connects to the character that
     * precedes it: Dual_Joining, Join_Causing and Right_Joining do.
     */
    protected function linksBackward(string $type): bool
    {
        return $type === 'D' || $type === 'C' || $type === 'R';
    }

    /**
     * True when the character at the given paragraph position connects to the preceding one.
     */
    protected function joinsPrev(int $pos): bool
    {
        return (
            $this->linksBackward($this->getJoiningType($pos))
            && $this->linksForward($this->getJoiningType($this->getPrevPosition($pos)))
        );
    }

    /**
     * True when the character at the given paragraph position connects to the following one.
     */
    protected function joinsNext(int $pos): bool
    {
        return (
            $this->linksForward($this->getJoiningType($pos))
            && $this->linksBackward($this->getJoiningType($this->getNextPosition($pos)))
        );
    }

    /**
     * Index of the presentation form of the character at the given paragraph position.
     */
    protected function getForm(int $pos): int
    {
        $prev = $this->joinsPrev($pos);
        $next = $this->joinsNext($pos);

        return match (true) {
            $prev && $next => self::FORM_MEDIAL,
            $prev => self::FORM_FINAL,
            $next => self::FORM_INITIAL,
            default => self::FORM_ISOLATED,
        };
    }

    /**
     * Replace the character of a sequence item, or mark it for deletion with -1.
     */
    protected function setNewChar(int $idx, int $char): void
    {
        $item = $this->newchardata[$idx] ?? null;
        assert($item !== null, 'Expected shaped character at the requested index');
        $item['char'] = $char;
        $this->newchardata[$idx] = $item;
    }

    /**
     * Returns the codepoint at the given paragraph position, or null outside of it.
     */
    protected function getParagraphChar(?int $pos): ?int
    {
        if ($pos === null) {
            return null;
        }

        return $this->paragraph[$pos] ?? null;
    }
}
