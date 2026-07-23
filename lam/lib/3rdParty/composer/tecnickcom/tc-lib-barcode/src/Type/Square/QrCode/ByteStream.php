<?php

declare(strict_types=1);

/**
 * ByteStream.php
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
 * Com\Tecnick\Barcode\Type\Square\QrCode\ByteStream
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
class ByteStream extends \Com\Tecnick\Barcode\Type\Square\QrCode\Encode
{
    /**
     * @param array<int, int> $bstream
     */
    protected function getBitValue(array $bstream, int $pos): int
    {
        return $bstream[$pos] ?? 0;
    }

    /**
     * Initialize
     *
     * @param int $hint    Encoding mode
     * @param int $version Code version
     * @param int $level   Error Correction Level
     */
    public function __construct(int $hint, int $version, int $level)
    {
        $this->hint = $hint;
        $this->version = $version;
        $this->level = $level;
    }

    /**
     * Pack all bit streams padding bits into a byte array
     *
     * @param array<int, Item> $items Items
     *
     * @return array<int, int> padded merged byte stream
     *
     * @throws BarcodeException
     */
    public function getByteStream(array $items): array
    {
        return $this->bitstreamToByte($this->appendPaddingBit($this->mergeBitStream($items)));
    }

    /**
     * merge the bit stream
     *
     * @param array<int, Item> $items Items
     *
     * @return array<int, int> bitstream
     *
     * @throws BarcodeException
     */
    protected function mergeBitStream(array $items): array
    {
        $items = $this->convertData($items);
        $bstream = [];
        foreach ($items as $item) {
            $bstream = $this->appendBitstream($bstream, $item['bstream']);
        }

        return $bstream;
    }

    /**
     * Append Padding Bit to bitstream
     *
     * @param array<int, int> $bstream Bit stream
     *
     * @return array<int, int> bitstream
     */
    protected function appendPaddingBit(array $bstream): array
    {
        if ($bstream === []) {
            return [];
        }

        $bits = \count($bstream);
        $spec = new Spec();
        $maxwords = $spec->getDataLength($this->version, $this->level);
        $maxbits = $maxwords * 8;
        if ($maxbits === $bits) {
            return $bstream;
        }

        if (($maxbits - $bits) < 5) {
            return $this->appendNum($bstream, $maxbits - $bits, 0);
        }

        $bits += 4;
        $words = (int) (($bits + 7) / 8);
        $padding = [];
        $padding = $this->appendNum($padding, ($words * 8) - $bits + 4, 0);

        $padlen = $maxwords - $words;
        if ($padlen > 0) {
            $padbuf = [];
            for ($idx = 0; $idx < $padlen; ++$idx) {
                $padbuf[$idx] = ($idx & 1) !== 0 ? 0x11 : 0xec;
            }

            $padding = $this->appendBytes($padding, $padlen, $padbuf);
        }

        return $this->appendBitstream($bstream, $padding);
    }

    /**
     * Convert bitstream to bytes
     *
     * @param array<int, int> $bstream Original bitstream
     *
     * @return array<int, int> of bytes
     */
    protected function bitstreamToByte(array $bstream): array
    {
        $size = \count($bstream);
        if ($size === 0) {
            return [];
        }

        $data = \array_fill(0, \max(0, (int) (($size + 7) / 8)), 0);
        $bytes = (int) ($size / 8);
        $pos = 0;
        for ($idx = 0; $idx < $bytes; ++$idx) {
            $val = 0;
            for ($jdx = 0; $jdx < 8; ++$jdx) {
                $val <<= 1;
                $val |= $this->getBitValue($bstream, $pos);
                ++$pos;
            }

            $data[$idx] = $val;
        }

        if (($size & 7) !== 0) {
            $val = 0;
            for ($jdx = 0; $jdx < ($size & 7); ++$jdx) {
                $val <<= 1;
                $val |= $this->getBitValue($bstream, $pos);
                ++$pos;
            }

            $data[$bytes] = $val;
        }

        return $data;
    }

    /**
     * convertData
     *
     * @param array<int, Item> $items Items
     *
     * @return array<int, Item>
     *
     * @throws BarcodeException
     */
    protected function convertData(array $items): array
    {
        $ver = $this->estimateVersion($items, $this->level);
        if ($ver > $this->version) {
            $this->version = $ver;
        }

        while (true) {
            $cbs = $this->createBitStream($items);
            $items = $cbs[0];
            $bits = $cbs[1];
            if ($bits < 0) {
                throw new BarcodeException('Negative Bits value');
            }

            $ver = $this->getMinimumVersion((int) (($bits + 7) / 8), $this->level);
            if ($ver > $this->version) {
                $this->version = $ver;
                continue;
            }

            break;
        }

        return $items;
    }

    /**
     * Create BitStream
     *
     * @param array<int, Item> $items Items
     *
     * @return array{
     *           0: array<int, Item>,
     *           1: int,
     *       }
     *
     * @throws BarcodeException
     */
    protected function createBitStream(array $items): array
    {
        $total = 0;
        foreach ($items as $key => $item) {
            $items[$key] = $this->encodeBitStream($item, $this->version);
            $bits = \count($items[$key]['bstream']);
            $total += $bits;
        }

        return [$items, $total];
    }

    /**
     * Encode BitStream
     *
     * @param Item $inputitem Input item
     * @param int $version Code version
     *
     * @return Item
     *
     * @throws BarcodeException
     */
    public function encodeBitStream(array $inputitem, int $version): array
    {
        $inputitem['bstream'] = [];
        $spec = new Spec();
        $words = $spec->maximumWords($inputitem['mode'], $version);

        if ($inputitem['size'] <= $words) {
            return match ($inputitem['mode']) {
                $this->getEncModeValue('NM') => $this->encodeModeNum($inputitem, $version),
                $this->getEncModeValue('AN') => $this->encodeModeAn($inputitem, $version),
                $this->getEncModeValue('8B') => $this->encodeMode8($inputitem, $version),
                $this->getEncModeValue('KJ') => $this->encodeModeKanji($inputitem, $version),
                $this->getEncModeValue('ST') => $this->encodeModeStructure($inputitem),
                default => throw new BarcodeException('Invalid mode'),
            };
        }

        $st1 = $this->newInputItem($inputitem['mode'], $words, $inputitem['data']);
        $st2 = $this->newInputItem(
            $inputitem['mode'],
            $inputitem['size'] - $words,
            \array_slice($inputitem['data'], $words),
        );
        $st1 = $this->encodeBitStream($st1, $version);
        $st2 = $this->encodeBitStream($st2, $version);
        $inputitem['bstream'] = [];
        $inputitem['bstream'] = $this->appendBitstream($inputitem['bstream'], $st1['bstream']);
        $inputitem['bstream'] = $this->appendBitstream($inputitem['bstream'], $st2['bstream']);

        return $inputitem;
    }
}
