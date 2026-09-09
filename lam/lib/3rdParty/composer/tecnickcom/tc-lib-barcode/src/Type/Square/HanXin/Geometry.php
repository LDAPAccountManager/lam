<?php

declare(strict_types=1);

/**
 * Geometry.php
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
 * Com\Tecnick\Barcode\Type\Square\HanXin\Geometry
 *
 * Function pattern placement of the HanXin Barcode type class
 *
 * The matrix cells hold 1 for a dark module, 0 for a light one, the string 'I'
 * for a module of a function information area and null for a module of the
 * encoding region.
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Geometry
{
    /**
     * Module of a function information area.
     *
     * @var string
     */
    public const INFO = 'I';

    /**
     * Symbol size in modules.
     */
    protected int $size = 0;

    /**
     * Matrix of the function patterns.
     *
     * @var array<int, array<int, int|string|null>>
     */
    protected array $matrix = [];

    /**
     * Modules the function patterns take, which the alignment pattern and the
     * auxiliary alignment patterns may not overwrite.
     *
     * @var array<int, array<int, bool>>
     */
    protected array $fixed = [];

    /**
     * Build the function patterns of the symbol.
     *
     * @param int $version Symbol version, from 1 to 84.
     */
    public function __construct(int $version)
    {
        $this->size = Data::SIZE_BASE + (Data::SIZE_STEP * ($version - 1));
        $side = \max(0, $this->size);
        $this->matrix = \array_fill(0, $side, \array_fill(0, $side, null));
        $this->fixed = \array_fill(0, $side, \array_fill(0, $side, false));
        $this->setPositionDetectionPatterns();
        $this->setInfoAreas();
        $alignment = Data::ALIGNMENT[$version] ?? [];
        if ($alignment !== []) {
            $this->setAlignment($alignment[0] ?? 0, $alignment[1] ?? 0, $alignment[2] ?? 0);
        }
    }

    /**
     * Returns the symbol size in modules.
     */
    public function getSize(): int
    {
        return $this->size;
    }

    /**
     * Returns the matrix of the function patterns.
     *
     * @return array<int, array<int, int|string|null>>
     */
    public function getMatrix(): array
    {
        return $this->matrix;
    }

    /**
     * Returns the modules of the encoding region, row by row and left to right,
     * as pairs of row and column.
     *
     * @return list<array{int, int}>
     */
    public function getEncodingCells(): array
    {
        $cells = [];
        foreach ($this->matrix as $row => $line) {
            foreach ($line as $col => $module) {
                if ($module !== null) {
                    continue;
                }

                $cells[] = [$row, $col];
            }
        }

        return $cells;
    }

    /**
     * Returns the modules of the four function information areas, each one
     * counterclockwise, as pairs of row and column. The first and the third
     * area take the first half of the function information, the second and the
     * fourth take the second half.
     *
     * @return array{list<array{int, int}>, list<array{int, int}>, list<array{int, int}>, list<array{int, int}>}
     */
    public function getInfoCells(): array
    {
        $end = $this->size - 1;
        $mid = $this->size - 9;
        $topleft = [];
        $topright = [];
        $bottomright = [];
        $bottomleft = [];
        for ($idx = 0; $idx <= 8; ++$idx) {
            $topleft[] = [8, $idx];
            $topright[] = [$idx, $mid];
            $bottomright[] = [$mid, $end - $idx];
            $bottomleft[] = [$end - $idx, 8];
        }

        for ($idx = 7; $idx >= 0; --$idx) {
            $topleft[] = [$idx, 8];
            $topright[] = [8, $end - $idx];
            $bottomright[] = [$end - $idx, $mid];
            $bottomleft[] = [$mid, $idx];
        }

        return [$topleft, $topright, $bottomright, $bottomleft];
    }

    /**
     * Places the four position detection patterns and their separators.
     */
    protected function setPositionDetectionPatterns(): void
    {
        $end = $this->size - 1;
        $pattern = $this->getPositionDetectionPattern();
        for ($row = 0; $row < 7; ++$row) {
            for ($col = 0; $col < 7; ++$col) {
                $module = $pattern[$row][$col] ?? 0;
                $this->setFixed($row, $col, $module);
                $this->setFixed($row, $end - $col, $module);
                $this->setFixed($end - 6 + $row, 6 - $col, $module);
                $this->setFixed($end - $row, $end - $col, $module);
            }
        }

        for ($idx = 0; $idx <= 7; ++$idx) {
            $this->setFixed(7, $idx, 0);
            $this->setFixed($idx, 7, 0);
            $this->setFixed(7, $end - $idx, 0);
            $this->setFixed($idx, $end - 7, 0);
            $this->setFixed($end - 7, $idx, 0);
            $this->setFixed($end - $idx, 7, 0);
            $this->setFixed($end - 7, $end - $idx, 0);
            $this->setFixed($end - $idx, $end - 7, 0);
        }
    }

    /**
     * Returns the position detection pattern of the top left corner, five
     * concentric squares aligned on their bottom right corner.
     *
     * @return array<int, array<int, int>>
     */
    protected function getPositionDetectionPattern(): array
    {
        $pattern = [];
        for ($row = 0; $row < 7; ++$row) {
            $line = [];
            for ($col = 0; $col < 7; ++$col) {
                $ring = \min($row, $col);
                $line[] = $ring === 0 || $ring === 2 || $ring >= 4 ? 1 : 0;
            }

            $pattern[] = $line;
        }

        return $pattern;
    }

    /**
     * Reserves the modules of the four function information areas.
     */
    protected function setInfoAreas(): void
    {
        foreach ($this->getInfoCells() as $area) {
            foreach ($area as $cell) {
                $this->setFixed($cell[0], $cell[1], self::INFO);
            }
        }
    }

    /**
     * Places the alignment pattern and the auxiliary alignment patterns.
     *
     * The alignment lines follow a grid of rows counted from the bottom edge
     * and of columns counted from the left edge. A horizontal line lies on a
     * row of the grid and a vertical line on a column of it when the sum of
     * their indexes is odd. Every line but the one of the top right corner is
     * drawn, dark on the line and light on the row below it or on the column
     * left of it.
     *
     * @param int $rlen Length in modules of the two lines of the bottom left corner.
     * @param int $klen Length in modules of the other lines.
     * @param int $step Number of alignment line steps.
     */
    protected function setAlignment(int $rlen, int $klen, int $step): void
    {
        $rows = [$this->size - 1];
        $cols = [0];
        for ($idx = 0; $idx <= $step; ++$idx) {
            $rows[] = $this->size - $rlen - ($idx * $klen);
            $cols[] = $rlen - 1 + ($idx * $klen);
        }

        $corner = (2 * ($step + 1)) - 1;
        $dark = [];
        $light = [];
        for ($sum = 1; $sum < $corner; $sum += 2) {
            foreach ($this->getLineModules($rows, $cols, $sum) as $module) {
                if ($module[2] === 1) {
                    $dark[] = [$module[0], $module[1]];
                    continue;
                }

                $light[] = [$module[0], $module[1]];
            }
        }

        foreach ($dark as $cell) {
            $this->setModule($cell[0], $cell[1], 1);
        }

        foreach ($light as $cell) {
            $this->setModule($cell[0], $cell[1], 0);
        }

        for ($sum = 1; $sum <= $corner; $sum += 2) {
            foreach ($this->getAuxiliaryPoints($rows, $cols, $sum) as $point) {
                $this->setAuxiliary($point[0], $point[1]);
            }
        }
    }

    /**
     * Returns the modules of the alignment lines whose row and column indexes
     * sum to the given value, each as its row, its column and its colour.
     *
     * @param list<int> $rows Row of each grid index, counted from the bottom edge.
     * @param list<int> $cols Column of each grid index, counted from the left edge.
     * @param int       $sum  Sum of the row and column indexes of the lines.
     *
     * @return list<array{int, int, int}>
     */
    protected function getLineModules(array $rows, array $cols, int $sum): array
    {
        $last = \count($rows) - 1;
        $modules = [];
        for ($idx = 0; $idx <= $last; ++$idx) {
            $col = $sum - $idx;
            if ($col < 0 || $col > $last) {
                continue;
            }

            $row = $rows[$idx] ?? 0;
            if ($col < $last) {
                for ($pos = $cols[$col] ?? 0; $pos <= ($cols[$col + 1] ?? 0); ++$pos) {
                    $modules[] = [$row, $pos, 1];
                    $modules[] = [$row + 1, $pos, 0];
                }
            }

            if ($idx >= $last) {
                continue;
            }

            $line = $cols[$col] ?? 0;
            for ($pos = $rows[$idx + 1] ?? 0; $pos <= $row; ++$pos) {
                $modules[] = [$pos, $line, 1];
                $modules[] = [$pos, $line - 1, 0];
            }

            if ($col < $last) {
                $modules[] = [$row + 1, $line - 1, 0];
            }
        }

        return $modules;
    }

    /**
     * Returns the points where one alignment line meets an edge of the symbol,
     * which carry an auxiliary alignment pattern.
     *
     * @param list<int> $rows Row of each grid index, counted from the bottom edge.
     * @param list<int> $cols Column of each grid index, counted from the left edge.
     * @param int       $sum  Sum of the row and column indexes of the line.
     *
     * @return list<array{int, int}> Pairs of row and column.
     */
    protected function getAuxiliaryPoints(array $rows, array $cols, int $sum): array
    {
        $last = \count($rows) - 1;
        $ends = [];
        for ($idx = $last; $idx >= 0; --$idx) {
            $col = $sum - $idx;
            if ($col < 0 || $col > $last) {
                continue;
            }

            if ($idx < $last) {
                $ends[] = [$rows[$idx + 1] ?? 0, $cols[$col] ?? 0];
                $ends[] = [$rows[$idx] ?? 0, $cols[$col] ?? 0];
            }

            if ($col < $last) {
                $ends[] = [$rows[$idx] ?? 0, $cols[$col] ?? 0];
                $ends[] = [$rows[$idx] ?? 0, $cols[$col + 1] ?? 0];
            }
        }

        $edge = $this->size - 1;
        $points = [];
        $bounds = [$ends[0] ?? [0, 0], $ends[\count($ends) - 1] ?? [0, 0]];
        foreach ($bounds as $point) {
            $row = $point[0] ?? 0;
            $col = $point[1] ?? 0;
            if ($row === 0 || $row === $edge || $col === 0 || $col === $edge) {
                $points[] = [$row, $col];
            }
        }

        return $points;
    }

    /**
     * Places one auxiliary alignment pattern, the six modules of Figure 14
     * with the dark one on the edge of the symbol.
     *
     * @param int $row Row of the dark module.
     * @param int $col Column of the dark module.
     */
    protected function setAuxiliary(int $row, int $col): void
    {
        $end = $this->size - 1;
        if ($row === 0 || $row === $end) {
            $rows = $row === 0 ? [0, 1] : [$end - 1, $end];
            $cols = [$col - 1, $col, $col + 1];
        } else {
            $rows = [$row - 1, $row, $row + 1];
            $cols = $col === 0 ? [0, 1] : [$end - 1, $end];
        }

        foreach ($rows as $one) {
            foreach ($cols as $two) {
                $this->setModule($one, $two, 0);
            }
        }

        $this->setModule($row, $col, 1);
    }

    /**
     * Sets a module of a function pattern, which the alignment pattern may not
     * overwrite.
     *
     * @param int        $row    Row of the module.
     * @param int        $col    Column of the module.
     * @param int|string $module Module value.
     */
    protected function setFixed(int $row, int $col, int|string $module): void
    {
        $this->matrix[$row][$col] = $module;
        $this->fixed[$row][$col] = true;
    }

    /**
     * Sets a module of the alignment pattern, leaving the modules of the other
     * function patterns alone.
     *
     * @param int $row    Row of the module.
     * @param int $col    Column of the module.
     * @param int $module Module value.
     */
    protected function setModule(int $row, int $col, int $module): void
    {
        if (
            $row < 0
            || $row >= $this->size
            || $col < 0
            || $col >= $this->size
            || ($this->fixed[$row][$col] ?? false)
            || $module === 0 && ($this->matrix[$row][$col] ?? null) !== null
        ) {
            return;
        }

        $this->matrix[$row][$col] = $module;
    }
}
