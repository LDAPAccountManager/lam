<?php

declare(strict_types=1);

/**
 * Bidi.php
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

namespace Com\Tecnick\Unicode;

use Com\Tecnick\Unicode\Bidi\Shaping;
use Com\Tecnick\Unicode\Bidi\StepI;
use Com\Tecnick\Unicode\Bidi\StepL;
use Com\Tecnick\Unicode\Bidi\StepN;
use Com\Tecnick\Unicode\Bidi\StepP;
use Com\Tecnick\Unicode\Bidi\StepW;
use Com\Tecnick\Unicode\Bidi\StepX;
use Com\Tecnick\Unicode\Bidi\StepXten;
use Com\Tecnick\Unicode\Data\Constant as UniConstant;
use Com\Tecnick\Unicode\Data\Type as UniType;
use Com\Tecnick\Unicode\Exception as UnicodeException;

/**
 * Com\Tecnick\Unicode\Bidi
 *
 * Unicode Bidirectional Algorithm (UAX #9): reorders each paragraph of the input from
 * logical to visual order and applies the Arabic shaping. The shaped logical order with
 * the resolved levels is also available, to reorder each line after line breaking.
 * https://www.unicode.org/reports/tr9/
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
class Bidi
{
    /**
     * The input contains strong right-to-left characters (bidi class R, AL or AN)
     */
    public const CONTAINS_RTL = 1;

    /**
     * The input contains Arabic characters (bidi class AL or AN)
     */
    public const CONTAINS_ARABIC = 2;

    /**
     * The input contains explicit formatting characters
     * (LRE, RLE, PDF, LRO, RLO, LRI, RLI, FSI, PDI)
     */
    public const CONTAINS_FORMATTING = 4;

    /**
     * The input contains characters of bidi class BN, which rule X9 removes
     * (ZWJ, ZWNJ, ZWNBSP, SOFT HYPHEN and most of the control characters)
     */
    public const CONTAINS_REMOVED = 8;

    /**
     * Characters of bidi class BN that control line breaking and that are kept in the
     * logical output when the constructor is called with $keepbreaks set to true:
     * SOFT HYPHEN, ZERO WIDTH SPACE, WORD JOINER and ZERO WIDTH NO-BREAK SPACE.
     *
     * @var array<int, true>
     */
    public const LINE_BREAK_CONTROLS = [
        0x00AD => true,
        0x200B => true,
        0x2060 => true,
        0xFEFF => true,
    ];

    /**
     * String to process
     */
    protected string $str = '';

    /**
     * Array of UTF-8 chars
     *
     * @var array<string>
     */
    protected array $chrarr = [];

    /**
     * Array of UTF-8 codepoints
     *
     * @var array<int>
     */
    protected array $ordarr = [];

    /**
     * Processed string (null until it is built)
     */
    protected ?string $bidistr = null;

    /**
     * Array of processed UTF-8 chars (null until it is built)
     *
     * @var ?array<string>
     */
    protected ?array $bidichrarr = null;

    /**
     * Array of processed UTF-8 codepoints
     *
     * @var array<int>
     */
    protected array $bidiordarr = [];

    /**
     * Logical order output, after rule X9 and the shaping: codepoints ('ord'), resolved
     * embedding levels ('level') and paragraph embedding levels ('pel'), one entry per
     * codepoint.
     *
     * @var array{ord: array<int>, level: array<int>, pel: array<int>}
     */
    protected array $logical = [
        'ord' => [],
        'level' => [],
        'pel' => [],
    ];

    /**
     * If 'R' forces RTL, if 'L' forces LTR
     */
    protected string $forcedir = '';

    /**
     * Content flags of the input, as a combination of the CONTAINS_* constants
     */
    protected int $content = 0;

    /**
     * Convert object
     */
    protected Convert $conv;

    /**
     * Process the input with the Bidirectional Algorithm
     *
     * @param ?string $str      String to convert (if null it will be generated from $chrarr or $ordarr)
     * @param ?array<string>  $chrarr   Array of UTF-8 chars (if empty it will be generated from $str or $ordarr)
     * @param ?array<int>  $ordarr   Array of UTF-8 codepoints (if empty it will be generated from $str or $chrarr)
     * @param string|TextDirection $forcedir If 'R' forces RTL, if 'L' forces LTR ('' auto), or a TextDirection case
     * @param bool   $shaping  If true enable the shaping algorithm
     * @param bool   $keepbreaks If true the LINE_BREAK_CONTROLS characters are kept in the
     *                           logical output (getLogicalOrdArray()), each with the level of
     *                           the character before it, or the paragraph level at the start.
     *                           They take no part in the resolution of the levels.
     *
     * @throws UnicodeException
     *
     * @mago-expect lint:excessive-parameter-list
     */
    public function __construct(
        ?string $str = null,
        ?array $chrarr = null,
        ?array $ordarr = null,
        string|TextDirection $forcedir = '',
        bool $shaping = true,
        bool $keepbreaks = false,
    ) {
        if ($str === null && ($chrarr === null || $chrarr === []) && ($ordarr === null || $ordarr === [])) {
            throw new UnicodeException('empty input');
        }

        $this->conv = new Convert();
        $this->setInput($str, $chrarr, $ordarr, $forcedir);
        $this->scanInput($keepbreaks);

        // The algorithm is the identity on left-to-right text with nothing for rule X9 to
        // remove: the explicit formatting characters and the characters of bidi class BN
        // are dropped whatever the direction is.
        if (!$this->isRtlMode() && !$this->hasFormatting() && !$this->hasRemoved()) {
            $this->bidistr = $this->str;
            $this->bidichrarr = $this->chrarr;
            $this->bidiordarr = $this->ordarr;
            $levels = \array_fill(0, \count($this->ordarr), 0);
            $this->logical = [
                'ord' => $this->ordarr,
                'level' => $levels,
                'pel' => $levels,
            ];
            return;
        }

        $this->process($shaping && $this->isArabic(), $keepbreaks);
    }

    /**
     * Set Input data
     *
     * @param ?string $str      String to convert (if null it will be generated from $chrarr or $ordarr)
     * @param ?array<string>  $chrarr   Array of UTF-8 chars (if empty it will be generated from $str or $ordarr)
     * @param ?array<int>  $ordarr   Array of UTF-8 codepoints (if empty it will be generated from $str or $chrarr)
     * @param string|TextDirection $forcedir If 'R' forces RTL, if 'L' forces LTR ('' auto), or a TextDirection case
     *
     * @throws UnicodeException
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function setInput(
        ?string $str = null,
        ?array $chrarr = null,
        ?array $ordarr = null,
        string|TextDirection $forcedir = '',
    ): void {
        if ($str === null) {
            $str = '';
            if (($chrarr === null || $chrarr === []) && ($ordarr !== null && $ordarr !== [])) {
                $chrarr = $this->conv->ordArrToChrArr($ordarr);
            }
            if ($chrarr !== null && $chrarr !== []) {
                $str = \implode('', $chrarr);
            }
        }

        if ($chrarr === null || $chrarr === []) {
            $chrarr = $this->conv->strToChrArr($str);
        }

        if ($ordarr === null || $ordarr === []) {
            $ordarr = $this->conv->chrArrToOrdArr($chrarr);
        }

        if (\count($chrarr) !== \count($ordarr)) {
            throw new UnicodeException('the input forms contain a different number of characters');
        }

        $this->str = $str;
        $this->chrarr = $chrarr;
        $this->ordarr = $ordarr;
        $this->forcedir = TextDirection::fromLoose($forcedir)->value;
    }

    /**
     * Classify the input once into the content flags used to decide whether the
     * bidirectional algorithm and the shaping have to run.
     *
     * @param bool $keepbreaks True if the LINE_BREAK_CONTROLS characters are kept.
     */
    protected function scanInput(bool $keepbreaks = false): void
    {
        foreach ($this->ordarr as $ord) {
            if (
                $ord >= UniConstant::LRE && $ord <= UniConstant::RLO
                || $ord >= UniConstant::LRI && $ord <= UniConstant::PDI
            ) {
                $this->content |= self::CONTAINS_FORMATTING;
                continue;
            }

            $type = UniType::getType($ord);
            if ($type === 'AL' || $type === 'AN') {
                $this->content |= self::CONTAINS_ARABIC | self::CONTAINS_RTL;
                continue;
            }

            if ($type === 'R') {
                $this->content |= self::CONTAINS_RTL;
                continue;
            }

            if ($type === 'BN' && !($keepbreaks && isset(self::LINE_BREAK_CONTROLS[$ord]))) {
                $this->content |= self::CONTAINS_REMOVED;
            }
        }
    }

    /**
     * Returns the processed array of UTF-8 codepoints
     *
     * @return array<int>
     */
    public function getOrdArray(): array
    {
        return $this->bidiordarr;
    }

    /**
     * Returns the codepoints in logical order, without the characters removed by rule X9
     * and with the Arabic shaping applied (rules L1 to L4 not applied).
     *
     * @return array<int>
     */
    public function getLogicalOrdArray(): array
    {
        return $this->logical['ord'];
    }

    /**
     * Returns the resolved embedding level of each entry of getLogicalOrdArray().
     *
     * @return array<int>
     */
    public function getLogicalLevels(): array
    {
        return $this->logical['level'];
    }

    /**
     * Returns the paragraph embedding level of each entry of getLogicalOrdArray().
     *
     * @return array<int>
     */
    public function getLogicalParagraphLevels(): array
    {
        return $this->logical['pel'];
    }

    /**
     * Reorders one line of one paragraph from logical to visual order (rules L1, L2 and L4).
     *
     * @param array<int> $ordarr Codepoints of the line in logical order
     * @param array<int> $levels Resolved embedding level of each codepoint
     * @param int        $pel    Paragraph embedding level
     *
     * @return array<int> Codepoints in visual order
     *
     * @throws UnicodeException
     */
    public static function reorderLine(array $ordarr, array $levels, int $pel): array
    {
        $ordarr = \array_values($ordarr);
        $levels = \array_values($levels);
        if (\count($ordarr) !== \count($levels)) {
            throw new UnicodeException('the codepoints and the levels have a different number of entries');
        }

        $maxlevel = $pel;
        $chardata = [];
        foreach ($ordarr as $idx => $ord) {
            $level = $levels[$idx] ?? $pel;
            $type = UniType::getType($ord);
            $chardata[] = [
                'char' => $ord,
                'i' => $idx,
                'level' => $level,
                'otype' => $type,
                'pdimatch' => -1,
                'pos' => $idx,
                'type' => $type,
                'x' => $level,
            ];

            if ($level > $maxlevel) {
                $maxlevel = $level;
            }
        }

        $stepl = new StepL($chardata, $pel, $maxlevel);
        $visual = [];
        foreach ($stepl->getChrData() as $chardatum) {
            $visual[] = $chardatum['char'];
        }

        return $visual;
    }

    /**
     * Returns the processed array of UTF-8 chars
     *
     * @return array<string>
     *
     * @throws UnicodeException
     */
    public function getChrArray(): array
    {
        if ($this->bidichrarr === null) {
            $this->bidichrarr = $this->conv->ordArrToChrArr($this->bidiordarr);
        }

        return $this->bidichrarr;
    }

    /**
     * Returns the number of characters in the processed string
     *
     * @throws UnicodeException
     */
    public function getNumChars(): int
    {
        return \count($this->getChrArray());
    }

    /**
     * Returns the processed string
     *
     * @throws UnicodeException
     */
    public function getString(): string
    {
        if ($this->bidistr === null) {
            $this->bidistr = \implode('', $this->getChrArray());
        }

        return $this->bidistr;
    }

    /**
     * Returns an array with processed chars as keys
     *
     * @return array<int, true>
     */
    public function getCharKeys(): array
    {
        return \array_fill_keys($this->bidiordarr, true);
    }

    /**
     * P1. Split the text into separate paragraphs.
     *     A paragraph separator is kept with the previous paragraph.
     *
     * @return array<int, array<int, int>>
     */
    protected function getParagraphs(): array
    {
        $paragraph = [
            0 => [],
        ];
        $pdx = 0; // paragraphs index
        foreach ($this->ordarr as $ord) {
            $paragraph[$pdx][] = $ord;
            if (UniType::getType($ord) === 'B') {
                ++$pdx;
                $paragraph[$pdx] = [];
            }
        }

        return $paragraph;
    }

    /**
     * Process the string
     *
     * @param bool $shaping If true apply the Arabic shaping
     * @param bool $keepbreaks True to keep the LINE_BREAK_CONTROLS characters
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function process(bool $shaping, bool $keepbreaks = false): void
    {
        // split the text into separate paragraphs.
        $paragraph = $this->getParagraphs();

        // Within each paragraph, apply all the other rules of this algorithm.
        foreach ($paragraph as $par) {
            // A trailing paragraph separator produces an empty final paragraph.
            if ($par === []) {
                continue;
            }

            $pel = $this->getPel($par);
            $stepx = new StepX($par, $pel);
            $stepx10 = new StepXten($stepx->getChrData(), $pel);
            $ilrs = $stepx10->getIsolatedLevelRunSequences();
            // The joining context depends only on the paragraph, so it is shared by all
            // the isolating run sequences of that paragraph.
            $joining = $shaping ? Shaping::getJoiningTypes($par) : [];
            $chardata = [];
            $maxlevel = 0;
            foreach ($ilrs as $ilr) {
                $stepw = new StepW($ilr);
                $stepn = new StepN($stepw->getSequence());
                $stepi = new StepI($stepn->getSequence());
                $ilr = $stepi->getSequence();
                if ($shaping) {
                    $ilr = (new Shaping($ilr, $par, $joining))->getSequence();
                }

                \array_push($chardata, ...$ilr['item']);

                if ($ilr['maxlevel'] > $maxlevel) {
                    $maxlevel = $ilr['maxlevel'];
                }
            }

            // logical order, before the L rules
            \usort($chardata, static fn($apos, $bpos): int => $apos['pos'] - $bpos['pos']);
            $this->addLogicalChars($par, $chardata, $pel, $keepbreaks);

            $stepl = new StepL($chardata, $pel, $maxlevel);
            $chardata = $stepl->getChrData();
            foreach ($chardata as $chardatum) {
                $this->bidiordarr[] = $chardatum['char'];
            }

            // add back the paragraph separators ($par is non-empty, so end() returns a codepoint)
            $lastchar = \end($par);
            if ($lastchar < 0) {
                continue;
            }

            if (UniType::getType($lastchar) !== 'B') {
                continue;
            }

            $this->bidiordarr[] = $lastchar;
            $this->logical['ord'][] = $lastchar;
            $this->logical['level'][] = $pel;
            $this->logical['pel'][] = $pel;
        }
    }

    /**
     * Append the characters of a paragraph to the logical output, with the kept line
     * break controls inserted back at their positions.
     *
     * @param array<int> $par      Paragraph codepoints.
     * @param array<int, CharData> $chardata Resolved characters in logical order.
     * @param int        $pel      Paragraph embedding level.
     * @param bool       $keepbreaks True to insert the kept line break controls.
     */
    protected function addLogicalChars(array $par, array $chardata, int $pel, bool $keepbreaks): void
    {
        $kept = [];
        if ($keepbreaks) {
            foreach ($par as $pos => $ord) {
                if (!isset(self::LINE_BREAK_CONTROLS[$ord])) {
                    continue;
                }

                $kept[] = [$pos, $ord];
            }
        }

        $level = $pel;
        $next = 0;
        foreach ($chardata as $chardatum) {
            while (isset($kept[$next]) && $kept[$next][0] < $chardatum['pos']) {
                $this->addLogicalChar($kept[$next][1], $level, $pel);
                ++$next;
            }

            $this->addLogicalChar($chardatum['char'], $chardatum['level'], $pel);
            $level = $chardatum['level'];
        }

        while (isset($kept[$next])) {
            $this->addLogicalChar($kept[$next][1], $level, $pel);
            ++$next;
        }
    }

    /**
     * Append one character to the logical output.
     */
    protected function addLogicalChar(int $ord, int $level, int $pel): void
    {
        $this->logical['ord'][] = $ord;
        $this->logical['level'][] = $level;
        $this->logical['pel'][] = $pel;
    }

    /**
     * Get the paragraph embedding level
     *
     * @param array<int> $par Paragraph
     */
    protected function getPel(array $par): int
    {
        if ($this->forcedir === 'R') {
            return 1;
        }

        if ($this->forcedir === 'L') {
            return 0;
        }

        $stepp = new StepP($par);
        return $stepp->getPel();
    }

    /**
     * Check if the input contains Arabic characters
     */
    protected function isArabic(): bool
    {
        return ($this->content & self::CONTAINS_ARABIC) !== 0;
    }

    /**
     * Check if the input contains explicit formatting characters
     */
    protected function hasFormatting(): bool
    {
        return ($this->content & self::CONTAINS_FORMATTING) !== 0;
    }

    /**
     * Check if the input contains characters that rule X9 removes
     */
    protected function hasRemoved(): bool
    {
        return ($this->content & self::CONTAINS_REMOVED) !== 0;
    }

    /**
     * Check if the input contains right-to-left characters to process
     */
    protected function isRtlMode(): bool
    {
        return $this->forcedir === 'R' || ($this->content & self::CONTAINS_RTL) !== 0;
    }
}
