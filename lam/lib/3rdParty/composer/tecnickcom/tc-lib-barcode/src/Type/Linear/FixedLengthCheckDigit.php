<?php

declare(strict_types=1);

/**
 * FixedLengthCheckDigit.php
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
 * Com\Tecnick\Barcode\Type\Linear\FixedLengthCheckDigit
 *
 * FixedLengthCheckDigit trait
 *
 * Shared by the symbologies of a fixed digit count whose last digit is a check
 * digit: the input is either the data digits, which get the check digit
 * appended, or the whole code, which must carry the right one. A shorter input
 * is left padded with zeros. The types belong to different hierarchies, so the
 * rule is shared here.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
trait FixedLengthCheckDigit
{
    /**
     * Calculate the check digit of the data digits.
     *
     * @param string $code Data digits without the check digit
     */
    abstract protected function getCheckDigit(string $code): int;

    /**
     * Get the code of the full length, carrying its check digit.
     *
     * @param string $code   Input code, the check digit optional
     * @param int    $length Full length of the code, check digit included
     *
     * @throws BarcodeException if the supplied check digit is wrong
     */
    protected function getCheckedCode(string $code, int $length): string
    {
        $data_len = $length - 1;
        $code = \str_pad($code, $data_len, '0', STR_PAD_LEFT);
        if (\strlen($code) === $data_len) {
            return $code . $this->getCheckDigit($code);
        }

        $check = $this->getCheckDigit(\substr($code, 0, $data_len));
        if ($check !== (int) $code[$data_len]) {
            throw new BarcodeException('Invalid check digit: ' . $check);
        }

        return $code;
    }
}
