<?php

declare(strict_types=1);

/**
 * Encode.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\Datamatrix;

/**
 * Com\Tecnick\Barcode\Type\Square\Datamatrix\Encode
 *
 * Datamatrix Barcode type class
 * DATAMATRIX (ISO/IEC 16022)
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Encode extends \Com\Tecnick\Barcode\Type\Square\Datamatrix\EncodeTxt
{
    /**
     * @param array<int, int> $temp_cw
     */
    protected function getTempCodeword(array $temp_cw, int $index): int
    {
        return $temp_cw[$index] ?? 0;
    }

    /**
     * Initialize a new encode object
     *
     * @param string $shape Datamatrix shape key (S=square, R=rectangular)
     */
    public function __construct(string $shape = 'S')
    {
        $this->shape = $shape;
    }

    /**
     * Encode ASCII
     *
     * @param array<int, int>  $cdw         Codewords array
     * @param int    $cdw_num     Codewords number
     * @param int    $pos         Current position
     * @param int    $data_length Data length
     * @param string $data        Data string
     * @param int    $enc         Current encoding
     */
    public function encodeASCII(
        array &$cdw,
        int &$cdw_num,
        int &$pos,
        int &$data_length,
        string &$data,
        int &$enc,
    ): void {
        if (
            $data_length > 1
            && $pos < ($data_length - 1)
            && (
                $this->isCharMode(\ord($data[$pos]), Data::ENC_ASCII_NUM)
                && $this->isCharMode(\ord($data[$pos + 1]), Data::ENC_ASCII_NUM)
            )
        ) {
            // 1. If the next data sequence is at least 2 consecutive digits,
            // encode the next two digits as a double digit in ASCII mode.
            $cdw[] = (int) \substr($data, $pos, 2) + 130;
            ++$cdw_num;
            $pos += 2;
            return;
        }

        // 2. If the look-ahead test (starting at step J) indicates another mode, switch to that mode.
        $newenc = $this->lookAheadTest($data, $pos, $enc);
        if ($newenc !== $enc) {
            // switch to new encoding
            $enc = $newenc;
            $cdw[] = $this->getSwitchEncodingCodeword($enc);
            ++$cdw_num;
            return;
        }

        // get new byte
        $chr = \ord($data[$pos]);
        ++$pos;
        if ($this->isCharMode($chr, Data::ENC_ASCII_EXT)) {
            // 3. If the next data character is extended ASCII (greater than 127)
            // encode it in ASCII mode first using the Upper Shift (value 235) character.
            $cdw[] = 235;
            $cdw[] = $chr - 127;
            $cdw_num += 2;
            return;
        }

        // 4. Otherwise process the next data character in ASCII encodation.
        $cdw[] = $chr + 1;
        ++$cdw_num;
    }

    /**
     * Encode EDF4
     *
     * @param int    $epos         Current position
     * @param array<int, int>  $cdw          Codewords array
     * @param int    $cdw_num      Codewords number
     * @param int    $pos          Current position
     * @param int    $data_length  Data length
     * @param int    $field_length Field length
     * @param int    $enc          Current encoding
     * @param array<int, int>  $temp_cw      Temporary codewords array
     *
     * @return bool true to break the loop
     *
     * @throws \Com\Tecnick\Barcode\Exception in case of error
     */
    public function encodeEDFfour(
        int $epos,
        array &$cdw,
        int &$cdw_num,
        int &$pos,
        int &$data_length,
        int &$field_length,
        int &$enc,
        array &$temp_cw,
    ): bool {
        if ($epos === $data_length) {
            $enc = Data::ENC_ASCII;
            $params = Data::getPaddingSize($this->shape, $cdw_num + $field_length);
            if (($params[11] - $cdw_num) > 2) {
                $cdw[] = $this->getSwitchEncodingCodeword($enc);
                ++$cdw_num;
            }

            return true;
        }

        if ($field_length < 4) {
            $enc = Data::ENC_ASCII;
            $this->last_enc = $enc;
            $params = Data::getPaddingSize($this->shape, $cdw_num + $field_length + ($data_length - $epos));
            if (($params[11] - $cdw_num) <= 2) {
                return true;
            }

            // set unlatch character
            $temp_cw[] = 0x1f;
            ++$field_length;
            // fill empty characters
            for ($i = $field_length; $i < 4; ++$i) {
                $temp_cw[] = 0;
            }
        }

        // encodes four data characters in three codewords
        $cw0 = $this->getTempCodeword($temp_cw, 0);
        $cw1 = $this->getTempCodeword($temp_cw, 1);
        $cw2 = $this->getTempCodeword($temp_cw, 2);
        $cw3 = $this->getTempCodeword($temp_cw, 3);

        $cdw[] = (($cw0 & 0x3F) << 2) + (($cw1 & 0x30) >> 4);
        ++$cdw_num;
        if ($field_length > 1) {
            $cdw[] = (($cw1 & 0x0F) << 4) + (($cw2 & 0x3C) >> 2);
            ++$cdw_num;
        }

        if ($field_length > 2) {
            $cdw[] = (($cw2 & 0x03) << 6) + ($cw3 & 0x3F);
            ++$cdw_num;
        }

        $temp_cw = [];
        $pos = $epos;
        $field_length = 0;
        if ($enc === Data::ENC_ASCII) {
            return true; // exit from EDIFACT mode
        }

        return false;
    }

    /**
     * Encode EDF
     *
     * @param array<int, int>  $cdw          Codewords array
     * @param int    $cdw_num      Codewords number
     * @param int    $pos          Current position
     * @param int    $data_length  Data length
     * @param int    $field_length Field length
     * @param string $data         Data string
     * @param int    $enc          Current encoding
     */
    public function encodeEDF(
        array &$cdw,
        int &$cdw_num,
        int &$pos,
        int &$data_length,
        int &$field_length,
        string &$data,
        int &$enc,
    ): void {
        // initialize temporary array with 0 length
        $temp_cw = [];
        $epos = $pos;
        $field_length = 0;
        do {
            // 2. process the next character in EDIFACT encodation.
            $chr = \ord($data[$epos]);
            if ($this->isCharMode($chr, Data::ENC_EDF)) {
                ++$epos;
                $temp_cw[] = $chr;
                ++$field_length;
            }

            if (
                ($field_length === 4 || $epos === $data_length || !$this->isCharMode($chr, Data::ENC_EDF))
                && $this->encodeEDFfour($epos, $cdw, $cdw_num, $pos, $data_length, $field_length, $enc, $temp_cw)
            ) {
                break;
            }
        } while ($epos < $data_length);
    }

    /**
     * Encode Base256
     *
     * @param array<int, int>  $cdw          Codewords array
     * @param int    $cdw_num      Codewords number
     * @param int    $pos          Current position
     * @param int    $data_length  Data length
     * @param int    $field_length Field length
     * @param string $data         Data string
     * @param int    $enc          Current encoding
     */
    public function encodeBase256(
        array &$cdw,
        int &$cdw_num,
        int &$pos,
        int &$data_length,
        int &$field_length,
        string &$data,
        int &$enc,
    ): void {
        // initialize temporary array with 0 length
        $temp_cw = [];
        $field_length = 0;
        while ($pos < $data_length && $field_length <= 1555) {
            $newenc = $this->lookAheadTest($data, $pos, $enc);
            if ($newenc !== $enc) {
                // 1. If the look-ahead test (starting at step J)
                // indicates another mode, switch to that mode.
                $enc = $newenc;
                break; // exit from B256 mode
            }

            // 2. Otherwise, process the next character in Base 256 encodation.
            $chr = \ord($data[$pos]);
            ++$pos;
            $temp_cw[] = $chr;
            ++$field_length;
        }

        // set field length
        if ($field_length <= 249) {
            $cdw[] = $this->get255StateCodeword($field_length, $cdw_num + 1);
            ++$cdw_num;
        }

        if ($field_length > 249) {
            $cdw[] = $this->get255StateCodeword((int) \floor($field_length / 250) + 249, $cdw_num + 1);
            $cdw[] = $this->get255StateCodeword($field_length % 250, $cdw_num + 2);
            $cdw_num += 2;
        }

        // add B256 field
        foreach ($temp_cw as $cht) {
            $cdw[] = $this->get255StateCodeword($cht, $cdw_num + 1);
            ++$cdw_num;
        }
    }
}
