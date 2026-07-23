<?php

declare(strict_types=1);

/**
 * Template.php
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
 * Com\Tecnick\Color\Model\Template
 *
 * Color Model Interface
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
interface Template
{
    /**
     * Get an array with all color components.
     *
     * @return array<string, float>
     */
    public function getArray(): array;

    /**
     * Get an array with all color components for
     * the PDF appearance characteristics dictionary.
     *
     * The numbers that shall be in the range 0.0 to 1.0.
     * The number of array elements determines the colour space
     * in which the colour shall be defined:
     *   0 = No colour; transparent
     *   1 = DeviceGray
     *   3 = DeviceRGB
     *   4 = DeviceCMYK
     *
     * @return array<float>
     */
    public function getPDFacArray(): array;

    /**
     * Get an array with color components values normalized between 0 and $max.
     * NOTE: the alpha and other fraction component values are kept in the [0..1] range.
     *
     * @param int $max Maximum value to return (reference value)
     *
     * @return array<string, float>
     */
    public function getNormalizedArray(int $max): array;

    /**
     * Get the CSS representation of the color
     */
    public function getCssColor(): string;

    /**
     * Get the color format used in Acrobat JavaScript.
     * NOTE: the alpha channel is omitted from this representation unless is 0 = transparent
     */
    public function getJsPdfColor(): string;

    /**
     * Get a space separated string with color component values.
     */
    public function getComponentsString(): string;

    /**
     * Get the color components format used in PDF documents (RGB).
     * NOTE: the alpha channel is omitted.
     *
     * @param bool $stroke True for stroking (lines, drawing) and false for non-stroking (text and area filling).
     */
    public function getPdfColor(bool $stroke = false): string;

    /**
     * Get an array with Gray color components.
     *
     * @return array<string, float> with keys ('gray').
     */
    public function toGrayArray(): array;

    /**
     * Get an array with RGB color components.
     *
     * @return array<string, float> with keys ('red', 'green', 'blue', 'alpha').
     */
    public function toRgbArray(): array;

    /**
     * Get an array with HSL color components.
     *
     * @return array<string, float> with keys ('hue', 'saturation', 'lightness', 'alpha').
     */
    public function toHslArray(): array;

    /**
     * Get an array with CMYK color components.
     *
     * @return array<string, float> with keys ('cyan', 'magenta', 'yellow', 'key', 'alpha').
     */
    public function toCmykArray(): array;

    /**
     * Get an array with Lab color components.
     *
     * @return array<string, float> with keys ('lstar', 'astar', 'bstar', 'alpha').
     */
    public function toLabArray(): array;

    /**
     * Invert the color.
     */
    public function invertColor(): self;
}
