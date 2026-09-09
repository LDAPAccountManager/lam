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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Rgb extends \Com\Tecnick\Color\Model
{
    /**
     * Color Model type
     */
    protected string $type = 'RGB';

    /**
     * Value of the Red color component [0..1]
     */
    protected float $cmp_red = 0.0;

    /**
     * Value of the Green color component [0..1]
     */
    protected float $cmp_green = 0.0;

    /**
     * Value of the Blue color component [0..1]
     */
    protected float $cmp_blue = 0.0;

    /**
     * Initialize a new RGB color object.
     *
     * @param array<string, int|float|string> $components Color components, each clamped to [0..1].
     *
     * @throws \Com\Tecnick\Color\UnknownComponentException if a component name is not defined by this model
     */
    public function __construct(array $components)
    {
        self::checkComponents($components, ['red', 'green', 'blue', 'alpha']);
        $this->cmp_red = self::component($components, 'red');
        $this->cmp_green = self::component($components, 'green');
        $this->cmp_blue = self::component($components, 'blue');
        $this->cmp_alpha = self::component($components, 'alpha', 1.0);
    }

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
     * The values are in the range 0.0 to 1.0 and the number of array elements
     * determines the color space:
     * 3 = DeviceRGB
     *
     * @return array<float> DeviceRGB color components ('R', 'G', 'B')
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
     *
     * Channels are emitted as integers in [0..255].
     */
    public function getCssColor(): string
    {
        $colorType = 'rgb';
        $alpha = '';
        if ($this->cmp_alpha < 1.0) {
            $colorType = 'rgba';
            $alpha = ',' . $this->getCssValue($this->cmp_alpha, 1);
        }

        return (
            $colorType
            . '('
            . $this->getNormalizedValue($this->cmp_red, 255)
            . ','
            . $this->getNormalizedValue($this->cmp_green, 255)
            . ','
            . $this->getNormalizedValue($this->cmp_blue, 255)
            . $alpha
            . ')'
        );
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

        return \sprintf('["RGB",%F,%F,%F]', $this->cmp_red, $this->cmp_green, $this->cmp_blue);
    }

    /**
     * Get a space separated string with color component values.
     */
    public function getComponentsString(): string
    {
        return self::rgbComponentsString($this->toRgbArray());
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
        return self::rgbToLab($this->toRgbArray());
    }

    /**
     * Invert the color
     */
    public function invertColor(): static
    {
        $this->cmp_red = 1 - $this->cmp_red;
        $this->cmp_green = 1 - $this->cmp_green;
        $this->cmp_blue = 1 - $this->cmp_blue;
        return $this;
    }
}
