<?php

declare(strict_types=1);

/**
 * Encode.php
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

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\ReedSolomon;

/**
 * Com\Tecnick\Barcode\Type\Square\QrCode\Encode
 *
 * Symbol builder for the QrCode Barcode type class, sections 7.5 to 7.10 of
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
class Encode extends \Com\Tecnick\Barcode\Type\Square\QrCode\Compaction
{
    /**
     * Number of data mask patterns, Table 10 of ISO/IEC 18004.
     *
     * @var int
     */
    public const MASKS = 8;

    /**
     * Highest number of characters any version and mode can carry, which is the
     * numeric capacity of the version 40 at the error correction level L.
     *
     * @var int
     */
    public const MAX_CHARACTERS = 7089;

    /**
     * Module sequence of the 1:1:3:1:1 ratio pattern of Table 11 of
     * ISO/IEC 18004.
     *
     * @var string
     */
    protected const FINDER_PATTERN = '1011101';

    /**
     * Module sequence of the light area Table 11 of ISO/IEC 18004 requires
     * beside the 1:1:3:1:1 ratio pattern.
     *
     * @var string
     */
    protected const FINDER_LIGHT = '0000';

    /**
     * Number of modules of the encoding region by version, filled in as versions
     * are used. It follows from the geometry of the symbol, so it is computed
     * rather than tabulated.
     *
     * @var array<int, int>
     */
    protected static array $encodingModules = [];

    /**
     * Symbol version, from 1 to 40.
     */
    protected int $version = 1;

    /**
     * Error correction level, from 0 to 3, that is L, M, Q and H.
     */
    protected int $level = 0;

    /**
     * Symbol size in modules.
     */
    protected int $size = 21;

    /**
     * Data mask pattern reference, from 0 to 7.
     */
    protected int $mask = 0;

    /**
     * Module matrix: 1 is a dark module, 0 a light one.
     *
     * @var array<int, array<int, int>>
     */
    protected array $matrix = [];

    /**
     * Modules taken by the function patterns, the format information and the
     * version information.
     *
     * @var array<int, array<int, bool>>
     */
    protected array $function = [];

    /**
     * Build the symbol.
     *
     * @param string $code        Code to encode.
     * @param int    $level       Error correction level, from 0 to 3.
     * @param int    $version     Requested version, from 1 to 40, or 0 for the
     *                            smallest version that carries the code.
     * @param bool   $kanji       Whether the kanji mode may be used.
     * @param int    $randomMask  Number of randomly chosen masks to evaluate, or
     *                            a negative value to evaluate every mask.
     * @param bool   $bestMask    Whether to evaluate the masks at all.
     * @param int    $defaultMask Mask applied when the masks are not evaluated.
     *
     * @throws BarcodeException in case of error
     * @throws \Random\RandomException in case of random generation error
     */
    public function __construct(
        string $code,
        int $level,
        int $version,
        bool $kanji,
        int $randomMask = -1,
        bool $bestMask = true,
        int $defaultMask = 2,
    ) {
        $this->level = $level;
        $segments = $this->setVersion($code, $version, $kanji);
        $this->size = (4 * $this->version) + 17;
        $this->setFunctionPatterns();
        $this->setVersionInformation();
        $this->setData($this->getBitSequence($code, $segments));
        $this->setMask($randomMask, $bestMask, $defaultMask);
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
     * Returns the symbol version, from 1 to 40.
     */
    public function getVersion(): int
    {
        return $this->version;
    }

    /**
     * Returns the number of data codewords of a version and error correction
     * level: the codeword capacity of the version less the error correction
     * codewords of Table 9 of ISO/IEC 18004.
     *
     * @param int $version Symbol version.
     * @param int $level   Error correction level.
     */
    public static function getDataCodewords(int $version, int $level): int
    {
        $block = Data::ECC_BLOCKS[$version][$level] ?? [0, 0];

        return self::getTotalCodewords($version) - ($block[0] * $block[1]);
    }

    /**
     * Returns the codeword capacity of a version, that is the number of modules
     * of the encoding region divided by eight, section 7.4.10 of ISO/IEC 18004.
     *
     * @param int $version Symbol version.
     */
    public static function getTotalCodewords(int $version): int
    {
        return \intdiv(self::getEncodingModules($version), 8);
    }

    /**
     * Returns the number of remainder bits of a version, that is the modules of
     * the encoding region left over by the last codeword.
     *
     * @param int $version Symbol version.
     */
    public static function getRemainderBits(int $version): int
    {
        return self::getEncodingModules($version) % 8;
    }

    /**
     * Returns the number of modules of the encoding region of a version, that is
     * every module the function patterns, the format information and the version
     * information do not take.
     *
     * @param int $version Symbol version.
     */
    public static function getEncodingModules(int $version): int
    {
        if (isset(self::$encodingModules[$version])) {
            return self::$encodingModules[$version];
        }

        $geometry = new Geometry($version);
        $size = $geometry->getSize();
        $free = 0;
        for ($row = 0; $row < $size; ++$row) {
            for ($col = 0; $col < $size; ++$col) {
                if ($geometry->isFunctionModule($row, $col)) {
                    continue;
                }

                ++$free;
            }
        }

        self::$encodingModules[$version] = $free;

        return $free;
    }

    /**
     * Select the version that carries the code and return the segments it is
     * encoded in.
     *
     * @param string $code    Code to encode.
     * @param int    $version Requested version, or 0 for the smallest one.
     * @param bool   $kanji   Whether the kanji mode may be used.
     *
     * @return array<int, array{int, int, int}>
     *
     * @throws BarcodeException if the code does not fit in any symbol
     */
    protected function setVersion(string $code, int $version, bool $kanji): array
    {
        if (\strlen($code) > self::MAX_CHARACTERS) {
            throw new BarcodeException('The data does not fit in a QR Code symbol');
        }

        $first = $version > 0 ? $version : Data::VERSION_MIN;
        $last = $version > 0 ? $version : Data::VERSION_MAX;
        $cache = [];
        for ($candidate = $first; $candidate <= $last; ++$candidate) {
            $group = $this->getVersionGroup($candidate);
            $cache[$group] ??= $this->getSegments($code, $group, $kanji);
            $bits = $this->getStreamBits($cache[$group], $group);
            if ($bits <= (8 * self::getDataCodewords($candidate, $this->level))) {
                $this->version = $candidate;

                return $cache[$group];
            }
        }

        if ($version > 0) {
            throw new BarcodeException('The data does not fit in the requested symbol version');
        }

        throw new BarcodeException('The data does not fit in a QR Code symbol, try a lower error correction level');
    }

    /**
     * Returns the version group of Data::COUNT_BITS the version belongs to.
     *
     * @param int $version Symbol version.
     */
    protected function getVersionGroup(int $version): int
    {
        foreach (Data::COUNT_GROUP_MAX as $group => $max) {
            if ($version <= $max) {
                return $group;
            }
        }

        return \count(Data::COUNT_GROUP_MAX) - 1;
    }

    /**
     * Returns the bit sequence of the symbol: the interleaved data and error
     * correction codewords of every block, and the remainder bits, sections 7.5
     * and 7.6 of ISO/IEC 18004.
     *
     * @param string                           $code     Code to encode.
     * @param array<int, array{int, int, int}> $segments Segments of the code.
     */
    protected function getBitSequence(string $code, array $segments): string
    {
        $codewords = $this->getDataStream($code, $segments);
        $blocks = $this->getBlockSizes();
        $errorCorrection = new ReedSolomon(8, ReedSolomon::GF_QRCODE, 0);
        $ecc = Data::ECC_BLOCKS[$this->version][$this->level][0] ?? 0;
        $data = [];
        $check = [];
        $pos = 0;
        foreach ($blocks as $length) {
            $block = \array_slice($codewords, $pos, $length);
            $pos += $length;
            $data[] = $block;
            $check[] = $errorCorrection->checkwords($block, $ecc);
        }

        $bits = '';
        foreach ($this->interleave($data) as $codeword) {
            $bits .= $this->getBits($codeword, 8);
        }

        foreach ($this->interleave($check) as $codeword) {
            $bits .= $this->getBits($codeword, 8);
        }

        return $bits . \str_repeat('0', \max(0, self::getRemainderBits($this->version)));
    }

    /**
     * Returns the data codewords of the symbol: the bit stream of every segment,
     * the terminator, the padding bits and the pad codewords, sections 7.4.9 and
     * 7.4.10 of ISO/IEC 18004.
     *
     * @param string                           $code     Code to encode.
     * @param array<int, array{int, int, int}> $segments Segments of the code.
     *
     * @return array<int, int>
     */
    protected function getDataStream(string $code, array $segments): array
    {
        $group = $this->getVersionGroup($this->version);
        $capacity = 8 * self::getDataCodewords($this->version, $this->level);
        $bits = '';
        foreach ($segments as $segment) {
            $bits .= $this->getSegmentStream($code, $segment, $group);
        }

        $bits .= \str_repeat('0', \max(0, \min(Data::TERMINATOR_BITS, $capacity - \strlen($bits))));
        $bits .= \str_repeat('0', \max(0, (8 - (\strlen($bits) % 8)) % 8));
        $codewords = [];
        $len = \strlen($bits);
        for ($pos = 0; $pos < $len; $pos += 8) {
            $codewords[] = (int) \bindec(\substr($bits, $pos, 8));
        }

        $index = 0;
        while (\count($codewords) < ($capacity / 8)) {
            $codewords[] = Data::PAD_CODEWORDS[$index % 2] ?? 0;
            ++$index;
        }

        return $codewords;
    }

    /**
     * Returns the number of data codewords of each block of the symbol,
     * shortest first, section 7.6 of ISO/IEC 18004. Table 9 states the block
     * sizes; they follow from the number of blocks and the number of data
     * codewords, because the codewords are shared out as evenly as possible.
     *
     * @return array<int, int>
     */
    protected function getBlockSizes(): array
    {
        $count = Data::ECC_BLOCKS[$this->version][$this->level][1] ?? 1;
        $total = self::getDataCodewords($this->version, $this->level);
        $short = \intdiv($total, $count);
        $long = $total % $count;
        $sizes = \array_fill(0, \max(0, $count - $long), $short);
        for ($idx = 0; $idx < $long; ++$idx) {
            $sizes[] = $short + 1;
        }

        return $sizes;
    }

    /**
     * Returns the codewords of the blocks taken one from each block in turn,
     * skipping the blocks that are already exhausted, section 7.6 of
     * ISO/IEC 18004.
     *
     * @param array<int, array<int, int>> $blocks Codewords of each block.
     *
     * @return array<int, int>
     */
    protected function interleave(array $blocks): array
    {
        $result = [];
        $longest = 0;
        foreach ($blocks as $block) {
            $longest = \max($longest, \count($block));
        }

        for ($idx = 0; $idx < $longest; ++$idx) {
            foreach ($blocks as $block) {
                $codeword = $block[$idx] ?? null;
                if ($codeword === null) {
                    continue;
                }

                $result[] = $codeword;
            }
        }

        return $result;
    }

    /**
     * Draw the function patterns and reserve the modules of the format and
     * version information, section 7.7.2 of ISO/IEC 18004.
     */
    protected function setFunctionPatterns(): void
    {
        $geometry = new Geometry($this->version);
        $this->matrix = [];
        $this->function = [];
        for ($row = 0; $row < $this->size; ++$row) {
            for ($col = 0; $col < $this->size; ++$col) {
                $this->function[$row][$col] = $geometry->isFunctionModule($row, $col);
                $this->matrix[$row][$col] = $geometry->isDarkModule($row, $col) ? 1 : 0;
            }
        }
    }

    /**
     * Place the bit sequence in the encoding region: two module wide columns
     * from right to left, filled upwards and downwards in turn, skipping the
     * column of the vertical timing pattern, section 7.7.3 of ISO/IEC 18004.
     *
     * @param string $sequence Bit sequence of the symbol.
     */
    protected function setData(string $sequence): void
    {
        $pos = 0;
        $len = \strlen($sequence);
        $upward = true;
        for ($col = $this->size - 1; $col > 0; $col -= 2) {
            if ($col === 6) {
                --$col;
            }

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
     * Apply a data mask pattern and store its reference.
     *
     * @param int  $randomMask  Number of randomly chosen masks to evaluate, or a
     *                          negative value to evaluate every mask.
     * @param bool $bestMask    Whether to evaluate the masks at all.
     * @param int  $defaultMask Mask applied when the masks are not evaluated.
     *
     * @throws \Random\RandomException in case of random generation error
     */
    protected function setMask(int $randomMask, bool $bestMask, int $defaultMask): void
    {
        if (!$bestMask) {
            $this->mask = $defaultMask;
            $this->applyMask($this->mask);

            return;
        }

        $unmasked = $this->matrix;
        $best = \PHP_INT_MAX;
        foreach ($this->getCandidateMasks($randomMask) as $pattern) {
            $this->matrix = $unmasked;
            $this->applyMask($pattern);
            $this->setFormatInformation($pattern);
            $penalty = $this->getMaskPenalty();
            if ($penalty >= $best) {
                continue;
            }

            $best = $penalty;
            $this->mask = $pattern;
        }

        $this->matrix = $unmasked;
        $this->applyMask($this->mask);
    }

    /**
     * Returns the masks to evaluate.
     *
     * @param int $randomMask Number of randomly chosen masks, or a negative
     *                        value for every mask.
     *
     * @return array<int, int>
     *
     * @throws \Random\RandomException in case of random generation error
     */
    protected function getCandidateMasks(int $randomMask): array
    {
        $masks = \range(0, self::MASKS - 1);
        if ($randomMask < 0 || $randomMask >= self::MASKS) {
            return $masks;
        }

        $picked = [];
        for ($idx = 0; $idx < \max(1, $randomMask); ++$idx) {
            $picked[] = $masks[\random_int(0, self::MASKS - 1)] ?? 0;
        }

        return \array_values(\array_unique($picked));
    }

    /**
     * Invert the modules of the encoding region on which the mask condition
     * holds, section 7.8 of ISO/IEC 18004.
     *
     * @param int $pattern Data mask pattern reference, from 0 to 7.
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
     * @param int $pattern Data mask pattern reference, from 0 to 7.
     * @param int $row     Row of the module.
     * @param int $col     Column of the module.
     */
    protected function isMasked(int $pattern, int $row, int $col): bool
    {
        return match ($pattern) {
            0 => (($row + $col) % 2) === 0,
            1 => ($row % 2) === 0,
            2 => ($col % 3) === 0,
            3 => (($row + $col) % 3) === 0,
            4 => ((\intdiv($row, 2) + \intdiv($col, 3)) % 2) === 0,
            5 => ((($row * $col) % 2) + (($row * $col) % 3)) === 0,
            6 => (((($row * $col) % 2) + (($row * $col) % 3)) % 2) === 0,
            default => (((($row + $col) % 2) + (($row * $col) % 3)) % 2) === 0,
        };
    }

    /**
     * Returns the penalty points of the masked symbol, Table 11 of
     * ISO/IEC 18004. The whole symbol is evaluated, not only the encoding
     * region.
     */
    protected function getMaskPenalty(): int
    {
        $lines = $this->getLines();
        $penalty = $this->getRunPenalty($lines) + $this->getBlockPenalty();
        $penalty += $this->getFinderPenalty($lines) + $this->getBalancePenalty();

        return $penalty;
    }

    /**
     * Returns the penalty points of the runs of more than five modules of the
     * same colour in a row or a column.
     *
     * @param array<int, string> $lines Rows and columns of the symbol.
     */
    protected function getRunPenalty(array $lines): int
    {
        $penalty = 0;
        foreach ($lines as $line) {
            $run = 1;
            $len = \strlen($line);
            for ($idx = 1; $idx <= $len; ++$idx) {
                if ($idx < $len && $line[$idx] === $line[$idx - 1]) {
                    ++$run;
                    continue;
                }

                if ($run >= 5) {
                    $penalty += Data::N1 + $run - 5;
                }

                $run = 1;
            }
        }

        return $penalty;
    }

    /**
     * Returns the penalty points of the two by two blocks of modules of the same
     * colour.
     */
    protected function getBlockPenalty(): int
    {
        $penalty = 0;
        for ($row = 0; $row < ($this->size - 1); ++$row) {
            for ($col = 0; $col < ($this->size - 1); ++$col) {
                $value = $this->matrix[$row][$col] ?? 0;
                if (
                    $value !== ($this->matrix[$row][$col + 1] ?? -1)
                    || $value !== ($this->matrix[$row + 1][$col] ?? -1)
                    || $value !== ($this->matrix[$row + 1][$col + 1] ?? -1)
                ) {
                    continue;
                }

                $penalty += Data::N2;
            }
        }

        return $penalty;
    }

    /**
     * Returns the penalty points of the 1:1:3:1:1 patterns next to a light area
     * four modules wide, in a row or a column.
     *
     * Table 11 scores the existence of the pattern, so one that carries the
     * light area on both sides scores once. The light area is looked for in the
     * symbol alone, which section 7.8.3.1 states is the area evaluated: the
     * quiet zone of section 6.3.8 surrounds the symbol and is not part of it.
     *
     * @param array<int, string> $lines Rows and columns of the symbol.
     */
    protected function getFinderPenalty(array $lines): int
    {
        $width = \strlen(self::FINDER_PATTERN);
        $light = \strlen(self::FINDER_LIGHT);
        $penalty = 0;
        foreach ($lines as $line) {
            $pos = 0;
            while (($pos = \strpos($line, self::FINDER_PATTERN, $pos)) !== false) {
                $before = $pos < $light ? '' : \substr($line, $pos - $light, $light);
                if ($before === self::FINDER_LIGHT || \substr($line, $pos + $width, $light) === self::FINDER_LIGHT) {
                    $penalty += Data::N3;
                }

                ++$pos;
            }
        }

        return $penalty;
    }

    /**
     * Returns the penalty points of the departure of the proportion of dark
     * modules from one half, in steps of five percent.
     */
    protected function getBalancePenalty(): int
    {
        $dark = 0;
        $total = $this->size * $this->size;
        foreach ($this->matrix as $row) {
            $dark += \array_sum($row);
        }

        return Data::N4 * \intdiv(\abs((100 * $dark) - (50 * $total)), 5 * $total);
    }

    /**
     * Returns every row and every column of the symbol as a string of binary
     * digits.
     *
     * @return array<int, string>
     */
    protected function getLines(): array
    {
        $lines = [];
        for ($row = 0; $row < $this->size; ++$row) {
            $lines[] = \implode('', $this->matrix[$row] ?? []);
        }

        for ($col = 0; $col < $this->size; ++$col) {
            $line = '';
            for ($row = 0; $row < $this->size; ++$row) {
                $line .= (string) ($this->matrix[$row][$col] ?? 0);
            }

            $lines[] = $line;
        }

        return $lines;
    }

    /**
     * Place the two copies of the fifteen bits of the format information, the
     * most significant one in the module zero, section 7.9.1 of ISO/IEC 18004.
     *
     * @param ?int $mask Mask to declare, or null for the mask of the symbol.
     */
    protected function setFormatInformation(?int $mask = null): void
    {
        $data = ((Data::ECC_INDICATOR[$this->level] ?? 0) << 3) | ($mask ?? $this->mask);
        $format = $this->getBchCode($data, Data::FORMAT_GENERATOR, 5, 10) ^ Data::FORMAT_MASK;
        for ($pos = 0; $pos < 15; ++$pos) {
            $bit = ($format >> (14 - $pos)) & 1;
            $this->matrix[$this->getFormatRow($pos)][$this->getFormatCol($pos)] = $bit;
            if ($pos < 7) {
                $this->matrix[$this->size - 1 - $pos][8] = $bit;
                continue;
            }

            $this->matrix[8][$this->size - 15 + $pos] = $bit;
        }

        // the module below the format information of the bottom left corner is
        // always dark and is not part of it
        $this->matrix[$this->size - 8][8] = 1;
    }

    /**
     * Returns the row of the module that carries the given bit of the first copy
     * of the format information.
     *
     * @param int $pos Bit position, from 0 to 14.
     */
    protected function getFormatRow(int $pos): int
    {
        if ($pos < 8) {
            return 8;
        }

        return $pos === 8 ? 7 : 14 - $pos;
    }

    /**
     * Returns the column of the module that carries the given bit of the first
     * copy of the format information.
     *
     * @param int $pos Bit position, from 0 to 14.
     */
    protected function getFormatCol(int $pos): int
    {
        if ($pos < 6) {
            return $pos;
        }

        return $pos === 6 ? 7 : 8;
    }

    /**
     * Place the two copies of the eighteen bits of the version information, the
     * least significant one in the module zero, section 7.10 of ISO/IEC 18004.
     * Only the versions 7 and above carry it.
     */
    protected function setVersionInformation(): void
    {
        if ($this->version < Data::VERSION_INFO_MIN) {
            return;
        }

        $info = $this->getBchCode($this->version, Data::VERSION_GENERATOR, 6, 12);
        for ($pos = 0; $pos < 18; ++$pos) {
            $bit = ($info >> $pos) & 1;
            $this->matrix[$this->size - 11 + ($pos % 3)][\intdiv($pos, 3)] = $bit;
            $this->matrix[\intdiv($pos, 3)][$this->size - 11 + ($pos % 3)] = $bit;
        }
    }

    /**
     * Returns the data bits followed by the error correction bits of the BCH
     * code, Annexes C and D of ISO/IEC 18004.
     *
     * @param int $data      Data bits.
     * @param int $generator Generator polynomial.
     * @param int $dataBits  Number of data bits.
     * @param int $checkBits Number of error correction bits.
     */
    protected function getBchCode(int $data, int $generator, int $dataBits, int $checkBits): int
    {
        $value = $data << $checkBits;
        for ($pos = $dataBits + $checkBits - 1; $pos >= $checkBits; --$pos) {
            if (($value & (1 << $pos)) === 0) {
                continue;
            }

            $value ^= $generator << ($pos - $checkBits);
        }

        return ($data << $checkBits) | $value;
    }
}
