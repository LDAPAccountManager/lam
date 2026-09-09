<?php

declare(strict_types=1);

/**
 * ComponentNormalizer.php
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

/**
 * Com\Tecnick\Color\ComponentNormalizer
 *
 * Scales a parsed CSS component value into the [0..1] range used by the color
 * models. Held as a collaborator by Css.
 *
 * @since     2026-08-26
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class ComponentNormalizer
{
    /**
     * Get the normalized value from [0..$max] to [0..1]
     *
     * A percentage string (e.g. '50%') is divided by 100 and ignores $max; the
     * '%' must be the last character. Any other value is divided by $max.
     * Non-numeric values, and any value passed with a non-positive $max, yield
     * 0.0. The result is clamped to [0..1].
     *
     * @param mixed $value Value to convert
     * @param int   $max   Max input value (reference value), must be positive
     *
     * @return float value [0..1]
     */
    public function normalize(mixed $value, int $max): float
    {
        if (\is_string($value) && str_ends_with($value, '%')) {
            $percent = \str_replace('%', '', $value);
            if (!\is_numeric($percent)) {
                return 0.0;
            }

            return \max(0.0, \min(1.0, (float) $percent / 100.0));
        }

        if ($max <= 0 || !\is_numeric($value)) {
            return 0.0;
        }

        return \max(0.0, \min(1.0, (float) $value / $max));
    }
}
