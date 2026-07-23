<?php

declare(strict_types=1);

/**
 * Rgb.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Com\Tecnick\Color\Model;

/**
 * Com\Tecnick\Color\Model\Rgb
 *
 * RGB Color Model class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Rgb extends \Com\Tecnick\Color\Model
{
    /**
     * Color Model type
     *
     * @var string
     */
    protected $type = 'RGB';

    /**
     * Value of the Red color component [0..1]
     *
     * @var float
     */
    protected float $cmp_red = 0.0;

    /**
     * Value of the Green color component [0..1]
     *
     * @var float
     */
    protected float $cmp_green = 0.0;

    /**
     * Value of the Blue color component [0..1]
     *
     * @var float
     */
    protected float $cmp_blue = 0.0;

    /**
     * Get an array with all color components.
     *
     * @return array<string, float> with keys ('R', 'G', 'B', 'A')
     */
    public function getArray(): array
    {
        return [
            'R' => $this->cmp_red,
            'G' => $this->cmp_green,
            'B' => $this->cmp_blue,
            'A' => $this->cmp_alpha,
        ];
    }

    /**
     * Get an array with all color components for
     * the PDF appearance characteristics dictionary.
     *
     * The numbers that shall be in the range 0.0 to 1.0.
     * The number of array elements determines the colour space
     * in which the colour shall be defined:
     * 3 = DeviceRGB
     *
     * @return array<float> DeviceRGB color components('R', 'G', 'B')
     */
    public function getPDFacArray(): array
    {
        return [
            $this->cmp_red,
            $this->cmp_green,
            $this->cmp_blue,
        ];
    }

    /**
     * Get an array with color components values normalized between 0 and $max.
     * NOTE: the alpha and other fraction component values are kept in the [0..1] range.
     *
     * @param int $max Maximum value to return (reference value)
     *
     * @return array<string, float> with keys ('R', 'G', 'B', 'A')
     */
    public function getNormalizedArray(int $max): array
    {
        return [
            'R' => $this->getNormalizedValue($this->cmp_red, $max),
            'G' => $this->getNormalizedValue($this->cmp_green, $max),
            'B' => $this->getNormalizedValue($this->cmp_blue, $max),
            'A' => $this->cmp_alpha,
        ];
    }

    /**
     * Get the CSS representation of the color: rgb(R, G, B) or rgba(R, G, B, A)
     */
    public function getCssColor(): string
    {
        $colorType = 'rgb';
        $alpha = '';
        if ($this->cmp_alpha < 1.0) {
            $colorType = 'rgba';
            $alpha = ',' . $this->cmp_alpha;
        }

        return (
            $colorType
            . '('
            . $this->getNormalizedValue($this->cmp_red, 100)
            . '%,'
            . $this->getNormalizedValue($this->cmp_green, 100)
            . '%,'
            . $this->getNormalizedValue($this->cmp_blue, 100)
            . '%'
            . $alpha
            . ')'
        );
    }

    /**
     * Get the color format used in Acrobat JavaScript
     * NOTE: the alpha channel is omitted from this representation unless is 0 = transparent
     */
    public function getJsPdfColor(): string
    {
        if ($this->cmp_alpha === 0.0) {
            return '["T"]'; // transparent color
        }

        return \sprintf('["RGB",%F,%F,%F]', $this->cmp_red, $this->cmp_green, $this->cmp_blue);
    }

    /**
     * Get a space separated string with color component values.
     */
    public function getComponentsString(): string
    {
        return \sprintf('%F %F %F', $this->cmp_red, $this->cmp_green, $this->cmp_blue);
    }

    /**
     * Get the color components format used in PDF documents (RGB)
     * NOTE: the alpha channel is omitted
     *
     * @param bool $stroke True for stroking (lines, drawing) and false for non-stroking (text and area filling).
     */
    public function getPdfColor(bool $stroke = false): string
    {
        $mode = 'rg';
        if ($stroke) {
            $mode = \strtoupper($mode);
        }

        return $this->getComponentsString() . ' ' . $mode . "\n";
    }

    /**
     * Get an array with Gray color components
     *
     * @return array<string, float> with keys ('gray')
     */
    public function toGrayArray(): array
    {
        // convert using the SMPTE 295M-1997 standard conversion constants
        return [
            'gray' => \max(0, \min(
                1,
                (0.2126 * $this->cmp_red) + (0.7152 * $this->cmp_green) + (0.0722 * $this->cmp_blue),
            )),
            'alpha' => $this->cmp_alpha,
        ];
    }

    /**
     * Get an array with RGB color components
     *
     * @return array<string, float> with keys ('red', 'green', 'blue', 'alpha')
     */
    public function toRgbArray(): array
    {
        return [
            'red' => $this->cmp_red,
            'green' => $this->cmp_green,
            'blue' => $this->cmp_blue,
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
        $min = \min($this->cmp_red, $this->cmp_green, $this->cmp_blue);
        $max = \max($this->cmp_red, $this->cmp_green, $this->cmp_blue);
        $lightness = ($min + $max) / 2;
        $saturation = 0;
        $hue = 0;
        if ($min !== $max) {
            $diff = $max - $min;
            $saturation = $lightness < 0.5 ? $diff / ($max + $min) : $diff / (2.0 - $max - $min);

            switch ($max) {
                case $this->cmp_red:
                    $dgb = $this->cmp_green - $this->cmp_blue;
                    $hue = ($dgb / $diff) + ($dgb < 0 ? 6 : 0);
                    break;
                case $this->cmp_green:
                    $hue = 2.0 + (($this->cmp_blue - $this->cmp_red) / $diff);
                    break;
                case $this->cmp_blue:
                    $hue = 4.0 + (($this->cmp_red - $this->cmp_green) / $diff);
                    break;
            }

            $hue /= 6; // 6 = 360 / 60
        }

        return [
            'hue' => \max(0, \min(1, $hue)),
            'saturation' => \max(0, \min(1, $saturation)),
            'lightness' => \max(0, \min(1, $lightness)),
            'alpha' => $this->cmp_alpha,
        ];
    }

    /**
     * Get an array with CMYK color components
     *
     * @return array<string, float> with keys ('cyan', 'magenta', 'yellow', 'key', 'alpha')
     */
    public function toCmykArray(): array
    {
        $cyan = 1 - $this->cmp_red;
        $magenta = 1 - $this->cmp_green;
        $yellow = 1 - $this->cmp_blue;
        $key = 1.0;
        if ($cyan < $key) {
            $key = $cyan;
        }

        if ($magenta < $key) {
            $key = $magenta;
        }

        if ($yellow < $key) {
            $key = $yellow;
        }

        if ($key === 1.0) {
            // black
            $cyan = 0;
            $magenta = 0;
            $yellow = 0;
        }

        if ($key < 1.0) {
            $cyan = ($cyan - $key) / (1 - $key);
            $magenta = ($magenta - $key) / (1 - $key);
            $yellow = ($yellow - $key) / (1 - $key);
        }

        return [
            'cyan' => \max(0, \min(1, $cyan)),
            'magenta' => \max(0, \min(1, $magenta)),
            'yellow' => \max(0, \min(1, $yellow)),
            'key' => \max(0, \min(1, $key)),
            'alpha' => $this->cmp_alpha,
        ];
    }

    /**
     * Get an array with Lab color components
     *
     * @return array<string, float> with keys ('lstar', 'astar', 'bstar', 'alpha')
     */
    public function toLabArray(): array
    {
        $red = $this->srgbToLinear($this->cmp_red);
        $green = $this->srgbToLinear($this->cmp_green);
        $blue = $this->srgbToLinear($this->cmp_blue);

        $xTri = ((0.412_456_4 * $red) + (0.357_576_1 * $green) + (0.180_437_5 * $blue)) * 100.0;
        $yTri = ((0.212_672_9 * $red) + (0.715_152_2 * $green) + (0.072_175_0 * $blue)) * 100.0;
        $zTri = ((0.019_333_9 * $red) + (0.119_192_0 * $green) + (0.950_304_1 * $blue)) * 100.0;

        $fxn = $this->pivotXyzToLab($xTri / 95.047);
        $fyn = $this->pivotXyzToLab($yTri / 100.000);
        $fzn = $this->pivotXyzToLab($zTri / 108.883);

        return [
            'lstar' => \max(0.0, \min(100.0, (116.0 * $fyn) - 16.0)),
            'astar' => \max(-128.0, \min(127.0, 500.0 * ($fxn - $fyn))),
            'bstar' => \max(-128.0, \min(127.0, 200.0 * ($fyn - $fzn))),
            'alpha' => $this->cmp_alpha,
        ];
    }

    /**
     * Convert sRGB component in [0..1] to linear RGB.
     */
    private function srgbToLinear(float $component): float
    {
        if ($component <= 0.040_45) {
            return $component / 12.92;
        }

        return (float) \pow(($component + 0.055) / 1.055, 2.4);
    }

    /**
     * Apply the CIE Lab forward pivot function.
     */
    private function pivotXyzToLab(float $value): float
    {
        if ($value > 0.008_856_451_679_035_631) {
            return (float) \pow($value, 1.0 / 3.0);
        }

        return (7.787_037_037_037_037 * $value) + (16.0 / 116.0);
    }

    /**
     * Invert the color
     */
    public function invertColor(): self
    {
        $this->cmp_red = 1 - $this->cmp_red;
        $this->cmp_green = 1 - $this->cmp_green;
        $this->cmp_blue = 1 - $this->cmp_blue;
        return $this;
    }
}
