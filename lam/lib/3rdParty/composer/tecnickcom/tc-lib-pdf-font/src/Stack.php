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
 * @phpstan-type TStackItem array{
 *        'key': string,
 *        'style': string,
 *        'size': float,
 *        'spacing': float,
 *        'stretching': float,
 *    }
 *
 * @phpstan-type TFontMetric array{
 *     'ascent': float,
 *     'avgwidth': float,
 *     'capheight': float,
 *     'cbbox': array<int, TBBox>,
 *     'cratio': float,
 *     'cw': array<int, float>,
 *     'cwu': array<int, float>,
 *     'descent': float,
 *     'dw': float,
 *     'fbbox': array<int, float>,
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
     * Array containing font metrics for each fontkey-size combination.
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
     * @param ?bool  $subset     If true embed only a subset of the font (stores only the information related to
     *                           the used characters); If false embed full font; This option is valid only for
     *                           TrueTypeUnicode fonts and is disabled for PDF/A. If you want to enable users to
     *                           modify the document, set this parameter to false. If you subset the font, the person
     *                           who receives your PDF would need to have your same font in order to make changes to
     *                           your PDF. The file size of the PDF would also be smaller because you are embedding
     *                           only a subset.
     *                           Set this to null to use the default value.
     *                           NOTE: This option is computational and memory intensive.
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
        if ($subset === null) {
            $subset = $this->subset;
        }

        $size = $this->getInputSize($size);
        $spacing = $this->getInputSpacing($spacing);
        $stretching = $this->getInputStretching($stretching);

        // try to load the corresponding imported font
        /** @var ?FontException $err */
        $err = null;
        $keys = $this->getNormalizedFontKeys($font);
        $fontkey = '';
        foreach ($keys as $key) {
            try {
                $fontkey = $this->add($objnum, $key, $style, $ifile, $subset);
                $err = null;
                break;
            } catch (FontException $exc) {
                $err = $exc;
            }
        }

        if ($err !== null) {
            throw new FontException($err->getMessage());
        }

        // add this font in the stack
        $data = $this->getFont($fontkey);

        $this->stack[++$this->index] = [
            'key' => $fontkey,
            'style' => $data['style'],
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
     * The 'ifile' entry of a loaded font is the definition file of its own style,
     * so it cannot be reused as is for a different style.
     * The definition file of the requested style is searched in the same directory of the source one;
     * if it is not there, an empty string is returned to trigger the standard autodetection,
     * that also provides the artificial style fallback when no styled definition file exists.
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
     * @return string
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
     * @return string
     *
     * @throws FontException
     */
    public function getCurrentFontType(): string
    {
        return $this->getFont($this->getCurrentStackItem()['key'])['type'];
    }

    /**
     * Returns true if a current font is available on the stack.
     *
     * @return bool
     */
    public function hasCurrentFont(): bool
    {
        return $this->index >= 0 && $this->stack !== [];
    }

    /**
     * Returns the number of fonts currently stored in the stack.
     *
     * @return int
     */
    public function getStackSize(): int
    {
        return \count($this->stack);
    }

    /**
     * Returns the current font index in the stack.
     *
     * @return int
     */
    public function getCurrentFontIndex(): int
    {
        return $this->index;
    }

    /**
     * Returns the PDF code to use the current font.
     *
     * @return string
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
     * @return bool
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
     * @return bool
     *
     * @throws FontException
     */
    public function isCurrentUnicodeFont(): bool
    {
        $currentFontType = $this->getCurrentFontType();
        return $currentFontType === 'TrueTypeUnicode' || $currentFontType === 'cidfont0';
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
            if (isset($font['cw'][$uni])) {
                continue;
            }

            $alts = $subs[$uni] ?? null;
            if ($alts === null) {
                continue;
            }

            foreach ($alts as $alt) {
                if (!isset($font['cw'][$alt])) {
                    continue;
                }

                $uniarr[$pos] = $alt;
                break;
            }
        }

        return $uniarr;
    }

    /**
     * Returns true if the specified Unicode value is defined in the current font
     *
     * @param int $ord Unicode character value to convert
     *
     * @return bool
     *
     * @throws FontException
     */
    public function isCharDefined(int $ord): bool
    {
        $font = $this->getFontMetric($this->index);
        return isset($font['cw'][$ord]);
    }

    /**
     * Returns the width of the specified character
     *
     * @param int $ord Unicode character value.
     *
     * @return float
     *
     * @throws FontException
     */
    public function getCharWidth(int $ord): float
    {
        if ($ord === 173 || $ord === 8203) {
            // 173 = SHY character is not printed, as it is used for text hyphenation
            // 8203 = ZWSP character
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
     * @return float
     *
     * @throws FontException
     */
    public function getOrdArrWidth(array $uniarr): float
    {
        return $this->getOrdArrDims($uniarr)['totwidth'];
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
        $chars = \count($uniarr); // total number of chars
        $spaces = 0; // total number of spaces
        $totwidth = 0; // total string width
        $totspacewidth = 0; // total space width
        $words = 0; // total number of words
        $curfont = $this->getFontMetric($this->index);
        $fact = $curfont['spacing'] * $curfont['stretching'];
        $fkey = $curfont['key'];
        $subset = false;
        if (
            isset($this->font[$fkey])
            && \is_array($this->font[$fkey])
            && \array_key_exists('subset', $this->font[$fkey])
            && $this->font[$fkey]['subset'] === true
        ) {
            $subset = true;
        }
        $uniarr[] = 8203; // add null at the end to ensure that the last word is processed
        $split = [];
        $prevtotwidth = 0.0;
        foreach ($uniarr as $idx => $ord) {
            if ($subset) {
                $this->addSubsetChar($fkey, $ord);
            }

            $unitype = UnicodeType::UNI[$ord] ?? '';
            $bidiClass = BidiClass::tryFrom($unitype);
            // Inline the width lookup using the already-resolved $curfont metric: calling
            // getCharWidth() here would re-resolve getFontMetric($this->index) per character.
            $chrwidth = match ($ord) {
                173, 8203 => 0.0, // 173 = SHY (hyphenation), 8203 = ZWSP: not printed
                default => $curfont['cwu'][$ord] ?? $curfont['cw'][$ord] ?? $curfont['dw'],
            };
            // Split on paragraph/segment separators (B, S), whitespace (WS) and boundary neutrals (BN).
            if (
                $bidiClass === BidiClass::B
                || $bidiClass === BidiClass::S
                || $bidiClass === BidiClass::WS
                || $bidiClass === BidiClass::BN
            ) {
                $currenttotwidth = $totwidth + ($fact * ($idx - 1));
                $split[$words] = [
                    'pos' => $idx,
                    'ord' => $ord,
                    'spaces' => $spaces,
                    'septype' => $unitype,
                    'wordwidth' => $words > 0 ? $currenttotwidth - $prevtotwidth : 0,
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
        }
        $totwidth += $fact * \max(0, $chars - 1);
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
        return $font['cbbox'][$ord] ?? [0.0, 0.0, 0.0, 0.0]; // glyph without outline
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
        // Cheap, collision-free cache key from just the fields the metric depends on,
        // instead of md5(serialize($font)) which ran on every (mostly cache-hit) call.
        $mkey =
            $font['key']
            . '|'
            . $font['size']
            . '|'
            . $font['spacing']
            . '|'
            . $font['stretching']
            . '|'
            . $font['style'];
        if (isset($this->metric[$mkey])) {
            return $this->metric[$mkey];
        }

        $fontkey = $font['key'];
        $fontsize = $font['size'];
        $fontspacing = $font['spacing'];
        $fontstretching = $font['stretching'];
        $fontstyle = $font['style'];

        $usize = $fontsize / $this->kunit;
        $cratio = $fontsize / 1000;
        $wratio = $cratio * $fontstretching; // horizontal ratio
        $data = $this->getFont($fontkey);
        $desc = $data['desc'];
        // Build the glyph widths and bounding boxes already scaled to internal units in a
        // single pass, instead of first casting into intermediate arrays and then rescaling.
        $cw = [];
        foreach ($data['cw'] as $cid => $width) {
            $cw[(int) $cid] = (float) $width * $wratio;
        }

        $cwu = [];
        foreach ($data['cwu'] as $codepoint => $width) {
            $cwu[(int) $codepoint] = (float) $width * $wratio;
        }

        $cbbox = [];
        foreach ($data['cbbox'] as $cid => $val) {
            if (\count($val) !== 4) {
                continue;
            }

            $bbox = \array_values($val);
            $cbbox[(int) $cid] = [
                0 => (float) $bbox[0] * $wratio,
                1 => (float) $bbox[1] * $cratio,
                2 => (float) $bbox[2] * $wratio,
                3 => (float) $bbox[3] * $cratio,
            ];
        }

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
        $tbox = \array_pad(\explode(' ', \substr($fontbbox, 1, -1)), 4, '0');
        // add this font in the stack with metrics in internal units
        $this->metric[$mkey] = [
            'ascent' => $ascent * $cratio,
            'avgwidth' => $avgwidth * $cratio * $fontstretching,
            'capheight' => $capheight * $cratio,
            'cbbox' => $cbbox,
            'cratio' => $cratio,
            'cw' => $cw,
            'cwu' => $cwu,
            'descent' => $descent * $cratio,
            'dw' => $dw * $cratio * $fontstretching,
            'fbbox' => [
                0 => (\is_numeric($tbox[0]) ? (float) $tbox[0] : 0.0) * $wratio, // left
                1 => (\is_numeric($tbox[1]) ? (float) $tbox[1] : 0.0) * $cratio, // bottom
                2 => (\is_numeric($tbox[2]) ? (float) $tbox[2] : 0.0) * $wratio, // right
                3 => (\is_numeric($tbox[3]) ? (float) $tbox[3] : 0.0) * $cratio, // top
            ],
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
        return $this->metric[$mkey];
    }

    /**
     * Normalize the input size (minimum 0)
     *
     * @param ?float $size Font size in points (set to null to inherit the last font size).
     *
     * @return float
     *
     * @throws FontException
     */
    protected function getInputSize(?float $size = null): float
    {
        if ($size === null || $size < 0) {
            if ($this->index >= 0) {
                // inherit the size of the last inserted font
                return $this->getCurrentStackItem()['size'];
            }

            return self::DEFAULT_SIZE;
        }

        return \max(0, $size);
    }

    /**
     * Normalize the input spacing (minimum 0)
     *
     * @param ?float $spacing Extra spacing between characters.
     *
     * @return float
     *
     * @throws FontException
     */
    protected function getInputSpacing(?float $spacing = null): float
    {
        if ($spacing === null) {
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
     * @param ?float $stretching Horizontal character stretching ratio.
     *
     * @return float
     *
     * @throws FontException
     */
    protected function getInputStretching(?float $stretching = null): float
    {
        if ($stretching === null) {
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
        $fontfamily = \preg_replace('/[^a-z0-9_\,]/', '', \strtolower($fontfamily));
        if ($fontfamily === null) {
            throw new FontException('Invalid font family name');
        }

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
            $font = \preg_replace($fontpattern, $fontreplacement, $font);
            if ($font === null) {
                throw new FontException('Invalid font family name: ' . $fontfamily);
            }

            // replace common family names and core fonts
            $fontkey = \preg_replace($keypattern, $keyreplacement, $font);
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
     * @return string
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
