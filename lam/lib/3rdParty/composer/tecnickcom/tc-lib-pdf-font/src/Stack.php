<?php

declare(strict_types=1);

/**
 * Stack.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Unicode\Data\BidiClass;
use Com\Tecnick\Unicode\Data\Type as UnicodeType;

/**
 * Com\Tecnick\Pdf\Font\Stack
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-import-type TFontData from Load
 *
 * @phpstan-type TTextSplit array{
 *     'pos': int,
 *     'ord': int,
 *     'spaces': int,
 *     'septype': string,
 *     'wordwidth': float,
 *     'totwidth': float,
 *     'totspacewidth': float,
 * }
 *
 * @phpstan-type TTextDims array{
 *     'chars': int,
 *     'spaces': int,
 *     'words': int,
 *     'totwidth': float,
 *     'totspacewidth': float,
 *     'split': array<int, TTextSplit>,
 * }
 *
 * @phpstan-type TBBox array{float, float, float, float}
 *
 * The 'fakestyle' member holds the part of 'style' the font does not provide and that the
 * caller synthesizes when it draws the text. See Buffer::addFont().
 *
 * @phpstan-type TStackItem array{
 *        'key': string,
 *        'style': string,
 *        'fakestyle': string,
 *        'size': float,
 *        'spacing': float,
 *        'stretching': float,
 *    }
 *
 * The 'ascent', 'descent' and 'height' members measure the line box of the text and are not
 * the '/Ascent' and '/Descent' of the font descriptor. See getFontMetric().
 *
 * @phpstan-type TFontMetric array{
 *     'ascent': float,
 *     'avgwidth': float,
 *     'capheight': float,
 *     'cbbox': array<int, TBBox>,
 *     'cbboxu': array<int, TBBox>,
 *     'cratio': float,
 *     'cw': array<int, float>,
 *     'cwu': array<int, float>,
 *     'descent': float,
 *     'dw': float,
 *     'fakestyle': string,
 *     'fbbox': array<int, float>,
 *     'gw': array<int, float>,
 *     'height': float,
 *     'idx': int,
 *     'key': string,
 *     'maxwidth': float,
 *     'midpoint': float,
 *     'missingwidth': float,
 *     'out': string,
 *     'outraw': string,
 *     'size': float,
 *     'spacing': float,
 *     'stretching': float,
 *     'style': string,
 *     'type': string,
 *     'up': float,
 *     'usize': float,
 *     'ut': float,
 *     'xheight': float,
 * }
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
class Stack extends \Com\Tecnick\Pdf\Font\Buffer
{
    /**
     * Default font size in points
     */
    public const DEFAULT_SIZE = 10;

    /**
     * Codepoints that take no horizontal space.
     *
     * 173 = SHY, only rendered when the text is hyphenated at that point.
     * 8203 = ZWSP, a word boundary with no glyph.
     * 8288 = WJ and 65279 = ZWNBSP, which prevent a line break and have no glyph.
     *
     * @var array<int, bool>
     */
    protected const ZERO_WIDTH = [
        173 => true,
        8203 => true,
        8288 => true,
        65_279 => true,
    ];

    /**
     * Array (stack) containing fonts in order of insertion.
     * The last item is the current font.
     *
     * @var array<int, TStackItem>
     */
    protected array $stack = [];

    /**
     * Current font index
     */
    protected int $index = -1;

    /**
     * Number of font metrics kept in memory.
     *
     * Each entry holds the glyph width and bounding box maps of the font scaled to one size.
     */
    protected const METRIC_CACHE_SIZE = 16;

    /**
     * Array containing font metrics for each fontkey-size combination.
     *
     * Ordered from the least to the most recently used.
     *
     * @var array<string, TFontMetric>
     */
    protected array $metric = [];

    /**
     * Insert a font into the stack
     *
     * The definition file (and the font file itself when embedding) must be present either in the current directory
     * or in the one indicated by K_PATH_FONTS if the constant is defined.
     *
     * @param int    $objnum     Current PDF object number
     * @param string $font       Font family, or comma separated list of font families
     *                           If it is a standard family name, it will override the corresponding font.
     * @param string $style      Font style.
     *                           Possible values are (case-insensitive):
     *                           regular (default)
     *                           B: bold
     *                           I: italic
     *                           U: underline
     *                           D: strikeout (linethrough)
     *                           O: overline
     * @param ?float $size       Font size in points (set to null to inherit the last font size).
     * @param ?float $spacing    Extra spacing between characters.
     * @param ?float $stretching Horizontal character stretching ratio.
     * @param string $ifile      The font definition file (or empty for autodetect).
     *                           By default, the name is built from the family and style, in lower case with no spaces.
     * @param ?bool  $subset     If true, embed only the characters used by the document.
     *                           Valid only for TrueTypeUnicode fonts.
     *                           Set to null to use the default value for a new font,
     *                           or to keep the mode of a font already loaded.
     *                           Subsetting is computational and memory intensive.
     *
     * @return TFontMetric Font data
     *
     * @throws FontException in case of error
     */
    public function insert(
        int &$objnum,
        string $font,
        string $style = '',
        ?float $size = null,
        ?float $spacing = null,
        ?float $stretching = null,
        string $ifile = '',
        ?bool $subset = null,
    ): array {
        $size = $this->getInputSize($size);
        $spacing = $this->getInputSpacing($spacing);
        $stretching = $this->getInputStretching($stretching);

        // try to load the corresponding imported font
        /** @var ?FontException $err */
        $err = null;
        $keys = $this->getNormalizedFontKeys($font);
        $styled = ['key' => '', 'style' => '', 'fakestyle' => ''];
        foreach ($keys as $key) {
            try {
                $styled = $this->addFont($objnum, $key, $style, $ifile, $subset);
                $err = null;
                break;
            } catch (FontException $exc) {
                $err = $exc;
            }
        }

        if ($err !== null) {
            // the last failure is reported, keeping the original as the cause
            throw new FontException($err->getMessage(), 0, $err);
        }

        // add this font in the stack
        $this->stack[++$this->index] = [
            'key' => $styled['key'],
            'style' => $styled['style'],
            'fakestyle' => $styled['fakestyle'],
            'size' => $size,
            'spacing' => $spacing,
            'stretching' => $stretching,
        ];

        return $this->getFontMetric($this->index);
    }

    /**
     * Returns the current font data array.
     *
     * @return TFontMetric
     *
     * @throws FontException
     */
    public function getCurrentFont(): array
    {
        return $this->getFontMetric($this->index);
    }

    /**
     * Returns a clone of the specified font with new parameters.
     *
     * @param int     $objnum    Current PDF object number.
     * @param ?int    $idx       Font index. Leave it null to use the current font.
     * @param ?string $style     Font style.
     *                           Possible values are (case-insensitive):
     *                           regular (default)
     *                           B: bold
     *                           I: italic
     *                           U: underline
     *                           D: strikeout (linethrough)
     *                           O: overline
     * @param ?float $size       Font size in points (set to null to inherit the last font size).
     * @param ?float $spacing    Extra spacing between characters.
     * @param ?float $stretching Horizontal character stretching ratio.
     *
     * @return TFontMetric
     *
     * @throws FontException
     */
    public function cloneFont(
        int &$objnum,
        ?int $idx = null,
        ?string $style = null,
        ?float $size = null,
        ?float $spacing = null,
        ?float $stretching = null,
    ): array {
        if ($idx === null) {
            $idx = $this->index;
        } elseif ($idx < 0 || $idx > $this->index) {
            throw new FontException('Invalid font index');
        }

        $curfont = $this->getStackItem($idx);

        if ($style === null || $style === $curfont['style']) {
            $size = $this->getInputSize($size);
            $spacing = $this->getInputSpacing($spacing);
            $stretching = $this->getInputStretching($stretching);

            $this->stack[++$this->index] = [
                'key' => $curfont['key'],
                'style' => $curfont['style'],
                'fakestyle' => $curfont['fakestyle'],
                'size' => $size,
                'spacing' => $spacing,
                'stretching' => $stretching,
            ];

            return $this->getFontMetric($this->index);
        }

        $data = $this->getFont($curfont['key']);

        return $this->insert(
            $objnum,
            $data['family'],
            $style,
            $size,
            $spacing,
            $stretching,
            $this->getStyleFontFile($data, $style),
            $data['subset'],
        );
    }

    /**
     * Returns the font definition file to use to load a different style of an already loaded font.
     *
     * The definition file of the requested style is searched in the directory of the source one.
     * When it is not there, an empty string is returned to trigger the autodetection,
     * that also provides the artificial style fallback.
     *
     * @param TFontData $data  Data of the source font.
     * @param string    $style Requested font style.
     *
     * @return string The font definition file, or an empty string for autodetection.
     */
    protected function getStyleFontFile(array $data, string $style): string
    {
        if ($data['dir'] === '') {
            return '';
        }

        $style = \strtoupper($style);
        $suffix = (\str_contains($style, 'B') ? 'B' : '') . (\str_contains($style, 'I') ? 'I' : '');
        $ifile = $data['dir'] . DIRECTORY_SEPARATOR . \strtolower($data['family'] . $suffix) . '.json';

        return \is_readable($ifile) ? $ifile : '';
    }

    /**
     * Returns the current font key.
     *
     * @throws FontException
     */
    public function getCurrentFontKey(): string
    {
        return $this->getCurrentStackItem()['key'];
    }

    /**
     * Returns the current font type (i.e.: Core, TrueType, TrueTypeUnicode, Type1).
     *
     * @throws FontException
     */
    public function getCurrentFontType(): string
    {
        return $this->getFont($this->getCurrentStackItem()['key'])['type'];
    }

    /**
     * Returns true if a current font is available on the stack.
     */
    public function hasCurrentFont(): bool
    {
        return $this->index >= 0 && $this->stack !== [];
    }

    /**
     * Returns the number of fonts currently stored in the stack.
     */
    public function getStackSize(): int
    {
        return \count($this->stack);
    }

    /**
     * Returns the current font index in the stack.
     */
    public function getCurrentFontIndex(): int
    {
        return $this->index;
    }

    /**
     * Returns the PDF code to use the current font.
     *
     * @throws FontException
     */
    public function getOutCurrentFont(): string
    {
        return $this->getFontMetric($this->index)['out'];
    }

    /**
     * Returns true if the current font type is Core, TrueType or Type1.
     *
     * @throws FontException
     */
    public function isCurrentByteFont(): bool
    {
        $currentFontType = $this->getCurrentFontType();
        return $currentFontType === 'Core' || $currentFontType === 'TrueType' || $currentFontType === 'Type1';
    }

    /**
     * Returns true if the current font type is TrueTypeUnicode or cidfont0.
     *
     * @throws FontException
     */
    public function isCurrentUnicodeFont(): bool
    {
        $currentFontType = $this->getCurrentFontType();
        return $currentFontType === 'TrueTypeUnicode' || $currentFontType === 'cidfont0';
    }

    /**
     * Returns true if the current font encodes the text as glyph indices (CID == GID).
     *
     * @throws FontException
     */
    public function isCurrentGidEncoded(): bool
    {
        return $this->getFont($this->getCurrentStackItem()['key'])['gidenc'];
    }

    /**
     * Returns the glyph index of the specified codepoint in the current font.
     *
     * @param int $ord Unicode codepoint.
     *
     * @return int The glyph index, or 0 (.notdef) when the font has no glyph for it.
     *
     * @throws FontException
     */
    public function getGidForOrd(int $ord): int
    {
        return $this->getFontGidForOrd($this->getFont($this->getCurrentStackItem()['key']), $ord);
    }

    /**
     * Returns the glyph index of the specified codepoint in the given font.
     *
     * @param TFontData $font Font data.
     * @param int       $ord  Unicode codepoint.
     *
     * @throws FontException
     */
    protected function getFontGidForOrd(array $font, int $ord): int
    {
        if ($ord > 0xFFFF) {
            // codepoints above the BMP are stored in the 'ctgu' member of the definition file
            $gid = (int) ($font['ctgu'][$ord] ?? 0);
            // an entry outside the range of a glyph index is reported as .notdef
            return $gid >= 0 && $gid <= self::MAX_GID ? $gid : 0;
        }

        if ($ord < 0) {
            return 0;
        }

        if ($font['ctg'] === '') {
            // no CIDToGIDMap artifact, hence no glyph index to report
            return 0;
        }

        $table = $this->getCtgTable($font);
        return (\ord($table[$ord * 2]) << 8) | \ord($table[($ord * 2) + 1]);
    }

    /**
     * Returns true when the codepoint is a variation selector: U+180B..U+180D, U+180F,
     * U+FE00..U+FE0F or U+E0100..U+E01EF.
     *
     * @param int $ord Unicode codepoint.
     */
    public static function isVariationSelector(int $ord): bool
    {
        return (
            $ord >= 0xFE00
            && $ord <= 0xFE0F
            || $ord >= 0xE_0100
            && $ord <= 0xE_01EF
            || $ord >= 0x180B
            && $ord <= 0x180D
            || $ord === 0x180F
        );
    }

    /**
     * Returns the glyph index of a variation sequence in the current font.
     *
     * @param int $base     Base character.
     * @param int $selector Variation selector.
     *
     * @return int The glyph index, or 0 when the font does not map the sequence.
     *
     * @throws FontException
     */
    public function getVariationGid(int $base, int $selector): int
    {
        return $this->getFontVariationGid($this->getFont($this->getCurrentStackItem()['key']), $base, $selector);
    }

    /**
     * Returns the glyph index of a variation sequence in the given font.
     *
     * @param TFontData $font     Font data.
     * @param int       $base     Base character.
     * @param int       $selector Variation selector.
     *
     * @return int The glyph index, or 0 when the font does not map the sequence.
     */
    protected function getFontVariationGid(array $font, int $base, int $selector): int
    {
        $gid = $font['uvs'][$base][$selector] ?? 0;

        return $gid > 0 && $gid <= self::MAX_GID ? $gid : 0;
    }

    /**
     * Encode an array of codepoints as the 2-byte character codes of the current font.
     *
     * The codes are glyph indices, or alias codes of the glyphs shown for more than one
     * text (see Buffer::addUsedGlyph()). A base character followed by a variation selector
     * the font maps is shown with the glyph of the sequence; a variation selector shows no
     * glyph of its own. Every glyph is recorded on the font, and its base codepoint is
     * added to the subset.
     *
     * @param array<int, int> $uniarr Array of character codepoints.
     *
     * @throws FontException
     */
    public function ordArrToGidStr(array $uniarr): string
    {
        $fkey = $this->getCurrentStackItem()['key'];
        $font = $this->getFont($fkey);
        $subset = $font['subset'];
        $uniarr = \array_values($uniarr);
        $num = \count($uniarr);
        $str = '';
        for ($idx = 0; $idx < $num; ++$idx) {
            $ord = $uniarr[$idx];
            if (self::isVariationSelector($ord)) {
                continue;
            }

            $gid = $this->getFontGidForOrd($font, $ord);
            $text = [$ord];
            $next = $uniarr[$idx + 1] ?? -1;
            $vgid = self::isVariationSelector($next) ? $this->getFontVariationGid($font, $ord, $next) : 0;
            if ($vgid > 0) {
                $gid = $vgid;
                $text[] = $next;
            }

            $code = $gid;
            if ($gid !== 0) {
                $code = $this->addUsedGlyph($fkey, $gid, $text);
                if ($subset) {
                    $this->addSubsetChar($fkey, $ord);
                }
            }

            $str .= \chr($code >> 8) . \chr($code & 0xFF);
        }

        return $str;
    }

    /**
     * Remove and return the last inserted font
     *
     * @return TFontMetric
     *
     * @throws FontException
     */
    public function popLastFont(): array
    {
        if ($this->index < 0 || $this->stack === []) {
            throw new FontException('The font stack is empty');
        }

        $font = $this->getFontMetric($this->index);
        \array_pop($this->stack);
        --$this->index;
        return $font;
    }

    /**
     * Replace missing characters with selected substitutions
     *
     * A character is missing when the font has no width for it (see isCharDefined()).
     *
     * @param array<int, int>        $uniarr Array of character codepoints.
     * @param array<int, array<int>> $subs   Array of possible character substitutions.
     *                                       The key is the character to check (integer value),
     *                                       the value is an array of possible substitutes.
     *
     * @return array<int, int> Array of character codepoints.
     *
     * @throws FontException
     */
    public function replaceMissingChars(array $uniarr, array $subs = []): array
    {
        $font = $this->getFontMetric($this->index);
        foreach ($uniarr as $pos => $uni) {
            if (isset($font['cwu'][$uni]) || isset($font['cw'][$uni])) {
                continue;
            }

            $alts = $subs[$uni] ?? null;
            if ($alts === null) {
                continue;
            }

            foreach ($alts as $alt) {
                if (!isset($font['cwu'][$alt]) && !isset($font['cw'][$alt])) {
                    continue;
                }

                $uniarr[$pos] = $alt;
                break;
            }
        }

        return $uniarr;
    }

    /**
     * Returns true if the specified Unicode value is defined in the current font.
     *
     * A variation selector needs no glyph of its own and is always defined.
     *
     * @param int $ord Unicode character value to convert
     *
     * @throws FontException
     */
    public function isCharDefined(int $ord): bool
    {
        $font = $this->getFontMetric($this->index);
        // on a byte encoded font a glyph outside Latin-1 is only listed in the
        // codepoint-keyed map
        return isset($font['cwu'][$ord]) || isset($font['cw'][$ord]) || self::isVariationSelector($ord);
    }

    /**
     * Returns the width of the specified character.
     *
     * A variation selector takes no space.
     *
     * @param int $ord Unicode character value.
     *
     * @throws FontException
     */
    public function getCharWidth(int $ord): float
    {
        if (isset(self::ZERO_WIDTH[$ord]) || self::isVariationSelector($ord)) {
            return 0;
        }

        $font = $this->getFontMetric($this->index);
        if (isset($font['cwu'][$ord])) {
            return $font['cwu'][$ord];
        }

        return $font['cw'][$ord] ?? $font['dw'];
    }

    /**
     * Returns the length of the string specified using an array of codepoints.
     *
     * @param array<int, int> $uniarr Array of character codepoints.
     *
     * @throws FontException
     */
    public function getOrdArrWidth(array $uniarr): float
    {
        return $this->getOrdArrDims($uniarr)['totwidth'];
    }

    /**
     * Returns the width of each codepoint of the string, without the character spacing.
     *
     * A variation selector takes no space, and a base character followed by a variation
     * selector the font maps takes the width of the glyph of the sequence.
     *
     * @param array<int, int> $uniarr Array of character codepoints.
     *
     * @return array<int, float> Widths in the order of the codepoints.
     *
     * @throws FontException
     */
    public function getOrdArrCharWidths(array $uniarr): array
    {
        $uniarr = \array_values($uniarr);
        $metric = $this->getFontMetric($this->index);
        $font = $this->getFont($metric['key']);
        $widths = [];
        foreach ($uniarr as $idx => $ord) {
            $widths[] = $this->getSequenceCharWidth($metric, $font, $ord, $uniarr[$idx + 1] ?? -1);
        }

        return $widths;
    }

    /**
     * Returns the width of a codepoint followed by the given one.
     *
     * The zero-width codepoints and the variation selectors take no space; a base character
     * followed by a variation selector the font maps takes the width of the glyph of the
     * sequence.
     *
     * @param TFontMetric $metric Scaled metrics of the font.
     * @param TFontData   $font   Font data.
     * @param int         $ord    Unicode codepoint.
     * @param int         $next   Unicode codepoint that follows, or -1.
     */
    protected function getSequenceCharWidth(array $metric, array $font, int $ord, int $next): float
    {
        if (isset(self::ZERO_WIDTH[$ord]) || self::isVariationSelector($ord)) {
            return 0.0;
        }

        if (self::isVariationSelector($next)) {
            $gid = $this->getFontVariationGid($font, $ord, $next);
            if (isset($metric['gw'][$gid])) {
                return $metric['gw'][$gid];
            }
        }

        return $metric['cwu'][$ord] ?? $metric['cw'][$ord] ?? $metric['dw'];
    }

    /**
     * Returns various dimensions of the string specified using an array of codepoints.
     *
     * @param array<int, int> $uniarr Array of character codepoints.
     *
     * @return TTextDims
     *
     * @throws FontException
     */
    public function getOrdArrDims(array $uniarr): array
    {
        // the loop below reads the key of each entry as its position in the string
        $uniarr = \array_values($uniarr);
        $chars = \count($uniarr); // total number of chars
        $spaces = 0; // total number of spaces
        $totwidth = 0; // total string width
        $totspacewidth = 0; // total space width
        $words = 0; // total number of words
        $curfont = $this->getFontMetric($this->index);
        $fact = $curfont['spacing'] * $curfont['stretching'];
        $fkey = $curfont['key'];
        $font = $this->getFont($fkey);
        $subset = $font['subset'];
        $hasSequences = $font['uvs'] !== [];
        $sentinel = \count($uniarr); // position of the appended terminator
        $uniarr[] = 8203; // append a ZWSP to ensure that the last word is processed
        $split = [];
        $prevtotwidth = 0.0;
        // variation selectors show no glyph, so they get no character spacing
        $selectors = 0;
        foreach ($uniarr as $idx => $ord) {
            // U+180B is the lowest variation selector
            $isSelector = $ord >= 0x180B && self::isVariationSelector($ord);
            // the terminator is internal and must not enter the subset of the font
            if ($subset && $idx !== $sentinel && !$isSelector) {
                $this->addSubsetChar($fkey, $ord);
            }

            $bidiClass = UnicodeType::getBidiClass($ord);
            if ($hasSequences || $isSelector) {
                $chrwidth = $this->getSequenceCharWidth($curfont, $font, $ord, $uniarr[$idx + 1] ?? -1);
            } elseif (isset(self::ZERO_WIDTH[$ord])) {
                $chrwidth = 0.0;
            } else {
                $chrwidth = $curfont['cwu'][$ord] ?? $curfont['cw'][$ord] ?? $curfont['dw'];
            }
            // Split on paragraph/segment separators (B, S), whitespace (WS) and boundary neutrals (BN).
            if (
                $bidiClass === BidiClass::B
                || $bidiClass === BidiClass::S
                || $bidiClass === BidiClass::WS
                || $bidiClass === BidiClass::BN
            ) {
                $currenttotwidth = $totwidth + ($fact * \max(0, $idx - $selectors - 1));
                $split[$words] = [
                    'pos' => $idx,
                    'ord' => $ord,
                    'spaces' => $spaces,
                    'septype' => $bidiClass->value,
                    // width accumulated since the previous split point
                    'wordwidth' => $currenttotwidth - $prevtotwidth,
                    'totwidth' => $currenttotwidth,
                    'totspacewidth' => $totspacewidth + ($fact * \max(0, $spaces - 1)),
                ];
                $prevtotwidth = $currenttotwidth;
                $words++;
                if ($bidiClass === BidiClass::WS) {
                    ++$spaces;
                    $totspacewidth += $chrwidth;
                }
            }
            $totwidth += $chrwidth;
            if ($isSelector) {
                ++$selectors;
            }
        }
        $totwidth += $fact * \max(0, $chars - $selectors - 1);
        $totspacewidth += $fact * \max(0, $spaces - 1);
        return [
            'chars' => $chars,
            'spaces' => $spaces,
            'words' => $words,
            'totwidth' => $totwidth,
            'totspacewidth' => $totspacewidth,
            'split' => $split,
        ];
    }

    /**
     * Returns the glyph bounding box of the specified character in the current font in user units.
     *
     * @param int $ord Unicode character value.
     *
     * @return TBBox (xMin, yMin, xMax, yMax)
     *
     * @throws FontException
     */
    public function getCharBBox(int $ord): array
    {
        $font = $this->getFontMetric($this->index);
        // on a byte encoded font a glyph outside Latin-1 is only listed in the
        // codepoint-keyed map
        return $font['cbboxu'][$ord] ?? $font['cbbox'][$ord] ?? [0.0, 0.0, 0.0, 0.0]; // glyph without outline
    }

    /**
     * Replace a char if it is defined on the current font.
     *
     * @param int $oldchar Integer code (Unicode) of the character to replace.
     * @param int $newchar Integer code (Unicode) of the new character.
     *
     * @return int the replaced char or the old char in case the new char is not defined
     *
     * @throws FontException
     */
    public function replaceChar(int $oldchar, int $newchar): int
    {
        if ($this->isCharDefined($newchar)) {
            // add the new char on the subset list
            $this->addSubsetChar($this->getFontMetric($this->index)['key'], $newchar);
            // return the new character
            return $newchar;
        }

        // return the old char
        return $oldchar;
    }

    /**
     * Returns the font metrics associated to the input key.
     *
     * @param int $idx Font index in the stack.
     *
     * @return TFontMetric
     *
     * @throws FontException
     */
    protected function getFontMetric(int $idx): array
    {
        $font = $this->getStackItem($idx);
        // cache key built from the fields the metric is computed from; the stack index and
        // the style are not among them and are patched on the way out, so every style of a
        // font shares one set of scaled glyph widths and bounding boxes
        $mkey = $font['key'] . '|' . $font['size'] . '|' . $font['spacing'] . '|' . $font['stretching'];
        if (isset($this->metric[$mkey])) {
            $metric = $this->metric[$mkey];
            // move the entry to the most recent end
            unset($this->metric[$mkey]);
            $this->metric[$mkey] = $metric;
            $metric['idx'] = $idx;
            $metric['style'] = $font['style'];
            $metric['fakestyle'] = $font['fakestyle'];
            return $metric;
        }

        $fontkey = $font['key'];
        $fontsize = $font['size'];
        $fontspacing = $font['spacing'];
        $fontstretching = $font['stretching'];
        $fontstyle = $font['style'];
        $fakestyle = $font['fakestyle'];

        $usize = $fontsize / $this->kunit;
        $cratio = $fontsize / 1000;
        $wratio = $cratio * $fontstretching; // horizontal ratio
        $data = $this->getFont($fontkey);
        $desc = $data['desc'];
        // glyph widths and bounding boxes scaled to internal units
        $cw = [];
        foreach ($data['cw'] as $cid => $width) {
            $cw[(int) $cid] = (float) $width * $wratio;
        }

        $cwu = [];
        foreach ($data['cwu'] as $codepoint => $width) {
            $cwu[(int) $codepoint] = (float) $width * $wratio;
        }

        $gw = [];
        foreach ($data['gw'] as $gid => $width) {
            $gw[(int) $gid] = (float) $width * $wratio;
        }

        $cbbox = $this->scaleBBoxMap($data['cbbox'], $wratio, $cratio);
        $cbboxu = $this->scaleBBoxMap($data['cbboxu'], $wratio, $cratio);

        $ascent = (float) $desc['Ascent'];
        $descent = (float) $desc['Descent'];
        $avgwidth = (float) $desc['AvgWidth'];
        $capheight = (float) $desc['CapHeight'];
        $maxwidth = (float) $desc['MaxWidth'];
        $missingwidth = (float) $desc['MissingWidth'];
        $xheight = (float) $desc['XHeight'];
        $fontbbox = $desc['FontBBox'];
        $dw = (float) $data['dw'];
        $up = (float) $data['up'];
        $ut = (float) $data['ut'];
        $fonttype = $data['type'];
        $outfont = \sprintf('/F%d %F Tf', (int) $data['i'], $fontsize); // PDF output string
        // the four values may be written with any run of whitespace and with or without
        // the enclosing brackets
        $boxsplit = \preg_split('/[\t\n\x0B\f\r ]+/', \trim($fontbbox, "[] \t\n\r\0\x0B"), -1, PREG_SPLIT_NO_EMPTY);
        $tbox = \array_pad(\is_array($boxsplit) ? $boxsplit : [], 4, '0');
        // an AFM based font declares no line layout metric, so the line box is widened to
        // the FontBBox; the descriptor keeps the declared values
        if ($fonttype === 'Core' || $fonttype === 'Type1') {
            $ascent = \max($ascent, \is_numeric($tbox[3]) ? (float) $tbox[3] : 0.0);
            $descent = \min($descent, \is_numeric($tbox[1]) ? (float) $tbox[1] : 0.0);
        }

        // add this font in the stack with metrics in internal units
        $this->metric[$mkey] = [
            'ascent' => $ascent * $cratio,
            'avgwidth' => $avgwidth * $cratio * $fontstretching,
            'capheight' => $capheight * $cratio,
            'cbbox' => $cbbox,
            'cbboxu' => $cbboxu,
            'cratio' => $cratio,
            'cw' => $cw,
            'cwu' => $cwu,
            'descent' => $descent * $cratio,
            'dw' => $dw * $cratio * $fontstretching,
            'fakestyle' => $fakestyle,
            'fbbox' => [
                0 => (\is_numeric($tbox[0]) ? (float) $tbox[0] : 0.0) * $wratio, // left
                1 => (\is_numeric($tbox[1]) ? (float) $tbox[1] : 0.0) * $cratio, // bottom
                2 => (\is_numeric($tbox[2]) ? (float) $tbox[2] : 0.0) * $wratio, // right
                3 => (\is_numeric($tbox[3]) ? (float) $tbox[3] : 0.0) * $cratio, // top
            ],
            'gw' => $gw,
            'height' => ($ascent - $descent) * $cratio,
            'idx' => $idx,
            'key' => $fontkey,
            'maxwidth' => $maxwidth * $cratio * $fontstretching,
            'midpoint' => (($ascent + $descent) * $cratio) / 2,
            'missingwidth' => $missingwidth * $cratio * $fontstretching,
            'out' => 'BT ' . $outfont . ' ET' . "\r",
            'outraw' => $outfont,
            'size' => $fontsize,
            'spacing' => $fontspacing,
            'stretching' => $fontstretching,
            'style' => $fontstyle,
            'type' => $fonttype,
            'up' => $up * $cratio,
            'usize' => $usize,
            'ut' => $ut * $cratio,
            'xheight' => $xheight * $cratio,
        ];
        $this->evictOldestMetrics();
        return $this->metric[$mkey];
    }

    /**
     * Drop the least recently used metrics beyond the size of the cache.
     */
    protected function evictOldestMetrics(): void
    {
        $excess = \count($this->metric) - self::METRIC_CACHE_SIZE;
        if ($excess <= 0) {
            return;
        }

        foreach (\array_slice(\array_keys($this->metric), 0, $excess) as $oldest) {
            unset($this->metric[$oldest]);
        }
    }

    /**
     * Scale a map of glyph bounding boxes to internal units.
     *
     * @param array<int, array<int, int>> $map     Bounding boxes indexed by character code.
     * @param float                       $wratio  Horizontal ratio.
     * @param float                       $cratio  Vertical ratio.
     *
     * @return array<int, TBBox>
     */
    protected function scaleBBoxMap(array $map, float $wratio, float $cratio): array
    {
        $scaled = [];
        foreach ($map as $cid => $val) {
            if (\count($val) !== 4) {
                continue;
            }

            $bbox = \array_values($val);
            $scaled[(int) $cid] = [
                0 => (float) $bbox[0] * $wratio,
                1 => (float) $bbox[1] * $cratio,
                2 => (float) $bbox[2] * $wratio,
                3 => (float) $bbox[3] * $cratio,
            ];
        }

        return $scaled;
    }

    /**
     * Normalize the input size (minimum 0)
     *
     * A value that is not a finite number is treated as no value.
     *
     * @param ?float $size Font size in points (set to null to inherit the last font size).
     *
     * @throws FontException
     */
    protected function getInputSize(?float $size = null): float
    {
        if ($size === null || !\is_finite($size) || $size < 0) {
            if ($this->index >= 0) {
                // inherit the size of the last inserted font
                return $this->getCurrentStackItem()['size'];
            }

            return self::DEFAULT_SIZE;
        }

        return $size;
    }

    /**
     * Normalize the input spacing (minimum 0)
     *
     * A value that is not a finite number is treated as no value.
     *
     * @param ?float $spacing Extra spacing between characters.
     *
     * @throws FontException
     */
    protected function getInputSpacing(?float $spacing = null): float
    {
        if ($spacing === null || !\is_finite($spacing)) {
            if ($this->index >= 0) {
                // inherit the size of the last inserted font
                return $this->getCurrentStackItem()['spacing'];
            }

            return 0;
        }

        return $spacing;
    }

    /**
     * Normalize the input stretching
     *
     * A value that is not a finite number is treated as no value.
     *
     * @param ?float $stretching Horizontal character stretching ratio.
     *
     * @throws FontException
     */
    protected function getInputStretching(?float $stretching = null): float
    {
        if ($stretching === null || !\is_finite($stretching)) {
            if ($this->index >= 0) {
                // inherit the size of the last inserted font
                return $this->getCurrentStackItem()['stretching'];
            }

            return 1;
        }

        return $stretching;
    }

    /**
     * Returns the stack item at the given index.
     *
     * @param int $idx Font index in the stack.
     *
     * @return TStackItem
     *
     * @throws FontException
     */
    protected function getStackItem(int $idx): array
    {
        $item = $this->stack[$idx] ?? null;
        if ($item === null) {
            throw new FontException('Invalid font index');
        }

        return $item;
    }

    /**
     * Returns the current stack item.
     *
     * @return TStackItem
     *
     * @throws FontException
     */
    protected function getCurrentStackItem(): array
    {
        return $this->getStackItem($this->index);
    }

    /**
     * Return normalized font keys
     *
     * @param string $fontfamily Property string containing comma-separated font family names
     *
     * @return array<string>
     *
     * @throws FontException
     */
    protected function getNormalizedFontKeys(string $fontfamily): array
    {
        if ($fontfamily === '') {
            throw new FontException('Empty font family name');
        }

        $keys = [];
        // remove spaces and symbols
        $filtered = \preg_replace('/[^a-z0-9_\,]/', '', \strtolower($fontfamily));
        if ($filtered === null) {
            throw new FontException('Invalid font family name');
        }

        $fontfamily = $filtered;

        // extract all font names
        $fontslist = \preg_split('/[,]/', $fontfamily);
        if ($fontslist === false) {
            throw new FontException('Invalid font family name: ' . $fontfamily);
        }

        // replacement patterns

        $fontpattern = ['/regular$/', '/italic$/', '/oblique$/', '/bold([I]?)$/'];
        $fontreplacement = ['', 'I', 'I', 'B\\1'];

        $keypattern = ['/^serif|^cursive|^fantasy|^timesnewroman/', '/^sansserif/', '/^monospace/'];
        $keyreplacement = ['times', 'helvetica', 'courier'];

        // find first valid font name
        foreach ($fontslist as $font) {
            $styled = \preg_replace($fontpattern, $fontreplacement, $font);
            if ($styled === null) {
                throw new FontException('Invalid font family name: ' . $fontfamily);
            }

            // replace common family names and core fonts
            $fontkey = \preg_replace($keypattern, $keyreplacement, $styled);
            if ($fontkey === null) {
                throw new FontException('Invalid font family name: ' . $fontfamily);
            }

            $keys[] = $fontkey;
        }

        return $keys;
    }

    /**
     * Returns the normalized font family name or the current font name key.
     *
     * @param string $fontfamily Raw font family name.
     *
     * @throws FontException
     */
    public function getFontFamilyName(string $fontfamily): string
    {
        $fkeys = $this->getNormalizedFontKeys($fontfamily);
        foreach ($fkeys as $fkey) {
            if ($this->isValidKey($fkey)) {
                return $fkey;
            }

            $pdfakey = 'pdfa' . $fkey;
            if ($this->isValidKey($pdfakey)) {
                return $pdfakey;
            }
        }

        return $this->getCurrentFontKey();
    }
}
