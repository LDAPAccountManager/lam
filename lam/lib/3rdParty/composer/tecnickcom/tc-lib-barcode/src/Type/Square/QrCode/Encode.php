<?php

declare(strict_types=1);

/**
 * Encode.php
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
 * Com\Tecnick\Barcode\Type\Square\QrCode\Encode
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Encode extends \Com\Tecnick\Barcode\Type\Square\QrCode\EncodingMode
{
    protected function getEncModeValue(string $mode): int
    {
        return Data::ENC_MODES[$mode] ?? 0;
    }

    /**
     * @param array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } $inputitem
     */
    protected function getDataOrd(array $inputitem, int $idx): int
    {
        return \ord($inputitem['data'][$idx] ?? "\x00");
    }

    /**
     * encode Mode Num
     *
     * @param array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } $inputitem input item
     * @param int $version Code version
     *
     * @return array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } input item
     */
    protected function encodeModeNum(array $inputitem, int $version): array
    {
        $words = (int) ($inputitem['size'] / 3);
        $inputitem['bstream'] = [];
        $val = 0x1;
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, $val);
        $inputitem['bstream'] = $this->appendNum(
            $inputitem['bstream'],
            $this->getLengthIndicator($this->getEncModeValue('NM'), $version),
            $inputitem['size'],
        );
        for ($i = 0; $i < $words; ++$i) {
            $val = ($this->getDataOrd($inputitem, $i * 3) - \ord('0')) * 100;
            $val += ($this->getDataOrd($inputitem, ($i * 3) + 1) - \ord('0')) * 10;
            $val += $this->getDataOrd($inputitem, ($i * 3) + 2) - \ord('0');
            $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 10, $val);
        }

        $remaining = $inputitem['size'] - ($words * 3);
        if ($remaining === 1) {
            $val = $this->getDataOrd($inputitem, $words * 3) - \ord('0');
            $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, $val);
            return $inputitem;
        }

        if ($remaining === 2) {
            $val = ($this->getDataOrd($inputitem, $words * 3) - \ord('0')) * 10;
            $val += $this->getDataOrd($inputitem, ($words * 3) + 1) - \ord('0');
            $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 7, $val);
        }

        return $inputitem;
    }

    /**
     * encode Mode An
     *
     * @param array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } $inputitem input item
     * @param int $version Code version
     *
     * @return array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } input item
     */
    protected function encodeModeAn(array $inputitem, int $version): array
    {
        $words = (int) ($inputitem['size'] / 2);
        $inputitem['bstream'] = [];
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, 0x02);
        $inputitem['bstream'] = $this->appendNum(
            $inputitem['bstream'],
            $this->getLengthIndicator($this->getEncModeValue('AN'), $version),
            $inputitem['size'],
        );
        for ($idx = 0; $idx < $words; ++$idx) {
            $val = $this->lookAnTable($this->getDataOrd($inputitem, $idx * 2)) * 45;
            $val += $this->lookAnTable($this->getDataOrd($inputitem, ($idx * 2) + 1));
            $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 11, $val);
        }

        if (($inputitem['size'] & 1) !== 0) {
            $val = $this->lookAnTable($this->getDataOrd($inputitem, $words * 2));
            $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 6, $val);
        }

        return $inputitem;
    }

    /**
     * encode Mode 8
     *
     * @param array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } $inputitem input item
     * @param int $version Code version
     *
     * @return array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } input item
     */
    protected function encodeMode8(array $inputitem, int $version): array
    {
        $inputitem['bstream'] = [];
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, 0x4);
        $inputitem['bstream'] = $this->appendNum(
            $inputitem['bstream'],
            $this->getLengthIndicator($this->getEncModeValue('8B'), $version),
            $inputitem['size'],
        );
        for ($idx = 0; $idx < $inputitem['size']; ++$idx) {
            $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 8, $this->getDataOrd($inputitem, $idx));
        }

        return $inputitem;
    }

    /**
     * encode Mode Kanji
     *
     * @param array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } $inputitem input item
     * @param int $version Code version
     *
     * @return array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } input item
     */
    protected function encodeModeKanji(array $inputitem, int $version): array
    {
        $inputitem['bstream'] = [];
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, 0x8);
        $inputitem['bstream'] = $this->appendNum(
            $inputitem['bstream'],
            $this->getLengthIndicator($this->getEncModeValue('KJ'), $version),
            (int) ($inputitem['size'] / 2),
        );
        for ($idx = 0; $idx < $inputitem['size']; $idx += 2) {
            $val = ($this->getDataOrd($inputitem, $idx) << 8) | $this->getDataOrd($inputitem, $idx + 1);
            $valOffset = 0xc140;
            if ($val <= 0x9ffc) {
                $valOffset = 0x8140;
            }

            $val -= $valOffset;

            $val = ($val & 0xff) + (($val >> 8) * 0xc0);
            $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 13, $val);
        }

        return $inputitem;
    }

    /**
     * encode Mode Structure
     *
     * @param array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } $inputitem input item
     *
     * @return array{
     *             'mode': int,
     *             'size': int,
     *             'data': array<int, string>,
     *             'bstream': array<int, int>,
     *         } input item
     */
    protected function encodeModeStructure(array $inputitem): array
    {
        $inputitem['bstream'] = [];
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, 0x03);
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, $this->getDataOrd($inputitem, 1) - 1);
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 4, $this->getDataOrd($inputitem, 0) - 1);
        $inputitem['bstream'] = $this->appendNum($inputitem['bstream'], 8, $this->getDataOrd($inputitem, 2));
        return $inputitem;
    }
}
