<?php

declare(strict_types=1);

/**
 * AztecRange.php
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
 * Com\Tecnick\Barcode\Type\Square\Aztec\AztecRange
 *
 * Backed enum for the Aztec Code symbol range mode: A (automatic selection
 * between Compact and Full Range, default) or F (force Full Range).
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum AztecRange: string
{
    /** Automatic selection between Compact (priority) and Full Range (default). */
    case Automatic = 'A';

    /** Force Full Range mode. */
    case FullRange = 'F';

    /**
     * Resolve a loose Aztec range mode value to the matching enum case.
     *
     * Accepts the canonical letter or an enum instance (returned unchanged).
     * Unknown values fall back to Automatic, matching the lenient behavior of
     * Aztec.
     *
     * @param string|self $value Range mode letter or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? self::Automatic;
    }
}
