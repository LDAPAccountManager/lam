<?php

declare(strict_types=1);

/**
 * Css.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Com\Tecnick\Color;

use Com\Tecnick\Color\Exception as ColorException;

/**
 * Com\Tecnick\Color\Css
 *
 * Css Color class
 *
 * Parses the CSS and Acrobat JavaScript color notations. Component scaling is
 * delegated to a ComponentNormalizer held as a collaborator.
 *
 * Extension points: the protected parser methods.
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
abstract class Css
{
    /**
     * A CSS number: an optionally signed integer or decimal.
     *
     * A digit-and-dot run such as "1.2.3" does not match.
     */
    private const REX_NUMBER = '[+-]?(?:\d+(?:\.\d*)?|\.\d+)';

    /**
     * A CSS component value: a number, optionally expressed as a percentage.
     *
     * Out-of-range values are accepted here and clamped by the component readers.
     */
    private const REX_VALUE = self::REX_NUMBER . '%?';

    /**
     * The separator between two CSS component values: a comma or a space run.
     */
    private const REX_SEP = '(?:\s*,\s*|\s+)';

    /**
     * The optional alpha component: a comma or a slash, then a value.
     */
    private const REX_ALPHA = '(?:\s*[\/,]\s*(' . self::REX_VALUE . '))?';

    /**
     * Scales a parsed component value into the [0..1] range.
     */
    protected ComponentNormalizer $normalizer;

    /**
     * @param ComponentNormalizer|null $normalizer Component scaler; the default is used when omitted.
     */
    public function __construct(?ComponentNormalizer $normalizer = null)
    {
        $this->normalizer = $normalizer ?? new ComponentNormalizer();
    }

    /**
     * Match a color string against a regular expression.
     *
     * A PCRE failure raises; a color that does not match yields an empty array.
     *
     * @param string $rex   Regular expression to match.
     * @param string $color Color specification.
     *
     * @return array<array-key, string> the matched groups, empty if there is no match
     *
     * @throws ColorException if the expression cannot be evaluated
     */
    protected function tryMatchColor(string $rex, string $color): array
    {
        $col = [];
        if (\preg_match($rex, $color, $col) === false) {
            throw new ColorException('unable to parse the color (' . \preg_last_error_msg() . '): ' . $color);
        }

        return $col;
    }

    /**
     * Match a color string against a regular expression, requiring a match.
     *
     * @param string $rex   Regular expression to match.
     * @param string $color Color specification.
     * @param string $error Error message prefix for a color that does not match.
     *
     * @return array<array-key, string> the matched groups
     *
     * @throws ColorException if the color does not match or cannot be parsed
     */
    private function matchColor(string $rex, string $color, string $error): array
    {
        $col = $this->tryMatchColor($rex, $color);
        if ($col === []) {
            throw new ColorException($error . $color);
        }

        return $col;
    }

    /**
     * Get the color object from acrobat Javascript syntax
     *
     * @param string $color color specification (e.g.: ["RGB",0.1,0.3,1])
     *
     * @throws ColorException if the color syntax is invalid
     */
    protected function getColorObjFromJs(string $color): ?\Com\Tecnick\Color\Model
    {
        // the model is the quoted name (the input is lowercased by the caller)
        $col = $this->matchColor('/^\[\s*[\'"](t|g|rgb|cmyk)[\'"]/', $color, 'invalid javascript color: ');
        $num = '\s*,\s*(' . self::REX_NUMBER . ')';

        switch ($col[1] ?? '') {
            case 'g':
                $col = $this->matchColor('/^\[\s*[\'"]g[\'"]' . $num . '\s*\]$/', $color, 'invalid javascript color: ');

                return new \Com\Tecnick\Color\Model\Gray([
                    'gray' => $col[1] ?? '0',
                    'alpha' => 1,
                ]);
            case 'rgb':
                $col = $this->matchColor(
                    '/^\[\s*[\'"]rgb[\'"]' . $num . $num . $num . '\s*\]$/',
                    $color,
                    'invalid javascript color: ',
                );

                return new \Com\Tecnick\Color\Model\Rgb([
                    'red' => $col[1] ?? '0',
                    'green' => $col[2] ?? '0',
                    'blue' => $col[3] ?? '0',
                    'alpha' => 1,
                ]);
            case 'cmyk':
                $col = $this->matchColor(
                    '/^\[\s*[\'"]cmyk[\'"]' . $num . $num . $num . $num . '\s*\]$/',
                    $color,
                    'invalid javascript color: ',
                );

                return new \Com\Tecnick\Color\Model\Cmyk([
                    'cyan' => $col[1] ?? '0',
                    'magenta' => $col[2] ?? '0',
                    'yellow' => $col[3] ?? '0',
                    'key' => $col[4] ?? '0',
                    'alpha' => 1,
                ]);
        }

        // case 't': the transparent form carries no components
        $this->matchColor('/^\[\s*[\'"]t[\'"]\s*\]$/', $color, 'invalid javascript color: ');
        return null;
    }

    /**
     * Get the color object from a CSS color string
     *
     * @param string $type  color type: t, g, rgb, rgba, hsl, hsla, cmyk, cmyka, lab
     * @param string $color color specification (e.g.: rgb(255,128,64))
     *
     * @throws ColorException if the color syntax is invalid
     */
    protected function getColorObjFromCss(string $type, string $color): ?\Com\Tecnick\Color\Model
    {
        $this->checkSeparators($color);

        switch ($type) {
            case 'g':
                return $this->getColorObjFromCssGray($color);
            case 'rgb':
            case 'rgba':
                return $this->getColorObjFromCssRgb($color);
            case 'hsl':
            case 'hsla':
                return $this->getColorObjFromCssHsl($color);
            case 'cmyk':
            case 'cmyka':
                return $this->getColorObjFromCssCmyk($color);
            case 'lab':
                return $this->getColorObjFromCssLab($color);
        }

        // case 't': the transparent form carries no components
        $this->matchColor('/^t\s*\(\s*\)$/', $color, 'invalid css color: ');
        return null;
    }

    /**
     * Reject an argument list that mixes the two CSS component syntaxes.
     *
     * Either the comma-separated form, rgb(64, 128, 191, 0.5), or the
     * space-separated form with a slash before the alpha,
     * rgb(64 128 191 / 0.5). Whitespace around a comma is allowed.
     *
     * @param string $color color specification.
     *
     * @throws ColorException if the separators are mixed
     */
    private function checkSeparators(string $color): void
    {
        if (!str_contains($color, ',')) {
            return;
        }

        $mixed = str_contains($color, '/') || \preg_match('/[0-9%a-z]\s+[0-9.+-]/', $color) === 1;
        if ($mixed) {
            throw new ColorException('mixed css color separators: ' . $color);
        }
    }

    /**
     * Get the color object from a CSS Gray color string
     *
     * @param string $color color specification (e.g.: g(128))
     *
     * @throws ColorException if the color syntax is invalid
     */
    private function getColorObjFromCssGray(string $color): \Com\Tecnick\Color\Model\Gray
    {
        $rex = '/^g\s*\(\s*(' . self::REX_VALUE . ')\s*\)$/';
        $col = $this->matchColor($rex, $color, 'invalid css color: ');

        return new \Com\Tecnick\Color\Model\Gray([
            'gray' => $this->normalizer->normalize($col[1] ?? '0', 255),
            'alpha' => 1,
        ]);
    }

    /**
     * Get the color object from a CSS RGB/RGBA color string
     *
     * @param string $color color specification (e.g.: rgb(255,128,64))
     *
     * @throws ColorException if the color syntax is invalid
     */
    private function getColorObjFromCssRgb(string $color): \Com\Tecnick\Color\Model\Rgb
    {
        $rex =
            '/^rgba?\s*\(\s*('
            . self::REX_VALUE
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_VALUE
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_VALUE
            . ')'
            . self::REX_ALPHA
            . '\s*\)$/';
        $col = $this->matchColor($rex, $color, 'invalid css color: ');

        return new \Com\Tecnick\Color\Model\Rgb([
            'red' => $this->normalizer->normalize($col[1] ?? '0', 255),
            'green' => $this->normalizer->normalize($col[2] ?? '0', 255),
            'blue' => $this->normalizer->normalize($col[3] ?? '0', 255),
            'alpha' => $this->normalizeAlpha($col[4] ?? ''),
        ]);
    }

    /**
     * Get the color object from a CSS HSL/HSLA color string
     *
     * @param string $color color specification (e.g.: hsl(120,100%,50%))
     *
     * @throws ColorException if the color syntax is invalid
     */
    private function getColorObjFromCssHsl(string $color): \Com\Tecnick\Color\Model\Hsl
    {
        $rex =
            '/^hsla?\s*\(\s*('
            . self::REX_NUMBER
            . '(?:%|deg|grad|rad|turn)?)'
            . self::REX_SEP
            . '('
            . self::REX_VALUE
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_VALUE
            . ')'
            . self::REX_ALPHA
            . '\s*\)$/';
        $col = $this->matchColor($rex, $color, 'invalid css color: ');

        // saturation and lightness are percentages: a bare number is divided by 100
        return new \Com\Tecnick\Color\Model\Hsl([
            'hue' => $this->normalizeHue($col[1] ?? '0'),
            'saturation' => $this->normalizer->normalize($col[2] ?? '0', 100),
            'lightness' => $this->normalizer->normalize($col[3] ?? '0', 100),
            'alpha' => $this->normalizeAlpha($col[4] ?? ''),
        ]);
    }

    /**
     * Normalize a CSS hue angle to a fraction of a full turn.
     *
     * Accepts the CSS angle units (deg, grad, rad, turn); a bare number and a
     * percentage are read as degrees. The result is not clamped: the color
     * model wraps it into [0..1).
     */
    private function normalizeHue(string $hue): float
    {
        $units = [
            'grad' => 400.0,
            'turn' => 1.0,
            'deg' => 360.0,
            'rad' => 2 * \M_PI,
            '%' => 360.0,
        ];

        $value = $hue;
        $turn = 360.0;
        foreach ($units as $unit => $full) {
            if (!str_ends_with($hue, $unit)) {
                continue;
            }

            $value = \substr($hue, 0, -\strlen($unit));
            $turn = $full;
            break;
        }

        return \is_numeric($value) ? (float) $value / $turn : 0.0;
    }

    /**
     * Get the color object from a CSS CMYK color string
     *
     * @param string $color color specification (e.g.: cmyk(100,0,50,0))
     *
     * @throws ColorException if the color syntax is invalid
     */
    private function getColorObjFromCssCmyk(string $color): \Com\Tecnick\Color\Model\Cmyk
    {
        $rex =
            '/^cmyka?\s*\(\s*('
            . self::REX_VALUE
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_VALUE
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_VALUE
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_VALUE
            . ')'
            . self::REX_ALPHA
            . '\s*\)$/';
        $col = $this->matchColor($rex, $color, 'invalid css color: ');

        return new \Com\Tecnick\Color\Model\Cmyk([
            'cyan' => $this->normalizer->normalize($col[1] ?? '0', 100),
            'magenta' => $this->normalizer->normalize($col[2] ?? '0', 100),
            'yellow' => $this->normalizer->normalize($col[3] ?? '0', 100),
            'key' => $this->normalizer->normalize($col[4] ?? '0', 100),
            'alpha' => $this->normalizeAlpha($col[5] ?? ''),
        ]);
    }

    /**
     * Get the color object from a CSS Lab color string.
     *
     * Supports forms such as: lab(52% 0 -39), lab(52% 0 -39 / 0.85), lab(52,0,-39,0.85)
     *
     * @throws ColorException if the color syntax is invalid
     */
    private function getColorObjFromCssLab(string $color): \Com\Tecnick\Color\Model\Lab
    {
        $rex =
            '/^lab\s*\(\s*('
            . self::REX_VALUE
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_NUMBER
            . ')'
            . self::REX_SEP
            . '('
            . self::REX_NUMBER
            . ')'
            . self::REX_ALPHA
            . '\s*\)$/';
        $col = $this->matchColor($rex, $color, 'invalid css color: ');

        $lstarRaw = $col[1] ?? '0';
        $lstarVal = str_ends_with($lstarRaw, '%') ? \str_replace('%', '', $lstarRaw) : $lstarRaw;
        $lstar = \is_numeric($lstarVal) ? (float) $lstarVal : 0.0;

        return new \Com\Tecnick\Color\Model\Lab([
            'lstar' => $lstar,
            'astar' => $col[2] ?? '0',
            'bstar' => $col[3] ?? '0',
            'alpha' => $this->normalizeAlpha($col[4] ?? ''),
        ]);
    }

    /**
     * Normalize a CSS alpha value to a float in the [0..1] range.
     *
     * Accepts a fraction (e.g. "0.85") or a percentage (e.g. "85%"). An empty
     * string yields 1.0 (fully opaque).
     */
    private function normalizeAlpha(string $alpha): float
    {
        if ($alpha === '') {
            return 1.0;
        }

        if (str_ends_with($alpha, '%')) {
            $value = \str_replace('%', '', $alpha);
            return \is_numeric($value) ? \max(0.0, \min(1.0, (float) $value / 100.0)) : 1.0;
        }

        return \is_numeric($alpha) ? \max(0.0, \min(1.0, (float) $alpha)) : 1.0;
    }
}
