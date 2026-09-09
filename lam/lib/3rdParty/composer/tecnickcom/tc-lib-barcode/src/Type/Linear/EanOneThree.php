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
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
 * An optional 2 or 5 digit add-on symbol is appended to the code after a plus
 * sign, as in "9781234567897+12345". Main symbol and add-on are drawn as a
 * single symbol, separated by the right quiet zone of the main symbol.
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
     * Separation in modules between the main symbol and the add-on symbol,
     * equal to the right quiet zone of the main symbol.
     * A value of zero marks a symbology that admits no add-on symbol.
     */
    protected int $addon_separation = 7;

    /**
     * Digits of the add-on symbol, empty when there is no add-on
     */
    protected string $addon = '';

    /**
     * Code of the main symbol, without the add-on
     */
    protected string $maincode = '';

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
     * Split the optional add-on symbol from the input code
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        $this->maincode = $this->code;
        $plus = \strpos($this->code, '+');
        if ($plus === false) {
            return;
        }

        $this->maincode = \substr($this->code, 0, $plus);
        $this->addon = \substr($this->code, $plus + 1);
    }

    /**
     * Check that the input code is a digit string that fits the fixed length of this symbology.
     * Shorter codes are left-padded with zeros by formatCode().
     *
     * @throws BarcodeException if the code is not numeric or is too long
     */
    protected function validateCode(): void
    {
        if (!\ctype_digit($this->maincode)) {
            throw new BarcodeException('Input code must be a number');
        }

        if (\strlen($this->maincode) > $this->code_length) {
            throw new BarcodeException(
                'The code is too long: '
                . \strlen($this->maincode)
                . ' digits (maximum '
                . $this->code_length
                . ' for '
                . $this::FORMAT
                . ')',
            );
        }

        $this->validateAddon();
    }

    /**
     * Check the add-on symbol, which carries 2 or 5 digits.
     *
     * @throws BarcodeException if the add-on cannot be represented
     */
    protected function validateAddon(): void
    {
        if (!\str_contains($this->code, '+')) {
            return;
        }

        if ($this->addon_separation === 0) {
            throw new BarcodeException($this::FORMAT . ' admits no add-on symbol');
        }

        $addon_len = \strlen($this->addon);
        if ($addon_len !== 2 && $addon_len !== 5 || !\ctype_digit($this->addon)) {
            throw new BarcodeException('The add-on must be 2 or 5 digits: ' . $this->addon);
        }
    }

    /**
     * Get the modules of the separation and of the add-on symbol, or an empty
     * string when the symbol carries no add-on.
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color error
     */
    protected function getAddonSequence(): string
    {
        if ($this->addon === '') {
            return '';
        }

        $addon = \strlen($this->addon) === 2 ? new EanTwo($this->addon) : new EanFive($this->addon);

        return \str_repeat('0', \max(0, $this->addon_separation)) . \rtrim($addon->getGrid(), "\n");
    }

    /**
     * Add the add-on digits to the human readable interpretation,
     * once the symbol has been drawn.
     */
    protected function appendAddonToExtendedCode(): void
    {
        if ($this->addon === '') {
            return;
        }

        $this->extcode .= '+' . $this->addon;
    }

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
        $code = \str_pad($this->maincode, $this->code_length - 1, '0', STR_PAD_LEFT);
        // getChecksum() returns the missing check digit, or 0 when the input already carries it
        $check = $this->getChecksum($code);
        $this->extcode = \strlen($code) >= $this->code_length ? $code : $code . $check;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color error
     */
    protected function setBars(): void
    {
        $this->validateCode();
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
        $seq .= $this->getAddonSequence();
        $this->processBinarySequence($this->getRawCodeRows($seq));
        $this->appendAddonToExtendedCode();
    }
}
