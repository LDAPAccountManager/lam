<?php

declare(strict_types=1);

/**
 * Data.php
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Data
 *
 * Symbol tables for the QrCode Barcode type class.
 *
 * Only the tables ISO/IEC 18004 states without a rule that generates them are
 * held here. The codeword capacity of Table 1, the alignment pattern positions,
 * the format information of Table C.1, the version information of Table D.1 and
 * the data mask patterns of Table 10 all follow from a stated rule and are
 * computed in Encode.
 *
 * @since       2026-09-02
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
     * Lowest symbol version.
     *
     * @var int
     */
    public const VERSION_MIN = 1;

    /**
     * Highest symbol version.
     *
     * @var int
     */
    public const VERSION_MAX = 40;

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
     * Kanji encodation mode.
     *
     * @var int
     */
    public const MODE_KANJI = 3;

    /**
     * Mode indicators of Table 2 of ISO/IEC 18004, by encodation mode.
     *
     * @var array<int, int>
     */
    public const MODE_INDICATOR = [0b0001, 0b0010, 0b0100, 0b1000];

    /**
     * Size in bits of the mode indicator, Table 2 of ISO/IEC 18004.
     *
     * @var int
     */
    public const MODE_BITS = 4;

    /**
     * Size in bits of the terminator, Table 2 of ISO/IEC 18004.
     *
     * @var int
     */
    public const TERMINATOR_BITS = 4;

    /**
     * Size in bits of the character count indicator, Table 3 of ISO/IEC 18004,
     * by version group and encodation mode. The three version groups are the
     * versions 1 to 9, 10 to 26 and 27 to 40.
     *
     * @var array<int, array<int, int>>
     */
    public const COUNT_BITS = [
        [10, 9, 8, 8],
        [12, 11, 16, 10],
        [14, 13, 16, 12],
    ];

    /**
     * Highest version of each version group of COUNT_BITS.
     *
     * @var array<int, int>
     */
    public const COUNT_GROUP_MAX = [9, 26, 40];

    /**
     * Error correction levels in the order of ECC_INDICATOR and ECC_BLOCKS.
     *
     * @var array<string, int>
     */
    public const ECC_LEVELS = ['L' => 0, 'M' => 1, 'Q' => 2, 'H' => 3];

    /**
     * Error correction level indicators of Table 12 of ISO/IEC 18004, by level.
     *
     * @var array<int, int>
     */
    public const ECC_INDICATOR = [0b01, 0b00, 0b11, 0b10];

    /**
     * Alphanumeric character set of Table 5 of ISO/IEC 18004. The value of a
     * character is its position in this string.
     *
     * @var string
     */
    public const AN_CHARS = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ $%*+-./:';

    /**
     * Error correction characteristics of Table 9 of ISO/IEC 18004, by version
     * and error correction level: the number of error correction codewords per
     * block, and the number of blocks.
     *
     * Table 9 also states the (c, k, r) code of each block. It follows from
     * these two numbers and the codeword capacity of the version, because the
     * data codewords are shared out over the blocks as evenly as possible, so
     * it is computed in Encode::getBlockSizes() rather than tabulated.
     *
     * @var array<int, array<int, array{int, int}>>
     */
    public const ECC_BLOCKS = [
        1 => [[7, 1], [10, 1], [13, 1], [17, 1]],
        2 => [[10, 1], [16, 1], [22, 1], [28, 1]],
        3 => [[15, 1], [26, 1], [18, 2], [22, 2]],
        4 => [[20, 1], [18, 2], [26, 2], [16, 4]],
        5 => [[26, 1], [24, 2], [18, 4], [22, 4]],
        6 => [[18, 2], [16, 4], [24, 4], [28, 4]],
        7 => [[20, 2], [18, 4], [18, 6], [26, 5]],
        8 => [[24, 2], [22, 4], [22, 6], [26, 6]],
        9 => [[30, 2], [22, 5], [20, 8], [24, 8]],
        10 => [[18, 4], [26, 5], [24, 8], [28, 8]],
        11 => [[20, 4], [30, 5], [28, 8], [24, 11]],
        12 => [[24, 4], [22, 8], [26, 10], [28, 11]],
        13 => [[26, 4], [22, 9], [24, 12], [22, 16]],
        14 => [[30, 4], [24, 9], [20, 16], [24, 16]],
        15 => [[22, 6], [24, 10], [30, 12], [24, 18]],
        16 => [[24, 6], [28, 10], [24, 17], [30, 16]],
        17 => [[28, 6], [28, 11], [28, 16], [28, 19]],
        18 => [[30, 6], [26, 13], [28, 18], [28, 21]],
        19 => [[28, 7], [26, 14], [26, 21], [26, 25]],
        20 => [[28, 8], [26, 16], [30, 20], [28, 25]],
        21 => [[28, 8], [26, 17], [28, 23], [30, 25]],
        22 => [[28, 9], [28, 17], [30, 23], [24, 34]],
        23 => [[30, 9], [28, 18], [30, 25], [30, 30]],
        24 => [[30, 10], [28, 20], [30, 27], [30, 32]],
        25 => [[26, 12], [28, 21], [30, 29], [30, 35]],
        26 => [[28, 12], [28, 23], [28, 34], [30, 37]],
        27 => [[30, 12], [28, 25], [30, 34], [30, 40]],
        28 => [[30, 13], [28, 26], [30, 35], [30, 42]],
        29 => [[30, 14], [28, 28], [30, 38], [30, 45]],
        30 => [[30, 15], [28, 29], [30, 40], [30, 48]],
        31 => [[30, 16], [28, 31], [30, 43], [30, 51]],
        32 => [[30, 17], [28, 33], [30, 45], [30, 54]],
        33 => [[30, 18], [28, 35], [30, 48], [30, 57]],
        34 => [[30, 19], [28, 37], [30, 51], [30, 60]],
        35 => [[30, 19], [28, 38], [30, 53], [30, 63]],
        36 => [[30, 20], [28, 40], [30, 56], [30, 66]],
        37 => [[30, 21], [28, 43], [30, 59], [30, 70]],
        38 => [[30, 22], [28, 45], [30, 62], [30, 74]],
        39 => [[30, 24], [28, 47], [30, 65], [30, 77]],
        40 => [[30, 25], [28, 49], [30, 68], [30, 81]],
    ];

    /**
     * Row and column coordinates of the centre module of the alignment
     * patterns, Table E.1 of ISO/IEC 18004, by version. An alignment pattern is
     * centred on every pair of coordinates except the three that the finder
     * patterns cover. Version 1 carries no alignment pattern.
     *
     * @var array<int, array<int, int>>
     */
    public const ALIGN_CENTRES = [
        1 => [],
        2 => [6, 18],
        3 => [6, 22],
        4 => [6, 26],
        5 => [6, 30],
        6 => [6, 34],
        7 => [6, 22, 38],
        8 => [6, 24, 42],
        9 => [6, 26, 46],
        10 => [6, 28, 50],
        11 => [6, 30, 54],
        12 => [6, 32, 58],
        13 => [6, 34, 62],
        14 => [6, 26, 46, 66],
        15 => [6, 26, 48, 70],
        16 => [6, 26, 50, 74],
        17 => [6, 30, 54, 78],
        18 => [6, 30, 56, 82],
        19 => [6, 30, 58, 86],
        20 => [6, 34, 62, 90],
        21 => [6, 28, 50, 72, 94],
        22 => [6, 26, 50, 74, 98],
        23 => [6, 30, 54, 78, 102],
        24 => [6, 28, 54, 80, 106],
        25 => [6, 32, 58, 84, 110],
        26 => [6, 30, 58, 86, 114],
        27 => [6, 34, 62, 90, 118],
        28 => [6, 26, 50, 74, 98, 122],
        29 => [6, 30, 54, 78, 102, 126],
        30 => [6, 26, 52, 78, 104, 130],
        31 => [6, 30, 56, 82, 108, 134],
        32 => [6, 34, 60, 86, 112, 138],
        33 => [6, 30, 58, 86, 114, 142],
        34 => [6, 34, 62, 90, 118, 146],
        35 => [6, 30, 54, 78, 102, 126, 150],
        36 => [6, 24, 50, 76, 102, 128, 154],
        37 => [6, 28, 54, 80, 106, 132, 158],
        38 => [6, 32, 58, 84, 110, 136, 162],
        39 => [6, 26, 54, 82, 110, 138, 166],
        40 => [6, 30, 58, 86, 114, 142, 170],
    ];

    /**
     * Shift JIS ranges of the kanji mode, section 7.4.6 of ISO/IEC 18004: the
     * first and the last value of the range, and the value subtracted from a
     * character of it.
     *
     * @var array<int, array{int, int, int}>
     */
    public const KANJI_RANGES = [
        [0x8140, 0x9FFC, 0x8140],
        [0xE040, 0xEBBF, 0xC140],
    ];

    /**
     * Pad codewords of section 7.4.10 of ISO/IEC 18004, applied in turn to fill
     * the data capacity.
     *
     * @var array<int, int>
     */
    public const PAD_CODEWORDS = [0xEC, 0x11];

    /**
     * Generator polynomial of the BCH (15, 5) code of the format information,
     * x^10 + x^8 + x^5 + x^4 + x^2 + x + 1, Annex C of ISO/IEC 18004.
     *
     * @var int
     */
    public const FORMAT_GENERATOR = 0b101_0011_0111;

    /**
     * Mask applied to the format information, section 7.9.1 of ISO/IEC 18004.
     *
     * @var int
     */
    public const FORMAT_MASK = 0b101_0100_0001_0010;

    /**
     * Generator polynomial of the BCH (18, 6) code of the version information,
     * x^12 + x^11 + x^10 + x^9 + x^8 + x^5 + x^2 + 1, Annex D of ISO/IEC 18004.
     *
     * @var int
     */
    public const VERSION_GENERATOR = 0b1_1111_0010_0101;

    /**
     * Lowest version that carries the version information, section 7.10 of
     * ISO/IEC 18004.
     *
     * @var int
     */
    public const VERSION_INFO_MIN = 7;

    /**
     * Penalty points of a run of more than five modules of the same colour in a
     * row or a column, Table 11 of ISO/IEC 18004.
     *
     * @var int
     */
    public const N1 = 3;

    /**
     * Penalty points of a two by two block of modules of the same colour,
     * Table 11 of ISO/IEC 18004.
     *
     * @var int
     */
    public const N2 = 3;

    /**
     * Penalty points of a 1:1:3:1:1 pattern next to four light modules,
     * Table 11 of ISO/IEC 18004.
     *
     * @var int
     */
    public const N3 = 40;

    /**
     * Penalty points of each five percent by which the proportion of dark
     * modules departs from one half, Table 11 of ISO/IEC 18004.
     *
     * @var int
     */
    public const N4 = 10;
}
