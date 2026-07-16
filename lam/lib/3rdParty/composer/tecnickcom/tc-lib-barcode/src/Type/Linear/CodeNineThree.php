<?php

declare(strict_types=1);

/**
 * CodeNineThree.php
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
 * Com\Tecnick\Barcode\Type\Linear\CodeNineThree;
 *
 * CodeNineThree Barcode type class
 * CODE 93 - USS-93
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class CodeNineThree extends \Com\Tecnick\Barcode\Type\Linear\CodeThreeNineExtCheck
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'C93';

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        32 => '311211', // space
        36 => '321111', // $
        37 => '211131', // %
        42 => '111141', // start-stop
        43 => '113121', // +
        45 => '121131', // -
        46 => '311112', // .
        47 => '112131', // /
        48 => '131112', // 0
        49 => '111213', // 1
        50 => '111312', // 2
        51 => '111411', // 3
        52 => '121113', // 4
        53 => '121212', // 5
        54 => '121311', // 6
        55 => '111114', // 7
        56 => '131211', // 8
        57 => '141111', // 9
        65 => '211113', // A
        66 => '211212', // B
        67 => '211311', // C
        68 => '221112', // D
        69 => '221211', // E
        70 => '231111', // F
        71 => '112113', // G
        72 => '112212', // H
        73 => '112311', // I
        74 => '122112', // J
        75 => '132111', // K
        76 => '111123', // L
        77 => '111222', // M
        78 => '111321', // N
        79 => '121122', // O
        80 => '131121', // P
        81 => '212112', // Q
        82 => '212211', // R
        83 => '211122', // S
        84 => '211221', // T
        85 => '221121', // U
        86 => '222111', // V
        87 => '112122', // W
        88 => '112221', // X
        89 => '122121', // Y
        90 => '123111', // Z
        128 => '121221', // ($)
        129 => '311121', // (/)
        130 => '122211', // (+)
        131 => '312111', // (%)
    ];

    /**
     * Map for extended characters
     *
     * @var array<string>
     */
    protected const EXTCODES = [
        "\x83U",
        "\x80A",
        "\x80B",
        "\x80C",
        "\x80D",
        "\x80E",
        "\x80F",
        "\x80G",
        "\x80H",
        "\x80I",
        "\x80J",
        "\x80K",
        "\x80L",
        "\x80M",
        "\x80N",
        "\x80O",
        "\x80P",
        "\x80Q",
        "\x80R",
        "\x80S",
        "\x80T",
        "\x80U",
        "\x80V",
        "\x80W",
        "\x80X",
        "\x80Y",
        "\x80Z",
        "\x83A",
        "\x83B",
        "\x83C",
        "\x83D",
        "\x83E",
        ' ',
        "\x81A",
        "\x81B",
        "\x81C",
        "\x81D",
        "\x81E",

        "\x81F",
        "\x81G",
        "\x81H",
        "\x81I",
        "\x81J",
        "\x81K",
        "\x81L",
        '-',
        '.',
        "\x81O",
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
        "\x81Z",
        "\x83F",
        "\x83G",
        "\x83H",
        "\x83I",
        "\x83J",
        "\x83V",
        'A',
        'B',
        'C',
        'D',
        'E',
        'F',
        'G',
        'H',
        'I',
        'J',
        'K',
        'L',
        'M',
        'N',
        'O',
        'P',
        'Q',
        'R',
        'S',
        'T',
        'U',
        'V',
        'W',
        'X',
        'Y',
        'Z',
        "\x83K",
        "\x83L",
        "\x83M",
        "\x83N",
        "\x83O",
        "\x83W",
        "\x82A",
        "\x82B",
        "\x82C",
        "\x82D",
        "\x82E",
        "\x82F",
        "\x82G",
        "\x82H",
        "\x82I",
        "\x82J",
        "\x82K",
        "\x82L",
        "\x82M",
        "\x82N",
        "\x82O",
        "\x82P",
        "\x82Q",
        "\x82R",
        "\x82S",
        "\x82T",
        "\x82U",
        "\x82V",
        "\x82W",
        "\x82X",
        "\x82Y",
        "\x82Z",
        "\x83P",
        "\x83Q",
        "\x83R",
        "\x83S",
        "\x83T",
    ];

    /**
     * Characters used for checksum
     *
     * @var array<string>
     */
    protected const CHKSUM = [
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
        'A',
        'B',
        'C',
        'D',
        'E',
        'F',
        'G',
        'H',
        'I',
        'J',
        'K',
        'L',
        'M',
        'N',
        'O',
        'P',
        'Q',
        'R',
        'S',
        'T',
        'U',
        'V',
        'W',
        'X',
        'Y',
        'Z',
        '-',
        '.',
        ' ',
        '$',
        '/',
        '+',
        '%',
        '<',
        '=',
        '>',
        '?',
    ];

    protected function getBarPattern(int $char): string
    {
        return $this::CHBAR[$char] ?? '000000';
    }

    /**
     * Calculate CODE 93 checksum (modulo 47).
     *
     * @param string $code Code to represent.
     *
     * @return string char checksum.
     */
    protected function getChecksum(string $code): string
    {
        // translate special characters
        $code = \strtr($code, \chr(128) . \chr(131) . \chr(129) . \chr(130), '<=>?');
        $clen = \strlen($code);
        // calculate check digit C
        $pck = 1;
        $check = 0;
        for ($idx = $clen - 1; $idx >= 0; --$idx) {
            $check += $this->getChecksumIndex($code[$idx]) * $pck;
            ++$pck;
            if ($pck > 20) {
                $pck = 1;
            }
        }

        $check %= 47;
        $chk = $this->getChecksumChar($check);
        $code .= $chk;
        // calculate check digit K
        $pck = 1;
        $check = 0;
        for ($idx = $clen; $idx >= 0; --$idx) {
            $check += $this->getChecksumIndex($code[$idx]) * $pck;
            ++$pck;
            if ($pck > 15) {
                $pck = 1;
            }
        }

        $check %= 47;
        $key = $this->getChecksumChar($check);
        $checksum = $chk . $key;
        // restore special characters
        return \strtr($checksum, '<=>?', \chr(128) . \chr(131) . \chr(129) . \chr(130));
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     *
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     */
    protected function setBars(): void
    {
        $this->ncols = 0;
        $this->nrows = 1;
        $this->bars = [];
        $this->formatCode();
        $clen = \strlen($this->extcode);
        for ($chr = 0; $chr < $clen; ++$chr) {
            $char = \ord($this->extcode[$chr]);
            $pattern = $this->getBarPattern($char);
            for ($pos = 0; $pos < 6; ++$pos) {
                $bar_width = (int) ($pattern[$pos] ?? '0');
                if (($pos % 2) === 0) {
                    $this->bars[] = [$this->ncols, 0, $bar_width, 1];
                }

                $this->ncols += $bar_width;
            }
        }

        $this->bars[] = [$this->ncols, 0, 1, 1];
        ++$this->ncols;
    }
}
