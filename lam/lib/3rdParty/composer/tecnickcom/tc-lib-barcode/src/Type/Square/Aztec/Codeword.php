<?php

declare(strict_types=1);

/**
 * Codeword.php
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
 * Com\Tecnick\Barcode\Type\Square\Aztec\Codeword
 *
 * Codeword utility methods for Aztec Barcode type class
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Codeword
{
    /**
     * @param array<int> $bitstream
     */
    protected function getBitstreamBit(array $bitstream, int $index): int
    {
        if (!\array_key_exists($index, $bitstream)) {
            return 1;
        }

        return $bitstream[$index] === 0 ? 0 : 1;
    }

    /**
     * @return array<int, array{int, int}>
     */
    protected function getLatchMap(int $fromMode, int $toMode): array
    {
        $map = Data::LATCH_MAP[$fromMode][$toMode] ?? [];
        return \array_values($map);
    }

    /**
     * @return array<int, array{int, int}>
     */
    protected function getShiftMap(int $fromMode, int $toMode): array
    {
        $map = Data::SHIFT_MAP[$fromMode][$toMode] ?? [];
        return \array_values($map);
    }

    /**
     * Current character encoding mode.
     */
    protected int $encmode = Data::MODE_UPPER;

    /**
     * Array containing the high-level encoding bitstream.
     *
     * @var array<int>
     */
    protected array $bitstream = [];

    /**
     * Temporary array of codewords.
     *
     * @var array<int, array{int, int}>
     */
    protected array $tmpCdws = [];

    /**
     * Count the total number of bits in the bitstream.
     */
    protected int $totbits = 0;

    /**
     * Encodes a character using the specified mode and ordinal value.
     *
     * @param int $mode The encoding mode.
     * @param int $ord The ordinal value of the character to encode.
     *
     * @return int The encoded character.
     */
    protected function charEnc(int $mode, int $ord): int
    {
        return Data::CHAR_ENC[$mode][$ord] ?? 0;
    }

    /**
     * Returns the character mode for a given ASCII code.
     *
     * @param int $ord The ASCII code of the character.
     *
     * @return int The character mode.
     */
    protected function charMode(int $ord): int
    {
        return Data::CHAR_MODES[$ord] ?? Data::MODE_BINARY;
    }

    /**
     * Checks if current character is supported by the current code.
     *
     * @param int $mode The mode to check.
     * @param int $ord The character ASCII value to compare against.
     *
     * @return bool Returns true if the mode is the same as the ordinal value, false otherwise.
     */
    protected function isSameMode(int $mode, int $ord): bool
    {
        return (
            $mode === $this->charMode($ord)
            || $ord === 32
            && $mode !== Data::MODE_PUNCT
            || $mode === Data::MODE_PUNCT
            && ($ord === 13 || $ord === 44 || $ord === 46)
        );
    }

    /**
     * Returns true if the character is in common between the PUNCT and DIGIT modes.
     * Characters ' ' (32), '.' (46) and ',' (44) are in common between the PUNCT and DIGIT modes.
     *
     * @param int $ord Integer ASCII code of the character to check.
     */
    protected function isPunctAndDigitChar(int $ord): bool
    {
        return $ord === 32 || $ord === 44 || $ord === 46;
    }

    /**
     * Returns the PUNCT two-bytes code if the given two characters are a punctuation pair.
     * Punct codes 2–5 encode two bytes each.
     *
     * @param int $ord The current curacter code.
     * @param int $next The next character code.
     */
    protected function punctPairMode(int $ord, int $next): int
    {
        $key = ($ord << 8) + $next;
        return match ($key) {
            (13 << 8) + 10 => 2,
            (46 << 8) + 32 => 3,
            (44 << 8) + 32 => 4,
            (58 << 8) + 32 => 5,
            default => 0,
        }; // no punct pair
    }

    /**
     * Append a new Codeword as a big-endian bit sequence.
     *
     * @param array<int> $bitstream Array of bits to append to.
     * @param int   $totbits   Number of bits in the bitstream.
     * @param int   $wsize     The number of bits in the codeword.
     * @param int   $value     The value of the codeword.
     */
    protected function appendWordToBitstream(array &$bitstream, int &$totbits, int $wsize, int $value): void
    {
        for ($idx = $wsize - 1; $idx >= 0; --$idx) {
            $bitstream[] = ($value >> $idx) & 1;
        }

        $totbits += $wsize;
    }

    /**
     * Convert the bitstream to words.
     *
     * @param array<int> $bitstream Array of bits to convert.
     * @param int   $totbits   Number of bits in the bitstream.
     * @param int   $wsize     The word size.
     *
     * @return array<int> Array of words.
     */
    protected function bitstreamToWords(array $bitstream, int $totbits, int $wsize): array
    {
        $words = [];
        $numwords = (int) \ceil($totbits / $wsize);
        for ($idx = 0; $idx < $numwords; ++$idx) {
            $wrd = 0;
            for ($bit = 0; $bit < $wsize; ++$bit) {
                $pos = ($idx * $wsize) + $bit;
                if ($this->getBitstreamBit($bitstream, $pos) === 1) {
                    $wrd |= 1 << ($wsize - $bit - 1); // reverse order
                }
            }

            $words[] = $wrd;
        }

        return $words;
    }

    /**
     * Add a new Codeword as a big-endian bit sequence.
     *
     * @param int $bits The number of bits in the codeword.
     * @param int $value The value of the codeword.
     */
    protected function addRawCwd(int $bits, int $value): void
    {
        $this->appendWordToBitstream($this->bitstream, $this->totbits, $bits, $value);
    }

    /**
     * Adds a Codeword.
     *
     * @param int $mode The encoding mode.
     * @param int $value The value to encode.
     */
    protected function addCdw(int $mode, int $value): void
    {
        $this->addRawCwd(Data::MODE_BITS[$mode] ?? 0, $value);
    }

    /**
     * Latch to another mode.
     *
     * @param int $mode The new encoding mode.
     */
    protected function addLatch(int $mode): void
    {
        $latch = $this->getLatchMap($this->encmode, $mode);
        foreach ($latch as $cdw) {
            $this->addRawCwd($cdw[0], $cdw[1]);
        }

        $this->encmode = $mode;
    }

    /**
     * Shift to another mode.
     */
    protected function addShift(int $mode): void
    {
        $shift = $this->getShiftMap($this->encmode, $mode);
        foreach ($shift as $cdw) {
            $this->addRawCwd($cdw[0], $cdw[1]);
        }
    }

    /**
     * Merges the temporary codewords array with the current codewords array.
     * Shift to the specified mode.
     *
     * @param int $mode The encoding mode for the codewords.
     */
    protected function mergeTmpCwdWithShift(int $mode): void
    {
        foreach ($this->tmpCdws as $tmpCdw) {
            $this->addShift($mode);
            $this->addRawCwd($tmpCdw[0], $tmpCdw[1]);
        }
    }

    /**
     * Merges the temporary codewords array with the current codewords array.
     * No shift is performed.
     */
    protected function mergeTmpCwdRaw(): void
    {
        foreach ($this->tmpCdws as $tmpCdw) {
            $this->addRawCwd($tmpCdw[0], $tmpCdw[1]);
        }
    }

    /**
     * Merge temporary codewords with current codewords based on the encoding mode.
     *
     * @param int $mode The encoding mode to use for merging codewords.
     *                  If negative, the current encoding mode will be used.
     */
    protected function mergeTmpCwd(int $mode = -1): void
    {
        if ($mode < 0 || $this->encmode === $mode) {
            $this->mergeTmpCwdRaw();
            $this->tmpCdws = [];
            return;
        }

        $this->mergeTmpCwdWithShift($mode);
        $this->tmpCdws = [];
    }

    /**
     * Adds the FLG (Function Length Group) codeword to the data codewords.
     *
     * @param int $eci Extended Channel Interpretation value. If negative, the function does nothing.
     */
    protected function addFLG(int $eci): void
    {
        if ($eci < 0) {
            return;
        }

        if ($this->encmode !== Data::MODE_PUNCT) {
            $this->addShift(Data::MODE_PUNCT);
        }

        if ($eci === 0) {
            $this->addRawCwd(3, 0); // FNC1
            return;
        }

        $seci = (string) $eci;
        $digits = \strlen($seci);
        $this->addRawCwd(3, $digits); // 1–6 digits
        for ($idx = 0; $idx < $digits; ++$idx) {
            $this->addCdw(Data::MODE_DIGIT, $this->charEnc(Data::MODE_DIGIT, \ord($seci[$idx])));
        }
    }
}
