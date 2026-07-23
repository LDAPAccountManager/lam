<?php

declare(strict_types=1);

/**
 * Cmyk.php
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
 * Com\Tecnick\Color\Model\Cmyk
 *
 * CMYK Color Model class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Cmyk extends \Com\Tecnick\Color\Model
{
    /**
     * Color Model type
     *
     * @var string
     */
    protected $type = 'CMYK';

    /**
     * Value of the Cyan color component [0..1]
     *
     * @var float
     */
    protected float $cmp_cyan = 0.0;

    /**
     * Value of the Magenta color component [0..1]
     *
     * @var float
     */
    protected float $cmp_magenta = 0.0;

    /**
     * Value of the Yellow color component [0..1]
     *
     * @var float
     */
    protected float $cmp_yellow = 0.0;

    /**
     * Value of the Key (Black) color component [0..1]
     *
     * @var float
     */
    protected float $cmp_key = 0.0;

    /**
     * Get an array with all color components.
     *
     * @return array<string, float> with keys ('C', 'M', 'Y', 'K', 'A')
     */
    public function getArray(): array
    {
        return [
            'C' => $this->cmp_cyan,
            'M' => $this->cmp_magenta,
            'Y' => $this->cmp_yellow,
            'K' => $this->cmp_key,
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
     * 4 = DeviceCMYK
     *
     * @return array<float> DeviceCMYK color components ('C', 'M', 'Y', 'K')
     */
    public function getPDFacArray(): array
    {
        return [
            $this->cmp_cyan,
            $this->cmp_magenta,
            $this->cmp_yellow,
            $this->cmp_key,
        ];
    }

    /**
     * Get an array with color components values normalized between 0 and $max.
     * NOTE: the alpha and other fraction component values are kept in the [0..1] range.
     *
     * @param int $max Maximum value to return (reference value - it should be 100)
     *
     * @return array<string, float> with keys ('C', 'M', 'Y', 'K', 'A')
     */
    public function getNormalizedArray(int $max): array
    {
        return [
            'C' => $this->getNormalizedValue($this->cmp_cyan, $max),
            'M' => $this->getNormalizedValue($this->cmp_magenta, $max),
            'Y' => $this->getNormalizedValue($this->cmp_yellow, $max),
            'K' => $this->getNormalizedValue($this->cmp_key, $max),
            'A' => $this->cmp_alpha,
        ];
    }

    /**
     * Get the CSS representation of the color: cmyk(C, M, Y, K) or cmyka(C, M, Y, K, A)
     */
    public function getCssColor(): string
    {
        $colorType = 'cmyk';
        $alpha = '';
        if ($this->cmp_alpha < 1.0) {
            $colorType = 'cmyka';
            $alpha = ',' . $this->cmp_alpha;
        }

        return (
            $colorType
            . '('
            . $this->getNormalizedValue($this->cmp_cyan, 100)
            . '%,'
            . $this->getNormalizedValue($this->cmp_magenta, 100)
            . '%,'
            . $this->getNormalizedValue($this->cmp_yellow, 100)
            . '%,'
            . $this->getNormalizedValue($this->cmp_key, 100)
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

        return \sprintf('["CMYK",%F,%F,%F,%F]', $this->cmp_cyan, $this->cmp_magenta, $this->cmp_yellow, $this->cmp_key);
    }

    /**
     * Get a space separated string with color component values.
     */
    public function getComponentsString(): string
    {
        return \sprintf('%F %F %F %F', $this->cmp_cyan, $this->cmp_magenta, $this->cmp_yellow, $this->cmp_key);
    }

    /**
     * Get the color components format used in PDF documents (CMYK)
     * NOTE: the alpha channel is omitted
     *
     * @param bool $stroke True for stroking (lines, drawing) and false for non-stroking (text and area filling).
     */
    public function getPdfColor(bool $stroke = false): string
    {
        $mode = 'k';
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
        return [
            'red' => \max(0, \min(1, 1 - (($this->cmp_cyan * (1 - $this->cmp_key)) + $this->cmp_key))),
            'green' => \max(0, \min(1, 1 - (($this->cmp_magenta * (1 - $this->cmp_key)) + $this->cmp_key))),
            'blue' => \max(0, \min(1, 1 - (($this->cmp_yellow * (1 - $this->cmp_key)) + $this->cmp_key))),
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
        return [
            'cyan' => $this->cmp_cyan,
            'magenta' => $this->cmp_magenta,
            'yellow' => $this->cmp_yellow,
            'key' => $this->cmp_key,
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
        $rgb = new \Com\Tecnick\Color\Model\Rgb($this->toRgbArray());
        return $rgb->toLabArray();
    }

    /**
     * Invert the color
     */
    public function invertColor(): self
    {
        $this->cmp_cyan = 1 - $this->cmp_cyan;
        $this->cmp_magenta = 1 - $this->cmp_magenta;
        $this->cmp_yellow = 1 - $this->cmp_yellow;
        $this->cmp_key = 1 - $this->cmp_key;
        return $this;
    }
}
