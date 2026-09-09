<?php

declare(strict_types=1);

/**
 * HibcPayload.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\HibcPayload
 *
 * HibcPayload trait
 *
 * Wires the HIBC data structure of Hibc to a carrier symbology. The carriers
 * belong to different type hierarchies, so the wiring is shared here.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
trait HibcPayload
{
    /**
     * Data structure handed to the carrier symbology
     */
    protected string $payload = '';

    /**
     * Get the character sequence to encode.
     */
    protected function getEncodedPayload(): string
    {
        return $this->payload;
    }

    /**
     * Store the data structure to encode and return the human readable
     * interpretation. The asterisk is never encoded: it only bounds the human
     * readable interpretation, where it doubles as the CODE 39 start and stop
     * character.
     *
     * @param string $code Data structure without the check character.
     *
     * @throws BarcodeException if the data structure is malformed
     */
    protected function getHibcExtendedCode(string $code): string
    {
        $this->payload = (new Hibc())->format($code);
        return '*' . $this->payload . '*';
    }
}
