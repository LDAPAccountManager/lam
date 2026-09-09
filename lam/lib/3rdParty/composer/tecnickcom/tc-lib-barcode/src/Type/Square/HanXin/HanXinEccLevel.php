<?php

declare(strict_types=1);

/**
 * HanXinEccLevel.php
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\HanXin;

/**
 * Com\Tecnick\Barcode\Type\Square\HanXin\HanXinEccLevel
 *
 * Backed enum for the Han Xin Code error correction level.
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum HanXinEccLevel: string
{
    /** Recovers 8% of the codewords. */
    case L1 = 'L1';

    /** Recovers 15% of the codewords. */
    case L2 = 'L2';

    /** Recovers 23% of the codewords. */
    case L3 = 'L3';

    /** Recovers 30% of the codewords. */
    case L4 = 'L4';

    /**
     * Resolve a loose error correction level value to the matching enum case.
     *
     * Accepts the canonical token, the level number alone or an enum instance
     * (returned unchanged). Unknown values fall back to L1.
     *
     * @param string|self $value Error correction level token or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::tryFrom('L' . $value) ?? self::L1;
    }

    /**
     * Returns the level number, from 1 to 4.
     */
    public function getLevel(): int
    {
        return (int) \substr($this->value, 1);
    }
}
