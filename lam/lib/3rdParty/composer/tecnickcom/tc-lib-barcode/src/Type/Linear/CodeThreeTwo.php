<?php

declare(strict_types=1);

/**
 * CodeThreeTwo.php
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
 * Com\Tecnick\Barcode\Type\Linear\CodeThreeTwo;
 *
 * CodeThreeTwo Barcode type class
 * CODE 32 (Italian Pharmacode - IMH - Radix 32)
 *
 * Encodes the 9-digit Italian pharmaceutical product code (8 data digits and a
 * modulo 10 check digit) as a 6-character base 32 number drawn with the CODE 39
 * character set and no CODE 39 check character. The base 32 alphabet is made of
 * the ten digits and the consonants, which is what keeps the 9-digit value
 * within 6 characters. The human readable interpretation is the letter A
 * followed by the 9 digits; the letter is not part of the symbol.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class CodeThreeTwo extends \Com\Tecnick\Barcode\Type\Linear\CodeThreeNine
{
    use FixedLengthCheckDigit;

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'C32';

    /**
     * Fixed code length, check digit included
     */
    protected const CODE_LENGTH = 9;

    /**
     * Number of base 32 characters of the encoded value
     */
    protected const ENCODED_LENGTH = 6;

    /**
     * Base 32 alphabet: the ten digits followed by the consonants
     *
     * @var string
     */
    protected const ALPHABET = '0123456789BCDFGHJKLMNPQRSTUVWXYZ';

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
     * Calculate the modulo 10 check digit of the data digits.
     * Digits in odd positions are doubled and reduced by 9 when the product exceeds 9.
     *
     * @param string $code Data digits without the check digit.
     */
    protected function getCheckDigit(string $code): int
    {
        $sum = 0;
        $clen = \strlen($code);
        for ($chr = 0; $chr < $clen; ++$chr) {
            $digit = (int) $code[$chr];
            if (($chr % 2) === 1) {
                $digit *= 2;
                if ($digit > 9) {
                    $digit -= 9;
                }
            }

            $sum += $digit;
        }

        return $sum % 10;
    }

    /**
     * Convert the code to a zero-padded base 32 string of fixed length.
     *
     * @param int $value Code value including the check digit.
     */
    protected function getBaseThirtyTwo(int $value): string
    {
        $radix = \strlen($this::ALPHABET);
        $encoded = '';
        for ($chr = 0; $chr < $this::ENCODED_LENGTH; ++$chr) {
            $encoded = $this::ALPHABET[$value % $radix] . $encoded;
            $value = \intdiv($value, $radix);
        }

        return $encoded;
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
        $this->extcode = '*' . $this->getBaseThirtyTwo((int) $code) . '*';
    }
}
