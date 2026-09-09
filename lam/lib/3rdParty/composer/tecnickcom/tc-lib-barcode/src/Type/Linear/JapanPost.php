<?php

declare(strict_types=1);

/**
 * JapanPost.php
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
 * Com\Tecnick\Barcode\Type\Linear\JapanPost;
 *
 * JapanPost Barcode type class
 * Japan Post Customer Barcode
 *
 * The symbol is the start code, the seven digit postal code, the thirteen
 * character address display number, the check digit and the stop code. Every
 * character is three bars of four states. A letter is a control code followed
 * by a digit, so it takes two of the thirteen positions.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class JapanPost extends \Com\Tecnick\Barcode\Type\Linear\FourState
{
    /**
     * Bar identifier of the long bar
     *
     * @var string
     */
    protected const FULL = '1';

    /**
     * Bar identifier of the upper semi-long bar
     *
     * @var string
     */
    protected const ASCENDER = '2';

    /**
     * Bar identifier of the lower semi-long bar, the timing bar being every
     * other identifier
     *
     * @var string
     */
    protected const DESCENDER = '3';

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'JPPOST';

    /**
     * Number of digits of the postal code
     */
    protected const POSTAL_CODE_LENGTH = 7;

    /**
     * Number of characters of the address display number
     */
    protected const ADDRESS_LENGTH = 13;

    /**
     * Modulus of the check digit
     */
    protected const MODULUS = 19;

    /**
     * Character used to fill the address display number
     *
     * @var string
     */
    protected const FILLER = 'CC4';

    /**
     * Start code
     *
     * @var string
     */
    protected const START = '13';

    /**
     * Stop code
     *
     * @var string
     */
    protected const STOP = '31';

    /**
     * Map characters to bars.
     * A long bar is 1, an upper semi-long bar 2, a lower semi-long bar 3 and a
     * timing bar 4.
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '144',
        '1' => '114',
        '2' => '132',
        '3' => '312',
        '4' => '123',
        '5' => '141',
        '6' => '321',
        '7' => '213',
        '8' => '231',
        '9' => '411',
        '-' => '414',
        'CC1' => '324',
        'CC2' => '342',
        'CC3' => '234',
        'CC4' => '432',
        'CC5' => '243',
        'CC6' => '423',
        'CC7' => '441',
        'CC8' => '111',
    ];

    /**
     * Check digit value of each character, in the order of the values 0 to 18
     *
     * @var array<int, string>
     */
    protected const CHECK_VALUE = [
        '0',
        '1',
        '2',
        '3',
        '4',
        '5',
        '6',
        '7',
        '8',
        '9',
        '-',
        'CC1',
        'CC2',
        'CC3',
        'CC4',
        'CC5',
        'CC6',
        'CC7',
        'CC8',
    ];

    /**
     * Control code of each block of ten letters
     *
     * @var array<int, string>
     */
    protected const LETTER_CONTROL = ['CC1', 'CC2', 'CC3'];

    /**
     * Get the characters of a letter, a control code followed by a digit
     *
     * @return array<int, string>
     */
    protected function getLetterChars(string $letter): array
    {
        $index = \ord($letter) - \ord('A');
        return [$this::LETTER_CONTROL[\intdiv($index, 10)] ?? '', (string) ($index % 10)];
    }

    /**
     * Get the characters of a code, expanding the letters
     *
     * @return array<int, string>
     *
     * @throws BarcodeException in case of an unsupported character
     */
    protected function getChars(string $code): array
    {
        $chars = [];
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $char = $code[$pos];
            if ($char >= 'A' && $char <= 'Z') {
                $chars = \array_merge($chars, $this->getLetterChars($char));
                continue;
            }

            if (!\array_key_exists($char, $this::CHBAR)) {
                throw new BarcodeException('Invalid character: ' . (\ord($char) & 0xFF));
            }

            $chars[] = $char;
        }

        return $chars;
    }

    /**
     * Get the check digit character.
     * The check values of the postal code and of the address display number sum
     * to a multiple of the modulus once the check digit is added.
     *
     * @param array<int, string> $chars Characters of the postal code and the address display number
     */
    protected function getChecksum(array $chars): string
    {
        $sum = 0;
        foreach ($chars as $char) {
            $value = \array_search($char, $this::CHECK_VALUE, true);
            $sum += \is_int($value) ? $value : 0;
        }

        return $this::CHECK_VALUE[($this::MODULUS - ($sum % $this::MODULUS)) % $this::MODULUS] ?? '0';
    }

    /**
     * Split the input code into the postal code and the address display number.
     * The hyphen between the third and the fourth digit of the postal code is
     * dropped, as are the leading and trailing spaces of the address.
     *
     * @return array{string, string}
     *
     * @throws BarcodeException if the postal code is not seven digits
     */
    protected function getFields(): array
    {
        $code = \strtoupper(\str_replace(' ', '', $this->code));
        if (\preg_match('/^(\d{3})-(\d{4})/', $code) === 1) {
            $code = \substr_replace($code, '', 3, 1);
        }

        $postal = \substr($code, 0, $this::POSTAL_CODE_LENGTH);
        if (!\ctype_digit($postal) || \strlen($postal) !== $this::POSTAL_CODE_LENGTH) {
            throw new BarcodeException(
                'The code must start with the ' . $this::POSTAL_CODE_LENGTH . ' digit postal code',
            );
        }

        return [$postal, \substr($code, $this::POSTAL_CODE_LENGTH)];
    }

    /**
     * Format code.
     * The address display number is cut to the thirteen positions of the field
     * and filled with the filler character.
     *
     * @return array<int, string> Characters of the whole symbol, without the start and stop codes
     *
     * @throws BarcodeException in case of an unsupported character
     */
    protected function getSymbolChars(): array
    {
        [$postal, $address] = $this->getFields();
        $field = \array_pad(
            \array_slice($this->getChars($address), 0, $this::ADDRESS_LENGTH),
            $this::ADDRESS_LENGTH,
            $this::FILLER,
        );
        $chars = \array_merge($this->getChars($postal), $field);
        $chars[] = $this->getChecksum($chars);
        $this->extcode = \implode('', $chars);

        return $chars;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $values = $this::START;
        foreach ($this->getSymbolChars() as $char) {
            $values .= $this::CHBAR[$char] ?? '';
        }

        $values .= $this::STOP;

        $this->setStateBars($values);
    }
}
