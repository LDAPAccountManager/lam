<?php

declare(strict_types=1);

/**
 * CodeOneSixK.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\CodeOneSixK;
 *
 * CodeOneSixK Barcode type class
 * CODE 16K
 *
 * A multi-row symbology of two to sixteen rows of five symbol characters. The
 * symbol characters are those of CODE 128 read space first, so the data
 * encodation of CODE 128 applies unchanged. The first symbol character is the
 * starting character, which carries the number of rows and the initial code
 * set, and the last two are the modulo 107 check characters. Each row is
 * identified by its own pair of start and stop patterns.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class CodeOneSixK extends \Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'C16K';

    /**
     * Number of symbol characters of a row
     */
    protected const ROW_CHARS = 5;

    /**
     * Smallest and largest number of rows
     */
    protected const MIN_ROWS = 2;

    protected const MAX_ROWS = 16;

    /**
     * Number of check characters
     */
    protected const CHECK_CHARS = 2;

    /**
     * Modulus of the check characters
     */
    protected const MODULUS = 107;

    /**
     * Symbol character value of the pad character
     */
    protected const PAD = 103;

    /**
     * Symbol character value of the triple shift, the only one CODE 128 does not share
     */
    protected const TRIPLE_SHIFT = 106;

    /**
     * Bar and space pattern of the triple shift
     *
     * @var string
     */
    protected const TRIPLE_SHIFT_PATTERN = '211133';

    /**
     * Start patterns, bar space bar space
     *
     * @var array<int, string>
     */
    protected const START_PATTERN = ['3211', '2221', '2122', '1411', '1132', '1231', '1114', '3112'];

    /**
     * Width in modules of the guard bar that follows the start pattern
     */
    protected const GUARD = 1;

    /**
     * Height in modules of a row
     */
    protected const ROW_HEIGHT = 8;

    /**
     * Height in modules of a separator bar
     */
    protected const SEPARATOR = 1;

    /**
     * Starting mode of each CODE 128 start character value
     *
     * @var array<int, int>
     */
    protected const STARTING_MODE = [
        103 => 0,
        104 => 1,
        105 => 2,
    ];

    /**
     * Number of starting modes, the radix of the starting symbol character
     */
    protected const MODES = 7;

    /**
     * Symbol character value of the code set change to Code Set C
     */
    protected const CODE_C = 99;

    /**
     * Symbol character value of FNC 1
     */
    protected const FNC1 = 102;

    /**
     * Starting mode of the symbol
     */
    protected int $mode = 0;

    /**
     * Keep the data symbol characters and the starting mode, in place of the
     * CODE 128 start code, check character and stop pattern.
     *
     * @param array<int, int> $code_data Array of codepoints
     * @param int             $startid   CODE 128 start code
     *
     * @return array<int, int> Array of codepoints
     */
    protected function finalizeCodeData(array $code_data, int $startid): array
    {
        $this->mode = $this::STARTING_MODE[$startid] ?? 0;
        return $this->applyImpliedCharacter($code_data);
    }

    /**
     * Fold a leading FNC 1, or the code set change that follows one or two
     * leading Code Set B characters, into the starting symbol character.
     *
     * @param array<int, int> $code_data Array of codepoints
     *
     * @return array<int, int> Array of codepoints
     */
    protected function applyImpliedCharacter(array $code_data): array
    {
        if (($code_data[0] ?? null) === $this::FNC1 && $this->mode !== 0) {
            // Code Set B or Code Set C with an implied FNC 1
            $this->mode += 2;
            \array_shift($code_data);
            return $code_data;
        }

        if ($this->mode !== 1) {
            return $code_data;
        }

        foreach ([1, 2] as $offset) {
            if (($code_data[$offset] ?? null) !== $this::CODE_C) {
                continue;
            }

            // Code Set C with an implied shift over the leading Code Set B characters
            $this->mode = 4 + $offset;
            \array_splice($code_data, $offset, 1);
            return $code_data;
        }

        return $code_data;
    }

    /**
     * Get the bar and space pattern of a symbol character
     */
    protected function getSymbolPattern(int $value): string
    {
        if ($value === $this::TRIPLE_SHIFT) {
            return $this::TRIPLE_SHIFT_PATTERN;
        }

        return $this::CHBAR[$value] ?? '';
    }

    /**
     * Get the number of rows needed by the given number of data characters
     *
     * @throws BarcodeException if the data does not fit the largest symbol
     */
    protected function getRowCount(int $chars): int
    {
        $needed = $chars + 1 + $this::CHECK_CHARS;
        $rows = \max($this::MIN_ROWS, (int) \ceil($needed / $this::ROW_CHARS));
        if ($rows > $this::MAX_ROWS) {
            throw new BarcodeException(
                'The code is too long: '
                . $chars
                . ' symbol characters (maximum '
                . (($this::MAX_ROWS * $this::ROW_CHARS) - 1 - $this::CHECK_CHARS)
                . ')',
            );
        }

        return $rows;
    }

    /**
     * Get the two check characters of the symbol characters that precede them.
     * The weights of the first one start at two and those of the second at one.
     *
     * @param array<int, int> $chars Symbol characters, from the starting character
     *
     * @return array{int, int}
     */
    protected function getChecksum(array $chars): array
    {
        $first = 0;
        foreach ($chars as $key => $value) {
            $first += ($key + 2) * $value;
        }

        $first %= $this::MODULUS;
        $chars[] = $first;

        $second = 0;
        foreach ($chars as $key => $value) {
            $second += ($key + 1) * $value;
        }

        return [$first, $second % $this::MODULUS];
    }

    /**
     * Get every symbol character of the symbol, from the starting character to
     * the second check character
     *
     * @return array<int, int>
     *
     * @throws BarcodeException in case of error
     */
    protected function getSymbolChars(): array
    {
        $data = $this->getCodeData();
        $rows = $this->getRowCount(\count($data));
        $chars = \array_merge(
            [($this::MODES * ($rows - $this::MIN_ROWS)) + $this->mode],
            $data,
            \array_fill(0, \max(0, ($rows * $this::ROW_CHARS) - 1 - $this::CHECK_CHARS - \count($data)), $this::PAD),
        );

        return \array_merge($chars, $this->getChecksum($chars));
    }

    /**
     * Get the modules of a row: the start pattern, the guard bar, five symbol
     * characters and the stop pattern, alternating bars and spaces from the
     * leading bar of the start pattern.
     *
     * @param array<int, int> $chars Symbol characters of the row
     *
     * @return array<int, int> One entry per module, 1 for a bar and 0 for a space
     */
    protected function getRowModules(int $row, array $chars): array
    {
        $widths = $this::START_PATTERN[$row % 8] ?? '';
        $widths .= (string) $this::GUARD;
        foreach ($chars as $value) {
            $widths .= $this->getSymbolPattern($value);
        }

        // the stop pattern is the start pattern with the bars and the spaces exchanged
        $widths .= $this::START_PATTERN[$row < 8 ? $row : ($row + 4) % 8] ?? '';

        $modules = [];
        $wlen = \strlen($widths);
        for ($pos = 0; $pos < $wlen; ++$pos) {
            $modules = \array_merge($modules, \array_fill(0, \max(0, (int) $widths[$pos]), ($pos % 2) === 0 ? 1 : 0));
        }

        return $modules;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $chars = $this->getSymbolChars();
        $rows = \intdiv(\count($chars), $this::ROW_CHARS);

        $this->bars = [];
        $this->ncols = 0;
        $this->nrows = ($rows * ($this::ROW_HEIGHT + $this::SEPARATOR)) + $this::SEPARATOR;

        for ($row = 0; $row < $rows; ++$row) {
            $modules = $this->getRowModules($row, \array_slice($chars, $row * $this::ROW_CHARS, $this::ROW_CHARS));
            $this->ncols = \count($modules);
            $this->addModuleBars($modules, ($row * ($this::ROW_HEIGHT + $this::SEPARATOR)) + $this::SEPARATOR);
        }

        // the separator bars above, between and below the rows
        for ($row = 0; $row <= $rows; ++$row) {
            $this->bars[] = [0, $row * ($this::ROW_HEIGHT + $this::SEPARATOR), $this->ncols, $this::SEPARATOR];
        }
    }

    /**
     * Add one bar for each run of set modules of a row
     *
     * @param array<int, int> $modules One entry per module, 1 for a bar and 0 for a space
     * @param int             $posy    Row of the first module
     */
    protected function addModuleBars(array $modules, int $posy): void
    {
        $start = -1;
        $count = \count($modules);
        for ($posx = 0; $posx <= $count; ++$posx) {
            if (($modules[$posx] ?? 0) === 1) {
                if ($start < 0) {
                    $start = $posx;
                }

                continue;
            }

            if ($start >= 0) {
                $this->bars[] = [$start, $posy, $posx - $start, $this::ROW_HEIGHT];
                $start = -1;
            }
        }
    }
}
