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

namespace Com\Tecnick\Barcode\Type\Square\HanXin;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\ReedSolomon;

/**
 * Com\Tecnick\Barcode\Type\Square\HanXin\Encode
 *
 * Symbol builder for the HanXin Barcode type class
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Encode extends \Com\Tecnick\Barcode\Type\Square\HanXin\Compaction
{
    /**
     * Penalty of a row or a column that carries the feature ratio of the
     * position detection pattern.
     *
     * @var int
     */
    protected const PENALTY_FEATURE = 50;

    /**
     * Penalty of each module of a run of adjacent modules of the same colour.
     *
     * @var int
     */
    protected const PENALTY_RUN = 4;

    /**
     * Shortest run of adjacent modules of the same colour that is penalised.
     *
     * @var int
     */
    protected const PENALTY_RUN_MIN = 3;

    /**
     * Feature ratio of the position detection pattern, as the module sequence
     * of the two scanning directions.
     *
     * @var array<int, string>
     */
    protected const FEATURE = ['1010111', '1110101'];

    /**
     * Symbol version, from 1 to 84.
     */
    protected int $version = 1;

    /**
     * Error correction level, from 1 to 4, that is L1 to L4.
     */
    protected int $level = 1;

    /**
     * Data mask pattern reference, from 0 to 3.
     */
    protected int $mask = 0;

    /**
     * Symbol size in modules.
     */
    protected int $size = 0;

    /**
     * Function pattern layout of the symbol.
     */
    protected Geometry $geometry;

    /**
     * Module matrix: 1 is a dark module, 0 a light one.
     *
     * @var array<int, array<int, int>>
     */
    protected array $matrix = [];

    /**
     * Build the symbol.
     *
     * @param string $code    Code to encode.
     * @param int    $level   Error correction level, from 1 to 4.
     * @param int    $version Symbol version, from 1 to 84, or 0 to select the
     *                        smallest symbol able to carry the data.
     * @param int    $mask    Data mask pattern reference, from 0 to 3, or a
     *                        negative value to select the one with the lowest
     *                        penalty score.
     *
     * @throws BarcodeException in case of error
     */
    public function __construct(string $code, int $level, int $version, int $mask)
    {
        $this->level = $level;
        $bits = $this->getBitStream($code);
        $this->version = $this->getVersionFor($bits, $version);
        $this->geometry = new Geometry($this->version);
        $this->size = $this->geometry->getSize();
        $stream = $this->getDataStream($bits);
        $this->mask = $mask < 0 ? $this->getBestMask($stream) : $mask;
        $this->matrix = $this->getSymbol($stream, $this->mask);
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
     * Returns the symbol version, from 1 to 84.
     */
    public function getVersion(): int
    {
        return $this->version;
    }

    /**
     * Returns the error correction level, from 1 to 4.
     */
    public function getLevel(): int
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
     * Returns the number of data codewords of a version and error correction
     * level.
     *
     * @param int $version Symbol version.
     * @param int $level   Error correction level.
     */
    public static function getDataCodewords(int $version, int $level): int
    {
        $count = 0;
        foreach (Data::BLOCKS[$version][$level] ?? [] as $block) {
            $count += $block[0] * $block[2];
        }

        return $count;
    }

    /**
     * Returns the total number of codewords of a version and error correction
     * level.
     *
     * @param int $version Symbol version.
     * @param int $level   Error correction level.
     */
    public static function getTotalCodewords(int $version, int $level): int
    {
        $count = 0;
        foreach (Data::BLOCKS[$version][$level] ?? [] as $block) {
            $count += $block[0] * $block[1];
        }

        return $count;
    }

    /**
     * Returns the version able to carry the information bit stream.
     *
     * @param string $bits    Information bit stream.
     * @param int    $version Requested version, or 0 to select the smallest one.
     *
     * @throws BarcodeException if the data does not fit in the symbol
     */
    protected function getVersionFor(string $bits, int $version): int
    {
        $len = \strlen($bits);
        if ($version > 0) {
            if ($len > (self::getDataCodewords($version, $this->level) * 8)) {
                throw new BarcodeException('The data does not fit in the requested symbol version');
            }

            return $version;
        }

        for ($idx = 1; $idx <= Data::VERSION_MAX; ++$idx) {
            if ($len <= (self::getDataCodewords($idx, $this->level) * 8)) {
                return $idx;
            }
        }

        throw new BarcodeException('The data does not fit in a Han Xin Code symbol');
    }

    /**
     * Returns the final data codeword sequence: the information codewords of
     * each error correction block followed by the error correction codewords
     * of that block.
     *
     * @param string $bits Information bit stream.
     *
     * @return array<int, int> Data codewords.
     */
    protected function getDataStream(string $bits): array
    {
        $data = $this->getInformationCodewords($bits);
        $reed = new ReedSolomon(8, Data::GF_DATA);
        $stream = [];
        $pos = 0;
        foreach (Data::BLOCKS[$this->version][$this->level] ?? [] as $block) {
            for ($idx = 0; $idx < $block[0]; ++$idx) {
                $words = \array_slice($data, $pos, $block[2]);
                $pos += $block[2];
                $stream = \array_merge($stream, $words, $reed->checkwords($words, $block[1] - $block[2]));
            }
        }

        return $stream;
    }

    /**
     * Returns the information codewords, the bit stream padded with zeros to
     * the data capacity of the symbol.
     *
     * @param string $bits Information bit stream.
     *
     * @return array<int, int> Information codewords.
     */
    protected function getInformationCodewords(string $bits): array
    {
        $count = self::getDataCodewords($this->version, $this->level);
        $bits = \str_pad($bits, $count * 8, '0');
        $words = [];
        for ($idx = 0; $idx < $count; ++$idx) {
            $words[] = (int) \bindec(\substr($bits, $idx * 8, 8));
        }

        return $words;
    }

    /**
     * Returns the data codeword sequence in placement order: the codewords are
     * grouped by thirteen and the groups are then read column by column.
     *
     * @param array<int, int> $stream Data codewords.
     *
     * @return array<int, int> Data codewords in placement order.
     */
    protected function getPlacementOrder(array $stream): array
    {
        $count = \count($stream);
        $order = [];
        for ($pos = 0; $pos < Data::GROUP_SIZE; ++$pos) {
            for ($start = 0; $start < $count; $start += Data::GROUP_SIZE) {
                if (($start + $pos) >= $count) {
                    continue;
                }

                $order[] = $stream[$start + $pos] ?? 0;
            }
        }

        return $order;
    }

    /**
     * Returns the module matrix of the symbol.
     *
     * @param array<int, int> $stream Data codewords.
     * @param int             $mask   Data mask pattern reference.
     *
     * @return array<int, array<int, int>> Module matrix.
     */
    protected function getSymbol(array $stream, int $mask): array
    {
        $matrix = [];
        foreach ($this->geometry->getMatrix() as $row => $line) {
            foreach ($line as $col => $module) {
                $matrix[$row][$col] = \is_int($module) ? $module : 0;
            }
        }

        $bits = '';
        foreach ($this->getPlacementOrder($stream) as $word) {
            $bits .= \str_pad(\decbin($word), 8, '0', \STR_PAD_LEFT);
        }

        $len = \strlen($bits);
        foreach ($this->geometry->getEncodingCells() as $idx => $cell) {
            $module = $idx < $len ? (int) $bits[$idx] : 0;
            $matrix[$cell[0]][$cell[1]] = $module ^ $this->getMaskModule($mask, $cell[0], $cell[1]);
        }

        $info = $this->getInfoStream($mask);
        $half = (int) (\strlen($info) / 2);
        foreach ($this->geometry->getInfoCells() as $area => $cells) {
            $part = ($area % 2) === 0 ? \substr($info, 0, $half) : \substr($info, $half);
            foreach ($cells as $idx => $cell) {
                $matrix[$cell[0]][$cell[1]] = (int) ($part[$idx] ?? '0');
            }
        }

        return $matrix;
    }

    /**
     * Returns the module of the mask pattern of Table 14 at the given position.
     * The rows and the columns are counted from one.
     *
     * @param int $mask Data mask pattern reference.
     * @param int $row  Row of the module.
     * @param int $col  Column of the module.
     */
    protected function getMaskModule(int $mask, int $row, int $col): int
    {
        $vpos = $row + 1;
        $hpos = $col + 1;

        return match ($mask) {
            1 => (($vpos + $hpos) % 2) === 0 ? 1 : 0,
            2 => (((($vpos + $hpos) % 3) + ($hpos % 3)) % 2) === 0 ? 1 : 0,
            3 => ((($vpos % $hpos) + ($hpos % $vpos) + ($vpos % 3) + ($hpos % 3)) % 2) === 0 ? 1 : 0,
            default => 0,
        };
    }

    /**
     * Returns the function information bit stream: the version, the error
     * correction level and the mask pattern reference in twelve bits, the four
     * error correction codewords of Annex G and the padding of the two areas
     * that carry them.
     *
     * @param int $mask Data mask pattern reference.
     */
    protected function getInfoStream(int $mask): string
    {
        $value = (($this->version + Data::VERSION_OFFSET) << 4) | (($this->level - 1) << 2) | $mask;
        $words = [($value >> 8) & 0x0F, ($value >> 4) & 0x0F, $value & 0x0F];
        $reed = new ReedSolomon(4);
        $bits = '';
        foreach (\array_merge($words, $reed->checkwords($words, 4)) as $word) {
            $bits .= \str_pad(\decbin($word), 4, '0', \STR_PAD_LEFT);
        }

        return \str_pad($bits, 2 * Data::INFO_AREA_MODULES, '0');
    }

    /**
     * Returns the mask pattern reference with the lowest penalty score.
     *
     * @param array<int, int> $stream Data codewords.
     */
    protected function getBestMask(array $stream): int
    {
        $best = 0;
        $lowest = -1;
        for ($mask = 0; $mask <= 3; ++$mask) {
            $score = $this->getPenalty($this->getSymbol($stream, $mask));
            if ($lowest < 0 || $score < $lowest) {
                $lowest = $score;
                $best = $mask;
            }
        }

        return $best;
    }

    /**
     * Returns the penalty score of a symbol, by the rules of Table 15.
     *
     * @param array<int, array<int, int>> $matrix Module matrix.
     */
    protected function getPenalty(array $matrix): int
    {
        $rows = [];
        $cols = \array_fill(0, \max(0, $this->size), '');
        foreach ($matrix as $line) {
            $rows[] = \implode('', $line);
            foreach ($line as $col => $module) {
                $cols[$col] = ($cols[$col] ?? '') . $module;
            }
        }

        $score = 0;
        foreach (\array_merge($rows, $cols) as $line) {
            $score += $this->getLinePenalty($line);
        }

        return $score;
    }

    /**
     * Returns the penalty score of one row or column.
     *
     * @param string $line Modules of the row or column as binary digits.
     */
    protected function getLinePenalty(string $line): int
    {
        $score = 0;
        foreach (self::FEATURE as $feature) {
            $pos = \strpos($line, $feature);
            while ($pos !== false) {
                $score += self::PENALTY_FEATURE;
                $pos = \strpos($line, $feature, $pos + 1);
            }
        }

        $runs = \preg_split('/(?<=0)(?=1)|(?<=1)(?=0)/', $line);
        foreach ($runs === false ? [] : $runs as $run) {
            $len = \strlen($run);
            if ($len >= self::PENALTY_RUN_MIN) {
                $score += self::PENALTY_RUN * $len;
            }
        }

        return $score;
    }
}
