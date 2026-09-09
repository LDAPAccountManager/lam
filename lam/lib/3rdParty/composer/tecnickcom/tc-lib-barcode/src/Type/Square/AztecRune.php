<?php

declare(strict_types=1);

/**
 * AztecRune.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\Square\Aztec\Rune;

/**
 * Com\Tecnick\Barcode\Type\Square\AztecRune
 *
 * AztecRune Barcode type class
 * Aztec Rune (ISO/IEC 24778:2008 Annex A)
 *
 * Fixed size symbol carrying a single value from 0 to 255.
 *     Symbol size:     11x11 modules
 *     Input:           the decimal value, from 0 to 255
 *     Transmitted as:  three decimal digits with leading zeros
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class AztecRune extends \Com\Tecnick\Barcode\Type\Square
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'AZTECRUNE';

    /**
     * Get the bars array
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (\preg_match('/^[0-9]{1,3}\z/', $this->code) !== 1 || (int) $this->code > 255) {
            throw new BarcodeException('AZTECRUNE: the code must be a number between 0 and 255');
        }

        // the value is transmitted as three decimal digits with leading zeros
        $this->extcode = \str_pad(\strval((int) $this->code), 3, '0', STR_PAD_LEFT);

        try {
            $rune = new Rune((int) $this->code);
            $this->processBinarySequence($rune->getGrid());
        } catch (BarcodeException $barcodeException) {
            throw new BarcodeException('AZTECRUNE: ' . $barcodeException->getMessage());
        }
    }
}
