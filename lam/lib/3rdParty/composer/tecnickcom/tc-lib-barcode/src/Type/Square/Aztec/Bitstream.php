<?php

declare(strict_types=1);

/**
 * Bitstream.php
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

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Square\Aztec\Bitstream
 *
 * Bitstream for Aztec Barcode type class
 *
 * @since       2023-10-13
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2023-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Bitstream extends \Com\Tecnick\Barcode\Type\Square\Aztec\Layers
{
    /**
     * Performs the high-level encoding for the given code and ECI mode.
     *
     * @param string $code The code to encode.
     * @param int $eci The ECI mode to use.
     * @param string $hint The mode to use.
     *
     * @throws BarcodeException in case of error
     */
    protected function highLevelEncoding(string $code, int $eci = 0, string $hint = 'A'): void
    {
        $this->addFLG($eci);
        $chrarr = \unpack('C*', $code);
        if ($chrarr === false) {
            throw new BarcodeException('Unable to unpack the code');
        }

        $chars = \array_values($chrarr);
        $chrlen = \count($chars);
        if ($hint === 'B') {
            $this->binaryEncode($chars, $chrlen); // @phpstan-ignore argument.type
            return;
        }

        $this->autoEncode($chars, $chrlen); // @phpstan-ignore argument.type
    }

    /**
     * Forced binary encoding for the given characters.
     *
     * @param list<int> $chars  Integer ASCII values of the characters to encode.
     * @param int   $chrlen Length of the $chars array.
     */
    protected function binaryEncode(array $chars, int $chrlen): void
    {
        $bits = Data::MODE_BITS[Data::MODE_BINARY] ?? 8;
        $this->addShift(Data::MODE_BINARY);
        if ($chrlen > 62) {
            $this->addRawCwd(5, 0);
            $this->addRawCwd(11, $chrlen - 31);
            for ($idx = 0; $idx < $chrlen; ++$idx) {
                $this->addRawCwd($bits, $chars[$idx] ?? 0);
            }

            return;
        }

        if ($chrlen > 31) {
            $this->addRawCwd(5, 31);
            for ($idx = 0; $idx < 31; ++$idx) {
                $this->addRawCwd($bits, $chars[$idx] ?? 0);
            }

            $this->addShift(Data::MODE_BINARY);
            $this->addRawCwd(5, $chrlen - 31);
            for ($idx = 31; $idx < $chrlen; ++$idx) {
                $this->addRawCwd($bits, $chars[$idx] ?? 0);
            }

            return;
        }

        $this->addRawCwd(5, $chrlen);
        for ($idx = 0; $idx < $chrlen; ++$idx) {
            $this->addRawCwd($bits, $chars[$idx] ?? 0);
        }
    }

    /**
     * Automatic encoding for the given characters.
     *
     * @param list<int> $chars  Integer ASCII values of the characters to encode.
     * @param int   $chrlen Length of the $chars array.
     */
    protected function autoEncode(array $chars, int $chrlen): void
    {
        $idx = 0;
        while ($idx < $chrlen) {
            if ($this->processBinaryChars($chars, $idx, $chrlen)) {
                continue;
            }

            if ($this->processPunctPairs($chars, $idx, $chrlen)) {
                continue;
            }

            $this->processModeChars($chars, $idx, $chrlen);
        }
    }

    /**
     * Process mode characters.
     *
     * @param list<int> $chars The array of characters.
     * @param int $idx The current character index.
     * @param int $chrlen The total number of characters to process.
     */
    protected function processModeChars(array &$chars, int &$idx, int $chrlen): void
    {
        $ord = $chars[$idx] ?? 0;
        $mode = $this->isSameMode($this->encmode, $ord) ? $this->encmode : $this->charMode($ord);

        $nchr = $this->countModeChars($chars, $idx, $chrlen, $mode);
        if ($this->encmode !== $mode) {
            $shiftMap = Data::SHIFT_MAP[$this->encmode] ?? [];
            $canShift = $nchr === 1 && (\array_key_exists($mode, $shiftMap) && $shiftMap[$mode] !== []);
            if ($canShift) {
                $this->addShift($mode);
                $this->mergeTmpCwd();
                $idx += $nchr;
                return;
            }

            $this->addLatch($mode);
        }

        $this->mergeTmpCwd();
        $idx += $nchr;
    }

    /**
     * Count consecutive characters in the same mode.
     *
     * @param list<int> $chars The array of characters.
     * @param int $idx The current character index.
     * @param int $chrlen The total number of characters to process.
     * @param int $mode The current mode.
     */
    protected function countModeChars(array &$chars, int $idx, int $chrlen, int $mode): int
    {
        $this->tmpCdws = [];
        $nbits = Data::MODE_BITS[$mode] ?? 0;
        $count = 0;
        do {
            $ord = $chars[$idx] ?? 0;
            if (
                !$this->isSameMode($mode, $ord)
                || $idx < ($chrlen - 1) && $this->punctPairMode($ord, $chars[$idx + 1] ?? 0) > 0
            ) {
                return $count;
            }

            $this->tmpCdws[] = [$nbits, $this->charEnc($mode, $ord)];
            ++$count;
            ++$idx;
        } while ($idx < $chrlen);

        return $count;
    }

    /**
     * Process consecutive binary characters.
     *
     * @param list<int> $chars The array of characters.
     * @param int $idx The current character index.
     * @param int $chrlen The total number of characters to process.
     *
     * @return bool True if binary characters have been found and processed.
     */
    protected function processBinaryChars(array &$chars, int &$idx, int $chrlen): bool
    {
        $binchrs = $this->countBinaryChars($chars, $idx, $chrlen);
        if ($binchrs === 0) {
            return false;
        }

        $encmode = $this->encmode;
        $this->addShift(Data::MODE_BINARY);
        if ($binchrs > 62) {
            $this->addRawCwd(5, 0);
            $this->addRawCwd(11, $binchrs - 31);
            $this->mergeTmpCwdRaw();
            $idx += $binchrs;
            $this->encmode = $encmode;
            return true;
        }

        if ($binchrs > 31) {
            $nbits = Data::MODE_BITS[Data::MODE_BINARY] ?? 8;
            $this->addRawCwd(5, 31);
            for ($bcw = 0; $bcw < 31; ++$bcw) {
                $tmpCdw = $this->tmpCdws[$bcw] ?? [0, 0];
                $this->addRawCwd($nbits, $tmpCdw[1]);
            }

            $this->addShift(Data::MODE_BINARY);
            $this->addRawCwd(5, $binchrs - 31);
            for ($bcw = 31; $bcw < $binchrs; ++$bcw) {
                $tmpCdw = $this->tmpCdws[$bcw] ?? [0, 0];
                $this->addRawCwd($nbits, $tmpCdw[1]);
            }

            $idx += $binchrs;
            $this->encmode = $encmode;
            return true;
        }

        $this->addRawCwd(5, $binchrs);
        $this->mergeTmpCwdRaw();
        $idx += $binchrs;
        $this->encmode = $encmode;
        return true;
    }

    /**
     * Count consecutive binary characters.
     *
     * @param list<int> $chars The array of characters.
     * @param int $idx The current character index.
     * @param int $chrlen The total number of characters to process.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function countBinaryChars(array &$chars, int $idx, int $chrlen): int
    {
        $this->tmpCdws = [];
        $count = 0;
        $nbits = Data::MODE_BITS[Data::MODE_BINARY] ?? 8;
        while ($idx < $chrlen && $count < 2048) {
            $ord = $chars[$idx] ?? 0;
            if ($this->charMode($ord) !== Data::MODE_BINARY) {
                return $count;
            }

            $this->tmpCdws[] = [$nbits, $ord];
            ++$count;
            ++$idx;
        }

        return $count;
    }

    /**
     * Process consecutive special Punctuation Pairs.
     *
     * @param list<int> $chars The array of characters.
     * @param int $idx The current character index.
     * @param int $chrlen The total number of characters to process.
     *
     * @return bool True if pair characters have been found and processed.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function processPunctPairs(array &$chars, int &$idx, int $chrlen): bool
    {
        $ppairs = $this->countPunctPairs($chars, $idx, $chrlen);
        if ($ppairs === 0) {
            return false;
        }

        switch ($this->encmode) {
            case Data::MODE_PUNCT:
                break;
            case Data::MODE_MIXED:
                $this->addLatch(Data::MODE_PUNCT);
                break;
            case Data::MODE_UPPER:
            case Data::MODE_LOWER:
                if ($ppairs > 1) {
                    $this->addLatch(Data::MODE_PUNCT);
                }

                break;
            case Data::MODE_DIGIT:
                $common = $this->countPunctAndDigitChars($chars, $idx, $chrlen);
                $clen = \count($common);
                if ($clen > 0 && $clen < 6) {
                    $this->tmpCdws = $common;
                    $this->mergeTmpCwdRaw();
                    $idx += $clen;
                    return true;
                }

                if ($ppairs > 2) {
                    $this->addLatch(Data::MODE_PUNCT);
                }

                break;
        }

        $this->mergeTmpCwd(Data::MODE_PUNCT);
        $idx += $ppairs * 2;
        return true;
    }

    /**
     * Count consecutive special Punctuation Pairs.
     *
     * @param list<int> $chars The array of characters.
     * @param int $idx The current character index.
     * @param int $chrlen The total number of characters to process.
     */
    protected function countPunctPairs(array &$chars, int $idx, int $chrlen): int
    {
        $this->tmpCdws = [];
        $pairs = 0;
        $maxidx = $chrlen - 1;
        while ($idx < $maxidx) {
            $pmode = $this->punctPairMode($chars[$idx] ?? 0, $chars[$idx + 1] ?? 0);
            if ($pmode === 0) {
                return $pairs;
            }

            $this->tmpCdws[] = [5, $pmode];
            ++$pairs;
            $idx += 2;
        }

        return $pairs;
    }

    /**
     * Counts the number of consecutive charcters that are in both PUNCT or DIGIT modes.
     * Returns the array with the codewords.
     *
     * @param list<int> $chars The string to count the characters in.
     * @param int   $idx    The starting index to count from.
     * @param int   $chrlen The length of the string to count.
     *
     * @return array<int, array{int, int}> The array of codewords.
     */
    protected function countPunctAndDigitChars(array $chars, int $idx, int $chrlen): array
    {
        $words = [];
        while ($idx < $chrlen) {
            $ord = $chars[$idx] ?? 0;
            if (!$this->isPunctAndDigitChar($ord)) {
                return $words;
            }

            $words[] = [4, $this->charEnc(Data::MODE_DIGIT, $ord)];
            ++$idx;
        }

        return $words;
    }
}
