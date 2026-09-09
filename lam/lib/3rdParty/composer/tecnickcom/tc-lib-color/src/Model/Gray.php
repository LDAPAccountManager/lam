<?php

declare(strict_types=1);

/**
 * Gray.php
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
 * Com\Tecnick\Color\Model\Gray
 *
 * Gray Color Model class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Gray extends \Com\Tecnick\Color\Model
{
    /**
     * Color Model type
     */
    protected string $type = 'GRAY';

    /**
     * Value of the Gray color component [0..1]
     */
    protected float $cmp_gray = 0.0;

    /**
     * Initialize a new Gray color object.
     *
     * @param array<string, int|float|string> $components Color components, each clamped to [0..1].
     *
     * @throws \Com\Tecnick\Color\UnknownComponentException if a component name is not defined by this model
     */
    public function __construct(array $components)
    {
        self::checkComponents($components, ['gray', 'alpha']);
        $this->cmp_gray = self::component($components, 'gray');
        $this->cmp_alpha = self::component($components, 'alpha', 1.0);
    }

    /**
     * Get an array with all color components.
     *
     * @return array<string, float> with keys ('G', 'A')
     */
    public function getArray(): array
    {
        return [
            'G' => $this->cmp_gray,
            'A' => $this->cmp_alpha,
        ];
    }

    /**
     * Get an array with all color components for
     * the PDF appearance characteristics dictionary.
     *
     * The values are in the range 0.0 to 1.0 and the number of array elements
     * determines the color space:
     * 1 = DeviceGray
     *
     * @return array<float> DeviceGray color component ('G')
     */
    public function getPDFacArray(): array
    {
        return [
            $this->cmp_gray,
        ];
    }

    /**
     * Get an array with color components values normalized between 0 and $max.
     * NOTE: the alpha and other fraction component values are kept in the [0..1] range.
     *
     * @param int $max Maximum value to return (reference value)
     *
     * @return array<string, float> with keys ('G', 'A')
     */
    public function getNormalizedArray(int $max): array
    {
        return [
            'G' => $this->getNormalizedValue($this->cmp_gray, $max),
            'A' => $this->cmp_alpha,
        ];
    }

    /**
     * Get the CSS representation of the color: g(G) or rgba(R, G, B, A).
     *
     * The gray level is emitted as an integer in [0..255].
     */
    public function getCssColor(): string
    {
        $gray = $this->getNormalizedValue($this->cmp_gray, 255);
        if ($this->cmp_alpha < 1.0) {
            return 'rgba(' . $gray . ',' . $gray . ',' . $gray . ',' . $this->getCssValue($this->cmp_alpha, 1) . ')';
        }

        return 'g(' . $gray . ')';
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

        return \sprintf('["G",%F]', $this->cmp_gray);
    }

    /**
     * Get a space separated string with color component values.
     */
    public function getComponentsString(): string
    {
        return \sprintf('%F', $this->cmp_gray);
    }

    /**
     * Get the color components format used in PDF documents (G)
     * NOTE: the alpha channel is omitted
     *
     * @param bool $stroke True for stroking (lines, drawing) and false for non-stroking (text and area filling).
     */
    public function getPdfColor(bool $stroke = false): string
    {
        $mode = 'g';
        if ($stroke) {
            $mode = \strtoupper($mode);
        }

        return $this->getComponentsString() . ' ' . $mode . "\n";
    }

    /**
     * Get an array with Gray color components
     *
     * @return array<string, float> with keys ('gray', 'alpha')
     */
    public function toGrayArray(): array
    {
        return [
            'gray' => $this->cmp_gray,
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
            'red' => $this->cmp_gray,
            'green' => $this->cmp_gray,
            'blue' => $this->cmp_gray,
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
        return [
            'hue' => 0.0,
            'saturation' => 0.0,
            'lightness' => $this->cmp_gray,
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
        $this->cmp_gray = 1 - $this->cmp_gray;
        return $this;
    }
}
