<?php

declare(strict_types=1);

/**
 * TextDirection.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Com\Tecnick\Unicode;

/**
 * Com\Tecnick\Unicode\TextDirection
 *
 * Backed enum for the forced paragraph direction accepted by Bidi. The backing
 * value matches the normalized value stored internally: '' (auto detection),
 * 'R' (force right-to-left) or 'L' (force left-to-right).
 *
 * @since     2026-07-17
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
enum TextDirection: string
{
    case Auto = '';

    case Rtl = 'R';

    case Ltr = 'L';

    /**
     * Resolve a loose forced-direction value to the matching enum case.
     *
     * Accepts an enum instance (returned unchanged) or a string. The string is
     * interpreted exactly as Bidi always has: the empty string means auto, and
     * otherwise the first character (case-insensitive) selects the direction;
     * anything that is not R or L falls back to auto (never throws).
     *
     * @param string|self $value Forced direction identifier or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        if ($value === '') {
            return self::Auto;
        }

        return match (\strtoupper($value[0])) {
            'R' => self::Rtl,
            'L' => self::Ltr,
            default => self::Auto,
        };
    }
}
