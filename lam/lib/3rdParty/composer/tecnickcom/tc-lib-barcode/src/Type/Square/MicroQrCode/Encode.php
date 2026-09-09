<?php

declare(strict_types=1);

/**
 * Encode.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\MicroQrCode;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\ReedSolomon;

/**
 * Com\Tecnick\Barcode\Type\Square\MicroQrCode\Encode
 *
 * Symbol builder for the MicroQrCode Barcode type class
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Encode extends \Com\Tecnick\Barcode\Type\Square\MicroQrCode\Compaction
{
    /**
     * Symbol number, from 0 to 7, as in Table 13 of ISO/IEC 18004.
     */
    protected int $symbol = 0;

    /**
     * Symbol version, from 1 to 4, that is M1 to M4.
     */
    protected int $version = 1;

    /**
     * Symbol size in modules.
     */
    protected int $size = 11;

    /**
     * Encodation mode.
     */
    protected int $mode = 0;

    /**
     * Data mask pattern reference, from 0 to 3.
     */
    protected int $mask = 0;

    /**
     * Error correction level, empty for the version M1.
     */
    protected string $level = '';

    /**
     * Data capacity of the symbol in bits.
     */
    protected int $capacity = 0;

    /**
     * Number of error correction codewords of the symbol.
     */
    protected int $checkwords = 0;

    /**
     * Module matrix: 1 is a dark module, 0 a light one.
     *
     * @var array<int, array<int, int>>
     */
    protected array $matrix = [];

    /**
     * Modules taken by the function patterns and the format information.
     *
     * @var array<int, array<int, bool>>
     */
    protected array $function = [];

    /**
     * Build the symbol.
     *
     * @param string $code    Code to encode.
     * @param string $level   Error correction level (L, M or Q), or an empty
     *                        string to select the smallest symbol.
     * @param int    $version Symbol version, from 1 to 4, or 0 to select the
     *                        smallest symbol.
     * @param int    $mode    Encodation mode, or a negative value to select the
     *                        mode that yields the shortest bit stream.
     *
     * @throws BarcodeException in case of error
     */
    public function __construct(string $code, string $level, int $version, int $mode)
    {
        if ($mode < 0) {
            $mode = $this->detectMode($code);
        }

        $this->checkMode($code, $mode);
        $this->mode = $mode;
        $this->symbol = $this->getSymbolNumber($code, $level, $version);
        $symbol = Data::SYMBOLS[$this->symbol] ?? [];
        $this->version = $symbol[0] ?? 1;
        $this->level = $symbol[1] ?? '';
        $this->capacity = $symbol[3] ?? 20;
        $this->checkwords = $symbol[4] ?? 2;
        $this->size = Data::SIZE[$this->version] ?? 11;
        $sequence = $this->getBitSequence($code);
        $this->setFunctionPatterns();
        $this->setData($sequence);
        $this->setBestMask();
        $this->setFormatInformation();
    }

    /**
     * Returns the module matrix as an array of rows of binary digits.
     *
     * @return array<int, string>
     */
    public function getGrid(): array
    {
        $grid = [];
        foreach ($this->matrix as $row) {
            $grid[] = \implode('', $row);
        }

        return $grid;
    }

    /**
     * Returns the symbol version, from 1 to 4, that is M1 to M4.
     */
    public function getVersion(): int
    {
        return $this->version;
    }

    /**
     * Returns the error correction level, or an empty string for the version M1,
     * which carries error detection only.
     */
    public function getLevel(): string
    {
        return $this->level;
    }

    /**
     * Returns the data mask pattern reference, from 0 to 3.
     */
    public function getMask(): int
    {
        return $this->mask;
    }

    /**
     * Returns the smallest symbol number able to carry the code with the
     * requested error correction level and version.
     *
     * @param string $code    Code to encode.
     * @param string $level   Requested error correction level, or an empty string.
     * @param int    $version Requested version, or 0.
     *
     * @throws BarcodeException if no symbol can carry the code
     */
    protected function getSymbolNumber(string $code, string $level, int $version): int
    {
        $candidates = 0;
        foreach (Data::SYMBOLS as $number => $symbol) {
            if ($version !== 0 && $version !== $symbol[0]) {
                continue;
            }

            if ($level !== '' && $level !== $symbol[1]) {
                continue;
            }

            ++$candidates;
            if ($this->getRequiredBits($code, $symbol[0]) <= $symbol[3]) {
                return $number;
            }
        }

        throw new BarcodeException($this->getCapacityMessage($candidates, $level, $version));
    }

    /**
     * Returns the message of the exception thrown when no symbol carries the
     * code: whether Table 13 of ISO/IEC 18004 pairs the requested version and
     * error correction level at all, and which of the two narrowed the choice.
     *
     * @param int    $candidates Number of symbols the request left to choose from.
     * @param string $level      Requested error correction level, or an empty string.
     * @param int    $version    Requested version, or 0.
     */
    protected function getCapacityMessage(int $candidates, string $level, int $version): string
    {
        if ($candidates === 0) {
            return 'Table 13 of ISO/IEC 18004 has no symbol of the requested version and error correction level';
        }

        if ($version !== 0) {
            return 'The data does not fit in the requested symbol version';
        }

        if ($level !== '') {
            return 'The data does not fit in a Micro QR Code symbol, try a lower error correction level';
        }

        return 'The data does not fit in a Micro QR Code symbol';
    }

    /**
     * Returns the number of bits taken by the code in the given version,
     * including the mode and character count indicators, or a value beyond any
     * symbol capacity when the version does not support the encodation mode or
     * the character count.
     *
     * @param string $code    Code to encode.
     * @param int    $version Symbol version.
     */
    protected function getRequiredBits(string $code, int $version): int
    {
        $countBits = Data::COUNT_BITS[$version][$this->mode] ?? -1;
        if ($countBits < 0 || \strlen($code) >= (1 << $countBits)) {
            return \PHP_INT_MAX;
        }

        return (Data::MODE_BITS[$version] ?? 0) + $countBits + $this->getDataBits($code, $this->mode);
    }

    /**
     * Returns the bit sequence of the symbol: the data bit stream, the padding
     * and the error correction codewords.
     *
     * @param string $code Code to encode.
     */
    protected function getBitSequence(string $code): string
    {
        $bits = $this->getBits($this->mode, Data::MODE_BITS[$this->version] ?? 0);
        $bits .= $this->getBits(\strlen($code), Data::COUNT_BITS[$this->version][$this->mode] ?? 0);
        $bits .= $this->getDataStream($code, $this->mode);
        $terminator = \min(Data::TERMINATOR_BITS[$this->version] ?? 0, $this->capacity - \strlen($bits));
        $bits .= \str_repeat('0', \max(0, $terminator));
        $bits = $this->getPaddedStream($bits);
        $errorCorrection = new ReedSolomon(8, ReedSolomon::GF_QRCODE, 0);
        $checkwords = $errorCorrection->checkwords($this->getDataCodewords($bits), $this->checkwords);
        foreach ($checkwords as $checkword) {
            $bits .= $this->getBits($checkword, 8);
        }

        return $bits;
    }

    /**
     * Returns the data bit stream extended to the data capacity of the symbol
     * with the zero bits that complete the last codeword and with the pad
     * codewords. A pad codeword that falls on the four bit final codeword of the
     * versions M1 and M3 is encoded as four zero bits.
     *
     * @param string $bits Data bit stream.
     */
    protected function getPaddedStream(string $bits): string
    {
        $full = $this->capacity - ($this->capacity % 8);
        if (\strlen($bits) > $full) {
            return $bits . \str_repeat('0', \max(0, $this->capacity - \strlen($bits)));
        }

        $bits .= \str_repeat('0', \max(0, (8 - (\strlen($bits) % 8)) % 8));
        $index = 0;
        while (\strlen($bits) < $full) {
            $bits .= $this->getBits(Data::PAD_CODEWORDS[$index % 2] ?? 0, 8);
            ++$index;
        }

        return $bits . \str_repeat('0', \max(0, $this->capacity - $full));
    }

    /**
     * Returns the data codewords of the bit stream. The four bit final codeword
     * of the versions M1 and M3 is extended with four zero bits.
     *
     * @param string $bits Data bit stream.
     *
     * @return array<int, int>
     */
    protected function getDataCodewords(string $bits): array
    {
        $codewords = [];
        $full = $this->capacity - ($this->capacity % 8);
        for ($pos = 0; $pos < $full; $pos += 8) {
            $codewords[] = (int) \bindec(\substr($bits, $pos, 8));
        }

        if ($this->capacity > $full) {
            $codewords[] = (int) \bindec(\substr($bits, $full, 4)) << 4;
        }

        return $codewords;
    }

    /**
     * Draw the finder pattern, the separator and the timing patterns, and
     * reserve the modules of the format information.
     */
    protected function setFunctionPatterns(): void
    {
        $size = \max(0, $this->size);
        $this->matrix = \array_fill(0, $size, \array_fill(0, $size, 0));
        $this->function = \array_fill(0, $size, \array_fill(0, $size, false));
        // the finder pattern, its separator and the format information take the
        // whole top left corner of nine by nine modules
        for ($row = 0; $row <= 8; ++$row) {
            for ($col = 0; $col <= 8; ++$col) {
                $this->function[$row][$col] = true;
                $this->matrix[$row][$col] = $this->isFinderModule($row, $col) ? 1 : 0;
            }
        }

        // the timing patterns run along the row and the column zero
        for ($pos = 8; $pos < $this->size; ++$pos) {
            $this->function[0][$pos] = true;
            $this->matrix[0][$pos] = 1 - ($pos % 2);
            $this->function[$pos][0] = true;
            $this->matrix[$pos][0] = 1 - ($pos % 2);
        }
    }

    /**
     * Returns whether the module of the top left corner is a dark module of the
     * finder pattern.
     *
     * @param int $row Row of the module.
     * @param int $col Column of the module.
     */
    protected function isFinderModule(int $row, int $col): bool
    {
        if ($row > 6 || $col > 6) {
            return false;
        }

        if ($row === 0 || $row === 6 || $col === 0 || $col === 6) {
            return true;
        }

        return $row >= 2 && $row <= 4 && $col >= 2 && $col <= 4;
    }

    /**
     * Place the bit sequence in the encoding region: two module wide columns
     * from right to left, filled upwards and downwards in turn.
     *
     * @param string $sequence Bit sequence of the symbol.
     */
    protected function setData(string $sequence): void
    {
        $pos = 0;
        $len = \strlen($sequence);
        $upward = true;
        for ($col = $this->size - 1; $col > 0; $col -= 2) {
            for ($idx = 0; $idx < $this->size; ++$idx) {
                $row = $upward ? $this->size - 1 - $idx : $idx;
                foreach ([$col, $col - 1] as $current) {
                    if (($this->function[$row][$current] ?? true) || $pos >= $len) {
                        continue;
                    }

                    $this->matrix[$row][$current] = $sequence[$pos] === '1' ? 1 : 0;
                    ++$pos;
                }
            }

            $upward = !$upward;
        }
    }

    /**
     * Apply the data mask pattern that yields the highest score and store its
     * reference.
     */
    protected function setBestMask(): void
    {
        $unmasked = $this->matrix;
        $best = -1;
        for ($pattern = 0; $pattern < 4; ++$pattern) {
            $this->matrix = $unmasked;
            $this->applyMask($pattern);
            $score = $this->getMaskScore();
            if ($score > $best) {
                $best = $score;
                $this->mask = $pattern;
            }
        }

        $this->matrix = $unmasked;
        $this->applyMask($this->mask);
    }

    /**
     * Invert the modules of the encoding region on which the mask condition holds.
     *
     * @param int $pattern Data mask pattern reference, from 0 to 3.
     */
    protected function applyMask(int $pattern): void
    {
        for ($row = 0; $row < $this->size; ++$row) {
            for ($col = 0; $col < $this->size; ++$col) {
                if (($this->function[$row][$col] ?? true) || !$this->isMasked($pattern, $row, $col)) {
                    continue;
                }

                $this->matrix[$row][$col] = 1 - ($this->matrix[$row][$col] ?? 0);
            }
        }
    }

    /**
     * Returns whether the mask condition of Table 10 of ISO/IEC 18004 holds for
     * the given module.
     *
     * @param int $pattern Data mask pattern reference, from 0 to 3.
     * @param int $row     Row of the module.
     * @param int $col     Column of the module.
     */
    protected function isMasked(int $pattern, int $row, int $col): bool
    {
        return match ($pattern) {
            0 => ($row % 2) === 0,
            1 => ((\intdiv($row, 2) + \intdiv($col, 3)) % 2) === 0,
            2 => (((($row * $col) % 2) + (($row * $col) % 3)) % 2) === 0,
            default => (((($row + $col) % 2) + (($row * $col) % 3)) % 2) === 0,
        };
    }

    /**
     * Returns the score of the masked symbol: the number of dark modules of the
     * right and the bottom edge, excluding the last module of each timing
     * pattern, combined as in section 7.8.3.2 of ISO/IEC 18004.
     */
    protected function getMaskScore(): int
    {
        $right = 0;
        $bottom = 0;
        for ($pos = 1; $pos < $this->size; ++$pos) {
            $right += $this->matrix[$pos][$this->size - 1] ?? 0;
            $bottom += $this->matrix[$this->size - 1][$pos] ?? 0;
        }

        return (\min($right, $bottom) * 16) + \max($right, $bottom);
    }

    /**
     * Place the fifteen bits of the format information, the least significant
     * one in the module zero.
     */
    protected function setFormatInformation(): void
    {
        $format = $this->getFormatBits(($this->symbol << 2) | $this->mask);
        for ($pos = 0; $pos < 15; ++$pos) {
            $bit = ($format >> $pos) & 1;
            if ($pos < 8) {
                $this->matrix[$pos + 1][8] = $bit;
                continue;
            }

            $this->matrix[8][15 - $pos] = $bit;
        }
    }

    /**
     * Returns the fifteen bits of the format information: the five data bits,
     * the ten error correction bits of the BCH (15, 5) code and the mask of
     * section 7.9.2 of ISO/IEC 18004.
     *
     * @param int $data Five bits of the symbol number and the mask pattern reference.
     */
    protected function getFormatBits(int $data): int
    {
        $value = $data << 10;
        for ($pos = 14; $pos >= 10; --$pos) {
            if (($value & (1 << $pos)) === 0) {
                continue;
            }

            $value ^= Data::FORMAT_GENERATOR << ($pos - 10);
        }

        return (($data << 10) | $value) ^ Data::FORMAT_MASK;
    }
}
