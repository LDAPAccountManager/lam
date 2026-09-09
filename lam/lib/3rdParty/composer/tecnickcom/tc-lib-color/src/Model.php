<?php

declare(strict_types=1);

/**
 * Model.php
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
 * Com\Tecnick\Color\Model
 *
 * Color Model class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
abstract class Model implements \Com\Tecnick\Color\Model\Template
{
    /**
     * X tristimulus value of the D65 reference white.
     *
     * The three REF_WHITE_* values are the row sums of the sRGB to XYZ matrix
     * used by rgbToLab(), so that white maps to exactly (1, 1, 1).
     */
    protected const REF_WHITE_X = 0.950_470_0;

    /**
     * Y tristimulus value of the D65 reference white.
     */
    protected const REF_WHITE_Y = 1.000_000_1;

    /**
     * Z tristimulus value of the D65 reference white.
     */
    protected const REF_WHITE_Z = 1.088_830_0;

    /**
     * Color Model type (GRAY, RGB, HSL, CMYK, LAB)
     */
    protected string $type;

    /**
     * Value of the Alpha channel component.
     * Values range between 0.0 (fully transparent) and 1.0 (fully opaque)
     */
    protected float $cmp_alpha = 1.0;

    /**
     * Create a color model instance of the given type.
     *
     * @param ColorModelType|string           $type       Color model type.
     * @param array<string, int|float|string> $components Color components.
     *
     * @throws ColorException            if the type is unknown
     * @throws UnknownComponentException if a component name is not defined by the model
     */
    public static function create(ColorModelType|string $type, array $components = []): self
    {
        return match (ColorModelType::fromLoose($type)) {
            ColorModelType::Gray => new \Com\Tecnick\Color\Model\Gray($components),
            ColorModelType::Rgb => new \Com\Tecnick\Color\Model\Rgb($components),
            ColorModelType::Hsl => new \Com\Tecnick\Color\Model\Hsl($components),
            ColorModelType::Cmyk => new \Com\Tecnick\Color\Model\Cmyk($components),
            ColorModelType::Lab => new \Com\Tecnick\Color\Model\Lab($components),
        };
    }

    /**
     * Reject component names that the model does not define.
     *
     * @param array<string, int|float|string> $components Components as supplied.
     * @param list<string>                    $known      Names the model defines.
     *
     * @throws UnknownComponentException if $components carries an unknown name
     */
    protected static function checkComponents(array $components, array $known): void
    {
        $unknown = \array_diff(\array_keys($components), $known);
        if ($unknown !== []) {
            throw new UnknownComponentException('unknown color components: ' . \implode(', ', $unknown));
        }
    }

    /**
     * Read a color component and clamp it into [$min..$max].
     *
     * Non-numeric values and NAN fall back to $default. INF and -INF are
     * clamped to $max and $min.
     *
     * @param array<string, int|float|string> $components Components as supplied.
     */
    protected static function component(
        array $components,
        string $name,
        float $default = 0.0,
        float $min = 0.0,
        float $max = 1.0,
    ): float {
        return \max($min, \min($max, self::numericValue($components[$name] ?? $default, $default)));
    }

    /**
     * Read the hue component as a fraction of a full turn, wrapped into [0..1).
     *
     * Hue is an angle: out-of-range values wrap around instead of being clamped.
     *
     * @param array<string, int|float|string> $components Components as supplied.
     */
    protected static function hueComponent(array $components, string $name): float
    {
        $hue = \fmod(self::numericValue($components[$name] ?? 0.0, 0.0), 1.0);
        return $hue < 0.0 ? $hue + 1.0 : $hue;
    }

    /**
     * Convert a component value to a float, falling back to $default.
     *
     * NAN is rejected as well as non-numeric values.
     *
     * @param int|float|string $value Value as supplied.
     */
    private static function numericValue(int|float|string $value, float $default): float
    {
        $num = \is_numeric($value) ? (float) $value : $default;
        return \is_nan($num) ? $default : $num;
    }

    /**
     * Convert an RGB component array to Gray.
     *
     * @param array<string, float> $rgb Keys ('red', 'green', 'blue', 'alpha')
     *
     * @return array<string, float> with keys ('gray', 'alpha')
     */
    protected static function rgbToGray(array $rgb): array
    {
        // convert using the ITU-R BT.709 (Rec. 709) luma coefficients
        return [
            'gray' => \max(0.0, \min(
                1.0,
                (0.2126 * ($rgb['red'] ?? 0.0)) + (0.7152 * ($rgb['green'] ?? 0.0)) + (0.0722 * ($rgb['blue'] ?? 0.0)),
            )),
            'alpha' => $rgb['alpha'] ?? 1.0,
        ];
    }

    /**
     * Convert an RGB component array to HSL.
     *
     * @param array<string, float> $rgb Keys ('red', 'green', 'blue', 'alpha')
     *
     * @return array<string, float> with keys ('hue', 'saturation', 'lightness', 'alpha')
     */
    protected static function rgbToHsl(array $rgb): array
    {
        $red = $rgb['red'] ?? 0.0;
        $green = $rgb['green'] ?? 0.0;
        $blue = $rgb['blue'] ?? 0.0;

        $min = \min($red, $green, $blue);
        $max = \max($red, $green, $blue);
        $lightness = ($min + $max) / 2;
        $saturation = 0.0;
        $hue = 0.0;
        if ($min !== $max) {
            $diff = $max - $min;
            $saturation = $lightness < 0.5 ? $diff / ($max + $min) : $diff / (2.0 - $max - $min);

            // the sector is picked by the channel holding the maximum
            if ($max === $red) {
                $dgb = $green - $blue;
                $hue = ($dgb / $diff) + ($dgb < 0 ? 6 : 0);
            } elseif ($max === $green) {
                $hue = 2.0 + (($blue - $red) / $diff);
            } else {
                $hue = 4.0 + (($red - $green) / $diff);
            }

            $hue /= 6; // 6 = 360 / 60
        }

        return [
            'hue' => \max(0.0, \min(1.0, $hue)),
            'saturation' => \max(0.0, \min(1.0, $saturation)),
            'lightness' => \max(0.0, \min(1.0, $lightness)),
            'alpha' => $rgb['alpha'] ?? 1.0,
        ];
    }

    /**
     * Convert an RGB component array to CMYK.
     *
     * @param array<string, float> $rgb Keys ('red', 'green', 'blue', 'alpha')
     *
     * @return array<string, float> with keys ('cyan', 'magenta', 'yellow', 'key', 'alpha')
     */
    protected static function rgbToCmyk(array $rgb): array
    {
        $cyan = 1 - ($rgb['red'] ?? 0.0);
        $magenta = 1 - ($rgb['green'] ?? 0.0);
        $yellow = 1 - ($rgb['blue'] ?? 0.0);
        $key = \min(1.0, $cyan, $magenta, $yellow);

        if ($key === 1.0) {
            // black
            $cyan = 0.0;
            $magenta = 0.0;
            $yellow = 0.0;
        } else {
            $cyan = ($cyan - $key) / (1 - $key);
            $magenta = ($magenta - $key) / (1 - $key);
            $yellow = ($yellow - $key) / (1 - $key);
        }

        return [
            'cyan' => \max(0.0, \min(1.0, $cyan)),
            'magenta' => \max(0.0, \min(1.0, $magenta)),
            'yellow' => \max(0.0, \min(1.0, $yellow)),
            'key' => \max(0.0, \min(1.0, $key)),
            'alpha' => $rgb['alpha'] ?? 1.0,
        ];
    }

    /**
     * Convert an RGB component array to CIE Lab.
     *
     * @param array<string, float> $rgb Keys ('red', 'green', 'blue', 'alpha')
     *
     * @return array<string, float> with keys ('lstar', 'astar', 'bstar', 'alpha')
     */
    protected static function rgbToLab(array $rgb): array
    {
        $red = self::srgbToLinear($rgb['red'] ?? 0.0);
        $green = self::srgbToLinear($rgb['green'] ?? 0.0);
        $blue = self::srgbToLinear($rgb['blue'] ?? 0.0);

        $xTri = (0.412_456_4 * $red) + (0.357_576_1 * $green) + (0.180_437_5 * $blue);
        $yTri = (0.212_672_9 * $red) + (0.715_152_2 * $green) + (0.072_175_0 * $blue);
        $zTri = (0.019_333_9 * $red) + (0.119_192_0 * $green) + (0.950_304_1 * $blue);

        $fxn = self::pivotXyzToLab($xTri / self::REF_WHITE_X);
        $fyn = self::pivotXyzToLab($yTri / self::REF_WHITE_Y);
        $fzn = self::pivotXyzToLab($zTri / self::REF_WHITE_Z);

        return [
            'lstar' => \max(0.0, \min(100.0, (116.0 * $fyn) - 16.0)),
            'astar' => \max(-128.0, \min(127.0, 500.0 * ($fxn - $fyn))),
            'bstar' => \max(-128.0, \min(127.0, 200.0 * ($fyn - $fzn))),
            'alpha' => $rgb['alpha'] ?? 1.0,
        ];
    }

    /**
     * Complement every RGB channel, leaving the alpha channel untouched.
     *
     * @param array<string, float> $rgb Keys ('red', 'green', 'blue', 'alpha')
     *
     * @return array<string, float> with keys ('red', 'green', 'blue', 'alpha')
     */
    protected static function invertRgb(array $rgb): array
    {
        return [
            'red' => 1 - ($rgb['red'] ?? 0.0),
            'green' => 1 - ($rgb['green'] ?? 0.0),
            'blue' => 1 - ($rgb['blue'] ?? 0.0),
            'alpha' => $rgb['alpha'] ?? 1.0,
        ];
    }

    /**
     * Get a space separated string with the RGB component values.
     *
     * @param array<string, float> $rgb Keys ('red', 'green', 'blue')
     */
    protected static function rgbComponentsString(array $rgb): string
    {
        return \sprintf('%F %F %F', $rgb['red'] ?? 0.0, $rgb['green'] ?? 0.0, $rgb['blue'] ?? 0.0);
    }

    /**
     * Get the DeviceRGB color operator used in PDF documents.
     *
     * @param array<string, float> $rgb    Keys ('red', 'green', 'blue')
     * @param bool                 $stroke True for stroking, false for non-stroking.
     */
    protected static function rgbPdfColor(array $rgb, bool $stroke): string
    {
        $mode = $stroke ? 'RG' : 'rg';
        return self::rgbComponentsString($rgb) . ' ' . $mode . "\n";
    }

    /**
     * Convert an sRGB component in [0..1] to linear RGB.
     */
    private static function srgbToLinear(float $component): float
    {
        if ($component <= 0.040_45) {
            return $component / 12.92;
        }

        return (float) \pow(($component + 0.055) / 1.055, 2.4);
    }

    /**
     * Apply the CIE Lab forward pivot function.
     */
    private static function pivotXyzToLab(float $value): float
    {
        if ($value > 0.008_856_451_679_035_631) {
            return (float) \pow($value, 1.0 / 3.0);
        }

        return (7.787_037_037_037_037 * $value) + (16.0 / 116.0);
    }

    /**
     * Get the color model type (GRAY, RGB, HSL, CMYK, LAB)
     */
    public function getType(): string
    {
        return $this->type;
    }

    /**
     * Get the color model type as a backed enum case.
     *
     * @throws ColorException if the model declares an unknown type
     */
    public function getTypeEnum(): ColorModelType
    {
        return ColorModelType::fromLoose($this->type);
    }

    /**
     * Get the normalized integer value of the specified float fraction
     *
     * @param float $value Fraction value to convert [0..1]
     * @param int   $max   Maximum value to return (reference value)
     *
     * @return float value [0..$max]
     */
    public function getNormalizedValue(float $value, int $max): float
    {
        // the inner round compensates the float representation of the input
        return \round(\max(0, \min($max, $max * \round($value, 14))));
    }

    /**
     * Format a color component scaled to [0..$max] for CSS output.
     *
     * Keeps up to 4 decimals, with the trailing zeros dropped.
     *
     * @param float $value Fraction value to convert [0..1]
     * @param int   $max   Maximum value to return (reference value)
     */
    protected function getCssValue(float $value, int $max): string
    {
        return $this->getCssNumber(\max(0.0, \min(1.0, $value)) * $max);
    }

    /**
     * Format a number in fixed notation for CSS output.
     *
     * Keeps up to 4 decimals, with the trailing zeros dropped.
     *
     * @param float $value Value to format.
     */
    protected function getCssNumber(float $value): string
    {
        $out = \rtrim(\rtrim(\number_format($value, 4, '.', ''), '0'), '.');
        return $out === '-0' ? '0' : $out;
    }

    /**
     * Get the normalized hexadecimal value of the specified float fraction
     *
     * The result is zero-padded to 2 digits.
     *
     * @param float $value Fraction value to convert [0..1]
     * @param int   $max   Maximum value to return (reference value) [0..255]
     */
    public function getHexValue(float $value, int $max): string
    {
        return \sprintf('%02x', $this->getNormalizedValue($value, $max));
    }

    /**
     * Get the Hexadecimal representation of the color with alpha channel: #RRGGBBAA
     */
    public function getRgbaHexColor(): string
    {
        $rgba = $this->toRgbArray();
        return (
            '#'
            . $this->getHexValue($rgba['red'] ?? 0.0, 255)
            . $this->getHexValue($rgba['green'] ?? 0.0, 255)
            . $this->getHexValue($rgba['blue'] ?? 0.0, 255)
            . $this->getHexValue($rgba['alpha'] ?? 1.0, 255)
        );
    }

    /**
     * Get a copy of the color, inverted.
     *
     * The receiver is left untouched, unlike invertColor().
     */
    public function withInvertedColor(): static
    {
        return (clone $this)->invertColor();
    }

    /**
     * Get the Hexadecimal representation of the color: #RRGGBB
     */
    public function getRgbHexColor(): string
    {
        $rgba = $this->toRgbArray();
        return (
            '#'
            . $this->getHexValue($rgba['red'] ?? 0.0, 255)
            . $this->getHexValue($rgba['green'] ?? 0.0, 255)
            . $this->getHexValue($rgba['blue'] ?? 0.0, 255)
        );
    }
}
