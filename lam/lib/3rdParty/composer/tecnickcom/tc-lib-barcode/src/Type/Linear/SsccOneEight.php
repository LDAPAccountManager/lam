<?php

declare(strict_types=1);

/**
 * SsccOneEight.php
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

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\SsccOneEight;
 *
 * SsccOneEight Barcode type class
 * SSCC-18 (GS1 General Specifications)
 *
 * GS1-128 symbol carrying the single element string of the Serial Shipping
 * Container Code, Application Identifier (00). The input is the plain digit
 * string: a code of the full length must carry a valid check digit, a shorter
 * one is left-padded with zeros and the check digit is appended.
 *
 * GS1 and GS1-128 are registered trademarks of GS1 AISBL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class SsccOneEight extends \Com\Tecnick\Barcode\Type\Linear\GsOneOneTwoEight
{
    use FixedLengthCheckDigit;

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'SSCC18';

    /**
     * Application Identifier of the element string
     *
     * @var string
     */
    protected const APPID = '00';

    /**
     * Fixed code length, check digit included
     */
    protected const CODE_LENGTH = 18;

    /**
     * Check that the input code is a digit string that fits the fixed length.
     * Shorter codes are left-padded with zeros.
     *
     * @throws BarcodeException if the code is not numeric or is too long
     */
    protected function validateCode(): void
    {
        if (!\ctype_digit($this->code)) {
            throw new BarcodeException('Input code must be a number');
        }

        if (\strlen($this->code) > $this::CODE_LENGTH) {
            throw new BarcodeException(
                'The code is too long: '
                . \strlen($this->code)
                . ' digits (maximum '
                . $this::CODE_LENGTH
                . ' for '
                . $this::FORMAT
                . ')',
            );
        }
    }

    /**
     * Build the bracketed element string of the fixed length key.
     *
     * @throws BarcodeException if the supplied check digit is wrong
     */
    protected function getBracketedCode(): string
    {
        $this->validateCode();

        return '(' . $this::APPID . ')' . $this->getCheckedCode($this->code, $this::CODE_LENGTH);
    }
}
