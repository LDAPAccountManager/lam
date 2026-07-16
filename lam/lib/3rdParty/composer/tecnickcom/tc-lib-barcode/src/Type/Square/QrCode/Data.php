<?php

declare(strict_types=1);

/**
 * Data.php
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

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Data
 *
 * Data for QrCode Barcode type class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Data
{
    /**
     * Maximum QR Code version.
     *
     * @var int
     */
    public const QRSPEC_VERSION_MAX = 40;

    /**
     * Maximum matrix size for maximum version (version 40 is 177*177 matrix).
     *
     * @var int
     */
    public const QRSPEC_WIDTH_MAX = 177;

    // -----------------------------------------------------

    /**
     * Encoding mode: variable.
     *
     * @var int
     */
    public const MODE_NL = -1;

    /**
     * Encoding mode: numeric.
     *
     * @var int
     */
    public const MODE_NM = 0;

    /**
     * Encoding mode: alphanumeric.
     *
     * @var int
     */
    public const MODE_AN = 1;

    /**
     * Encoding mode: 8-bit byte.
     *
     * @var int
     */
    public const MODE_8B = 2;

    /**
     * Encoding mode: kanji.
     *
     * @var int
     */
    public const MODE_KJ = 3;

    /**
     * Encoding mode: structured append.
     *
     * @var int
     */
    public const MODE_ST = 4;

    // -----------------------------------------------------

    /**
     * Matrix index to get width from CAPACITY array.
     *
     * @var int
     */
    public const QRCAP_WIDTH = 0;

    /**
     * Matrix index to get number of words from CAPACITY array.
     *
     * @var int
     */
    public const QRCAP_WORDS = 1;

    /**
     * Matrix index to get remainder from CAPACITY array.
     *
     * @var int
     */
    public const QRCAP_REMINDER = 2;

    /**
     * Matrix index to get error correction level from CAPACITY array.
     *
     * @var int
     */
    public const QRCAP_EC = 3;

    // -----------------------------------------------------

    // Structure (currently usupported)

    /**
     * Number of header bits for structured mode
     *
     * @var int
     */
    public const STRUCTURE_HEADER_BITS = 20;

    /**
     * Max number of symbols for structured mode
     *
     * @var int
     */
    public const MAX_STRUCTURED_SYMBOLS = 16;

    // -----------------------------------------------------

    // Masks

    /**
     * Down point base value for case 1 mask pattern (concatenation of same color in a line or a column)
     *
     * @var int
     */
    public const N1 = 3;

    /**
     * Down point base value for case 2 mask pattern (module block of same color)
     *
     * @var int
     */
    public const N2 = 3;

    /**
     * Down point base value for case 3 mask pattern
     * (1:1:3:1:1 (dark:bright:dark:bright:dark) pattern in a line or a column)
     *
     * @var int
     */
    public const N3 = 40;

    /**
     * Down point base value for case 4 mask pattern (ration of dark modules in whole)
     *
     * @var int
     */
    public const N4 = 10;

    /**
     * Encoding modes (characters which can be encoded in QRcode)
     *
     * NL : variable
     * NM : Encoding mode numeric (0-9). 3 characters are encoded to 10bit length.
     * AN : Encoding mode alphanumeric (0-9A-Z $%*+-./:) 45characters. 2 characters are encoded to 11bit length.
     * 8B : Encoding mode 8bit byte data. In theory, 2953 characters or less can be stored in a QRcode.
     * KJ : Encoding mode KANJI. A KANJI character (multibyte character) is encoded to 13bit length.
     * ST : Encoding mode STRUCTURED
     *
     * @var array<string, int>
     */
    public const ENC_MODES = [
        'NL' => self::MODE_NL,
        'NM' => self::MODE_NM,
        'AN' => self::MODE_AN,
        '8B' => self::MODE_8B,
        'KJ' => self::MODE_KJ,
        'ST' => self::MODE_ST,
    ];

    /**
     * Array of valid error correction levels
     * QRcode has a function of an error correcting for miss reading that white is black.
     * Error correcting is defined in 4 level as below.
     * L : About 7% or less errors can be corrected.
     * M : About 15% or less errors can be corrected.
     * Q : About 25% or less errors can be corrected.
     * H : About 30% or less errors can be corrected.
     *
     * @var array<string, int>
     */
    public const ECC_LEVELS = [
        'L' => 0,
        'M' => 1,
        'Q' => 2,
        'H' => 3,
    ];

    /**
     * Alphabet-numeric conversion table.
     *
     * @var array<int>
     */
    public const AN_TABLE = [
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,

        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,

        36,
        -1,
        -1,
        -1,
        37,
        38,
        -1,
        -1,
        -1,
        -1,
        39,
        40,
        -1,
        41,
        42,
        43,

        0,
        1,
        2,
        3,
        4,
        5,
        6,
        7,
        8,
        9,
        44,
        -1,
        -1,
        -1,
        -1,
        -1,

        -1,
        10,
        11,
        12,
        13,
        14,
        15,
        16,
        17,
        18,
        19,
        20,
        21,
        22,
        23,
        24,

        25,
        26,
        27,
        28,
        29,
        30,
        31,
        32,
        33,
        34,
        35,
        -1,
        -1,
        -1,
        -1,
        -1,

        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,

        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
        -1,
    ];

    /**
     * Array Table of the capacity of symbols.
     * See Table 1 (pp.13) and Table 12-16 (pp.30-36), JIS X0510:2004.
     *
     * @var array<array{int, int, int, array{int, int, int, int}}>
     */
    public const CAPACITY = [
        [0, 0, 0, [0, 0, 0, 0]],
        [21, 26, 0, [7, 10, 13, 17]],
        [25, 44, 7, [10, 16, 22, 28]],
        [29, 70, 7, [15, 26, 36, 44]],
        [33, 100, 7, [20, 36, 52, 64]],
        [37, 134, 7, [26, 48, 72, 88]],
        [41, 172, 7, [36, 64, 96, 112]],
        [45, 196, 0, [40, 72, 108, 130]],
        [49, 242, 0, [48, 88, 132, 156]],
        [53, 292, 0, [60, 110, 160, 192]],
        [57, 346, 0, [72, 130, 192, 224]],
        [61, 404, 0, [80, 150, 224, 264]],
        [65, 466, 0, [96, 176, 260, 308]],
        [69, 532, 0, [104, 198, 288, 352]],
        [73, 581, 3, [120, 216, 320, 384]],
        [77, 655, 3, [132, 240, 360, 432]],
        [81, 733, 3, [144, 280, 408, 480]],
        [85, 815, 3, [168, 308, 448, 532]],
        [89, 901, 3, [180, 338, 504, 588]],
        [93, 991, 3, [196, 364, 546, 650]],
        [97, 1085, 3, [224, 416, 600, 700]],
        [101, 1156, 4, [224, 442, 644, 750]],
        [105, 1258, 4, [252, 476, 690, 816]],
        [109, 1364, 4, [270, 504, 750, 900]],
        [113, 1474, 4, [300, 560, 810, 960]],
        [117, 1588, 4, [312, 588, 870, 1050]],
        [121, 1706, 4, [336, 644, 952, 1110]],
        [125, 1828, 4, [360, 700, 1020, 1200]],
        [129, 1921, 3, [390, 728, 1050, 1260]],
        [133, 2051, 3, [420, 784, 1140, 1350]],
        [137, 2185, 3, [450, 812, 1200, 1440]],
        [141, 2323, 3, [480, 868, 1290, 1530]],
        [145, 2465, 3, [510, 924, 1350, 1620]],
        [149, 2611, 3, [540, 980, 1440, 1710]],
        [153, 2761, 3, [570, 1036, 1530, 1800]],
        [157, 2876, 0, [570, 1064, 1590, 1890]],
        [161, 3034, 0, [600, 1120, 1680, 1980]],
        [165, 3196, 0, [630, 1204, 1770, 2100]],
        [169, 3362, 0, [660, 1260, 1860, 2220]],
        [173, 3532, 0, [720, 1316, 1950, 2310]],
        [177, 3706, 0, [750, 1372, 2040, 2430]],
    ];

    /**
     * Array Length indicator.
     *
     * @var array<array{int, int, int}>
     */
    public const LEN_TABLE_BITS = [
        [10, 12, 14],
        [9, 11, 13],
        [8, 16, 16],
        [8, 10, 12],
    ];

    /**
     * Array Table of the error correction code (Reed-Solomon block).
     * See Table 12-16 (pp.30-36), JIS X0510:2004.
     *
     * @var array<array{array{int, int}, array{int, int}, array{int, int}, array{int, int}}>
     */
    public const ECC_TABLE = [
        [[0, 0], [0, 0], [0, 0], [0, 0]],
        [[1, 0], [1, 0], [1, 0], [1, 0]],
        [[1, 0], [1, 0], [1, 0], [1, 0]],
        [[1, 0], [1, 0], [2, 0], [2, 0]],
        [[1, 0], [2, 0], [2, 0], [4, 0]],
        [[1, 0], [2, 0], [2, 2], [2, 2]],
        [[2, 0], [4, 0], [4, 0], [4, 0]],
        [[2, 0], [4, 0], [2, 4], [4, 1]],
        [[2, 0], [2, 2], [4, 2], [4, 2]],
        [[2, 0], [3, 2], [4, 4], [4, 4]],
        [[2, 2], [4, 1], [6, 2], [6, 2]],
        [[4, 0], [1, 4], [4, 4], [3, 8]],
        [[2, 2], [6, 2], [4, 6], [7, 4]],
        [[4, 0], [8, 1], [8, 4], [12, 4]],
        [[3, 1], [4, 5], [11, 5], [11, 5]],
        [[5, 1], [5, 5], [5, 7], [11, 7]],
        [[5, 1], [7, 3], [15, 2], [3, 13]],
        [[1, 5], [10, 1], [1, 15], [2, 17]],
        [[5, 1], [9, 4], [17, 1], [2, 19]],
        [[3, 4], [3, 11], [17, 4], [9, 16]],
        [[3, 5], [3, 13], [15, 5], [15, 10]],
        [[4, 4], [17, 0], [17, 6], [19, 6]],
        [[2, 7], [17, 0], [7, 16], [34, 0]],
        [[4, 5], [4, 14], [11, 14], [16, 14]],
        [[6, 4], [6, 14], [11, 16], [30, 2]],
        [[8, 4], [8, 13], [7, 22], [22, 13]],
        [[10, 2], [19, 4], [28, 6], [33, 4]],
        [[8, 4], [22, 3], [8, 26], [12, 28]],
        [[3, 10], [3, 23], [4, 31], [11, 31]],
        [[7, 7], [21, 7], [1, 37], [19, 26]],
        [[5, 10], [19, 10], [15, 25], [23, 25]],
        [[13, 3], [2, 29], [42, 1], [23, 28]],
        [[17, 0], [10, 23], [10, 35], [19, 35]],
        [[17, 1], [14, 21], [29, 19], [11, 46]],
        [[13, 6], [14, 23], [44, 7], [59, 1]],
        [[12, 7], [12, 26], [39, 14], [22, 41]],
        [[6, 14], [6, 34], [46, 10], [2, 64]],
        [[17, 4], [29, 14], [49, 10], [24, 46]],
        [[4, 18], [13, 32], [48, 14], [42, 32]],
        [[20, 4], [40, 7], [43, 22], [10, 67]],
        [[19, 6], [18, 31], [34, 34], [20, 61]],
    ];

    /**
     * Array Positions of alignment patterns.
     * This array includes only the second and the third position of the alignment patterns.
     * Rest of them can be calculated from the distance between them.
     * See Table 1 in Appendix E (pp.71) of JIS X0510:2004.
     *
     * @var array<array{int, int}>
     */
    public const ALIGN_PATTERN = [
        [0, 0],
        [0, 0],
        [18, 0],
        [22, 0],
        [26, 0],
        [30, 0],
        [34, 0],
        [22, 38],
        [24, 42],
        [26, 46],
        [28, 50],
        [30, 54],
        [32, 58],
        [34, 62],
        [26, 46],
        [26, 48],
        [26, 50],
        [30, 54],
        [30, 56],
        [30, 58],
        [34, 62],
        [28, 50],
        [26, 50],
        [30, 54],
        [28, 54],
        [32, 58],
        [30, 58],
        [34, 62],
        [26, 50],
        [30, 54],
        [26, 52],
        [30, 56],
        [34, 60],
        [30, 58],
        [34, 62],
        [30, 54],
        [24, 50],
        [28, 54],
        [32, 58],
        [26, 54],
        [30, 58],
    ];

    /**
     * Array Version information pattern (BCH coded).
     * See Table 1 in Appendix D (pp.68) of JIS X0510:2004.
     * size: [QRSPEC_VERSION_MAX - 6]
     *
     * @var array<int>
     */
    public const VERSION_PATTERN = [
        0x0_7c94,
        0x0_85bc,
        0x0_9a99,
        0x0_a4d3,
        0x0_bbf6,
        0x0_c762,
        0x0_d847,
        0x0_e60d,
        0x0_f928,
        0x1_0b78,
        0x1_145d,
        0x1_2a17,
        0x1_3532,
        0x1_49a6,
        0x1_5683,
        0x1_68c9,
        0x1_77ec,
        0x1_8ec4,
        0x1_91e1,
        0x1_afab,
        0x1_b08e,
        0x1_cc1a,
        0x1_d33f,
        0x1_ed75,
        0x1_f250,
        0x2_09d5,
        0x2_16f0,
        0x2_28ba,
        0x2_379f,
        0x2_4b0b,
        0x2_542e,
        0x2_6a64,
        0x2_7541,
        0x2_8c69,
    ];

    /**
     * Array Format information
     *
     * @var array<array{int, int, int, int, int, int, int, int}>
     */
    public const FORMAT_INFO = [
        [0x77c4, 0x72f3, 0x7daa, 0x789d, 0x662f, 0x6318, 0x6c41, 0x6976],
        [0x5412, 0x5125, 0x5e7c, 0x5b4b, 0x45f9, 0x40ce, 0x4f97, 0x4aa0],
        [0x355f, 0x3068, 0x3f31, 0x3a06, 0x24b4, 0x2183, 0x2eda, 0x2bed],
        [0x1689, 0x13be, 0x1ce7, 0x19d0, 0x0762, 0x0255, 0x0d0c, 0x083b],
    ];
}
