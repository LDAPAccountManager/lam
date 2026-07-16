<?php

/**
 * LzwProxy.php
 *
 * @since     2026-04-19
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Test;

class LzwProxy extends \Com\Tecnick\Pdf\Filter\Type\Lzw
{
    /**
     * @param array<int, string> $dictionary
     */
    public function callProcess(
        string &$decoded,
        string &$bitstring,
        int &$bitlen,
        int &$data_length,
        int &$index,
        array &$dictionary,
        int &$dix,
        int &$prev_index,
    ): void {
        // remove read bits from string
        $bitstring = \substr($bitstring, $bitlen);
        // update number of bits
        $data_length -= $bitlen;
        if ($index === 256) {
            $bitlen = 9;
            $dix = 258;
            $prev_index = 256;
            $dictionary = [];
            for ($i = 0; $i < 256; ++$i) {
                $dictionary[$i] = \chr($i);
            }

            return;
        }

        if ($prev_index === 256) {
            $decoded .= $dictionary[$index] ?? '';
            $prev_index = $index;

            return;
        }

        if ($index < $dix) {
            $current = $dictionary[$index] ?? '';
            $previous = $dictionary[$prev_index] ?? '';
            $decoded .= $current;
            $dic_val = $previous . ($current[0] ?? '');
            $prev_index = $index;
        } else {
            $previous = $dictionary[$prev_index] ?? '';
            $dic_val = $previous . ($previous[0] ?? '');
            $decoded .= $dic_val;
        }

        $dictionary[$dix] = $dic_val;
        ++$dix;
        if ($dix === 2047) {
            $bitlen = 12;
            return;
        }

        if ($dix === 1023) {
            $bitlen = 11;
            return;
        }

        if ($dix === 511) {
            $bitlen = 10;
        }
    }
}
