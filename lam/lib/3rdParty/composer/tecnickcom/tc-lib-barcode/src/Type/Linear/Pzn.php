<?php

declare(strict_types=1);

/**
 * Pzn.php
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
 * Com\Tecnick\Barcode\Type\Linear\Pzn;
 *
 * Pzn Barcode type class
 * PZN (Pharmazentralnummer, IFA coding system)
 *
 * CODE 39 symbol encoding the minus sign followed by the eight digits of the
 * German pharmaceutical product number, of which the last one is a modulo 11
 * check digit. The minus sign is the data identifier of the PZN and there is no
 * CODE 39 check character. The human readable interpretation is "PZN - " plus
 * the eight digits; those characters are not part of the symbol.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Pzn extends \Com\Tecnick\Barcode\Type\Linear\CodeThreeNine
{
    use FixedLengthCheckDigit;

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'PZN';

    /**
     * Fixed code length, check digit included
     */
    protected const CODE_LENGTH = 8;

    /**
     * Data identifier of the PZN
     *
     * @var string
     */
    protected const IDENTIFIER = '-';

    /**
     * Check that the input code is a digit string that fits the fixed length.
     * Shorter codes are left-padded with zeros by formatCode().
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
     * Calculate the modulo 11 check digit of the data digits.
     * Each digit is weighted with its one-based position and the remainder of
     * the division of the sum by 11 is the check digit. A remainder of 10 marks
     * a digit sequence that is never allocated as a PZN.
     *
     * @param string $code Data digits without the check digit.
     *
     * @throws BarcodeException if the digit sequence has no check digit
     */
    protected function getCheckDigit(string $code): int
    {
        $sum = 0;
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $sum += (int) $code[$pos] * ($pos + 1);
        }

        $check = $sum % 11;
        if ($check === 10) {
            throw new BarcodeException('The digit sequence is not a valid PZN: ' . $code);
        }

        return $check;
    }

    /**
     * Format code.
     * A code of the full length must carry a valid check digit, a shorter one is
     * left-padded with zeros to the data length and the check digit is appended.
     *
     * @throws BarcodeException if the supplied check digit is wrong
     */
    protected function formatCode(): void
    {
        $this->validateCode();

        $code = $this->getCheckedCode($this->code, $this::CODE_LENGTH);
        $this->extcode = '*' . $this::IDENTIFIER . $code . '*';
    }
}
