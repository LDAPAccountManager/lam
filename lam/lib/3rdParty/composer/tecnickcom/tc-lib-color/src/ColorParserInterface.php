<?php

declare(strict_types=1);

/**
 * ColorParserInterface.php
 *
 * @since     2026-08-26
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
 * Com\Tecnick\Color\ColorParserInterface
 *
 * Parses a color string into a color model.
 *
 * Implemented by Web.
 *
 * @since     2026-08-26
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
interface ColorParserInterface
{
    /**
     * Parse the input color string and return the corresponding color object.
     *
     * @param string $color String containing web color definition
     *
     * @throws ColorException if the color cannot be parsed
     */
    public function getColorObj(string $color): ?Model;

    /**
     * Parse the input color string, returning null instead of raising.
     *
     * @param string $color String containing web color definition
     */
    public function tryGetColorObj(string $color): ?Model;

    /**
     * Get the color hexadecimal hash code from name.
     *
     * @param string $name Name of the color to search (e.g.: 'turquoise')
     *
     * @return string color hexadecimal code (e.g.: '40e0d0ff')
     *
     * @throws ColorException if the color is not found
     */
    public function getHexFromName(string $name): string;

    /**
     * Get the color name from a hexadecimal hash.
     *
     * @param string $hex hexadecimal color hash (i.e. #RRGGBBAA)
     *
     * @throws ColorException if the color is not found
     */
    public function getNameFromHex(string $hex): string;
}
