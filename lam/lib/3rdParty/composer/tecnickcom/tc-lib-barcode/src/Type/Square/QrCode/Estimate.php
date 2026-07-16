<?php

declare(strict_types=1);

/**
 * Estimate.php
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

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Estimate
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * @phpstan-type Item array{
 *            'mode': int,
 *            'size': int,
 *            'data': array<int, string>,
 *            'bstream': array<int, int>,
 *        }
 */
abstract class Estimate
{
    protected function getEncModeValue(string $mode): int
    {
        return match ($mode) {
            'NL' => Data::MODE_NL,
            'NM' => Data::MODE_NM,
            'AN' => Data::MODE_AN,
            '8B' => Data::MODE_8B,
            'KJ' => Data::MODE_KJ,
            'ST' => Data::MODE_ST,
            default => 0,
        };
    }

    protected function getLenTableBitsValue(int $mode, int $len): int
    {
        return Data::LEN_TABLE_BITS[$mode][$len] ?? 0;
    }

    protected function getCapacityWordsValue(int $version): int
    {
        $capacity = Data::CAPACITY[$version] ?? null;
        if (!\is_array($capacity)) {
            return 0;
        }

        $words = $capacity[Data::QRCAP_WORDS] ?? 0;
        return \is_int($words) ? $words : 0;
    }

    protected function getCapacityEcValue(int $version, int $level): int
    {
        $capacity = Data::CAPACITY[$version] ?? null;
        if (!\is_array($capacity)) {
            return 0;
        }

        $ecLevel = $capacity[Data::QRCAP_EC] ?? [];
        if (!\is_array($ecLevel)) {
            return 0;
        }

        return $ecLevel[$level] ?? 0;
    }

    /**
     * Encoding mode
     */
    protected int $hint = 2;

    /**
     * QR code version.
     * The Size of QRcode is defined as version. Version is an integer value from 1 to 40.
     * Version 1 is 21*21 matrix. And 4 modules increases whenever 1 version increases.
     * So version 40 is 177*177 matrix.
     */
    public int $version = 0;

    /**
     * Error correction level
     */
    protected int $level = 0;

    /**
     * Return the size of length indicator for the mode and version
     *
     * @param int $mode    Encoding mode
     * @param int $version Version
     *
     * @return int the size of the appropriate length indicator (bits).
     */
    public function getLengthIndicator(int $mode, int $version): int
    {
        $modeSt = $this->getEncModeValue('ST');
        $modeNl = $this->getEncModeValue('NL');
        if ($mode === $modeSt || $mode < $modeNl || $mode > $modeSt) {
            return 0;
        }

        $len = match (true) {
            $version <= 9 => 0,
            $version <= 26 => 1,
            default => 2,
        };

        return $this->getLenTableBitsValue($mode, $len);
    }

    /**
     * estimateBitsModeNum
     *
     * @return int number of bits
     */
    public function estimateBitsModeNum(int $size): int
    {
        $wdt = (int) ($size / 3);
        $bits = $wdt * 10;
        match ($size - ($wdt * 3)) {
            1 => $bits += 4,
            2 => $bits += 7,
            default => $bits,
        };
        return $bits;
    }

    /**
     * estimateBitsModeAn
     *
     * @return int number of bits
     */
    public function estimateBitsModeAn(int $size): int
    {
        $bits = (int) ($size * 5.5); // (size / 2 ) * 11
        if (($size & 1) !== 0) {
            $bits += 6;
        }

        return $bits;
    }

    /**
     * estimateBitsMode8
     *
     * @return int number of bits
     */
    public function estimateBitsMode8(int $size): int
    {
        return $size * 8;
    }

    /**
     * estimateBitsModeKanji
     *
     * @return int number of bits
     */
    public function estimateBitsModeKanji(int $size): int
    {
        return (int) ($size * 6.5); // (size / 2 ) * 13
    }

    /**
     * Estimate version
     *
     * @param array<int, Item> $items Items
     * @param int $level Error correction level
     *
     * @return int version
     *
     * @throws BarcodeException
     */
    public function estimateVersion(array $items, int $level): int
    {
        $version = 0;
        $prev = 0;
        do {
            $prev = $version;
            $bits = $this->estimateBitStreamSize($items, $prev);
            $version = $this->getMinimumVersion((int) (($bits + 7) / 8), $level);
            if ($version < 0) {
                return -1;
            }
        } while ($version > $prev);

        return $version;
    }

    /**
     * Return a version number that satisfies the input code length.
     *
     * @param int $size  Input code length (bytes)
     * @param int $level Error correction level
     *
     * @return int version number
     *
     * @throws BarcodeException
     */
    protected function getMinimumVersion(int $size, int $level): int
    {
        for ($idx = 1; $idx <= Data::QRSPEC_VERSION_MAX; ++$idx) {
            $words = $this->getCapacityWordsValue($idx) - $this->getCapacityEcValue($idx, $level);
            if ($words >= $size) {
                return $idx;
            }
        }

        throw new BarcodeException(
            'The size of input data is greater than Data::QR capacity, try to lower the error correction mode',
        );
    }

    /**
     * estimateBitStreamSize
     *
     * @param array<int, Item> $items Items
     * @param int $version Code version
     *
     * @return int bits
     */
    protected function estimateBitStreamSize(array $items, int $version): int
    {
        $bits = 0;
        if ($version === 0) {
            $version = 1;
        }

        foreach ($items as $item) {
            switch ($item['mode']) {
                case $this->getEncModeValue('NM'):
                    $bits = $this->estimateBitsModeNum($item['size']);
                    break;
                case $this->getEncModeValue('AN'):
                    $bits = $this->estimateBitsModeAn($item['size']);
                    break;
                case $this->getEncModeValue('8B'):
                    $bits = $this->estimateBitsMode8($item['size']);
                    break;
                case $this->getEncModeValue('KJ'):
                    $bits = $this->estimateBitsModeKanji($item['size']);
                    break;
                case $this->getEncModeValue('ST'):
                    return Data::STRUCTURE_HEADER_BITS;
                default:
                    return 0;
            }

            $len = $this->getLengthIndicator($item['mode'], $version);
            $mod = 1 << $len;
            $num = (int) (($item['size'] + $mod - 1) / $mod);
            $bits += $num * (4 + $len);
        }

        return $bits;
    }
}
