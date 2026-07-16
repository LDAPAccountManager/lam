<?php

declare(strict_types=1);

/**
 * Web.php
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

use Com\Tecnick\Color\Exception as ColorException;

/**
 * Com\Tecnick\Color\Web
 *
 * Web Color class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Web extends \Com\Tecnick\Color\Css
{
    /**
     * Maps WEB safe color names with theur hexadecimal representation (#RRGGBBAA).
     *
     * @var array<string, string>
     */
    public const WEBHEX = [
        'aliceblue' => 'f0f8ffff',
        'antiquewhite' => 'faebd7ff',
        'aqua' => '00ffffff',
        'aquamarine' => '7fffd4ff',
        'azure' => 'f0ffffff',
        'beige' => 'f5f5dcff',
        'bisque' => 'ffe4c4ff',
        'black' => '000000ff',
        'blanchedalmond' => 'ffebcdff',
        'blue' => '0000ffff',
        'blueviolet' => '8a2be2ff',
        'brown' => 'a52a2aff',
        'burlywood' => 'deb887ff',
        'cadetblue' => '5f9ea0ff',
        'chartreuse' => '7fff00ff',
        'chocolate' => 'd2691eff',
        'coral' => 'ff7f50ff',
        'cornflowerblue' => '6495edff',
        'cornsilk' => 'fff8dcff',
        'crimson' => 'dc143cff',
        'cyan' => '00ffffff',
        'darkblue' => '00008bff',
        'darkcyan' => '008b8bff',
        'darkgoldenrod' => 'b8860bff',
        'dkgray' => 'a9a9a9ff',
        'darkgray' => 'a9a9a9ff',
        'darkgrey' => 'a9a9a9ff',
        'darkgreen' => '006400ff',
        'darkkhaki' => 'bdb76bff',
        'darkmagenta' => '8b008bff',
        'darkolivegreen' => '556b2fff',
        'darkorange' => 'ff8c00ff',
        'darkorchid' => '9932ccff',
        'darkred' => '8b0000ff',
        'darksalmon' => 'e9967aff',
        'darkseagreen' => '8fbc8fff',
        'darkslateblue' => '483d8bff',
        'darkslategray' => '2f4f4fff',
        'darkslategrey' => '2f4f4fff',
        'darkturquoise' => '00ced1ff',
        'darkviolet' => '9400d3ff',
        'deeppink' => 'ff1493ff',
        'deepskyblue' => '00bfffff',
        'dimgray' => '696969ff',
        'dimgrey' => '696969ff',
        'dodgerblue' => '1e90ffff',
        'firebrick' => 'b22222ff',
        'floralwhite' => 'fffaf0ff',
        'forestgreen' => '228b22ff',
        'fuchsia' => 'ff00ffff',
        'gainsboro' => 'dcdcdcff',
        'ghostwhite' => 'f8f8ffff',
        'gold' => 'ffd700ff',
        'goldenrod' => 'daa520ff',
        'gray' => '808080ff',
        'grey' => '808080ff',
        'green' => '008000ff',
        'greenyellow' => 'adff2fff',
        'honeydew' => 'f0fff0ff',
        'hotpink' => 'ff69b4ff',
        'indianred' => 'cd5c5cff',
        'indigo' => '4b0082ff',
        'ivory' => 'fffff0ff',
        'khaki' => 'f0e68cff',
        'lavender' => 'e6e6faff',
        'lavenderblush' => 'fff0f5ff',
        'lawngreen' => '7cfc00ff',
        'lemonchiffon' => 'fffacdff',
        'lightblue' => 'add8e6ff',
        'lightcoral' => 'f08080ff',
        'lightcyan' => 'e0ffffff',
        'lightgoldenrodyellow' => 'fafad2ff',
        'ltgray' => 'd3d3d3ff',
        'lightgray' => 'd3d3d3ff',
        'lightgrey' => 'd3d3d3ff',
        'lightgreen' => '90ee90ff',
        'lightpink' => 'ffb6c1ff',
        'lightsalmon' => 'ffa07aff',
        'lightseagreen' => '20b2aaff',
        'lightskyblue' => '87cefaff',
        'lightslategray' => '778899ff',
        'lightslategrey' => '778899ff',
        'lightsteelblue' => 'b0c4deff',
        'lightyellow' => 'ffffe0ff',
        'lime' => '00ff00ff',
        'limegreen' => '32cd32ff',
        'linen' => 'faf0e6ff',
        'magenta' => 'ff00ffff',
        'maroon' => '800000ff',
        'mediumaquamarine' => '66cdaaff',
        'mediumblue' => '0000cdff',
        'mediumorchid' => 'ba55d3ff',
        'mediumpurple' => '9370d8ff',
        'mediumseagreen' => '3cb371ff',
        'mediumslateblue' => '7b68eeff',
        'mediumspringgreen' => '00fa9aff',
        'mediumturquoise' => '48d1ccff',
        'mediumvioletred' => 'c71585ff',
        'midnightblue' => '191970ff',
        'mintcream' => 'f5fffaff',
        'mistyrose' => 'ffe4e1ff',
        'moccasin' => 'ffe4b5ff',
        'navajowhite' => 'ffdeadff',
        'navy' => '000080ff',
        'oldlace' => 'fdf5e6ff',
        'olive' => '808000ff',
        'olivedrab' => '6b8e23ff',
        'orange' => 'ffa500ff',
        'orangered' => 'ff4500ff',
        'orchid' => 'da70d6ff',
        'palegoldenrod' => 'eee8aaff',
        'palegreen' => '98fb98ff',
        'paleturquoise' => 'afeeeeff',
        'palevioletred' => 'd87093ff',
        'papayawhip' => 'ffefd5ff',
        'peachpuff' => 'ffdab9ff',
        'peru' => 'cd853fff',
        'pink' => 'ffc0cbff',
        'plum' => 'dda0ddff',
        'powderblue' => 'b0e0e6ff',
        'purple' => '800080ff',
        'red' => 'ff0000ff',
        'rosybrown' => 'bc8f8fff',
        'royalblue' => '4169e1ff',
        'saddlebrown' => '8b4513ff',
        'salmon' => 'fa8072ff',
        'sandybrown' => 'f4a460ff',
        'seagreen' => '2e8b57ff',
        'seashell' => 'fff5eeff',
        'sienna' => 'a0522dff',
        'silver' => 'c0c0c0ff',
        'skyblue' => '87ceebff',
        'slateblue' => '6a5acdff',
        'slategray' => '708090ff',
        'slategrey' => '708090ff',
        'snow' => 'fffafaff',
        'springgreen' => '00ff7fff',
        'steelblue' => '4682b4ff',
        'tan' => 'd2b48cff',
        'teal' => '008080ff',
        'thistle' => 'd8bfd8ff',
        'tomato' => 'ff6347ff',
        'turquoise' => '40e0d0ff',
        'violet' => 'ee82eeff',
        'wheat' => 'f5deb3ff',
        'white' => 'ffffffff',
        'whitesmoke' => 'f5f5f5ff',
        'yellow' => 'ffff00ff',
        'yellowgreen' => '9acd32ff',
    ];

    /**
     * Get the color hexadecimal hash code from name
     *
     * @param string $name Name of the color to search (e.g.: 'turquoise')
     *
     * @return string color hexadecimal code (e.g.: '40e0d0ff')
     *
     * @throws ColorException if the color is not found
     */
    public function getHexFromName(string $name): string
    {
        $name = \strtolower($name);
        if (($dotpos = \strpos($name, '.')) !== false) {
            // remove parent name (i.e.: color.green)
            $name = \substr($name, $dotpos + 1);
        }

        $hex = self::WEBHEX[$name] ?? null;
        if (!\is_string($hex)) {
            throw new ColorException('unable to find the color hex for the name: ' . $name);
        }

        return $hex;
    }

    /**
     * Get the color name code from hexadecimal hash
     *
     * @param string $hex hexadecimal color hash (i.e. #RRGGBBAA)
     *
     * @return string color name
     *
     * @throws ColorException if the color is not found
     */
    public function getNameFromHex(string $hex): string
    {
        $name = \array_search($this->extractHexCode($hex), self::WEBHEX, true);
        if (!\is_string($name)) {
            throw new ColorException('unable to find the color name for the hex code: ' . $hex);
        }

        return $name;
    }

    /**
     * Extract the hexadecimal code from the input string and add the alpha channel if missing
     *
     * @param string $hex string containing the hexadecimal color hash (i.e. #RGB, #RGBA, #RRGGBB, #RRGGBBAA)
     *
     * @return string the hash code (e.g.: '40e0d0')
     *
     * @throws ColorException if the hash is not found or has an invalid format
     */
    public function extractHexCode(string $hex): string
    {
        $match = [];
        if (\preg_match('/^[#]?([0-9a-f]{3,8})$/', \strtolower($hex), $match) !== 1) {
            throw new ColorException('unable to extract the color hash: ' . $hex);
        }

        $hash = $match[1] ?? '';
        if (\strlen($hash) === 3 || \strlen($hash) === 4) {
            $hash = $this->expandHexHash($hash);
        }

        if (\strlen($hash) === 6) {
            return $hash . 'ff';
        }

        if (\strlen($hash) === 8) {
            return $hash;
        }

        throw new ColorException('unable to extract the color hash: ' . $hash);
    }

    /**
     * Expand shorthand hex digits (RGB/RGBA) to full-length form.
     */
    private function expandHexHash(string $hash): string
    {
        $expanded = '';
        foreach (\str_split($hash) as $digit) {
            $expanded .= $digit . $digit;
        }

        return $expanded;
    }

    /**
     * Get the RGB color object from hexadecimal hash
     *
     * @param string $hex hexadecimal color hash (i.e. #RGB, #RGBA, #RRGGBB, #RRGGBBAA)
     *
     * @throws ColorException if the color is not found
     */
    public function getRgbObjFromHex(string $hex): \Com\Tecnick\Color\Model\Rgb
    {
        return new \Com\Tecnick\Color\Model\Rgb($this->getHexArray($this->extractHexCode($hex)));
    }

    /**
     * Get the RGB color object from color name
     *
     * @param string $name Color name
     *
     * @return \Com\Tecnick\Color\Model\Rgb object
     *
     * @throws ColorException if the color is not found
     */
    public function getRgbObjFromName(string $name): \Com\Tecnick\Color\Model\Rgb
    {
        return new \Com\Tecnick\Color\Model\Rgb($this->getHexArray($this->getHexFromName($name)));
    }

    /**
     * Get the RGB array from hexadecimal hash
     *
     * @param string $hex hexadecimal color hash (i.e. RRGGBBAA)
     *
     * @return array<string, float> with keys ('red', 'green', 'blue', 'alpha')
     */
    private function getHexArray(string $hex): array
    {
        return [
            'red' => \hexdec(\substr($hex, 0, 2)) / 255,
            'green' => \hexdec(\substr($hex, 2, 2)) / 255,
            'blue' => \hexdec(\substr($hex, 4, 2)) / 255,
            'alpha' => \hexdec(\substr($hex, 6, 2)) / 255,
        ];
    }

    /**
     * Get the normalized integer value from [0..$max] to [0..1]
     *
     * @param mixed $value Value to convert
     * @param int    $max   Max input value
     *
     * @return float value [0..1]
     */
    public function normalizeValue(mixed $value, int $max): float
    {
        if (\is_string($value) && str_contains($value, '%')) {
            $percent = \str_replace('%', '', $value);
            if (!\is_numeric($percent)) {
                return 0.0;
            }

            return \max(0, \min(1, (float) $percent / 100));
        }

        if (\is_int($value) || \is_float($value)) {
            return \max(0, \min(1, (float) $value / $max));
        }

        if (\is_string($value) && \is_numeric($value)) {
            return \max(0, \min(1, (float) $value / $max));
        }

        return 0.0;
    }

    /**
     * Parse the input color string and return the correspondent color Object
     *
     * @param string $color String containing web color definition
     *
     * @throws ColorException in case of error
     */
    public function getColorObj(string $color): ?\Com\Tecnick\Color\Model
    {
        $col = [];
        $color = \strtolower(\trim($color));
        if ($color === '' || str_contains($color, 'transparent')) {
            return null;
        }

        if ($color[0] === '#') {
            return $this->getRgbObjFromHex($color);
        }

        if ($color[0] === '[') {
            return $this->getColorObjFromJs($color);
        }

        $rex = '/^(t|g|rgba|rgb|hsla|hsl|cmyka|cmyk|lab)\s*[\(]/';
        if (\preg_match($rex, $color, $col) === 1) {
            return $this->getColorObjFromCss($col[1] ?? '', $color);
        }

        return $this->getRgbObjFromName($color);
    }

    /**
     * Parse the input color string and return the corresponding color object.
     *
     * @param string $color String containing web color definition
     */
    public function tryGetColorObj(string $color): ?\Com\Tecnick\Color\Model
    {
        try {
            return $this->getColorObj($color);
        } catch (ColorException $colorException) {
            unset($colorException);
        }

        return null;
    }

    /**
     * Get the square of the distance between 2 RGB points
     *
     * @param array<string, float> $cola First color as RGB array
     * @param array<string, float> $colb Second color as RGB array
     */
    public function getRgbSquareDistance(array $cola, array $colb): float
    {
        return (
            (($cola['red'] ?? 0.0) - ($colb['red'] ?? 0.0)) ** 2
            + (($cola['green'] ?? 0.0) - ($colb['green'] ?? 0.0)) ** 2
            + (($cola['blue'] ?? 0.0) - ($colb['blue'] ?? 0.0)) ** 2
        );
    }

    /**
     * Get the name of the closest web color
     *
     * @param array<string, float> $col Color as RGB array (keys: 'red', 'green', 'blue')
     */
    public function getClosestWebColor(array $col): string
    {
        $color = '';
        // Seed above the maximum possible squared distance in the RGB unit cube
        // (1^2 + 1^2 + 1^2 = 3) and use a strict "<" so the nearest match is
        // always returned and ties resolve to the first (canonical) color name.
        $mindist = \PHP_FLOAT_MAX;
        foreach (self::WEBHEX as $name => $hex) {
            $dist = $this->getRgbSquareDistance($col, $this->getHexArray($hex));
            if ($dist < $mindist) {
                $mindist = $dist;
                $color = $name;
            }
        }

        return $color;
    }

    /**
     * Get the name of the closest web color from a color definition string.
     *
     * @param string $color String containing web color definition
     */
    public function getClosestWebColorFromString(string $color): string
    {
        $obj = $this->tryGetColorObj($color);
        if (!$obj instanceof \Com\Tecnick\Color\Model) {
            return '';
        }

        return $this->getClosestWebColor($obj->toRgbArray());
    }
}
