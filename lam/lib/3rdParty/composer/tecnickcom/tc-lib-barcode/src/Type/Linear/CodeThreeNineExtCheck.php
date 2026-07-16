<?php

declare(strict_types=1);

/**
 * CodeThreeNineExtCheck.php
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
 * Com\Tecnick\Barcode\Type\Linear\CodeThreeNineExtCheck
 *
 * CodeThreeNineExtCheck Barcode type class
 * CODE 39 EXTENDED + CHECKSUM
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class CodeThreeNineExtCheck extends \Com\Tecnick\Barcode\Type\Linear
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'C39E+';

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '111331311',
        '1' => '311311113',
        '2' => '113311113',
        '3' => '313311111',
        '4' => '111331113',
        '5' => '311331111',
        '6' => '113331111',
        '7' => '111311313',
        '8' => '311311311',
        '9' => '113311311',
        'A' => '311113113',
        'B' => '113113113',
        'C' => '313113111',
        'D' => '111133113',
        'E' => '311133111',
        'F' => '113133111',
        'G' => '111113313',
        'H' => '311113311',
        'I' => '113113311',
        'J' => '111133311',
        'K' => '311111133',
        'L' => '113111133',
        'M' => '313111131',
        'N' => '111131133',
        'O' => '311131131',
        'P' => '113131131',
        'Q' => '111111333',
        'R' => '311111331',
        'S' => '113111331',
        'T' => '111131331',
        'U' => '331111113',
        'V' => '133111113',
        'W' => '333111111',
        'X' => '131131113',
        'Y' => '331131111',
        'Z' => '133131111',
        '-' => '131111313',
        '.' => '331111311',
        ' ' => '133111311',
        '$' => '131313111',
        '/' => '131311131',
        '+' => '131113131',
        '%' => '111313131',
        '*' => '131131311',
    ];

    /**
     * Map for extended characters
     *
     * @var array<string>
     */
    protected const EXTCODES = [
        '%U',
        '$A',
        '$B',
        '$C',
        '$D',
        '$E',
        '$F',
        '$G',
        '$H',
        '$I',
        '$J',
        '$K',
        '$L',
        '$M',
        '$N',
        '$O',
        '$P',
        '$Q',
        '$R',
        '$S',
        '$T',
        '$U',
        '$V',
        '$W',
        '$X',
        '$Y',
        '$Z',
        '%A',
        '%B',
        '%C',
        '%D',
        '%E',
        ' ',
        '/A',
        '/B',
        '/C',
        '/D',
        '/E',
        '/F',
        '/G',
        '/H',
        '/I',
        '/J',
        '/K',
        '/L',
        '-',
        '.',
        '/O',
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
        '/Z',
        '%F',
        '%G',
        '%H',
        '%I',
        '%J',
        '%V',
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
        '%K',
        '%L',
        '%M',
        '%N',
        '%O',
        '%W',
        '+A',
        '+B',
        '+C',
        '+D',
        '+E',
        '+F',
        '+G',
        '+H',
        '+I',
        '+J',
        '+K',
        '+L',
        '+M',
        '+N',
        '+O',
        '+P',
        '+Q',
        '+R',
        '+S',
        '+T',
        '+U',
        '+V',
        '+W',
        '+X',
        '+Y',
        '+Z',
        '%P',
        '%Q',
        '%R',
        '%S',
        '%T',
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
    ];

    protected function getExtendedCodeValue(int $item): string
    {
        return $this::EXTCODES[$item] ?? '';
    }

    protected function getChecksumIndex(string $char): int
    {
        $index = \array_search($char, $this::CHKSUM, true);
        if (\is_int($index)) {
            return $index;
        }

        return 0;
    }

    protected function getChecksumChar(int $index): string
    {
        return $this::CHKSUM[$index] ?? '';
    }

    /**
     * Encode a string to be used for CODE 39 Extended mode.
     *
     * @param string $code Code to extend
     *
     * @throws BarcodeException in case of error
     */
    protected function getExtendCode(string $code): string
    {
        $ext = '';
        $clen = \strlen($code);
        for ($chr = 0; $chr < $clen; ++$chr) {
            $item = \ord($code[$chr]);
            if ($item > 127) {
                throw new BarcodeException('Invalid character: ' . ($item & 0xFF));
            }

            $ext .= $this->getExtendedCodeValue($item);
        }

        return $ext;
    }

    /**
     * Calculate CODE 39 checksum (modulo 43).
     *
     * @param string $code Code to represent.
     *
     * @return string char checksum.
     */
    protected function getChecksum(string $code): string
    {
        $sum = 0;
        $clen = \strlen($code);
        for ($chr = 0; $chr < $clen; ++$chr) {
            $sum += $this->getChecksumIndex($code[$chr]);
        }

        $idx = $sum % 43;
        return $this->getChecksumChar($idx);
    }

    /**
     * Format code
     *
     * @throws BarcodeException in case of error
     */
    protected function formatCode(): void
    {
        $code = $this->getExtendCode(\strtoupper($this->code));
        $this->extcode = '*' . $code . $this->getChecksum($code) . '*';
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->ncols = 0;
        $this->nrows = 1;
        $this->bars = [];
        $this->formatCode();
        $clen = \strlen($this->extcode);
        for ($chr = 0; $chr < $clen; ++$chr) {
            $char = $this->extcode[$chr];
            $pattern = $this::CHBAR[$char] ?? null;
            if ($pattern === null) {
                throw new BarcodeException('Invalid character: ' . (\ord($char) & 0xFF));
            }

            for ($pos = 0; $pos < 9; ++$pos) {
                $bar_width = (int) ($pattern[$pos] ?? '0');
                if (($pos % 2) === 0 && $bar_width > 0) {
                    $this->bars[] = [$this->ncols, 0, $bar_width, 1];
                }

                $this->ncols += $bar_width;
            }

            // intercharacter gap
            ++$this->ncols;
        }

        --$this->ncols;
    }
}
