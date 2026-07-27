<?php

declare(strict_types=1);

/**
 * ColorModelType.php
 *
 * @since     2026-07-17
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
 * Com\Tecnick\Color\ColorModelType
 *
 * Backed enum for the color model type.
 * The backing value of each case is the canonical string returned by
 * Com\Tecnick\Color\Model::getType() (GRAY, RGB, HSL, CMYK, LAB).
 *
 * @since     2026-07-17
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
enum ColorModelType: string
{
    case Gray = 'GRAY';

    case Rgb = 'RGB';

    case Hsl = 'HSL';

    case Cmyk = 'CMYK';

    case Lab = 'LAB';

    /**
     * Resolve a loose color model type value to the matching enum case.
     *
     * Accepts the canonical string returned by Model::getType() (case-insensitive)
     * or an enum instance (returned unchanged). Unknown values throw, matching the
     * closed set enforced by the Model subclasses.
     *
     * @param string|self $value Canonical color model type or enum case.
     *
     * @throws ColorException if the value does not match a known color model type.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return match (\strtoupper($value)) {
            'GRAY' => self::Gray,
            'RGB' => self::Rgb,
            'HSL' => self::Hsl,
            'CMYK' => self::Cmyk,
            'LAB' => self::Lab,
            default => throw new ColorException('unknown color model type: ' . $value),
        };
    }
}
