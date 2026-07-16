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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Com\Tecnick\Color;

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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
abstract class Model implements \Com\Tecnick\Color\Model\Template
{
    /**
     * Color Model type (GRAY, RGB, HSL, CMYK, LAB)
     *
     * @var string
     */
    protected $type;

    /**
     * Value of the Alpha channel component.
     * Values range between 0.0 (fully transparent) and 1.0 (fully opaque)
     *
     * @var float
     */
    protected float $cmp_alpha = 1.0;

    /**
     * Initialize a new color object.
     *
     * @param array<string, int|float|string> $components color components.
     */
    public function __construct(array $components)
    {
        foreach ($components as $color => $value) {
            $this->setComponentValue($color, \max(0, \min(1, (float) $value)));
        }
    }

    /**
     * Set a known component value while keeping dynamic property writes analyzable.
     */
    private function setComponentValue(string $color, float $value): void
    {
        switch ($color) {
            case 'alpha':
                $this->cmp_alpha = $value;
                break;
            case 'gray':
                if (\property_exists($this, 'cmp_gray')) {
                    $this->cmp_gray = $value;
                }

                break;
            case 'red':
                if (\property_exists($this, 'cmp_red')) {
                    $this->cmp_red = $value;
                }

                break;
            case 'green':
                if (\property_exists($this, 'cmp_green')) {
                    $this->cmp_green = $value;
                }

                break;
            case 'blue':
                if (\property_exists($this, 'cmp_blue')) {
                    $this->cmp_blue = $value;
                }

                break;
            case 'hue':
                if (\property_exists($this, 'cmp_hue')) {
                    $this->cmp_hue = $value;
                }

                break;
            case 'saturation':
                if (\property_exists($this, 'cmp_saturation')) {
                    $this->cmp_saturation = $value;
                }

                break;
            case 'lightness':
                if (\property_exists($this, 'cmp_lightness')) {
                    $this->cmp_lightness = $value;
                }

                break;
            case 'cyan':
                if (\property_exists($this, 'cmp_cyan')) {
                    $this->cmp_cyan = $value;
                }

                break;
            case 'magenta':
                if (\property_exists($this, 'cmp_magenta')) {
                    $this->cmp_magenta = $value;
                }

                break;
            case 'yellow':
                if (\property_exists($this, 'cmp_yellow')) {
                    $this->cmp_yellow = $value;
                }

                break;
            case 'key':
                if (\property_exists($this, 'cmp_key')) {
                    $this->cmp_key = $value;
                }

                break;
            case 'lstar':
                if (\property_exists($this, 'cmp_lstar')) {
                    $this->cmp_lstar = $value;
                }

                break;
            case 'astar':
                if (\property_exists($this, 'cmp_astar')) {
                    $this->cmp_astar = $value;
                }

                break;
            case 'bstar':
                if (\property_exists($this, 'cmp_bstar')) {
                    $this->cmp_bstar = $value;
                }

                break;
        }
    }

    /**
     * Get the color model type (GRAY, RGB, HSL, CMYK, LAB)
     */
    public function getType(): string
    {
        return $this->type;
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
        // NOTE: The last round has been added for backward compatibility because of:
        // https://github.com/php/php-src/issues/14332
        return \round(\max(0, \min($max, $max * \round($value, 14))));
    }

    /**
     * Get the normalized hexadecimal value of the specified float fraction
     *
     * @param float $value Fraction value to convert [0..1]
     * @param int   $max   Maximum value to return (reference value)
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
