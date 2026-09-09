<?php

declare(strict_types=1);

/**
 * DeutschePost.php
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
 * Com\Tecnick\Barcode\Type\Linear\DeutschePost;
 *
 * Shared layer of the Deutsche Post Identcode and Leitcode
 *
 * Both are Interleaved 2 of 5 symbols of a fixed digit count whose check digit
 * weights the data digits with 4 and 9 instead of the 3 and 1 of the symbology.
 *
 * Identcode and Leitcode are trademarks of Deutsche Post DHL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class DeutschePost extends \Com\Tecnick\Barcode\Type\Linear\InterleavedTwoOfFiveCheck
{
    /**
     * Total number of digits, including the check digit
     */
    protected const CODE_LENGTH = 0;

    /**
     * Weights applied to the data digits, from the leftmost one
     *
     * @var array<int, int>
     */
    protected const WEIGHT = [4, 9];

    /**
     * Calculate the check digit of the data digits
     *
     * @param string $code Data digits, without the check digit
     */
    protected function getChecksum(string $code): int
    {
        $sum = 0;
        $clen = \strlen($code);
        for ($idx = 0; $idx < $clen; ++$idx) {
            $sum += (int) $code[$idx] * ($this::WEIGHT[$idx % 2] ?? 0);
        }

        return (10 - ($sum % 10)) % 10;
    }

    /**
     * Check that the input code is a digit string of the data or the full length
     *
     * @throws BarcodeException if the code is not numeric or has the wrong length
     */
    protected function validateCode(): void
    {
        if (!\ctype_digit($this->code)) {
            throw new BarcodeException('Input code must be a number');
        }

        $len = \strlen($this->code);
        $data_len = $this::CODE_LENGTH - 1;
        if (!\in_array($len, [$data_len, $this::CODE_LENGTH], true)) {
            throw new BarcodeException(
                'The code must be '
                . $data_len
                . ' digits without the check digit or '
                . $this::CODE_LENGTH
                . ' with it, '
                . $len
                . ' given',
            );
        }
    }

    /**
     * Format code.
     * A code of the data length gets the check digit appended, a code of the
     * full length must carry the correct one.
     *
     * @throws BarcodeException if the supplied check digit is wrong
     */
    protected function formatCode(): void
    {
        $data_len = $this::CODE_LENGTH - 1;
        $data = \substr($this->code, 0, $data_len);
        $check = $this->getChecksum($data);
        if (\strlen($this->code) === $this::CODE_LENGTH && $check !== (int) $this->code[$data_len]) {
            throw new BarcodeException('Invalid check digit: ' . $check);
        }

        $this->extcode = $data . (string) $check;
    }

    /**
     * Set the bars array.
     * The digits are interleaved in pairs between the start and stop patterns,
     * as in Interleaved 2 of 5.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->validateCode();
        $this->formatCode();

        $pairs = 'AA' . $this->extcode . 'ZA';
        $this->ncols = 0;
        $this->nrows = 1;
        $this->bars = [];
        $clen = \strlen($pairs);
        for ($idx = 0; $idx < $clen; $idx += 2) {
            $bar_pattern = $this->getPattern($pairs[$idx]);
            $space_pattern = $this->getPattern($pairs[$idx + 1]);
            $chrlen = \strlen($bar_pattern);
            for ($pos = 0; $pos < $chrlen; ++$pos) {
                $this->bars[] = [$this->ncols, 0, (int) $bar_pattern[$pos], 1];
                $this->ncols += (int) $bar_pattern[$pos] + (int) ($space_pattern[$pos] ?? '0');
            }
        }

        // the trailing space of the stop pattern is part of the quiet zone
        --$this->ncols;
    }
}
