<?php

declare(strict_types=1);

/**
 * AztecHint.php
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Aztec;

/**
 * Com\Tecnick\Barcode\Type\Square\Aztec\AztecHint
 *
 * Backed enum for the Aztec Code encoding hint: A (automatic, default) or B
 * (binary).
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum AztecHint: string
{
    /** Automatic encoding (default). */
    case Automatic = 'A';

    /** Binary encoding. */
    case Binary = 'B';

    /**
     * Resolve a loose Aztec hint value to the matching enum case.
     *
     * Accepts the canonical letter or an enum instance (returned unchanged).
     * Unknown values fall back to Automatic, matching the lenient behavior of
     * Aztec.
     *
     * @param string|self $value Hint letter or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::Automatic;
    }
}
