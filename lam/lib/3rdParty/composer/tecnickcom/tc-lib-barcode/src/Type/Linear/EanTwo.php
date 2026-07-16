<?php

declare(strict_types=1);

/**
 * EanTwo.php
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
 * Com\Tecnick\Barcode\Type\Linear\EanTwo;
 *
 * EanTwo Barcode type class
 * EAN 2-Digits UPC-Based Extension
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class EanTwo extends \Com\Tecnick\Barcode\Type\Linear
{
    protected function getCharAt(string $value, int $index): string
    {
        return $value[$index] ?? '0';
    }

    /**
     * @return array<int, string>
     */
    protected function getParityPattern(int $check): array
    {
        $pattern = $this::PARITIES[$check] ?? ['A', 'A'];
        return \array_values($pattern);
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
    protected const FORMAT = 'EAN2';

    /**
     * Fixed code length
     */
    protected int $code_length = 2;

    /**
     * Map characters to barcodes
     *
     * @var array<string, array<int|string, string>>
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
    ];

    /**
     * Map parities
     *
     * @var array<int|string, array<string>>
     */
    protected const PARITIES = [
        '0' => ['A', 'A'],
        '1' => ['A', 'B'],
        '2' => ['B', 'A'],
        '3' => ['B', 'B'],
    ];

    /**
     * Calculate checksum
     *
     * @param string $code Code to represent.
     *
     * @return int char checksum.
     */
    protected function getChecksum(string $code): int
    {
        return (int) $code % 4;
    }

    /**
     * Format code
     */
    protected function formatCode(): void
    {
        $this->extcode = \str_pad($this->code, $this->code_length, '0', STR_PAD_LEFT);
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->formatCode();
        $chk = $this->getChecksum($this->extcode);
        $parity = $this->getParityPattern($chk);
        $seq = '1011'; // left guard bar
        $seq .= $this->getBarPattern($this->getCharAt($parity[0] ?? 'A', 0), $this->getCharAt($this->extcode, 0));
        $len = \strlen($this->extcode);
        for ($pos = 1; $pos < $len; ++$pos) {
            $seq .= '01'; // separator
            $seq .= $this->getBarPattern(
                $this->getCharAt($parity[$pos] ?? 'A', 0),
                $this->getCharAt($this->extcode, $pos),
            );
        }

        $this->processBinarySequence($this->getRawCodeRows($seq));
    }
}
