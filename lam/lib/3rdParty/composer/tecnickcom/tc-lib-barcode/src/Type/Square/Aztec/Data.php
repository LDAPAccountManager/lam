<?php

declare(strict_types=1);

/**
 * Data.php
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Aztec;

/**
 * Com\Tecnick\Barcode\Type\Square\Aztec\Data
 *
 * Data for Aztec Barcode type class
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Data
{
    /**
     * Code character encoding mode for uppercase letters.
     *
     * @var int
     */
    public const MODE_UPPER = 0;

    /**
     * Code character encoding mode for lowercase letters.
     *
     * @var int
     */
    public const MODE_LOWER = 1;

    /**
     * Code character encoding mode for digits.
     *
     * @var int
     */
    public const MODE_DIGIT = 2;

    /**
     * Code character encoding mode for mixed cases.
     *
     * @var int
     */
    public const MODE_MIXED = 3;

    /**
     * Code character encoding mode for punctuation.
     *
     * @var int
     */
    public const MODE_PUNCT = 4;

    /**
     * Code character encoding mode for binary.
     *
     * @var int
     */
    public const MODE_BINARY = 5;

    /**
     * Number of bits for each character encoding mode.
     *
     * @var array{int, int, int, int, int, int}
     */
    public const MODE_BITS = [
        5, // 0 = MODE_UPPER
        5, // 1 = MODE_LOWER
        4, // 2 = MODE_DIGIT
        5, // 3 = MODE_MIXED
        5, // 4 = MODE_PUNCT
        8, // 5 = MODE_BINARY
    ];

    /**
     * Code character encoding for each mode.
     *
     * @var array<int, array<int>>
     */
    public const CHAR_ENC = [
        // MODE_UPPER (initial mode)
        0 => [
            32 => 1, // ' ' (SP)
            65 => 2, // 'A'
            66 => 3, // 'B'
            67 => 4, // 'C'
            68 => 5, // 'D'
            69 => 6, // 'E'
            70 => 7, // 'F'
            71 => 8, // 'G'
            72 => 9, // 'H'
            73 => 10, // 'I'
            74 => 11, // 'J'
            75 => 12, // 'K'
            76 => 13, // 'L'
            77 => 14, // 'M'
            78 => 15, // 'N'
            79 => 16, // 'O'
            80 => 17, // 'P'
            81 => 18, // 'Q'
            82 => 19, // 'R'
            83 => 20, // 'S'
            84 => 21, // 'T'
            85 => 22, // 'U'
            86 => 23, // 'V'
            87 => 24, // 'W'
            88 => 25, // 'X'
            89 => 26, // 'Y'
            90 => 27, // 'Z'
        ],
        // MODE_LOWER
        1 => [
            32 => 1, // ' ' (SP)
            97 => 2, // 'a'
            98 => 3, // 'b'
            99 => 4, // 'c'
            100 => 5, // 'd'
            101 => 6, // 'e'
            102 => 7, // 'f'
            103 => 8, // 'g'
            104 => 9, // 'h'
            105 => 10, // 'i'
            106 => 11, // 'j'
            107 => 12, // 'k'
            108 => 13, // 'l'
            109 => 14, // 'm'
            110 => 15, // 'n'
            111 => 16, // 'o'
            112 => 17, // 'p'
            113 => 18, // 'q'
            114 => 19, // 'r'
            115 => 20, // 's'
            116 => 21, // 't'
            117 => 22, // 'u'
            118 => 23, // 'v'
            119 => 24, // 'w'
            120 => 25, // 'x'
            121 => 26, // 'y'
            122 => 27, // 'z'
        ],
        // MODE_DIGIT
        2 => [
            32 => 1, // ' ' (SP)
            44 => 12, // ','
            46 => 13, // '.'
            48 => 2, // '0'
            49 => 3, // '1'
            50 => 4, // '2'
            51 => 5, // '3'
            52 => 6, // '4'
            53 => 7, // '5'
            54 => 8, // '6'
            55 => 9, // '7'
            56 => 10, // '8'
            57 => 11, // '9'
        ],
        // MODE_MIXED
        3 => [
            32 => 1, // ' ' (SP)
            1 => 2, // '^A' (SOH)
            2 => 3, // '^B' (STX)
            3 => 4, // '^C' (ETX)
            4 => 5, // '^D' (EOT)
            5 => 6, // '^E' (ENQ)
            6 => 7, // '^F' (ACK)
            7 => 8, // '^G' (BEL)
            8 => 9, // '^H' (BS)
            9 => 10, // '^I' (HT)
            10 => 11, // '^J' (LF)
            11 => 12, // '^K' (VT)
            12 => 13, // '^L' (FF)
            13 => 14, // '^M' (CR)
            27 => 15, // '^[' (ESC)
            28 => 16, // '^\' (FS)
            29 => 17, // '^]' (GS)
            30 => 18, // '^^' (RS)
            31 => 19, // '^_' (US)
            64 => 20, // '@'
            92 => 21, // '\'
            94 => 22, // '^'
            95 => 23, // '_'
            96 => 24, // '`'
            124 => 25, // '|'
            126 => 26, // '~'
            127 => 27, // '^?' (DEL)
        ],
        // MODE_PUNCT
        4 => [
            13 => 1, // '\r' (CR)
            33 => 6, // '!'
            34 => 7, // '"'
            35 => 8, // '#'
            36 => 9, // '$'
            37 => 10, // '%'
            38 => 11, // '&'
            39 => 12, // '''
            40 => 13, // '('
            41 => 14, // ')'
            42 => 15, // '*'
            43 => 16, // '+'
            44 => 17, // ','
            45 => 18, // '-'
            46 => 19, // '.'
            47 => 20, // '/'
            58 => 21, // ':'
            59 => 22, // ';'
            60 => 23, // '<'
            61 => 24, // '='
            62 => 25, // '>'
            63 => 26, // '?'
            91 => 27, // '['
            93 => 28, // ']'
            123 => 29, // '{'
            125 => 30, // '}'
        ],
        // MODE_BINARY (all 8-bit values are valid)
        5 => [],
    ];

    /**
     * Map character ASCII codes to their non-binary mode.
     * Exceptions are:
     *   - the space ' ' character (32) that maps for modes 0,1,2.
     *   - the carriage return '\r' character (13) that maps for modes 3,4.
     *   - the comma ',' and dot '.' characters (44,46) that map for modes 2,4.
     *
     * @var array<int>
     */
    public const CHAR_MODES = [
        1 => 3, // '^A' (SOH)
        2 => 3, // '^B' (STX)
        3 => 3, // '^C' (ETX)
        4 => 3, // '^D' (EOT)
        5 => 3, // '^E' (ENQ)
        6 => 3, // '^F' (ACK)
        7 => 3, // '^G' (BEL)
        8 => 3, // '^H' (BS)
        9 => 3, // '^I' (HT)
        10 => 3, // '^J' (LF)
        11 => 3, // '^K' (VT)
        12 => 3, // '^L' (FF)
        13 => 3, // '^M' (CR) [3,4]
        27 => 3, // '^[' (ESC)
        28 => 3, // '^\' (FS)
        29 => 3, // '^]' (GS)
        30 => 3, // '^^' (RS)
        31 => 3, // '^_' (US)
        32 => 0, // ' ' [0,1,2]
        33 => 4, // '!'
        34 => 4, // '"'
        35 => 4, // '#'
        36 => 4, // '$'
        37 => 4, // '%'
        38 => 4, // '&'
        39 => 4, // '''
        40 => 4, // '('
        41 => 4, // ')'
        42 => 4, // '*'
        43 => 4, // '+'f
        44 => 2, // ',' [2,4]
        45 => 4, // '-'
        46 => 2, // '.' [2,4]
        47 => 4, // '/'
        48 => 2, // '0'
        49 => 2, // '1'
        50 => 2, // '2'
        51 => 2, // '3'
        52 => 2, // '4'
        53 => 2, // '5'
        54 => 2, // '6'
        55 => 2, // '7'
        56 => 2, // '8'
        57 => 2, // '9'
        58 => 4, // ':'
        59 => 4, // ';'
        60 => 4, // '<'
        61 => 4, // '='
        62 => 4, // '>'
        63 => 4, // '?'
        64 => 3, // '@'
        65 => 0, // 'A'
        66 => 0, // 'B'
        67 => 0, // 'C'
        68 => 0, // 'D'
        69 => 0, // 'E'
        70 => 0, // 'F'
        71 => 0, // 'G'
        72 => 0, // 'H'
        73 => 0, // 'I'
        74 => 0, // 'J'
        75 => 0, // 'K'
        76 => 0, // 'L'
        77 => 0, // 'M'
        78 => 0, // 'N'
        79 => 0, // 'O'
        80 => 0, // 'P'
        81 => 0, // 'Q'
        82 => 0, // 'R'
        83 => 0, // 'S'
        84 => 0, // 'T'
        85 => 0, // 'U'
        86 => 0, // 'V'
        87 => 0, // 'W'
        88 => 0, // 'X'
        89 => 0, // 'Y'
        90 => 0, // 'Z'
        91 => 4, // '['
        92 => 3, // '\'
        93 => 4, // ']'
        94 => 3, // '^'
        95 => 3, // '_'
        96 => 3, // '`'
        97 => 1, // 'a'
        98 => 1, // 'b'
        99 => 1, // 'c'
        100 => 1, // 'd'
        101 => 1, // 'e'
        102 => 1, // 'f'
        103 => 1, // 'g'
        104 => 1, // 'h'
        105 => 1, // 'i'
        106 => 1, // 'j'
        107 => 1, // 'k'
        108 => 1, // 'l'
        109 => 1, // 'm'
        110 => 1, // 'n'
        111 => 1, // 'o'
        112 => 1, // 'p'
        113 => 1, // 'q'
        114 => 1, // 'r'
        115 => 1, // 's'
        116 => 1, // 't'
        117 => 1, // 'u'
        118 => 1, // 'v'
        119 => 1, // 'w'
        120 => 1, // 'x'
        121 => 1, // 'y'
        122 => 1, // 'z'
        123 => 4, // '{'
        124 => 3, // '|'
        125 => 4, // '}'
        126 => 3, // '~'
        127 => 3, // '^?' (DEL)
    ];

    /**
     * Latch map for changing character encoding mode.
     * Numbers represent: [number of bits to change, latch code value].
     *
     * @var array<int, array<int, array<array{int, int}>>>
     */
    public const LATCH_MAP = [
        // MODE_UPPER
        0 => [
            1 => [[5, 28]], // -> LOWER
            2 => [[5, 30]], // -> DIGIT
            3 => [[5, 29]], // -> MIXED
            4 => [[5, 29], [5, 30]], // -> MIXED -> PUNCT
        ],
        // MODE_LOWER
        1 => [
            0 => [[5, 30], [4, 14]], // -> DIGIT -> UPPER
            2 => [[5, 30]], // -> DIGIT
            3 => [[5, 29]], // -> MIXED
            4 => [[5, 29], [5, 30]], // -> MIXED -> PUNCT
        ],
        // MODE_DIGIT
        2 => [
            0 => [[4, 14]], // -> UPPER
            1 => [[4, 14], [5, 28]], // -> UPPER -> LOWER
            3 => [[4, 14], [5, 29]], // -> UPPER -> MIXED
            4 => [[4, 14], [5, 29], [5, 30]], // -> UPPER -> MIXED -> PUNCT
        ],
        // MODE_MIXED
        3 => [
            0 => [[5, 29]], // -> UPPER
            1 => [[5, 28]], // -> LOWER
            2 => [[5, 29], [5, 30]], // -> UPPER -> DIGIT
            4 => [[5, 30]], // -> PUNCT
        ],
        // MODE_PUNCT
        4 => [
            0 => [[5, 31]], // -> UPPER
            1 => [[5, 31], [5, 28]], // -> UPPER -> LOWER
            2 => [[5, 31], [5, 30]], // -> UPPER -> DIGIT
            3 => [[5, 31], [5, 29]], // -> UPPER -> MIXED
        ],
    ];

    /**
     * Shift map for changing character encoding mode.
     * Numbers represent: [number of bits to change, shift code value].
     *
     * @var array<int, array<int, array<array{int, int}>>>
     */
    public const SHIFT_MAP = [
        // MODE_UPPER
        0 => [
            1 => [],
            2 => [],
            3 => [],
            4 => [[5, 0]], // -> PUNCT
            5 => [[5, 31]], // -> BINARY
        ],
        // MODE_LOWER
        1 => [
            0 => [[5, 28]], // -> UPPER
            2 => [],
            3 => [],
            4 => [[5, 0]], // -> PUNCT
            5 => [[5, 31]], // -> BINARY
        ],
        // MODE_DIGIT
        2 => [
            0 => [[4, 15]], // -> UPPER
            1 => [],
            3 => [],
            4 => [[4, 0]], // -> PUNCT
            5 => [[4, 14], [5, 31]], // -> LATCH UPPER -> BINARY
        ],
        // MODE_MIXED
        3 => [
            0 => [],
            1 => [],
            2 => [],
            4 => [[5, 0]], // -> PUNCT
            5 => [[5, 31]], // -> BINARY
        ],
        // MODE_PUNCT
        4 => [
            0 => [],
            1 => [],
            2 => [],
            3 => [],
            5 => [[5, 31], [5, 31]], // -> LATCH UPPER -> BINARY
        ],
    ];

    /**
     * Extended Channel Interpretation (ECI) codes.
     *
     * @var array<int, string>
     */
    public const ECI = [
        0 => 'FNC1', // Function 1 character
        2 => 'Cp437', // Code page 437
        3 => 'ISO-8859-1', // ISO/IEC 8859-1 - Latin-1 (Default encoding)
        4 => 'ISO-8859-2', // ISO/IEC 8859-2 - Latin-2
        5 => 'ISO-8859-3', // ISO/IEC 8859-3 - Latin-3
        6 => 'ISO-8859-4', // ISO/IEC 8859-4 - Latin-4
        7 => 'ISO-8859-5', // ISO/IEC 8859-5 - Latin/Cyrillic
        8 => 'ISO-8859-6', // ISO/IEC 8859-6 - Latin/Arabic
        9 => 'ISO-8859-7', // ISO/IEC 8859-7 - Latin/Greek
        10 => 'ISO-8859-8', // ISO/IEC 8859-8 - Latin/Hebrew
        11 => 'ISO-8859-9', // ISO/IEC 8859-9 - Latin-5
        12 => 'ISO-8859-10', // ISO/IEC 8859-10 - Latin-6
        13 => 'ISO-8859-11', // ISO/IEC 8859-11 - Latin/Thai
        15 => 'ISO-8859-13', // ISO/IEC 8859-13 - Latin-7
        16 => 'ISO-8859-14', // ISO/IEC 8859-14 - Latin-8 (Celtic)
        17 => 'ISO-8859-15', // ISO/IEC 8859-15 - Latin-9
        18 => 'ISO-8859-16', // ISO/IEC 8859-16 - Latin-10
        20 => 'Shift JIS',
        21 => 'Cp1250', // Windows-1250 - Superset of Latin-2
        22 => 'Cp1251', // Windows-1251 - Latin/Cyrillic
        23 => 'Cp1252', // Windows-1252 - Superset of Latin-1
        24 => 'Cp1256', // Windows-1256 - Arabic
        25 => 'UTF-16BE', // UnicodeBig, UnicodeBigUnmarked
        26 => 'UTF-8',
        27 => 'US-ASCII',
        28 => 'Big5',
        29 => 'GB18030', // GB2312, EUC_CN, GBK
        30 => 'EUC-KR',
    ];

    /**
     * Size and capacities of Aztec Compact Code symbols by number of layers.
     * The array entries are:
     *   - 0: symbol x size;
     *   - 1: codeword count;
     *   - 2: codeword size;
     *   - 3: symbol bit capacity;
     *   - 4: symbol data digits capacity;
     *   - 5: symbol data text capacity;
     *   - 6: symbol data bytes capacity.
     *
     * @var array<int, array{int, int, int, int, int, int, int}>
     */
    public const SIZE_COMPACT = [
        1 => [15, 17, 6, 102, 13, 12, 6],
        2 => [19, 40, 6, 240, 40, 33, 19],
        3 => [23, 51, 8, 408, 70, 57, 33],
        4 => [27, 76, 8, 608, 110, 89, 53],
    ];

    /**
     * Size and capacities of Aztec Full-range Code symbols by number of layers.
     * The array entries are:
     *   - 0: symbol x size;
     *   - 1: codeword count;
     *   - 2: codeword size;
     *   - 3: symbol bit capacity;
     *   - 4: symbol data digits capacity;
     *   - 5: symbol data text capacity;
     *   - 6: symbol data bytes capacity.
     *
     * @var array<int, array{int, int, int, int, int, int, int}>
     */
    public const SIZE_FULL = [
        1 => [19, 21, 6, 126, 18, 15, 8],
        2 => [23, 48, 6, 288, 49, 40, 24],
        3 => [27, 60, 8, 480, 84, 68, 40],
        4 => [31, 88, 8, 704, 128, 104, 62],
        5 => [37, 120, 8, 960, 178, 144, 87],
        6 => [41, 156, 8, 1248, 232, 187, 114],
        7 => [45, 196, 8, 1568, 294, 236, 145],
        8 => [49, 240, 8, 1920, 362, 291, 179],
        9 => [53, 230, 10, 2300, 433, 348, 214],
        10 => [57, 272, 10, 2720, 516, 414, 256],
        11 => [61, 316, 10, 3160, 601, 482, 298],
        12 => [67, 364, 10, 3640, 691, 554, 343],
        13 => [71, 416, 10, 4160, 793, 636, 394],
        14 => [75, 470, 10, 4700, 896, 718, 446],
        15 => [79, 528, 10, 5280, 1008, 808, 502],
        16 => [83, 588, 10, 5880, 1123, 900, 559],
        17 => [87, 652, 10, 6520, 1246, 998, 621],
        18 => [91, 720, 10, 7200, 1378, 1104, 687],
        19 => [95, 790, 10, 7900, 1511, 1210, 753],
        20 => [101, 864, 10, 8640, 1653, 1324, 824],
        21 => [105, 940, 10, 9400, 1801, 1442, 898],
        22 => [109, 1020, 10, 10_200, 1956, 1566, 976],
        23 => [113, 920, 12, 11_040, 2116, 1694, 1056],
        24 => [117, 992, 12, 11_904, 2281, 1826, 1138],
        25 => [121, 1066, 12, 12_792, 2452, 1963, 1224],
        26 => [125, 1144, 12, 13_728, 2632, 2107, 1314],
        27 => [131, 1224, 12, 14_688, 2818, 2256, 1407],
        28 => [135, 1306, 12, 15_672, 3007, 2407, 1501],
        29 => [139, 1392, 12, 16_704, 3205, 2565, 1600],
        30 => [143, 1480, 12, 17_760, 3409, 2728, 1702],
        31 => [147, 1570, 12, 18_840, 3616, 2894, 1806],
        32 => [151, 1664, 12, 19_968, 3832, 3067, 1914],
    ];
}
