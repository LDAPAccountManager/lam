<?php

declare(strict_types=1);

/**
 * BidiClass.php
 *
 * @since       2026-07-17
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 *
 * This file is part of tc-lib-unicode-data software library.
 */

namespace Com\Tecnick\Unicode\Data;

/**
 * Com\Tecnick\Unicode\Data\BidiClass
 *
 * Backed enum for the strong, weak and neutral bidirectional character classes.
 * The backing value of each case is the canonical class abbreviation used as the
 * keys of Type::STRONG, Type::WEAK and Type::NEUTRAL. The explicit formatting
 * codes (LRE, LRO, RLE, RLO, PDF, ...) are intentionally not part of this set.
 *
 * @since       2026-07-17
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 */
enum BidiClass: string
{
    // Strong types (Type::STRONG).
    case L = 'L';

    case R = 'R';

    case AL = 'AL';

    // Weak types (Type::WEAK).
    case EN = 'EN';

    case ES = 'ES';

    case ET = 'ET';

    case AN = 'AN';

    case CS = 'CS';

    case NSM = 'NSM';

    case BN = 'BN';

    // Neutral types (Type::NEUTRAL).
    case B = 'B';

    case S = 'S';

    case WS = 'WS';

    case ON = 'ON';

    /**
     * Resolve a loose bidirectional class value to the matching enum case.
     *
     * Accepts the canonical class abbreviation or an enum instance (returned
     * unchanged). Unknown values throw a \ValueError, matching the closed set
     * defined by Type::STRONG, Type::WEAK and Type::NEUTRAL.
     *
     * @param string|self $value Bidirectional class abbreviation or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::from($value);
    }

    /**
     * True for a strong type (L, R, AL).
     */
    public function isStrong(): bool
    {
        return \array_key_exists($this->value, Type::STRONG);
    }

    /**
     * True for a weak type (EN, ES, ET, AN, CS, NSM, BN).
     */
    public function isWeak(): bool
    {
        return \array_key_exists($this->value, Type::WEAK);
    }

    /**
     * True for a neutral type (B, S, WS, ON).
     */
    public function isNeutral(): bool
    {
        return \array_key_exists($this->value, Type::NEUTRAL);
    }
}
