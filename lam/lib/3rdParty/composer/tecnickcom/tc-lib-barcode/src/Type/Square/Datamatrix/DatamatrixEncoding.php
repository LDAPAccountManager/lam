<?php

declare(strict_types=1);

/**
 * DatamatrixEncoding.php
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

namespace Com\Tecnick\Barcode\Type\Square\Datamatrix;

/**
 * Com\Tecnick\Barcode\Type\Square\Datamatrix\DatamatrixEncoding
 *
 * Backed enum for the Data Matrix default encoding scheme. The backing value of
 * each case is a public key of Data::ENCOPTS.
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum DatamatrixEncoding: string
{
    case ASCII = 'ASCII';

    case C40 = 'C40';

    case TXT = 'TXT';

    case X12 = 'X12';

    case EDF = 'EDF';

    case BASE256 = 'BASE256';

    /**
     * Alternative spellings accepted by fromLoose().
     *
     * @var array<string, string>
     */
    private const ALIASES = ['EDIFACT' => 'EDF'];

    /**
     * Resolve a loose Data Matrix encoding value to the matching enum case.
     *
     * Accepts the canonical scheme name, one of the ALIASES, or an enum instance
     * (returned unchanged). Unknown values fall back to ASCII, matching the
     * lenient behavior of Datamatrix.
     *
     * @param string|self $value Encoding scheme name or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom(self::ALIASES[$value] ?? $value) ?? self::ASCII;
    }
}
