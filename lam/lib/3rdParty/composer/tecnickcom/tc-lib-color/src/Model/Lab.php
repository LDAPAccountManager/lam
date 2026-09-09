<?php

declare(strict_types=1);

/**
 * Lab.php
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

namespace Com\Tecnick\Color\Model;

/**
 * Com\Tecnick\Color\Model\Lab
 *
 * CIE Lab Color Model class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Lab extends \Com\Tecnick\Color\Model
{
    /**
     * Linear red response of the XYZ to sRGB matrix for the reference white.
     *
     * The three LINEAR_WHITE_* values normalize the matrix rows so that the
     * reference white maps back to exactly (1, 1, 1).
     */
    private const LINEAR_WHITE_R =
        (3.240_454_2 * self::REF_WHITE_X) - (1.537_138_5 * self::REF_WHITE_Y) - (0.498_531_4 * self::REF_WHITE_Z);

    /**
     * Linear green response of the XYZ to sRGB matrix for the reference white.
     */
    private const LINEAR_WHITE_G =
        (1.876_010_8 * self::REF_WHITE_Y) - (0.969_266_0 * self::REF_WHITE_X) + (0.041_556_0 * self::REF_WHITE_Z);

    /**
     * Linear blue response of the XYZ to sRGB matrix for the reference white.
     */
    private const LINEAR_WHITE_B =
        (0.055_643_4 * self::REF_WHITE_X) - (0.204_025_9 * self::REF_WHITE_Y) + (1.057_225_2 * self::REF_WHITE_Z);

    /**
     * Color Model type
     */
    protected string $type = 'LAB';

    /**
     * Value of the Lab L* component [0..100]
     */
    protected float $cmp_lstar = 0.0;

    /**
     * Value of the Lab a* component [-128..127]
     */
    protected float $cmp_astar = 0.0;

    /**
     * Value of the Lab b* component [-128..127]
     */
    protected float $cmp_bstar = 0.0;

    /**
     * Initialize a new Lab color object.
     *
     * L* is clamped to [0..100], a* and b* to [-128..127] and alpha to [0..1].
     *
     * @param array<string, int|float|string> $components Color components.
     *
     * @throws \Com\Tecnick\Color\UnknownComponentException if a component name is not defined by this model
     */
    public function __construct(array $components)
    {
        self::checkComponents($components, ['lstar', 'astar', 'bstar', 'alpha']);
        $this->cmp_lstar = self::component($components, 'lstar', 0.0, 0.0, 100.0);
        $this->cmp_astar = self::component($components, 'astar', 0.0, -128.0, 127.0);
        $this->cmp_bstar = self::component($components, 'bstar', 0.0, -128.0, 127.0);
        $this->cmp_alpha = self::component($components, 'alpha', 1.0);
    }

    /**
     * Get an array with all color components.
     *
     * @return array<string, float> with keys ('L', 'a', 'b', 'A')
     */
    public function getArray(): array
    {
        return [
            'L' => $this->cmp_lstar,
            'a' => $this->cmp_astar,
            'b' => $this->cmp_bstar,
            'A' => $this->cmp_alpha,
        ];
    }

    /**
     * Get an array with all color components for
     * the PDF appearance characteristics dictionary.
     *
     * The values are in the range 0.0 to 1.0 and the number of array elements
     * determines the color space:
     * 3 = DeviceRGB
     *
     * @return array<float> DeviceRGB color components ('R', 'G', 'B')
     */
    public function getPDFacArray(): array
    {
        $rgb = $this->toRgbArray();
        return [
            $rgb['red'] ?? 0.0,
            $rgb['green'] ?? 0.0,
            $rgb['blue'] ?? 0.0,
        ];
    }

    /**
     * Get an array with color components values normalized.
     *
     * NOTE: the $max argument is not applied here. L* is returned in its own
     * [0..100] range, a* and b* in [-128..127], and alpha is a fraction
     * component kept in the [0..1] range.
     *
     * @param int $max Unused; kept for interface compatibility.
     *
     * @return array<string, float> with keys ('L', 'a', 'b', 'A')
     */
    public function getNormalizedArray(int $max): array
    {
        return [
            'L' => \round($this->cmp_lstar),
            'a' => \round($this->cmp_astar),
            'b' => \round($this->cmp_bstar),
            'A' => $this->cmp_alpha,
        ];
    }

    /**
     * Get the CSS representation of the color: lab(L a b) or lab(L a b / A)
     *
     * Components are emitted in fixed notation with up to 4 decimals.
     */
    public function getCssColor(): string
    {
        $color =
            'lab('
            . $this->getCssNumber($this->cmp_lstar)
            . '% '
            . $this->getCssNumber($this->cmp_astar)
            . ' '
            . $this->getCssNumber($this->cmp_bstar);
        if ($this->cmp_alpha < 1.0) {
            $color .= ' / ' . $this->getCssValue($this->cmp_alpha, 1);
        }

        return $color . ')';
    }

    /**
     * Get the color format used in Acrobat JavaScript
     * NOTE: the alpha channel is omitted from this representation unless it is 0 = transparent
     */
    public function getJsPdfColor(): string
    {
        if ($this->cmp_alpha === 0.0) {
            return '["T"]'; // transparent color
        }

        $rgb = $this->toRgbArray();
        return \sprintf('["RGB",%F,%F,%F]', $rgb['red'] ?? 0.0, $rgb['green'] ?? 0.0, $rgb['blue'] ?? 0.0);
    }

    /**
     * Get a space separated string with Lab component values.
     */
    public function getComponentsString(): string
    {
        return \sprintf('%F %F %F', $this->cmp_lstar, $this->cmp_astar, $this->cmp_bstar);
    }

    /**
     * Get the color components format used in PDF documents (RGB)
     * NOTE: the alpha channel is omitted
     *
     * @param bool $stroke True for stroking (lines, drawing) and false for non-stroking (text and area filling).
     */
    public function getPdfColor(bool $stroke = false): string
    {
        return self::rgbPdfColor($this->toRgbArray(), $stroke);
    }

    /**
     * Get an array with Gray color components
     *
     * @return array<string, float> with keys ('gray', 'alpha')
     */
    public function toGrayArray(): array
    {
        return self::rgbToGray($this->toRgbArray());
    }

    /**
     * Get an array with RGB color components
     *
     * @return array<string, float> with keys ('red', 'green', 'blue', 'alpha')
     */
    public function toRgbArray(): array
    {
        $fyn = ($this->cmp_lstar + 16.0) / 116.0;
        $fxn = $fyn + ($this->cmp_astar / 500.0);
        $fzn = $fyn - ($this->cmp_bstar / 200.0);

        $xRel = $this->pivotLabToXyz($fxn);
        $yRel = (float) (
            $this->cmp_lstar > 8.0 ? \pow(($this->cmp_lstar + 16.0) / 116.0, 3.0) : $this->cmp_lstar / 903.3
        );
        $zRel = $this->pivotLabToXyz($fzn);

        $xTri = $xRel * self::REF_WHITE_X;
        $yTri = $yRel * self::REF_WHITE_Y;
        $zTri = $zRel * self::REF_WHITE_Z;

        $red = ((3.240_454_2 * $xTri) + (-1.537_138_5 * $yTri) + (-0.498_531_4 * $zTri)) / self::LINEAR_WHITE_R;
        $green = ((-0.969_266_0 * $xTri) + (1.876_010_8 * $yTri) + (0.041_556_0 * $zTri)) / self::LINEAR_WHITE_G;
        $blue = ((0.055_643_4 * $xTri) + (-0.204_025_9 * $yTri) + (1.057_225_2 * $zTri)) / self::LINEAR_WHITE_B;

        return [
            'red' => \max(0.0, \min(1.0, $this->linearToSrgb($red))),
            'green' => \max(0.0, \min(1.0, $this->linearToSrgb($green))),
            'blue' => \max(0.0, \min(1.0, $this->linearToSrgb($blue))),
            'alpha' => $this->cmp_alpha,
        ];
    }

    /**
     * Get an array with HSL color components
     *
     * @return array<string, float> with keys ('hue', 'saturation', 'lightness', 'alpha')
     */
    public function toHslArray(): array
    {
        return self::rgbToHsl($this->toRgbArray());
    }

    /**
     * Get an array with CMYK color components
     *
     * @return array<string, float> with keys ('cyan', 'magenta', 'yellow', 'key', 'alpha')
     */
    public function toCmykArray(): array
    {
        return self::rgbToCmyk($this->toRgbArray());
    }

    /**
     * Get an array with Lab color components
     *
     * @return array<string, float> with keys ('lstar', 'astar', 'bstar', 'alpha')
     */
    public function toLabArray(): array
    {
        return [
            'lstar' => $this->cmp_lstar,
            'astar' => $this->cmp_astar,
            'bstar' => $this->cmp_bstar,
            'alpha' => $this->cmp_alpha,
        ];
    }

    /**
     * Invert the color.
     */
    public function invertColor(): static
    {
        $lab = self::rgbToLab(self::invertRgb($this->toRgbArray()));
        $this->cmp_lstar = $lab['lstar'] ?? 0.0;
        $this->cmp_astar = $lab['astar'] ?? 0.0;
        $this->cmp_bstar = $lab['bstar'] ?? 0.0;
        return $this;
    }

    /**
     * Apply the CIE Lab inverse pivot function.
     */
    private function pivotLabToXyz(float $value): float
    {
        $cubed = (float) \pow($value, 3.0);
        if ($cubed > 0.008_856_451_679_035_631) {
            return $cubed;
        }

        return ($value - (16.0 / 116.0)) / 7.787_037_037_037_037;
    }

    /**
     * Convert linear RGB component to sRGB in [0..1].
     */
    private function linearToSrgb(float $component): float
    {
        if ($component <= 0.003_130_8) {
            return 12.92 * $component;
        }

        return (1.055 * (float) \pow($component, 1.0 / 2.4)) - 0.055;
    }
}
