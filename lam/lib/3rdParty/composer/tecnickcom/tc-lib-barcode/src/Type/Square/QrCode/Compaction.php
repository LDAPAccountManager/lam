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

namespace Com\Tecnick\Barcode\Type\Square\QrCode;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Compaction
 *
 * Encodation modes of the QrCode Barcode type class, sections 7.4.3 to 7.4.7 of
 * ISO/IEC 18004.
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
     * Number of characters of a group of each encodation mode. A numeric group
     * of three characters and an alphanumeric group of two are encoded as a
     * unit, so the bit count of a segment depends on the number of characters
     * modulo this value.
     *
     * @var array<int, int>
     */
    protected const GROUP_SIZE = [3, 2, 1, 1];

    /**
     * Number of bits added by the character that brings a group of the mode to
     * the given number of characters, section 7.4.3 to 7.4.6 of ISO/IEC 18004.
     * A numeric group is 4, 7 and 10 bits long and an alphanumeric one 6 and 11,
     * so the increments are the differences of those.
     *
     * @var array<int, array<int, int>>
     */
    protected const GROUP_INCREMENT = [
        [4, 3, 3],
        [6, 5],
        [8],
        [13],
    ];

    /**
     * Returns the value of a character in the alphanumeric set of Table 5 of
     * ISO/IEC 18004, or a negative value when the set does not hold it.
     *
     * @param int $ord Byte value of the character.
     */
    protected function alphanumericValue(int $ord): int
    {
        $pos = \strpos(Data::AN_CHARS, \chr($ord));

        return $pos === false ? -1 : $pos;
    }

    /**
     * Returns whether the byte at the given offset is a digit.
     *
     * @param string $code Code to encode.
     * @param int    $pos  Byte offset.
     */
    protected function isDigitAt(string $code, int $pos): bool
    {
        $chr = $code[$pos] ?? '';

        return $chr >= '0' && $chr <= '9';
    }

    /**
     * Returns whether the byte at the given offset belongs to the alphanumeric
     * character set.
     *
     * @param string $code Code to encode.
     * @param int    $pos  Byte offset.
     */
    protected function isAlphanumericAt(string $code, int $pos): bool
    {
        $chr = $code[$pos] ?? '';

        return $chr !== '' && $this->alphanumericValue(\ord($chr)) >= 0;
    }

    /**
     * Returns the thirteen bit value of the Shift JIS character pair at the
     * given offset, section 7.4.6 of ISO/IEC 18004, or a negative value when the
     * pair is outside the two ranges the kanji mode encodes.
     *
     * @param string $code Code to encode.
     * @param int    $pos  Byte offset.
     */
    protected function kanjiValueAt(string $code, int $pos): int
    {
        $high = $code[$pos] ?? '';
        $low = $code[$pos + 1] ?? '';
        if ($high === '' || $low === '') {
            return -1;
        }

        $value = (\ord($high) << 8) | \ord($low);
        foreach (Data::KANJI_RANGES as $range) {
            if ($value < $range[0] || $value > $range[1]) {
                continue;
            }

            $shifted = $value - $range[2];

            return (($shifted >> 8) * 0xC0) + ($shifted & 0xFF);
        }

        return -1;
    }

    /**
     * Returns whether the mode can encode the character at the given offset.
     *
     * @param string $code Code to encode.
     * @param int    $pos  Byte offset.
     * @param int    $mode Encodation mode.
     */
    protected function isEncodableAt(string $code, int $pos, int $mode): bool
    {
        return match ($mode) {
            Data::MODE_NUMERIC => $this->isDigitAt($code, $pos),
            Data::MODE_ALPHANUM => $this->isAlphanumericAt($code, $pos),
            Data::MODE_KANJI => $this->kanjiValueAt($code, $pos) >= 0,
            default => isset($code[$pos]),
        };
    }

    /**
     * Returns the number of bits taken by the given number of characters of the
     * mode, excluding the mode and character count indicators.
     *
     * @param int $mode  Encodation mode.
     * @param int $count Number of characters.
     */
    protected function getDataBits(int $mode, int $count): int
    {
        $group = self::GROUP_SIZE[$mode] ?? 1;
        $increments = self::GROUP_INCREMENT[$mode] ?? [8];
        $bits = \intdiv($count, $group) * \array_sum($increments);
        for ($idx = 0; $idx < ($count % $group); ++$idx) {
            $bits += $increments[$idx] ?? 0;
        }

        return $bits;
    }

    /**
     * Returns the number of bits of the mode and character count indicators of a
     * segment of the mode, Tables 2 and 3 of ISO/IEC 18004.
     *
     * @param int $mode  Encodation mode.
     * @param int $group Version group of Data::COUNT_BITS.
     */
    protected function getHeaderBits(int $mode, int $group): int
    {
        return Data::MODE_BITS + (Data::COUNT_BITS[$group][$mode] ?? 0);
    }

    /**
     * Returns the highest number of characters a segment of the mode can
     * declare in its character count indicator.
     *
     * @param int $mode  Encodation mode.
     * @param int $group Version group of Data::COUNT_BITS.
     */
    protected function getMaxCount(int $mode, int $group): int
    {
        return (1 << (Data::COUNT_BITS[$group][$mode] ?? 0)) - 1;
    }

    /**
     * Returns the number of bits taken by the whole bit stream of the segments,
     * excluding the terminator.
     *
     * @param array<int, array{int, int, int}> $segments Segments as mode, byte offset and number of characters.
     * @param int                              $group    Version group of Data::COUNT_BITS.
     */
    protected function getStreamBits(array $segments, int $group): int
    {
        $bits = 0;
        foreach ($segments as $segment) {
            $bits += $this->getHeaderBits($segment[0], $group) + $this->getDataBits($segment[0], $segment[2]);
        }

        return $bits;
    }

    /**
     * Returns the sequence of mode segments that encodes the code in the fewest
     * bits, as the mode, the byte offset and the number of characters of each.
     *
     * Section 7.4.7 of ISO/IEC 18004 allows the mode to change within a symbol
     * and Annex J gives a heuristic for choosing where. This is the exact
     * equivalent: the shortest path over the states of byte offset, mode and
     * number of characters modulo the group size of the mode, whose edge weights
     * are the bit counts the encodation clauses state.
     *
     * @param string $code  Code to encode.
     * @param int    $group Version group of Data::COUNT_BITS.
     * @param bool   $kanji Whether the kanji mode may be used.
     *
     * @return array<int, array{int, int, int}>
     */
    protected function getSegments(string $code, int $group, bool $kanji): array
    {
        $modes = $kanji
            ? [Data::MODE_NUMERIC, Data::MODE_ALPHANUM, Data::MODE_BYTE, Data::MODE_KANJI]
            : [Data::MODE_NUMERIC, Data::MODE_ALPHANUM, Data::MODE_BYTE];
        $len = \strlen($code);
        $cost = [];
        $from = [];
        foreach ($modes as $mode) {
            $cost[0][$mode][0] = $this->getHeaderBits($mode, $group);
            $from[0][$mode][0] = null;
        }

        for ($pos = 0; $pos <= $len; ++$pos) {
            $this->relaxModeChange($cost, $from, $pos, $modes, $group);
            if ($pos === $len) {
                continue;
            }

            $this->relaxSameMode($cost, $from, $pos, $modes, $code);
        }

        return $this->splitLongSegments($this->tracePath($from, $this->getBestState($cost, $len)), $group);
    }

    /**
     * Relax the transitions that close the current segment at the given offset
     * and open a new one in another mode.
     *
     * @param array<int, array<int, array<int, int>>>                    $cost  Cost of each state.
     * @param array<int, array<int, array<int, ?array{int, int, int}>>>  $from  Predecessor of each state.
     * @param int                                                        $pos   Byte offset.
     * @param array<int, int>                                            $modes Modes in use.
     * @param int                                                        $group Version group of Data::COUNT_BITS.
     */
    protected function relaxModeChange(array &$cost, array &$from, int $pos, array $modes, int $group): void
    {
        foreach ($modes as $mode) {
            foreach ($cost[$pos][$mode] ?? [] as $phase => $bits) {
                foreach ($modes as $next) {
                    if ($next === $mode) {
                        continue;
                    }

                    $value = $bits + $this->getHeaderBits($next, $group);
                    if (($cost[$pos][$next][0] ?? \PHP_INT_MAX) <= $value) {
                        continue;
                    }

                    $cost[$pos][$next][0] = $value;
                    $from[$pos][$next][0] = [$pos, $mode, $phase];
                }
            }
        }
    }

    /**
     * Relax the transitions that take the next character in the current mode.
     *
     * @param array<int, array<int, array<int, int>>>                   $cost  Cost of each state.
     * @param array<int, array<int, array<int, ?array{int, int, int}>>> $from  Predecessor of each state.
     * @param int                                                       $pos   Byte offset.
     * @param array<int, int>                                           $modes Modes in use.
     * @param string                                                    $code  Code to encode.
     */
    protected function relaxSameMode(array &$cost, array &$from, int $pos, array $modes, string $code): void
    {
        foreach ($modes as $mode) {
            if (!$this->isEncodableAt($code, $pos, $mode)) {
                continue;
            }

            $step = $mode === Data::MODE_KANJI ? 2 : 1;
            $size = self::GROUP_SIZE[$mode] ?? 1;
            foreach ($cost[$pos][$mode] ?? [] as $phase => $bits) {
                $value = $bits + (self::GROUP_INCREMENT[$mode][$phase] ?? 0);
                $next = ($phase + 1) % $size;
                if (($cost[$pos + $step][$mode][$next] ?? \PHP_INT_MAX) <= $value) {
                    continue;
                }

                $cost[$pos + $step][$mode][$next] = $value;
                $from[$pos + $step][$mode][$next] = [$pos, $mode, $phase];
            }
        }
    }

    /**
     * Returns the cheapest state at the end of the code.
     *
     * @param array<int, array<int, array<int, int>>> $cost Cost of each state.
     * @param int                                     $len  Length of the code in bytes.
     *
     * @return array{int, int, int}
     */
    protected function getBestState(array $cost, int $len): array
    {
        $best = [$len, Data::MODE_BYTE, 0];
        $bits = \PHP_INT_MAX;
        foreach ($cost[$len] ?? [] as $mode => $phases) {
            foreach ($phases as $phase => $value) {
                if ($value >= $bits) {
                    continue;
                }

                $bits = $value;
                $best = [$len, $mode, $phase];
            }
        }

        return $best;
    }

    /**
     * Walk the predecessors back from the given state and return the segments in
     * encoding order.
     *
     * @param array<int, array<int, array<int, ?array{int, int, int}>>> $from  Predecessor of each state.
     * @param array{int, int, int}                                      $state State to start from.
     *
     * @return array<int, array{int, int, int}>
     */
    protected function tracePath(array $from, array $state): array
    {
        $segments = [];
        $count = 0;
        while (true) {
            [$pos, $mode, $phase] = $state;
            $previous = $from[$pos][$mode][$phase] ?? null;
            // a predecessor in the same mode is a character of the current
            // segment, one in another mode is the start of the current segment
            if ($previous !== null && $previous[1] === $mode) {
                ++$count;
                $state = $previous;
                continue;
            }

            if ($count > 0) {
                $segments[] = [$mode, $pos, $count];
            }

            if ($previous === null) {
                break;
            }

            $count = 0;
            $state = $previous;
        }

        return \array_reverse($segments);
    }

    /**
     * Split every segment whose character count is beyond what its character
     * count indicator can hold, on a group boundary of the mode.
     *
     * @param array<int, array{int, int, int}> $segments Segments as mode, byte offset and number of characters.
     * @param int                              $group    Version group of Data::COUNT_BITS.
     *
     * @return array<int, array{int, int, int}>
     */
    protected function splitLongSegments(array $segments, int $group): array
    {
        $result = [];
        foreach ($segments as $segment) {
            [$mode, $offset, $count] = $segment;
            $size = self::GROUP_SIZE[$mode] ?? 1;
            $step = $mode === Data::MODE_KANJI ? 2 : 1;
            $max = $this->getMaxCount($mode, $group);
            $max -= $max % $size;
            while ($count > $max && $max > 0) {
                $result[] = [$mode, $offset, $max];
                $offset += $max * $step;
                $count -= $max;
            }

            $result[] = [$mode, $offset, $count];
        }

        return $result;
    }

    /**
     * Returns the bit stream of a segment: the mode indicator, the character
     * count indicator and the encoded characters.
     *
     * @param string             $code    Code to encode.
     * @param array{int, int, int} $segment Segment as mode, byte offset and number of characters.
     * @param int                $group   Version group of Data::COUNT_BITS.
     */
    protected function getSegmentStream(string $code, array $segment, int $group): string
    {
        [$mode, $offset, $count] = $segment;
        $bits = $this->getBits(Data::MODE_INDICATOR[$mode] ?? 0, Data::MODE_BITS);
        $bits .= $this->getBits($count, Data::COUNT_BITS[$group][$mode] ?? 0);

        return $bits . match ($mode) {
            Data::MODE_NUMERIC => $this->getNumericStream($code, $offset, $count),
            Data::MODE_ALPHANUM => $this->getAlphanumStream($code, $offset, $count),
            Data::MODE_KANJI => $this->getKanjiStream($code, $offset, $count),
            default => $this->getByteStream($code, $offset, $count),
        };
    }

    /**
     * Returns the bit stream of the numeric mode, section 7.4.3 of
     * ISO/IEC 18004: groups of three digits in ten bits, and one or two trailing
     * digits in four or seven bits.
     *
     * @param string $code   Code to encode.
     * @param int    $offset Byte offset of the segment.
     * @param int    $count  Number of characters.
     */
    protected function getNumericStream(string $code, int $offset, int $count): string
    {
        $bits = '';
        for ($idx = 0; $idx < $count; $idx += 3) {
            $group = \substr($code, $offset + $idx, \min(3, $count - $idx));
            $bits .= $this->getBits((int) $group, $this->getDataBits(Data::MODE_NUMERIC, \strlen($group)));
        }

        return $bits;
    }

    /**
     * Returns the bit stream of the alphanumeric mode, section 7.4.4 of
     * ISO/IEC 18004: pairs of characters in eleven bits, and one trailing
     * character in six bits.
     *
     * @param string $code   Code to encode.
     * @param int    $offset Byte offset of the segment.
     * @param int    $count  Number of characters.
     */
    protected function getAlphanumStream(string $code, int $offset, int $count): string
    {
        $bits = '';
        for ($idx = 0; $idx < $count; $idx += 2) {
            $first = $this->alphanumericValue(\ord($code[$offset + $idx] ?? '0'));
            if (($idx + 1) >= $count) {
                $bits .= $this->getBits(\max(0, $first), 6);
                break;
            }

            $second = $this->alphanumericValue(\ord($code[$offset + $idx + 1] ?? '0'));
            $bits .= $this->getBits((45 * \max(0, $first)) + \max(0, $second), 11);
        }

        return $bits;
    }

    /**
     * Returns the bit stream of the byte mode, section 7.4.5 of ISO/IEC 18004:
     * eight bits per character.
     *
     * @param string $code   Code to encode.
     * @param int    $offset Byte offset of the segment.
     * @param int    $count  Number of characters.
     */
    protected function getByteStream(string $code, int $offset, int $count): string
    {
        $bits = '';
        for ($idx = 0; $idx < $count; ++$idx) {
            $bits .= $this->getBits(\ord($code[$offset + $idx] ?? "\0"), 8);
        }

        return $bits;
    }

    /**
     * Returns the bit stream of the kanji mode, section 7.4.6 of ISO/IEC 18004:
     * thirteen bits per character pair.
     *
     * @param string $code   Code to encode.
     * @param int    $offset Byte offset of the segment.
     * @param int    $count  Number of characters.
     */
    protected function getKanjiStream(string $code, int $offset, int $count): string
    {
        $bits = '';
        for ($idx = 0; $idx < $count; ++$idx) {
            $bits .= $this->getBits(\max(0, $this->kanjiValueAt($code, $offset + (2 * $idx))), 13);
        }

        return $bits;
    }

    /**
     * Returns the binary representation of a value over a fixed number of bits.
     *
     * @param int $value Value to represent.
     * @param int $size  Number of bits.
     */
    protected function getBits(int $value, int $size): string
    {
        if ($size <= 0) {
            return '';
        }

        return \substr(\str_pad(\decbin($value), $size, '0', \STR_PAD_LEFT), -$size);
    }
}
