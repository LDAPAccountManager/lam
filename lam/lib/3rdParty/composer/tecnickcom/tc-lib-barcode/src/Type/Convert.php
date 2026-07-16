<?php

declare(strict_types=1);

/**
 * Convert.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Color\Model\Rgb as Color;

/**
 * Com\Tecnick\Barcode\Type\Convert
 *
 * Barcode Convert class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
abstract class Convert
{
    /**
     * Barcode type (linear or square)
     *
     * @var string
     */
    protected const TYPE = '';

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = '';

    /**
     * Array containing extra parameters for the specified barcode type
     *
     * @var array<int|float|string>
     */
    protected array $params = [];

    /**
     * Code to convert (barcode content)
     */
    protected string $code = '';

    /**
     * Resulting code after applying checksum etc.
     */
    protected string $extcode = '';

    /**
     * Total number of columns
     */
    protected int $ncols = 0;

    /**
     * Total number of rows
     */
    protected int $nrows = 1;

    /**
     * Array containing the position and dimensions of each barcode bar
     * (x, y, width, height)
     *
     * @var array<array{int, int, int, int}>
     */
    protected array $bars = [];

    /**
     * Barcode width
     */
    protected int $width = 0;

    /**
     * Barcode height
     */
    protected int $height = 0;

    /**
     * Additional padding to add around the barcode (top, right, bottom, left) in user units.
     * A negative value indicates the multiplication factor for each row or column.
     *
     * @var array{'T': int, 'R': int, 'B': int, 'L': int}
     */
    protected array $padding = [
        'T' => 0,
        'R' => 0,
        'B' => 0,
        'L' => 0,
    ];

    /**
     * Ratio between the barcode width and the number of columns
     */
    protected float $width_ratio = 0;

    /**
     * Ratio between the barcode height and the number of columns
     */
    protected float $height_ratio = 0;

    /**
     * Foreground Color object
     */
    protected Color $color_obj;

    /**
     * Background Color object
     */
    protected ?Color $bg_color_obj = null;

    /**
     * Process binary sequence rows.
     *
     * @param array<int, string|array<int>> $rows Binary sequence data to process
     *
     * @throws BarcodeException in case of error
     */
    protected function processBinarySequence(array $rows): void
    {
        if ($rows === []) {
            throw new BarcodeException('Empty input string');
        }

        $this->nrows = \count($rows);
        $firstRow = $rows[0] ?? '';
        $this->ncols = \is_array($firstRow) ? \count($firstRow) : \strlen($firstRow);

        if ($this->ncols === 0) {
            throw new BarcodeException('Empty columns');
        }

        $this->bars = [];
        foreach ($rows as $posy => $row) {
            if (!\is_array($row)) {
                $row = \str_split($row, 1);
            }

            $prevcol = '';
            $bar_width = 0;
            $row[] = '0';
            for ($posx = 0; $posx <= $this->ncols; ++$posx) {
                if (($row[$posx] ?? '0') !== $prevcol) {
                    if ($prevcol === '1') {
                        $this->bars[] = [$posx - $bar_width, $posy, $bar_width, 1];
                    }

                    $bar_width = 0;
                }

                ++$bar_width;
                $prevcol = (string) ($row[$posx] ?? '0');
            }
        }
    }

    /**
     * Extract rows from a binary sequence of comma-separated 01 strings.
     *
     * @return array<int, string>
     *
     * @throws BarcodeException in case of invalid input pattern
     */
    protected function getRawCodeRows(string $data): array
    {
        $search = [
            '/[\s]*/s', // remove spaces and newlines
            '/^[\[,]+/', // remove leading brackets or commas
            '/[\],]+$/', // remove trailing brackets or commas
            '/[\]][\[]/', // convert bracket-separated rows to comma-separated
        ];

        $replace = ['', '', '', ','];

        $code = \preg_replace($search, $replace, $data);
        if ($code === null) {
            throw new BarcodeException('Invalid input string');
        }

        return \explode(',', $code);
    }

    /**
     * Convert large integer number to hexadecimal representation.
     *
     * @param string $number Number to convert (as string)
     *
     * @return string hexadecimal representation
     */
    protected function convertDecToHex(string $number): string
    {
        if (!\preg_match('/^[0-9]+$/', $number)) {
            return '00';
        }

        /** @var numeric-string $number */
        if ($number === '0') {
            return '00';
        }

        $hex = [];
        while ($number > 0) {
            $hex[] = \strtoupper(\dechex((int) \bcmod($number, '16')));
            $number = \bcdiv($number, '16', 0);
        }

        $hex = \array_reverse($hex);
        return \implode('', $hex);
    }

    /**
     * Convert large hexadecimal number to decimal representation (string).
     *
     * @param string $hex Hexadecimal number to convert (as string)
     *
     * @return string hexadecimal representation
     */
    protected function convertHexToDec(string $hex): string
    {
        $dec = '0';
        $bitval = '1';
        $len = \strlen($hex);
        for ($pos = $len - 1; $pos >= 0; --$pos) {
            $dec = \bcadd($dec, \bcmul((string) \hexdec($hex[$pos]), $bitval));
            $bitval = \bcmul($bitval, '16');
        }

        return $dec;
    }

    /**
     * Get a raw barcode grid array
     *
     * @param string $space_char Character or string to use for filling empty spaces
     * @param string $bar_char   Character or string to use for filling bars
     *
     * @return array<int, array<int, string>>
     */
    public function getGridArray(string $space_char = '0', string $bar_char = '1'): array
    {
        $raw = \array_fill(0, \max(0, $this->nrows), \array_fill(0, \max(0, $this->ncols), $space_char));
        foreach ($this->bars as $bar) {
            if ($bar[2] <= 0) {
                continue;
            }

            if ($bar[3] <= 0) {
                continue;
            }

            for ($vert = 0; $vert < $bar[3]; ++$vert) {
                for ($horiz = 0; $horiz < $bar[2]; ++$horiz) {
                    $raw[$bar[1] + $vert][$bar[0] + $horiz] = $bar_char;
                }
            }
        }

        return $raw;
    }

    /**
     * Returns the bars array ordered by columns
     *
     * @return array<int, array{int, int, int, int}>
     */
    protected function getRotatedBarArray(): array
    {
        $grid = $this->getGridArray();
        if ($grid === []) {
            return [];
        }

        $cols = \array_map(null, ...$grid);
        $bars = [];
        foreach ($cols as $posx => $col) {
            $prevrow = '';
            $bar_height = 0;
            $col[] = '0';
            for ($posy = 0; $posy <= $this->nrows; ++$posy) {
                if (($col[$posy] ?? '0') !== $prevrow) {
                    if ($prevrow === '1') {
                        $bars[] = [$posx, $posy - $bar_height, 1, $bar_height];
                    }

                    $bar_height = 0;
                }

                ++$bar_height;
                $prevrow = $col[$posy] ?? '0';
            }
        }

        return $bars;
    }

    /**
     * Get the adjusted rectangular coordinates (x1,y1,x2,y2) for the specified bar
     *
     * @param array{int, int, int, int} $bar Raw bar coordinates
     *
     * @return array{float, float, float, float} Bar coordinates
     */
    protected function getBarRectXYXY(array $bar): array
    {
        return [
            $this->padding['L'] + ($bar[0] * $this->width_ratio),
            $this->padding['T'] + ($bar[1] * $this->height_ratio),
            $this->padding['L'] + (($bar[0] + $bar[2]) * $this->width_ratio) - 1,
            $this->padding['T'] + (($bar[1] + $bar[3]) * $this->height_ratio) - 1,
        ];
    }

    /**
     * Get the adjusted rectangular coordinates (x,y,w,h) for the specified bar
     *
     * @param array{int, int, int, int} $bar Raw bar coordinates
     *
     * @return array{float, float, float, float} Bar coordinates
     */
    protected function getBarRectXYWH(array $bar): array
    {
        return [
            $this->padding['L'] + ($bar[0] * $this->width_ratio),
            $this->padding['T'] + ($bar[1] * $this->height_ratio),
            $bar[2] * $this->width_ratio,
            $bar[3] * $this->height_ratio,
        ];
    }
}
