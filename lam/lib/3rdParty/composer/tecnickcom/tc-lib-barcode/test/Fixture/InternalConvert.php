<?php

/**
 * InternalConvert.php
 *
 * @since       2026-05-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test\Fixture;

class InternalConvert extends \Com\Tecnick\Barcode\Type\Convert
{
    /**
     * @param array<int, string|array<int>> $rows
     *
     * @throws \Com\Tecnick\Barcode\Exception
     */
    public function exposeProcessBinarySequence(array $rows): void
    {
        $this->processBinarySequence($rows);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     *
     * @return array<int, string>
     */
    public function exposeGetRawCodeRows(string $data): array
    {
        return $this->getRawCodeRows($data);
    }

    public function exposeConvertDecToHex(string $number): string
    {
        return $this->convertDecToHex($number);
    }

    public function exposeConvertHexToDec(string $hex): string
    {
        return $this->convertHexToDec($hex);
    }

    /**
     * @return array<int, array{int, int, int, int}>
     */
    public function exposeGetRotatedBarArray(): array
    {
        return $this->getRotatedBarArray();
    }

    /**
     * @param array<int, array{int, int, int, int}> $bars
     */
    public function setBars(array $bars): void
    {
        $this->bars = $bars;
    }

    public function setColsRows(int $ncols, int $nrows): void
    {
        $this->ncols = $ncols;
        $this->nrows = $nrows;
    }

    /**
     * @param array{'T': int, 'R': int, 'B': int, 'L': int} $padding
     */
    public function setRatiosAndPadding(float $widthRatio, float $heightRatio, array $padding): void
    {
        $this->width_ratio = $widthRatio;
        $this->height_ratio = $heightRatio;
        $this->padding = $padding;
    }

    /**
     * @param array{int, int, int, int} $bar
     *
     * @return array{float, float, float, float}
     */
    public function exposeGetBarRectXYXY(array $bar): array
    {
        return $this->getBarRectXYXY($bar);
    }

    /**
     * @param array{int, int, int, int} $bar
     *
     * @return array{float, float, float, float}
     */
    public function exposeGetBarRectXYWH(array $bar): array
    {
        return $this->getBarRectXYWH($bar);
    }

    /**
     * @return array<int, array{int, int, int, int}>
     */
    public function getBars(): array
    {
        return \array_values($this->bars);
    }
}
