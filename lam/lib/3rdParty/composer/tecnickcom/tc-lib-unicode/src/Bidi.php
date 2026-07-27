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
use Com\Tecnick\Unicode\Data\Pattern as UniPattern;
use Com\Tecnick\Unicode\Data\Type as UniType;
use Com\Tecnick\Unicode\Exception as UnicodeException;

/**
 * Com\Tecnick\Unicode\Bidi
 *
 * @since     2015-07-13
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class Bidi
{
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
     * Processed string
     */
    protected string $bidistr = '';

    /**
     * Array of processed UTF-8 chars
     *
     * @var array<string>
     */
    protected array $bidichrarr = [];

    /**
     * Array of processed UTF-8 codepoints
     *
     * @var array<int>
     */
    protected array $bidiordarr = [];

    /**
     * If 'R' forces RTL, if 'L' forces LTR
     */
    protected string $forcedir = '';

    /**
     * If true enable shaping
     */
    protected bool $shaping = true;

    /**
     * Array of character data
     *
     * @var array<int, array{
     *        'char': int,
     *        'i': int,
     *        'level': int,
     *        'otype': string,
     *        'pdimatch': int,
     *        'pos': int,
     *        'type': string,
     *        'x': int,
     *      }>
     */
    protected array $chardata = [];

    /**
     * Convert object
     */
    protected Convert $conv;

    /**
     * Reverse the RTL substrings using the Bidirectional Algorithm
     * http://unicode.org/reports/tr9/
     *
     * @param ?string $str      String to convert (if null it will be generated from $chrarr or $ordarr)
     * @param ?array<string>  $chrarr   Array of UTF-8 chars (if empty it will be generated from $str or $ordarr)
     * @param ?array<int>  $ordarr   Array of UTF-8 codepoints (if empty it will be generated from $str or $chrarr)
     * @param string|TextDirection $forcedir If 'R' forces RTL, if 'L' forces LTR ('' auto), or a TextDirection case
     * @param bool   $shaping  If true enable the shaping algorithm
     *
     * @throws UnicodeException
     */
    public function __construct(
        ?string $str = null,
        ?array $chrarr = null,
        ?array $ordarr = null,
        string|TextDirection $forcedir = '',
        bool $shaping = true,
    ) {
        if ($str === null && ($chrarr === null || $chrarr === []) && ($ordarr === null || $ordarr === [])) {
            throw new UnicodeException('empty input');
        }

        $this->conv = new Convert();
        $this->setInput($str, $chrarr, $ordarr, $forcedir);

        if (!$this->isRtlMode()) {
            $this->bidistr = $this->str;
            $this->bidichrarr = $this->chrarr;
            $this->bidiordarr = $this->ordarr;
            return;
        }

        $this->shaping = $shaping && $this->isArabic();

        $this->process();
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

        $this->str = $str;
        $this->chrarr = $chrarr;
        $this->ordarr = $ordarr;
        $this->forcedir = TextDirection::fromLoose($forcedir)->value;
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
     * Returns the processed array of UTF-8 chars
     *
     * @return array<string>
     *
     * @throws UnicodeException
     */
    public function getChrArray(): array
    {
        if ($this->bidichrarr === []) {
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
        if ($this->bidistr === '') {
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
        return \array_fill_keys(\array_values($this->bidiordarr), true);
    }

    /**
     * P1. Split the text into separate paragraphs.
     *     A paragraph separator is kept with the previous paragraph.
     *
     * @return array<int, array<int>>
     */
    protected function getParagraphs(): array
    {
        $paragraph = [
            0 => [],
        ];
        $pdx = 0; // paragraphs index
        foreach ($this->ordarr as $ord) {
            $paragraph[$pdx][] = $ord;
            $charType = UniType::UNI[$ord] ?? null;
            if ($charType === 'B') {
                ++$pdx;
                $paragraph[$pdx] = [];
            }
        }

        return $paragraph;
    }

    /**
     * Process the string
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function process(): void
    {
        // split the text into separate paragraphs.
        $paragraph = $this->getParagraphs();

        // Within each paragraph, apply all the other rules of this algorithm.
        foreach ($paragraph as $par) {
            // A trailing paragraph separator produces an empty final paragraph; it contributes
            // nothing and would otherwise run through the full Step* pipeline for no result.
            if ($par === []) {
                continue;
            }

            $pel = $this->getPel($par);
            $stepx = new StepX($par, $pel);
            $stepx10 = new StepXten($stepx->getChrData(), $pel);
            $ilrs = $stepx10->getIsolatedLevelRunSequences();
            $chardata = [];
            $maxlevel = 0;
            foreach ($ilrs as $ilr) {
                $stepw = new StepW($ilr);
                $stepn = new StepN($stepw->getSequence());
                $stepi = new StepI($stepn->getSequence());
                $ilr = $stepi->getSequence();
                if ($this->shaping) {
                    $shaping = new Shaping($ilr);
                    $ilr = $shaping->getSequence();
                }

                $chardata = \array_merge($chardata, $ilr['item']);

                if ($ilr['maxlevel'] > $maxlevel) {
                    $maxlevel = $ilr['maxlevel'];
                }
            }

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

            $lastCharType = UniType::UNI[$lastchar] ?? null;
            if ($lastCharType !== 'B') {
                continue;
            }

            $this->bidiordarr[] = $lastchar;
        }
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
     * Check if the input string contains Arabic characters
     */
    protected function isArabic(): bool
    {
        return (bool) \preg_match(UniPattern::ARABIC, $this->str);
    }

    /**
     * Check if the input string contains RTL characters to process
     */
    protected function isRtlMode(): bool
    {
        return $this->forcedir === 'R' || $this->isArabic() || (bool) \preg_match(UniPattern::RTL, $this->str);
    }
}
