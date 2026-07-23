<?php

declare(strict_types=1);

/**
 * Process.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven;

/**
 * Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven\Compaction
 *
 * Process for PdfFourOneSeven Barcode type class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Compaction extends \Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven\Sequence
{
    /**
     * @return array<int, int>
     */
    protected function getTextSubModeValues(int $submode): array
    {
        $result = [];
        foreach (Data::TEXT_SUB_MODES[$submode] ?? [] as $value) {
            $result[] = $value;
        }

        return $result;
    }

    /**
     * @return array<int, int>
     */
    protected function getTextLatchValues(int $submode, int $sub): array
    {
        $result = [];
        foreach (Data::TEXT_LATCH['' . $submode . $sub] ?? [] as $value) {
            $result[] = $value;
        }

        return $result;
    }

    protected function findTextSubModeKey(int $submode, int $chval): ?int
    {
        $key = \array_search($chval, $this->getTextSubModeValues($submode), true);
        if (\is_int($key)) {
            return $key;
        }

        return null;
    }

    protected function getCodeOrd(string $code, int $idx): int
    {
        return \ord($code[$idx] ?? "\x00");
    }

    /**
     * @return numeric-string
     */
    protected function getByteNumericString(string $code, int $idx): string
    {
        return (string) $this->getCodeOrd($code, $idx);
    }

    /**
     * @param array<int, int> $txtarr
     */
    protected function getTxtArrayValue(array $txtarr, int $idx): int
    {
        return $txtarr[$idx] ?? 0;
    }

    /**
     * @param string $value
     *
     * @return numeric-string
     */
    protected function normalizeNumericString(string $value): string
    {
        if (\ctype_digit($value)) {
            return $value;
        }

        return '0';
    }

    /**
     * @param numeric-string $left
     * @param numeric-string $right
     *
     * @return numeric-string
     */
    protected function mulNumeric(string $left, string $right): string
    {
        return $this->normalizeNumericString(\bcmul($left, $right));
    }

    /**
     * @param numeric-string $left
     * @param numeric-string $right
     *
     * @return numeric-string
     */
    protected function addNumeric(string $left, string $right): string
    {
        return $this->normalizeNumericString(\bcadd($left, $right));
    }

    /**
     * @param numeric-string $left
     * @param numeric-string $right
     */
    protected function modNumeric(string $left, string $right): int
    {
        return (int) \bcmod($left, $right);
    }

    /**
     * @param numeric-string $left
     * @param numeric-string $right
     *
     * @return numeric-string
     */
    protected function divNumeric(string $left, string $right): string
    {
        return $this->normalizeNumericString(\bcdiv($left, $right));
    }

    /**
     * Process Sub Text Compaction
     *
     * @param array<int, int> $txtarr  Array of characters and sub-mode switching characters
     * @param int             $submode Current submode
     * @param int             $sub     New submode
     * @param string          $code    Data to compact
     * @param int             $key     Character code
     * @param int             $idx     Current index
     * @param int             $codelen Code length
     */
    protected function processTextCompactionSub(
        array &$txtarr,
        int &$submode,
        int $sub,
        string $code,
        int $key,
        int $idx,
        int $codelen,
    ): void {
        // $sub is the new submode
        $useShift =
            (
                ($idx + 1) === $codelen
                || ($idx + 1) < $codelen
                && \in_array($this->getCodeOrd($code, $idx + 1), $this->getTextSubModeValues($submode), true)
            )
            && ($sub === 3 || $sub === 0 && $submode === 1);

        if ($useShift) {
            // shift (temporary change only for this char)
            $txtarr[] = $sub === 3 ? 29 : 27;
            // add character code to array
            $txtarr[] = $key;
            return;
        }

        // latch
        foreach ($this->getTextLatchValues($submode, $sub) as $latch) {
            $txtarr[] = $latch;
        }

        // set new submode
        $submode = $sub;

        // add character code to array
        $txtarr[] = $key;
    }

    /**
     * Process Text Compaction
     *
     * @param string          $code      Data to compact
     * @param array<int, int> $codewords Codewords
     */
    protected function processTextCompaction(string $code, array &$codewords): void
    {
        $submode = 0; // default Alpha sub-mode
        /** @var array<int, int> $txtarr */
        $txtarr = []; // array of characters and sub-mode switching characters
        $codelen = \strlen($code);
        for ($idx = 0; $idx < $codelen; ++$idx) {
            $chval = $this->getCodeOrd($code, $idx);
            $current_key = $this->findTextSubModeKey($submode, $chval);
            if ($current_key !== null) {
                // we are on the same sub-mode
                $txtarr[] = $current_key;
                continue;
            }

            // the sub-mode is changed
            for ($sub = 0; $sub < 4; ++$sub) {
                // search new sub-mode
                $sub_key = $this->findTextSubModeKey($sub, $chval);
                if ($sub !== $submode && $sub_key !== null) {
                    $this->processTextCompactionSub($txtarr, $submode, $sub, $code, $sub_key, $idx, $codelen);
                    break;
                }
            }
        }

        $txtarrlen = \count($txtarr);
        if (($txtarrlen % 2) !== 0) {
            // add padding
            $txtarr[] = 29;
            ++$txtarrlen;
        }

        // calculate codewords
        for ($idx = 0; $idx < $txtarrlen; $idx += 2) {
            $codewords[] = (30 * $this->getTxtArrayValue($txtarr, $idx)) + $this->getTxtArrayValue($txtarr, $idx + 1);
        }
    }

    /**
     * Process Byte Compaction
     *
     * @param string          $code      Data to compact
     * @param array<int, int> $codewords Codewords
     */
    protected function processByteCompaction(string $code, array &$codewords): void
    {
        while (($codelen = \strlen($code)) > 0) {
            $rest = '';
            $sublen = \strlen($code);
            if ($codelen > 6) {
                $rest = \substr($code, 6);
                $code = \substr($code, 0, 6);
                $sublen = 6;
            }

            if ($sublen === 6) {
                $tdg = $this->mulNumeric($this->getByteNumericString($code, 0), '1099511627776');
                $tdg = $this->addNumeric($tdg, $this->mulNumeric($this->getByteNumericString($code, 1), '4294967296'));
                $tdg = $this->addNumeric($tdg, $this->mulNumeric($this->getByteNumericString($code, 2), '16777216'));
                $tdg = $this->addNumeric($tdg, $this->mulNumeric($this->getByteNumericString($code, 3), '65536'));
                $tdg = $this->addNumeric($tdg, $this->mulNumeric($this->getByteNumericString($code, 4), '256'));
                $tdg = $this->addNumeric($tdg, $this->getByteNumericString($code, 5));
                // tmp array for the 6 bytes block
                /** @var array<int, int> $cw6 */
                $cw6 = [];
                for ($idx = 0; $idx < 5; ++$idx) {
                    $ddg = $this->modNumeric($tdg, '900');
                    $tdg = $this->divNumeric($tdg, '900');
                    // prepend the value to the beginning of the array
                    \array_unshift($cw6, $ddg);
                }

                // append the result array at the end
                foreach ($cw6 as $cw) {
                    $codewords[] = $cw;
                }
            }

            if ($sublen !== 6) {
                for ($idx = 0; $idx < $sublen; ++$idx) {
                    $codewords[] = $this->getCodeOrd($code, $idx);
                }
            }

            $code = $rest;
        }
    }

    /**
     * Process Numeric Compaction
     *
     * @param string          $code      Data to compact
     * @param array<int, int> $codewords Codewords
     */
    protected function processNumericCompaction(string $code, array &$codewords): void
    {
        $len = \strlen($code);
        // numbers are encoded in groups of up to 44 digits, emitted in left-to-right order
        for ($start = 0; $start < $len; $start += 44) {
            $tdg = $this->normalizeNumericString('1' . \substr($code, $start, 44));
            $group = [];
            do {
                // remainders come out least-significant first
                $group[] = $this->modNumeric($tdg, '900');
                $tdg = $this->divNumeric($tdg, '900');
            } while ($tdg !== '0');

            // reverse to big-endian, then append this group after the previous ones
            $codewords = \array_merge($codewords, \array_reverse($group));
        }
    }

    /**
     * Compact data by mode
     *
     * @param int    $mode    Compaction mode number
     * @param string $code    Data to compact
     * @param bool   $addmode If true add the mode codeword in the first position
     *
     * @return array<int, int> of codewords
     */
    protected function getCompaction(int $mode, string $code, bool $addmode = true): array
    {
        $codewords = []; // array of codewords to return
        switch ($mode) {
            case 900:
                // Text Compaction mode latch
                $this->processTextCompaction($code, $codewords);
                break;
            case 901:
            case 924:
                // Byte Compaction mode latch
                $this->processByteCompaction($code, $codewords);
                break;
            case 902:
                // Numeric Compaction mode latch
                $this->processNumericCompaction($code, $codewords);
                break;
            case 913:
                // Byte Compaction mode shift
                $codewords[] = \ord($code);
                break;
        }

        if ($addmode) {
            // add the compaction mode codeword at the beginning
            \array_unshift($codewords, $mode);
        }

        return $codewords;
    }
}
