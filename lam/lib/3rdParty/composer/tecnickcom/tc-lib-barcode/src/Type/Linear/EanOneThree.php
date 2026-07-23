<?php

declare(strict_types=1);

/**
 * EanOneThree.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\EanOneThree;
 *
 * EanOneThree Barcode type class
 * EAN 13
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class EanOneThree extends \Com\Tecnick\Barcode\Type\Linear
{
    protected function getCharAt(string $value, int $index): string
    {
        return $value[$index] ?? '0';
    }

    protected function getParityPattern(string $digit): string
    {
        return $this::PARITIES[$digit] ?? 'AAAAAA';
    }

    protected function getBarPattern(string $parity, string $digit): string
    {
        return $this::CHBAR[$parity][$digit] ?? '';
    }

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'EAN13';

    /**
     * Fixed code length
     */
    protected int $code_length = 13;

    /**
     * Check digit
     */
    protected int $check = 0;

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, array<int|string, string>>
     */
    protected const CHBAR = [
        'A' => [
            // left odd parity
            '0' => '0001101',
            '1' => '0011001',
            '2' => '0010011',
            '3' => '0111101',
            '4' => '0100011',
            '5' => '0110001',
            '6' => '0101111',
            '7' => '0111011',
            '8' => '0110111',
            '9' => '0001011',
        ],
        'B' => [
            // left even parity
            '0' => '0100111',
            '1' => '0110011',
            '2' => '0011011',
            '3' => '0100001',
            '4' => '0011101',
            '5' => '0111001',
            '6' => '0000101',
            '7' => '0010001',
            '8' => '0001001',
            '9' => '0010111',
        ],
        'C' => [
            // right
            '0' => '1110010',
            '1' => '1100110',
            '2' => '1101100',
            '3' => '1000010',
            '4' => '1011100',
            '5' => '1001110',
            '6' => '1010000',
            '7' => '1000100',
            '8' => '1001000',
            '9' => '1110100',
        ],
    ];

    /**
     * Map parities
     *
     * @var array<int|string, string>
     */
    protected const PARITIES = [
        '0' => 'AAAAAA',
        '1' => 'AABABB',
        '2' => 'AABBAB',
        '3' => 'AABBBA',
        '4' => 'ABAABB',
        '5' => 'ABBAAB',
        '6' => 'ABBBAA',
        '7' => 'ABABAB',
        '8' => 'ABABBA',
        '9' => 'ABBABA',
    ];

    /**
     * Calculate checksum
     *
     * @param string $code Code to represent.
     *
     * @return int char checksum.
     *
     * @throws BarcodeException in case of error
     */
    protected function getChecksum(string $code): int
    {
        $data_len = $this->code_length - 1;
        $code_len = \strlen($code);
        $sum_a = 0;
        for ($pos = 1; $pos < $data_len; $pos += 2) {
            $sum_a += (int) $code[$pos];
        }

        if ($this->code_length > 12) {
            $sum_a *= 3;
        }

        $sum_b = 0;
        for ($pos = 0; $pos < $data_len; $pos += 2) {
            $sum_b += (int) $code[$pos];
        }

        if ($this->code_length < 13) {
            $sum_b *= 3;
        }

        $this->check = ($sum_a + $sum_b) % 10;
        if ($this->check > 0) {
            $this->check = 10 - $this->check;
        }

        if ($code_len === $data_len) {
            // add check digit
            return $this->check;
        }

        if ($this->check !== (int) $code[$data_len]) {
            // wrong check digit
            throw new BarcodeException('Invalid check digit: ' . $this->check);
        }

        return 0;
    }

    /**
     * Format code
     *
     * @throws BarcodeException in case of error
     */
    protected function formatCode(): void
    {
        $code = \str_pad($this->code, $this->code_length - 1, '0', STR_PAD_LEFT);
        // getChecksum() returns the missing check digit, or validates (and returns 0)
        // when the input already carries it; in the latter case keep the code unchanged
        // to avoid appending a spurious extra digit to the extended code.
        $check = $this->getChecksum($code);
        $this->extcode = \strlen($code) >= $this->code_length ? $code : $code . $check;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (!\is_numeric($this->code)) {
            throw new BarcodeException('Input code must be a number');
        }

        $this->formatCode();
        $seq = '101'; // left guard bar
        $half_len = (int) \ceil($this->code_length / 2);
        $parity = $this->getParityPattern($this->getCharAt($this->extcode, 0));
        for ($pos = 1; $pos < $half_len; ++$pos) {
            $seq .= $this->getBarPattern($this->getCharAt($parity, $pos - 1), $this->getCharAt($this->extcode, $pos));
        }

        $seq .= '01010'; // center guard bar
        for ($pos = $half_len; $pos < $this->code_length; ++$pos) {
            $seq .= $this->getBarPattern('C', $this->getCharAt($this->extcode, $pos));
        }

        $seq .= '101'; // right guard bar
        $this->processBinarySequence($this->getRawCodeRows($seq));
    }
}
