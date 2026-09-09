<?php

declare(strict_types=1);

/**
 * Compaction.php
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square\HanXin;

/**
 * Com\Tecnick\Barcode\Type\Square\HanXin\Compaction
 *
 * Data analysis and information bit stream of the HanXin Barcode type class
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Compaction
{
    /**
     * Digit character.
     *
     * @var int
     */
    protected const UNIT_DIGIT = 0;

    /**
     * Character of the Text1 or Text2 sub mode.
     *
     * @var int
     */
    protected const UNIT_TEXT = 1;

    /**
     * Character of the common Chinese character region one.
     *
     * @var int
     */
    protected const UNIT_HANZI_ONE = 2;

    /**
     * Character of the common Chinese character region two.
     *
     * @var int
     */
    protected const UNIT_HANZI_TWO = 3;

    /**
     * Character of the GB 18030 two byte region outside the two common regions.
     *
     * @var int
     */
    protected const UNIT_GB_TWO = 4;

    /**
     * Character of the GB 18030 four byte region.
     *
     * @var int
     */
    protected const UNIT_GB_FOUR = 5;

    /**
     * Byte no other mode can represent.
     *
     * @var int
     */
    protected const UNIT_BYTE = 6;

    /**
     * State of the shortest path with no open segment.
     *
     * @var int
     */
    protected const STATE_NONE = 0;

    /**
     * State of an open Text segment left in the Text1 sub mode.
     *
     * @var int
     */
    protected const STATE_TEXT_ONE = 1;

    /**
     * State of an open Text segment left in the Text2 sub mode.
     *
     * @var int
     */
    protected const STATE_TEXT_TWO = 2;

    /**
     * State of an open binary byte segment.
     *
     * @var int
     */
    protected const STATE_BINARY = 3;

    /**
     * State of an open GB 18030 two byte region segment.
     *
     * @var int
     */
    protected const STATE_GB_TWO = 4;

    /**
     * Number of bits of the terminator of the open segment, by state.
     *
     * @var array<int, int>
     */
    protected const STATE_TERMINATOR = [0, 6, 6, 0, 15];

    /**
     * Number of bits of a mode indicator.
     *
     * @var int
     */
    protected const MODE_BITS = 4;

    /**
     * Returns the information bit stream of the code.
     *
     * @param string $code Code to encode.
     */
    protected function getBitStream(string $code): string
    {
        $bits = '';
        foreach ($this->getSegments($this->getUnits($code)) as $segment) {
            $bits .= $this->getSegmentStream($segment[0], $segment[1]);
        }

        return $bits;
    }

    /**
     * Splits the code into the characters the encoding modes represent.
     *
     * @param string $code Code to encode.
     *
     * @return list<array{int, string}> Unit class and bytes of each character.
     */
    protected function getUnits(string $code): array
    {
        $units = [];
        $len = \strlen($code);
        $pos = 0;
        while ($pos < $len) {
            $unit = $this->getUnitAt($code, $pos, $len);
            $units[] = $unit;
            $pos += \strlen($unit[1]);
        }

        return $units;
    }

    /**
     * Returns the class and the bytes of the character at the given offset.
     *
     * @param string $code Code to encode.
     * @param int    $pos  Byte offset.
     * @param int    $len  Length of the code in bytes.
     *
     * @return array{int, string} Unit class and bytes.
     */
    protected function getUnitAt(string $code, int $pos, int $len): array
    {
        $byte = \ord($code[$pos]);
        if ($byte >= 0x81 && $byte <= 0xFE) {
            $unit = $this->getMultiByteUnitAt($code, $pos, $len);
            if ($unit !== []) {
                return $unit;
            }
        }

        if ($byte >= 0x30 && $byte <= 0x39) {
            return [self::UNIT_DIGIT, $code[$pos]];
        }

        if ($this->getTextCode($byte) >= 0) {
            return [self::UNIT_TEXT, $code[$pos]];
        }

        return [self::UNIT_BYTE, $code[$pos]];
    }

    /**
     * Returns the class and the bytes of the GB 18030 character at the given
     * offset, or an empty array when the bytes are not one.
     *
     * @param string $code Code to encode.
     * @param int    $pos  Byte offset.
     * @param int    $len  Length of the code in bytes.
     *
     * @return array{int, string}|array{} Unit class and bytes.
     */
    protected function getMultiByteUnitAt(string $code, int $pos, int $len): array
    {
        if (($pos + 3) < $len) {
            $second = \ord($code[$pos + 1]);
            $third = \ord($code[$pos + 2]);
            $fourth = \ord($code[$pos + 3]);
            if (
                $second >= 0x30
                && $second <= 0x39
                && $third >= 0x81
                && $third <= 0xFE
                && $fourth >= 0x30
                && $fourth <= 0x39
            ) {
                return [self::UNIT_GB_FOUR, \substr($code, $pos, 4)];
            }
        }

        if (($pos + 1) >= $len) {
            return [];
        }

        $first = \ord($code[$pos]);
        $second = \ord($code[$pos + 1]);
        if ($second < 0x40 || $second === 0x7F || $second > 0xFE) {
            return [];
        }

        $pair = \substr($code, $pos, 2);
        if ($this->getHanziOneValue($first, $second) >= 0) {
            return [self::UNIT_HANZI_ONE, $pair];
        }

        if ($this->getHanziTwoValue($first, $second) >= 0) {
            return [self::UNIT_HANZI_TWO, $pair];
        }

        return [self::UNIT_GB_TWO, $pair];
    }

    /**
     * Groups the characters into the segments of the mixed mode structure of
     * Table 12. The characters are first grouped into the runs one mode can
     * carry, and the modes of the runs are then the sequence that yields the
     * shortest information bit stream.
     *
     * @param list<array{int, string}> $units Characters of the code.
     *
     * @return list<array{int, list<array{int, string}>}> Mode of each segment and its characters.
     */
    protected function getSegments(array $units): array
    {
        $runs = $this->getRuns($units);
        $modes = $this->getRunModes($runs);
        $segments = [];
        $open = -1;
        $current = [];
        foreach ($runs as $idx => $run) {
            $mode = $modes[$idx] ?? self::UNIT_BYTE;
            $split = $mode !== $open || $mode === self::UNIT_DIGIT || $mode === self::UNIT_GB_FOUR;
            if ($current !== [] && $split) {
                $segments[] = [$open, $current];
                $current = [];
            }

            $open = $mode;
            $current = \array_merge($current, $run[1]);
        }

        if ($current !== []) {
            $segments[] = [$open, $current];
        }

        return $segments;
    }

    /**
     * Groups the characters into the runs one mode can carry: the digits, the
     * Text characters, the common Chinese characters of both regions, the
     * other characters of the GB 18030 two byte region, each character of the
     * four byte region and the bytes no other mode represents.
     *
     * @param list<array{int, string}> $units Characters of the code.
     *
     * @return list<array{int, list<array{int, string}>}> Class and characters of each run.
     */
    protected function getRuns(array $units): array
    {
        $runs = [];
        $open = -1;
        $current = [];
        foreach ($units as $unit) {
            $class = $this->getRunClass($unit[0]);
            if ($current !== [] && ($class !== $open || $class === self::UNIT_GB_FOUR)) {
                $runs[] = [$open, $current];
                $current = [];
            }

            $open = $class;
            $current[] = $unit;
        }

        if ($current !== []) {
            $runs[] = [$open, $current];
        }

        return $runs;
    }

    /**
     * Returns the class of the run a character belongs to. The two common
     * Chinese character regions share one run, which the common Chinese
     * character modes and the GB 18030 two byte region mode can both carry.
     *
     * @param int $class Class of the character.
     */
    protected function getRunClass(int $class): int
    {
        return $class === self::UNIT_HANZI_TWO ? self::UNIT_HANZI_ONE : $class;
    }

    /**
     * Returns the mode of each run, as the sequence of modes that yields the
     * shortest information bit stream.
     *
     * @param list<array{int, list<array{int, string}>}> $runs Runs of the code.
     *
     * @return array<int, int> Mode of each run.
     */
    protected function getRunModes(array $runs): array
    {
        $count = \count(self::STATE_TERMINATOR);
        $cost = \array_fill(0, $count, \PHP_INT_MAX);
        $cost[self::STATE_NONE] = 0;
        $path = [];
        foreach ($runs as $idx => $run) {
            $next = \array_fill(0, $count, \PHP_INT_MAX);
            $back = \array_fill(0, $count, [self::STATE_NONE, self::UNIT_BYTE]);
            foreach ($cost as $state => $sofar) {
                if ($sofar === \PHP_INT_MAX) {
                    continue;
                }

                foreach ($this->getRunTransitions($state, $run[0], $run[1]) as $move) {
                    [$target, $bits, $mode] = $move;
                    if (($sofar + $bits) < ($next[$target] ?? \PHP_INT_MAX)) {
                        $next[$target] = $sofar + $bits;
                        $back[$target] = [$state, $mode];
                    }
                }
            }

            $cost = $next;
            $path[$idx] = $back;
        }

        return $this->getRunPath($cost, $path);
    }

    /**
     * Returns the modes of the shortest path, walking it back from its end.
     *
     * @param array<int, int>                          $cost Cost of each state at the last run.
     * @param array<int, array<int, array{int, int}>>   $path Previous state and mode of each state at each run.
     *
     * @return array<int, int> Mode of each run.
     */
    protected function getRunPath(array $cost, array $path): array
    {
        $best = self::STATE_NONE;
        $lowest = \PHP_INT_MAX;
        foreach ($cost as $state => $sofar) {
            if ($sofar === \PHP_INT_MAX) {
                continue;
            }

            $total = $sofar + (self::STATE_TERMINATOR[$state] ?? 0);
            if ($total < $lowest) {
                $lowest = $total;
                $best = $state;
            }
        }

        $modes = [];
        for ($idx = \count($path) - 1; $idx >= 0; --$idx) {
            $step = $path[$idx][$best] ?? [self::STATE_NONE, self::UNIT_BYTE];
            $modes[$idx] = $step[1];
            $best = $step[0];
        }

        \ksort($modes);

        return $modes;
    }

    /**
     * Returns the moves one run allows, each as the state it leaves the
     * encoder in, its cost in bits and the mode it uses.
     *
     * @param int                      $state State before the run.
     * @param int                      $class Class of the run.
     * @param list<array{int, string}>  $units Characters of the run.
     *
     * @return list<array{int, int, int}> Moves.
     */
    protected function getRunTransitions(int $state, int $class, array $units): array
    {
        $term = self::STATE_TERMINATOR[$state] ?? 0;
        $open = $term + self::MODE_BITS;
        $moves = [[self::STATE_BINARY, $this->getBinaryRunCost($state, $open, $units), self::UNIT_BYTE]];
        if ($class === self::UNIT_GB_FOUR) {
            $moves[] = [self::STATE_NONE, $open + 21, self::UNIT_GB_FOUR];

            return $moves;
        }

        if ($class === self::UNIT_DIGIT) {
            $moves[] = [
                self::STATE_NONE,
                $open + (10 * (int) \ceil(\count($units) / 3)) + 10,
                self::UNIT_DIGIT,
            ];
        }

        if ($class === self::UNIT_DIGIT || $class === self::UNIT_TEXT) {
            $moves[] = $this->getTextRunCost($state, $open, $units);
        }

        if ($class === self::UNIT_HANZI_ONE) {
            $moves[] = [self::STATE_NONE, $open + $this->getHanziRunBits($units) + 12, self::UNIT_HANZI_ONE];
        }

        if ($class === self::UNIT_HANZI_ONE || $class === self::UNIT_GB_TWO) {
            $bits = 15 * \count($units);
            $moves[] = [
                self::STATE_GB_TWO,
                $state === self::STATE_GB_TWO ? $bits : $open + $bits,
                self::UNIT_GB_TWO,
            ];
        }

        return $moves;
    }

    /**
     * Returns the cost in bits of carrying one run in the binary byte mode.
     *
     * @param int                      $state State before the run.
     * @param int                      $open  Cost of closing the open segment and opening a new one.
     * @param list<array{int, string}> $units Characters of the run.
     */
    protected function getBinaryRunCost(int $state, int $open, array $units): int
    {
        $bits = 0;
        foreach ($units as $unit) {
            $bits += 8 * \strlen($unit[1]);
        }

        return $state === self::STATE_BINARY ? $bits : $open + Data::BINARY_COUNT_BITS + $bits;
    }

    /**
     * Returns the move of carrying one run in the Text mode.
     *
     * @param int                      $state State before the run.
     * @param int                      $open  Cost of closing the open segment and opening a new one.
     * @param list<array{int, string}> $units Characters of the run.
     *
     * @return array{int, int, int} Move.
     */
    protected function getTextRunCost(int $state, int $open, array $units): array
    {
        $sub = $state === self::STATE_TEXT_TWO ? 1 : 0;
        $inside = $state === self::STATE_TEXT_ONE || $state === self::STATE_TEXT_TWO;
        $bits = $inside ? 0 : $open;
        foreach ($units as $unit) {
            $next = $this->isTextOne(\ord($unit[1])) ? 0 : 1;
            $bits += $next === $sub ? 6 : 12;
            $sub = $next;
        }

        return [$sub === 0 ? self::STATE_TEXT_ONE : self::STATE_TEXT_TWO, $bits, self::UNIT_TEXT];
    }

    /**
     * Returns the number of bits the characters of one run of common Chinese
     * characters take, the region switches included.
     *
     * @param list<array{int, string}> $units Characters of the run.
     */
    protected function getHanziRunBits(array $units): int
    {
        $bits = 0;
        $region = ($units[0] ?? [self::UNIT_HANZI_ONE, ''])[0];
        foreach ($units as $unit) {
            $next = $unit[0];
            $bits += $next === $region ? 12 : 24;
            $region = $next;
        }

        return $bits;
    }

    /**
     * Returns the bit stream of one segment.
     *
     * @param int                      $mode  Mode of the segment.
     * @param list<array{int, string}> $units Characters of the segment.
     */
    protected function getSegmentStream(int $mode, array $units): string
    {
        $bytes = '';
        foreach ($units as $unit) {
            $bytes .= $unit[1];
        }

        return match ($mode) {
            self::UNIT_DIGIT => $this->getNumericStream($bytes),
            self::UNIT_TEXT => $this->getTextStream($bytes),
            self::UNIT_HANZI_ONE => $this->getHanziStream($units),
            self::UNIT_GB_TWO => $this->getGbTwoStream($units),
            self::UNIT_GB_FOUR => $this->getGbFourStream($bytes),
            default => $this->getBinaryStream($bytes),
        };
    }

    /**
     * Returns the bit stream of a numeric mode segment: groups of three digits
     * in ten bits, closed by the terminator of the length of the last group.
     *
     * @param string $bytes Digits of the segment.
     */
    protected function getNumericStream(string $bytes): string
    {
        $bits = Data::MODE_NUMERIC;
        $len = \strlen($bytes);
        for ($idx = 0; $idx < $len; $idx += 3) {
            $group = \substr($bytes, $idx, 3);
            $bits .= \str_pad(\decbin((int) $group), 10, '0', \STR_PAD_LEFT);
        }

        return $bits . (Data::NUMERIC_TERMINATOR[(($len - 1) % 3) + 1] ?? '');
    }

    /**
     * Returns the bit stream of a Text mode segment: six bits per character,
     * with a switch between the sub modes of Table 3 and Table 4.
     *
     * @param string $bytes Characters of the segment.
     */
    protected function getTextStream(string $bytes): string
    {
        $bits = Data::MODE_TEXT;
        $sub = 0;
        $len = \strlen($bytes);
        for ($idx = 0; $idx < $len; ++$idx) {
            $byte = \ord($bytes[$idx]);
            $next = $this->isTextOne($byte) ? 0 : 1;
            if ($next !== $sub) {
                $bits .= Data::TEXT_SWITCH;
                $sub = $next;
            }

            $bits .= \str_pad(\decbin($this->getTextCode($byte)), 6, '0', \STR_PAD_LEFT);
        }

        return $bits . Data::TEXT_TERMINATOR;
    }

    /**
     * Returns the bit stream of a binary byte mode segment: the character
     * count indicator followed by the bytes.
     *
     * @param string $bytes Bytes of the segment.
     */
    protected function getBinaryStream(string $bytes): string
    {
        $len = \strlen($bytes);
        $bits = Data::MODE_BINARY . \str_pad(\decbin($len), Data::BINARY_COUNT_BITS, '0', \STR_PAD_LEFT);
        for ($idx = 0; $idx < $len; ++$idx) {
            $bits .= \str_pad(\decbin(\ord($bytes[$idx])), 8, '0', \STR_PAD_LEFT);
        }

        return $bits;
    }

    /**
     * Returns the bit stream of a common Chinese character segment: twelve
     * bits per character, with a switch between the two regions.
     *
     * @param list<array{int, string}> $units Characters of the segment.
     */
    protected function getHanziStream(array $units): string
    {
        $region = ($units[0] ?? [self::UNIT_HANZI_ONE, ''])[0];
        $bits = $region === self::UNIT_HANZI_ONE ? Data::MODE_HANZI_ONE : Data::MODE_HANZI_TWO;
        foreach ($units as $unit) {
            $pair = $unit[1];
            $next = $unit[0];
            if ($next !== $region) {
                $bits .= Data::HANZI_SWITCH;
                $region = $next;
            }

            $value = $next === self::UNIT_HANZI_ONE
                ? $this->getHanziOneValue(\ord($pair[0]), \ord($pair[1]))
                : $this->getHanziTwoValue(\ord($pair[0]), \ord($pair[1]));
            $bits .= \str_pad(\decbin($value), 12, '0', \STR_PAD_LEFT);
        }

        return $bits . Data::HANZI_TERMINATOR;
    }

    /**
     * Returns the bit stream of a GB 18030 two byte region segment: fifteen
     * bits per character.
     *
     * @param list<array{int, string}> $units Characters of the segment.
     */
    protected function getGbTwoStream(array $units): string
    {
        $bits = Data::MODE_GB_TWO;
        foreach ($units as $unit) {
            $pair = $unit[1];
            $value = $this->getGbTwoValue(\ord($pair[0]), \ord($pair[1]));
            $bits .= \str_pad(\decbin($value), 15, '0', \STR_PAD_LEFT);
        }

        return $bits . Data::GB_TWO_TERMINATOR;
    }

    /**
     * Returns the bit stream of a GB 18030 four byte region segment, which
     * carries one character in twenty one bits and has no terminator.
     *
     * @param string $bytes Bytes of the character.
     */
    protected function getGbFourStream(string $bytes): string
    {
        $value =
            \ord($bytes[3]) - 0x30
            + ((\ord($bytes[2]) - 0x81) * 0x0A)
            + ((\ord($bytes[1]) - 0x30) * 0x04EC)
            + ((\ord($bytes[0]) - 0x81) * 0x3138);

        return Data::MODE_GB_FOUR . \str_pad(\decbin($value), 21, '0', \STR_PAD_LEFT);
    }

    /**
     * Returns whether the byte is a character of the Text1 sub mode.
     *
     * @param int $byte Byte value.
     */
    protected function isTextOne(int $byte): bool
    {
        return $byte >= 0x30 && $byte <= 0x39 || $byte >= 0x41 && $byte <= 0x5A || $byte >= 0x61 && $byte <= 0x7A;
    }

    /**
     * Returns the six bit code of a Text mode character, or a negative value
     * when neither sub mode represents the byte.
     *
     * @param int $byte Byte value.
     */
    protected function getTextCode(int $byte): int
    {
        if ($byte >= 0x30 && $byte <= 0x39) {
            return $byte - 0x30;
        }

        if ($byte >= 0x41 && $byte <= 0x5A) {
            return $byte - 0x41 + 10;
        }

        if ($byte >= 0x61 && $byte <= 0x7A) {
            return $byte - 0x61 + 36;
        }

        foreach (Data::TEXT_TWO_RANGES as $range) {
            if ($byte >= $range[0] && $byte <= $range[1]) {
                return $byte - $range[2];
            }
        }

        return -1;
    }

    /**
     * Returns the twelve bit value of a character of the common Chinese
     * character region one, or a negative value when the byte pair is not one.
     *
     * @param int $first  First byte.
     * @param int $second Second byte.
     */
    protected function getHanziOneValue(int $first, int $second): int
    {
        if ($second >= 0xA1 && $second <= 0xFE) {
            if ($first >= 0xB0 && $first <= 0xD7) {
                return (($first - 0xB0) * 0x5E) + ($second - 0xA1);
            }

            if ($first >= 0xA1 && $first <= 0xA3) {
                return (($first - 0xA1) * 0x5E) + $second - 0xA1 + 0xEB0;
            }
        }

        if ($first === 0xA8 && $second >= 0xA1 && $second <= 0xC0) {
            return $second - 0xA1 + 0xFCA;
        }

        return -1;
    }

    /**
     * Returns the twelve bit value of a character of the common Chinese
     * character region two, or a negative value when the byte pair is not one.
     *
     * @param int $first  First byte.
     * @param int $second Second byte.
     */
    protected function getHanziTwoValue(int $first, int $second): int
    {
        if ($first >= 0xD8 && $first <= 0xF7 && $second >= 0xA1 && $second <= 0xFE) {
            return (($first - 0xD8) * 0x5E) + ($second - 0xA1);
        }

        return -1;
    }

    /**
     * Returns the fifteen bit value of a character of the GB 18030 two byte
     * region.
     *
     * @param int $first  First byte.
     * @param int $second Second byte.
     */
    protected function getGbTwoValue(int $first, int $second): int
    {
        $low = $second <= 0x7E ? $second - 0x40 : $second - 0x41;

        return (($first - 0x81) * 0xBE) + $low;
    }
}
