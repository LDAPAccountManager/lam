<?php

declare(strict_types=1);

/**
 * Plessey.php
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
 * Com\Tecnick\Barcode\Type\Linear\Plessey;
 *
 * Plessey Barcode type class
 * Plessey Code
 *
 * A pulse width modulated symbology: each hexadecimal character is four bits,
 * least significant first, and each bit is one pitch of a bar and a space. The
 * data is bounded by the forward start code, the cyclic redundancy check, the
 * termination bar and the reverse start code.
 *
 * One pitch is twelve modules, the smallest whole number of modules that keeps
 * all four element widths within the tolerances of the dimension table.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Plessey extends \Com\Tecnick\Barcode\Type\Linear
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'PLESSEY';

    /**
     * Forward start code, which opens the symbol
     *
     * @var string
     */
    protected const START = '1101';

    /**
     * Reverse start code, which closes the symbol with the elements of each bit
     * in the opposite order, so that the symbol is read in either direction
     *
     * @var string
     */
    protected const STOP = '0011';

    /**
     * Termination bar, one full pitch of bar between the check code and the
     * reverse start code
     *
     * @var string
     */
    protected const TERMINATOR = '111111111111';

    /**
     * Module widths of a bit whose bar precedes its space
     *
     * @var array<int|string, string>
     */
    protected const FORWARD = [
        '0' => '110000000000',
        '1' => '111111100000',
    ];

    /**
     * Module widths of a bit whose space precedes its bar
     *
     * @var array<int|string, string>
     */
    protected const REVERSE = [
        '0' => '000000000011',
        '1' => '000001111111',
    ];

    /**
     * Generator polynomial of the cyclic redundancy check, x^8 + x^7 + x^6 + x^5 + x^3 + 1
     *
     * @var array<int, int>
     */
    protected const POLYNOMIAL = [1, 1, 1, 1, 0, 1, 0, 0, 1];

    /**
     * Number of check bits
     */
    protected const CHECK_BITS = 8;

    /**
     * Encodable character set, in the order of their four bit values
     *
     * @var string
     */
    protected const KEYS = '0123456789ABCDEF';

    /**
     * Get the four bits of a hexadecimal character, least significant first
     *
     * @throws BarcodeException in case of an unsupported character
     */
    protected function getCharBits(string $char): string
    {
        $val = \strpos($this::KEYS, $char);
        if (!\is_int($val)) {
            throw new BarcodeException('Invalid character: ' . (\ord($char) & 0xFF));
        }

        return \strrev(\str_pad(\decbin($val), 4, '0', STR_PAD_LEFT));
    }

    /**
     * Get the check bits of a data bit string.
     * The generator polynomial is added to the stream at every set data bit,
     * and the check bits are what it leaves in the eight appended positions.
     */
    protected function getCheckBits(string $data): string
    {
        $len = \strlen($data);
        $bits = \array_map(\intval(...), \str_split($data . \str_repeat('0', $this::CHECK_BITS)));
        for ($idx = 0; $idx < $len; ++$idx) {
            if (($bits[$idx] ?? 0) === 0) {
                continue;
            }

            foreach ($this::POLYNOMIAL as $pos => $coefficient) {
                $bits[$idx + $pos] = ($bits[$idx + $pos] ?? 0) ^ $coefficient;
            }
        }

        return \implode('', \array_slice($bits, $len));
    }

    /**
     * Get the hexadecimal characters of a bit string, four bits per character
     */
    protected function getBitsCode(string $bits): string
    {
        $code = '';
        foreach (\str_split($bits, 4) as $nibble) {
            $code .= $this::KEYS[(int) \bindec(\strrev($nibble))];
        }

        return $code;
    }

    /**
     * Format code
     *
     * @throws BarcodeException in case of an unsupported character
     */
    protected function formatCode(): void
    {
        $code = \strtoupper($this->code);
        $bits = '';
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            $bits .= $this->getCharBits($code[$pos]);
        }

        if ($bits === '') {
            throw new BarcodeException('Empty input string');
        }

        $this->extcode = $code . $this->getBitsCode($this->getCheckBits($bits));
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->formatCode();

        $seq = '';
        foreach (\str_split($this::START) as $bit) {
            $seq .= $this::FORWARD[$bit] ?? '';
        }

        $clen = \strlen($this->extcode);
        for ($pos = 0; $pos < $clen; ++$pos) {
            foreach (\str_split($this->getCharBits($this->extcode[$pos])) as $bit) {
                $seq .= $this::FORWARD[$bit] ?? '';
            }
        }

        $seq .= $this::TERMINATOR;
        foreach (\str_split($this::STOP) as $bit) {
            $seq .= $this::REVERSE[$bit] ?? '';
        }

        $this->processBinarySequence($this->getRawCodeRows($seq));
    }
}
