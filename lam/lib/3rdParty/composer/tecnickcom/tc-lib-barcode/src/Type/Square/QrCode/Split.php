<?php

declare(strict_types=1);

/**
 * Split.php
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
 * Com\Tecnick\Barcode\Type\Square\QrCode\Split
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * @phpstan-import-type Item from \Com\Tecnick\Barcode\Type\Square\QrCode\Estimate
 */
class Split
{
    protected function getEncMode(string $key): int
    {
        return Data::ENC_MODES[$key] ?? 0;
    }

    /**
     * Input items
     *
     * @var array<int, Item>
     */
    protected array $items = [];

    /**
     * Initialize
     *
     * @param ByteStream $encodingMode ByteStream Class object
     * @param int          $hint    Encoding mode
     * @param int          $version Code version
     */
    public function __construct(
        /**
         * EncodingMode class object
         */
        protected EncodingMode $encodingMode,
        /**
         * Encoding mode
         */
        protected int $hint,
        /**
         * QR code version.
         * The Size of QRcode is defined as version. Version is an integer value from 1 to 40.
         * Version 1 is 21*21 matrix. And 4 modules increases whenever 1 version increases.
         * So version 40 is 177*177 matrix.
         */
        protected int $version,
    ) {}

    /**
     * Split the input string
     *
     * @param string $data Data
     *
     * @return array<int, Item> items
     *
     * @throws BarcodeException in case input data cannot be split
     */
    public function getSplittedString(string $data): array
    {
        $modeNm = $this->getEncMode('NM');
        $modeAn = $this->getEncMode('AN');
        $modeKj = $this->getEncMode('KJ');
        while (\strlen($data) > 0) {
            $mode = $this->encodingMode->getEncodingMode($data, 0);
            switch ($mode) {
                case $modeNm:
                    $length = $this->eatNum($data);
                    break;
                case $modeAn:
                    $length = $this->eatAn($data);
                    break;
                case $modeKj:
                    if ($this->hint === $modeKj) {
                        $length = $this->eatKanji($data);
                        break;
                    }

                    $length = $this->eat8($data);
                    break;
                default:
                    $length = $this->eat8($data);
                    break;
            }

            if ($length === 0) {
                break;
            }

            if ($length < 0) {
                throw new BarcodeException('Error while splitting the input data');
            }

            $data = \substr($data, $length);
        }

        return $this->items;
    }

    /**
     * eatNum
     *
     * @param string $data Data
     *
     * @return int run
     *
     * @throws BarcodeException in case of invalid split input item
     */
    protected function eatNum(string $data): int
    {
        $modeNm = $this->getEncMode('NM');
        $modeAn = $this->getEncMode('AN');
        $mode8b = $this->getEncMode('8B');
        $lng = $this->encodingMode->getLengthIndicator($modeNm, $this->version);
        $pos = 0;
        while ($this->encodingMode->isDigitAt($data, $pos)) {
            ++$pos;
        }

        $mode = $this->encodingMode->getEncodingMode($data, $pos);
        if ($mode === $mode8b) {
            $dif =
                $this->encodingMode->estimateBitsModeNum($pos) + 4 + $lng + $this->encodingMode->estimateBitsMode8(1) // + 4 + l8
                - $this->encodingMode->estimateBitsMode8($pos + 1); // - 4 - l8
            if ($dif > 0) {
                return $this->eat8($data);
            }
        }

        if ($mode === $modeAn) {
            $dif =
                $this->encodingMode->estimateBitsModeNum($pos) + 4 + $lng + $this->encodingMode->estimateBitsModeAn(1) // + 4 + la
                - $this->encodingMode->estimateBitsModeAn($pos + 1); // - 4 - la
            if ($dif > 0) {
                return $this->eatAn($data);
            }
        }

        $this->items = $this->encodingMode->appendNewInputItem($this->items, $modeNm, $pos, \str_split($data));
        return $pos;
    }

    /**
     * eatAn
     *
     * @param string $data Data
     *
     * @return int run
     *
     * @throws BarcodeException in case of invalid split input item
     */
    protected function eatAn(string $data): int
    {
        $modeAn = $this->getEncMode('AN');
        $modeNm = $this->getEncMode('NM');
        $lag = $this->encodingMode->getLengthIndicator($modeAn, $this->version);
        $lng = $this->encodingMode->getLengthIndicator($modeNm, $this->version);
        $pos = 1;
        while ($this->encodingMode->isAlphanumericAt($data, $pos)) {
            if ($this->encodingMode->isDigitAt($data, $pos)) {
                $qix = $pos;
                while ($this->encodingMode->isDigitAt($data, $qix)) {
                    ++$qix;
                }

                $dif =
                    $this->encodingMode->estimateBitsModeAn($pos) // + 4 + lag
                        + $this->encodingMode->estimateBitsModeNum($qix - $pos)
                        + 4
                        + $lng
                    - $this->encodingMode->estimateBitsModeAn($qix); // - 4 - la
                if ($dif < 0) {
                    break;
                }

                $pos = $qix;
                continue;
            }

            if (!$this->encodingMode->isDigitAt($data, $pos)) {
                ++$pos;
            }
        }

        if (!$this->encodingMode->isAlphanumericAt($data, $pos)) {
            $dif =
                $this->encodingMode->estimateBitsModeAn($pos) + 4 + $lag + $this->encodingMode->estimateBitsMode8(1) // + 4 + l8
                - $this->encodingMode->estimateBitsMode8($pos + 1); // - 4 - l8
            if ($dif > 0) {
                return $this->eat8($data);
            }
        }

        $this->items = $this->encodingMode->appendNewInputItem($this->items, $modeAn, $pos, \str_split($data));
        return $pos;
    }

    /**
     * eatKanji
     *
     * @param string $data Data
     *
     * @return int run
     *
     * @throws BarcodeException in case of invalid split input item
     */
    protected function eatKanji(string $data): int
    {
        $modeKj = $this->getEncMode('KJ');
        $pos = 0;
        while ($this->encodingMode->getEncodingMode($data, $pos) === $modeKj) {
            $pos += 2;
        }

        $this->items = $this->encodingMode->appendNewInputItem($this->items, $modeKj, $pos, \str_split($data));
        return $pos;
    }

    /**
     * eat8
     *
     * @param string $data Data
     *
     * @return int run
     *
     * @throws BarcodeException in case of invalid split input item
     */
    protected function eat8(string $data): int
    {
        $modeAn = $this->getEncMode('AN');
        $modeNm = $this->getEncMode('NM');
        $modeKj = $this->getEncMode('KJ');
        $mode8b = $this->getEncMode('8B');
        $lag = $this->encodingMode->getLengthIndicator($modeAn, $this->version);
        $lng = $this->encodingMode->getLengthIndicator($modeNm, $this->version);
        $pos = 1;
        $dataStrLen = \strlen($data);
        while ($pos < $dataStrLen) {
            $mode = $this->encodingMode->getEncodingMode($data, $pos);
            if ($mode === $modeKj) {
                break;
            }

            if ($mode === $modeNm) {
                $qix = $pos;
                while ($this->encodingMode->isDigitAt($data, $qix)) {
                    ++$qix;
                }

                $dif =
                    $this->encodingMode->estimateBitsMode8($pos) // + 4 + l8
                        + $this->encodingMode->estimateBitsModeNum($qix - $pos)
                        + 4
                        + $lng
                    - $this->encodingMode->estimateBitsMode8($qix); // - 4 - l8
                if ($dif < 0) {
                    break;
                }

                $pos = $qix;
                continue;
            }

            if ($mode === $modeAn) {
                $qix = $pos;
                while ($this->encodingMode->isAlphanumericAt($data, $qix)) {
                    ++$qix;
                }

                $dif =
                    $this->encodingMode->estimateBitsMode8($pos) // + 4 + l8
                        + $this->encodingMode->estimateBitsModeAn($qix - $pos)
                        + 4
                        + $lag
                    - $this->encodingMode->estimateBitsMode8($qix); // - 4 - l8
                if ($dif < 0) {
                    break;
                }

                $pos = $qix;
                continue;
            }

            if ($mode !== $modeNm && $mode !== $modeAn) {
                ++$pos;
            }
        }

        $this->items = $this->encodingMode->appendNewInputItem($this->items, $mode8b, $pos, \str_split($data));
        return $pos;
    }
}
