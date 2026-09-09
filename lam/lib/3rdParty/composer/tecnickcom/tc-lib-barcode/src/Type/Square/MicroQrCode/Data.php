<?php

declare(strict_types=1);

/**
 * Data.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\MicroQrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\MicroQrCode\Data
 *
 * Symbol tables for the MicroQrCode Barcode type class
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Data
{
    /**
     * Numeric encodation mode.
     *
     * @var int
     */
    public const MODE_NUMERIC = 0;

    /**
     * Alphanumeric encodation mode.
     *
     * @var int
     */
    public const MODE_ALPHANUM = 1;

    /**
     * Byte encodation mode.
     *
     * @var int
     */
    public const MODE_BYTE = 2;

    /**
     * Symbol size in modules, by version.
     *
     * @var array<int, int>
     */
    public const SIZE = [
        1 => 11,
        2 => 13,
        3 => 15,
        4 => 17,
    ];

    /**
     * Size in bits of the mode indicator, by version.
     * Version M1 carries no mode indicator.
     *
     * @var array<int, int>
     */
    public const MODE_BITS = [
        1 => 0,
        2 => 1,
        3 => 2,
        4 => 3,
    ];

    /**
     * Size in bits of the terminator, by version.
     *
     * @var array<int, int>
     */
    public const TERMINATOR_BITS = [
        1 => 3,
        2 => 5,
        3 => 7,
        4 => 9,
    ];

    /**
     * Size in bits of the character count indicator, by version and mode.
     * A negative value marks a mode that the version does not support.
     *
     * @var array<int, array<int, int>>
     */
    public const COUNT_BITS = [
        1 => [3, -1, -1],
        2 => [4, 3, -1],
        3 => [5, 4, 4],
        4 => [6, 5, 5],
    ];

    /**
     * Symbol characteristics by symbol number: version, error correction level,
     * number of data codewords, number of data bits and number of error
     * correction codewords. Version M1 carries error detection only and has no
     * error correction level.
     *
     * @var array<int, array{int, string, int, int, int}>
     */
    public const SYMBOLS = [
        0 => [1, '', 3, 20, 2],
        1 => [2, 'L', 5, 40, 5],
        2 => [2, 'M', 4, 32, 6],
        3 => [3, 'L', 11, 84, 6],
        4 => [3, 'M', 9, 68, 8],
        5 => [4, 'L', 16, 128, 8],
        6 => [4, 'M', 14, 112, 10],
        7 => [4, 'Q', 10, 80, 14],
    ];

    /**
     * Number of bits added by the trailing digits of a numeric group.
     *
     * @var array<int, int>
     */
    public const NUMERIC_REMAINDER_BITS = [0, 4, 7];

    /**
     * Pad codewords, applied in turn to fill the data capacity.
     *
     * @var array<int, int>
     */
    public const PAD_CODEWORDS = [0xEC, 0x11];

    /**
     * Generator polynomial of the BCH (15, 5) code of the format information.
     *
     * @var int
     */
    public const FORMAT_GENERATOR = 0b101_0011_0111;

    /**
     * Mask applied to the format information.
     *
     * @var int
     */
    public const FORMAT_MASK = 0b100_0100_0100_0101;
}
