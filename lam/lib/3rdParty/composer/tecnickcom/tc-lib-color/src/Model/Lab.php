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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Lab extends \Com\Tecnick\Color\Model
{
    /**
     * Color Model type
     *
     * @var string
     */
    protected $type = 'LAB';

    /**
     * Value of the Lab L* component [0..100]
     *
     * @var float
     */
    protected float $cmp_lstar = 0.0;

    /**
     * Value of the Lab a* component [-128..127]
     *
     * @var float
     */
    protected float $cmp_astar = 0.0;

    /**
     * Value of the Lab b* component [-128..127]
     *
     * @var float
     */
    protected float $cmp_bstar = 0.0;

    /**
     * Initialize a new Lab color object.
     *
     * @param array<string, int|float|string> $components Color components.
     */
    public function __construct(array $components)
    {
        $this->cmp_alpha = \max(0.0, \min(1.0, (float) ($components['alpha'] ?? 1.0)));
        $this->cmp_lstar = \max(0.0, \min(100.0, (float) ($components['lstar'] ?? 0.0)));
        $this->cmp_astar = \max(-128.0, \min(127.0, (float) ($components['astar'] ?? 0.0)));
        $this->cmp_bstar = \max(-128.0, \min(127.0, (float) ($components['bstar'] ?? 0.0)));
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
     * @param int $max Unused parameter for interface compatibility.
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
     */
    public function getCssColor(): string
    {
        $color = 'lab(' . $this->cmp_lstar . '% ' . $this->cmp_astar . ' ' . $this->cmp_bstar;
        if ($this->cmp_alpha < 1.0) {
            $color .= ' / ' . $this->cmp_alpha;
        }

        return $color . ')';
    }

    /**
     * Get the color format used in Acrobat JavaScript.
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
     * NOTE: the alpha channel is omitted.
     *
     * @param bool $stroke True for stroking (lines, drawing) and false for non-stroking (text and area filling).
     */
    public function getPdfColor(bool $stroke = false): string
    {
        $rgb = new \Com\Tecnick\Color\Model\Rgb($this->toRgbArray());
        return $rgb->getPdfColor($stroke);
    }

    /**
     * Get an array with Gray color components
     *
     * @return array<string, float> with keys ('gray', 'alpha')
     */
    public function toGrayArray(): array
    {
        $rgb = new \Com\Tecnick\Color\Model\Rgb($this->toRgbArray());
        return $rgb->toGrayArray();
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

        $xTri = ($xRel * 95.047) / 100.0;
        $yTri = ($yRel * 100.000) / 100.0;
        $zTri = ($zRel * 108.883) / 100.0;

        $red = (3.240_454_2 * $xTri) + (-1.537_138_5 * $yTri) + (-0.498_531_4 * $zTri);
        $green = (-0.969_266_0 * $xTri) + (1.876_010_8 * $yTri) + (0.041_556_0 * $zTri);
        $blue = (0.055_643_4 * $xTri) + (-0.204_025_9 * $yTri) + (1.057_225_2 * $zTri);

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
        $rgb = new \Com\Tecnick\Color\Model\Rgb($this->toRgbArray());
        return $rgb->toHslArray();
    }

    /**
     * Get an array with CMYK color components
     *
     * @return array<string, float> with keys ('cyan', 'magenta', 'yellow', 'key', 'alpha')
     */
    public function toCmykArray(): array
    {
        $rgb = new \Com\Tecnick\Color\Model\Rgb($this->toRgbArray());
        return $rgb->toCmykArray();
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
    public function invertColor(): self
    {
        $rgb = new \Com\Tecnick\Color\Model\Rgb($this->toRgbArray());
        $rgb->invertColor();
        $lab = new self($rgb->toLabArray());
        $this->cmp_lstar = $lab->cmp_lstar;
        $this->cmp_astar = $lab->cmp_astar;
        $this->cmp_bstar = $lab->cmp_bstar;
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
