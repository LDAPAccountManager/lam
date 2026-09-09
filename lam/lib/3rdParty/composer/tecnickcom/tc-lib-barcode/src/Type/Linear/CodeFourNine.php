<?php

declare(strict_types=1);

/**
 * CodeFourNine.php
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
use Com\Tecnick\Barcode\Type\Linear\CodeFourNine\Data;

/**
 * Com\Tecnick\Barcode\Type\Linear\CodeFourNine;
 *
 * CodeFourNine Barcode type class
 * CODE 49
 *
 * A multi-row symbology of two to eight rows of four symbol characters, each
 * carrying two of the eight code characters of the row, the last of which is
 * the modulo 49 row check character. Rows are identified by the parity of
 * their symbol characters. The last row carries the row count and the starting
 * mode, and the two or three modulo 2401 symbol check characters.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class CodeFourNine extends \Com\Tecnick\Barcode\Type\Linear
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'C49';

    /**
     * Number of symbol characters of a row
     */
    protected const ROW_SYMBOLS = 4;

    /**
     * Number of code characters of a row, the last of which is the row check character
     */
    protected const ROW_CHARS = 8;

    /**
     * Smallest and largest number of rows
     */
    protected const MIN_ROWS = 2;

    protected const MAX_ROWS = 8;

    /**
     * Largest number of rows that carries two symbol check characters.
     * A larger symbol carries a third one.
     */
    protected const TWO_CHECKS_ROWS = 6;

    /**
     * Number of symbol characters of the last row that may be check characters
     */
    protected const CHECK_SYMBOLS = 3;

    /**
     * Number of elements of the encodation pattern of a symbol character
     */
    protected const PATTERN_LENGTH = 8;

    /**
     * Modulus of the code characters and radix of a symbol character
     */
    protected const MODULUS = 49;

    /**
     * Modulus of the symbol check characters
     */
    protected const SYMBOL_MODULUS = 2401;

    /**
     * Radix of the numeric encodation
     */
    protected const NUMERIC_RADIX = 48;

    /**
     * Offset added to the four digit groups of the numeric encodation
     */
    protected const NUMERIC_OFFSET = 100_000;

    /**
     * Starting mode of a symbol that begins with alphanumeric encodation
     */
    protected const MODE_ALPHANUMERIC = 0;

    /**
     * Starting mode of a symbol that begins with numeric encodation
     */
    protected const MODE_NUMERIC = 2;

    /**
     * Number of starting modes, the radix of the row count and mode character
     */
    protected const MODES = 7;

    /**
     * Start pattern, a bar and a space
     *
     * @var string
     */
    protected const START_PATTERN = '11';

    /**
     * Stop pattern, a bar
     *
     * @var string
     */
    protected const STOP_PATTERN = '4';

    /**
     * Height in modules of a row
     */
    protected const ROW_HEIGHT = 8;

    /**
     * Height in modules of a separator bar
     */
    protected const SEPARATOR = 1;

    /**
     * Starting mode of the symbol
     */
    protected int $mode = self::MODE_ALPHANUMERIC;

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (\strlen($this->code) === 0) {
            throw new BarcodeException('Empty input');
        }

        $data = $this->getCodeData();
        $rows = $this->getRowCount(\count($data));
        $symbols = $this->getSymbolChars($rows, $data);

        $this->bars = [];
        $this->ncols = 0;
        $this->nrows = ($rows * ($this::ROW_HEIGHT + $this::SEPARATOR)) + $this::SEPARATOR;

        foreach ($symbols as $row => $values) {
            $modules = $this->getRowModules($row, $rows, $values);
            $this->ncols = \count($modules);
            $this->addModuleBars($modules, ($row * ($this::ROW_HEIGHT + $this::SEPARATOR)) + $this::SEPARATOR);
        }

        // the separator bars above, between and below the rows
        for ($row = 0; $row <= $rows; ++$row) {
            $this->bars[] = [0, $row * ($this::ROW_HEIGHT + $this::SEPARATOR), $this->ncols, $this::SEPARATOR];
        }
    }

    /**
     * Get the code characters of the input, and set the starting mode to the
     * one that encodes it in the fewest code characters.
     *
     * @return array<int, int> Code character values
     *
     * @throws BarcodeException if the input contains a character outside the ASCII set
     */
    protected function getCodeData(): array
    {
        $length = \strlen($this->code);
        for ($pos = 0; $pos < $length; ++$pos) {
            if (\ord($this->code[$pos]) > 127) {
                throw new BarcodeException('Invalid character: ' . $this->code[$pos]);
            }
        }

        $cost = $this->getEncodingCost();
        $alphanumeric = $this->encodeFrom(0, $cost, false);
        $this->mode = $this::MODE_ALPHANUMERIC;
        if (($cost[0] ?? 0) <= $this->getNumericStartCost($cost)) {
            return $alphanumeric;
        }

        $this->mode = $this::MODE_NUMERIC;
        return $this->encodeFrom(0, $cost, true);
    }

    /**
     * Get the number of code characters needed to encode the input from each
     * position on, in the alphanumeric encodation.
     *
     * @return array<int, int> One entry per position, from the end of the input
     */
    protected function getEncodingCost(): array
    {
        $length = \strlen($this->code);
        $cost = [$length => 0];
        for ($pos = $length - 1; $pos >= 0; --$pos) {
            $best = \count(Data::ASCII[\ord($this->code[$pos])] ?? []) + ($cost[$pos + 1] ?? 0);
            foreach ($this->getNumericRuns($pos) as $run => $chars) {
                // a numeric shift enters the numeric encodation and another one leaves it
                $value = 1 + $chars + (($pos + $run) < $length ? 1 : 0) + ($cost[$pos + $run] ?? 0);
                $best = \min($best, $value);
            }

            $cost[$pos] = $best;
        }

        return $cost;
    }

    /**
     * Get the number of code characters needed by a symbol that starts in the
     * numeric encodation, or the cost of the alphanumeric one when the input
     * does not begin with a digit.
     *
     * @param array<int, int> $cost Cost of encoding the input from each position on
     */
    protected function getNumericStartCost(array $cost): int
    {
        $length = \strlen($this->code);
        $best = $cost[0] ?? 0;
        foreach ($this->getNumericRuns(0) as $run => $chars) {
            $best = \min($best, $chars + ($run < $length ? 1 : 0) + ($cost[$run] ?? 0));
        }

        return $best;
    }

    /**
     * Get the number of code characters of every numeric run that may start at
     * the given position.
     *
     * @return array<int, int> Number of code characters, keyed by run length
     */
    protected function getNumericRuns(int $pos): array
    {
        $runs = [];
        $length = \strlen($this->code);
        for ($run = 1; ($pos + $run) <= $length; ++$run) {
            if (!\ctype_digit($this->code[$pos + $run - 1])) {
                break;
            }

            $chars = $this->getNumericLength($run);
            if ($chars > 0) {
                $runs[$run] = $chars;
            }
        }

        return $runs;
    }

    /**
     * Get the number of code characters of a numeric run of the given number of
     * digits, or zero when the run cannot be encoded. Five digits are three
     * code characters; the remainder of the division by five is encoded by the
     * rules of section 2.2.2, the one of two digits over the last seven.
     */
    protected function getNumericLength(int $digits): int
    {
        return match ($digits % 5) {
            0 => \intdiv($digits, 5) * 3,
            1 => (\intdiv($digits, 5) * 3) + 1,
            2 => $digits < 7 ? 0 : (\intdiv($digits - 7, 5) * 3) + 5,
            3 => (\intdiv($digits, 5) * 3) + 2,
            default => (\intdiv($digits, 5) * 3) + 3,
        };
    }

    /**
     * Encode the input from the given position on, following the cheapest path.
     *
     * @param array<int, int> $cost    Cost of encoding the input from each position on
     * @param bool            $numeric Whether the first run is encoded without a leading numeric shift
     *
     * @return array<int, int> Code character values
     */
    protected function encodeFrom(int $pos, array $cost, bool $numeric): array
    {
        $out = [];
        $length = \strlen($this->code);
        while ($pos < $length) {
            $run = $this->getCheapestRun($pos, $cost, $numeric);
            if ($run > 0) {
                if (!$numeric) {
                    $out[] = Data::NUMERIC_SHIFT;
                }

                $out = \array_merge($out, $this->encodeNumeric(\substr($this->code, $pos, $run)));
                $pos += $run;
                if ($pos < $length) {
                    $out[] = Data::NUMERIC_SHIFT;
                }

                $numeric = false;
                continue;
            }

            $out = \array_merge($out, Data::ASCII[\ord($this->code[$pos])] ?? []);
            ++$pos;
            $numeric = false;
        }

        return $out;
    }

    /**
     * Get the length of the numeric run to encode at the given position, or
     * zero when the alphanumeric encodation is at least as short.
     *
     * @param array<int, int> $cost    Cost of encoding the input from each position on
     * @param bool            $numeric Whether the run is encoded without a leading numeric shift
     */
    protected function getCheapestRun(int $pos, array $cost, bool $numeric): int
    {
        $length = \strlen($this->code);
        $best = $numeric ? \PHP_INT_MAX : \count(Data::ASCII[\ord($this->code[$pos])] ?? []) + ($cost[$pos + 1] ?? 0);
        $found = 0;
        foreach ($this->getNumericRuns($pos) as $run => $chars) {
            $value = ($numeric ? 0 : 1) + $chars + (($pos + $run) < $length ? 1 : 0) + ($cost[$pos + $run] ?? 0);
            if ($value < $best) {
                $best = $value;
                $found = $run;
            }
        }

        return $found;
    }

    /**
     * Get the code characters of a run of digits in the numeric encodation.
     *
     * @return array<int, int> Code character values
     */
    protected function encodeNumeric(string $digits): array
    {
        $length = \strlen($digits);
        $tail = match ($length % 5) {
            0 => 0,
            1 => 1,
            2 => 7,
            3 => 3,
            default => 4,
        };

        $out = [];
        $head = $length - $tail;
        for ($pos = 0; $pos < $head; $pos += 5) {
            $out = \array_merge($out, $this->toRadix((int) \substr($digits, $pos, 5), 3));
        }

        return \array_merge($out, $this->encodeNumericTail(\substr($digits, $head)));
    }

    /**
     * Get the code characters of the digits left over by the groups of five.
     *
     * @return array<int, int> Code character values
     */
    protected function encodeNumericTail(string $digits): array
    {
        return match (\strlen($digits)) {
            0 => [],
            1 => [(int) $digits],
            3 => $this->toRadix((int) $digits, 2),
            4 => $this->toRadix($this::NUMERIC_OFFSET + (int) $digits, 3),
            // seven digits are four digits followed by three
            default => \array_merge(
                $this->toRadix($this::NUMERIC_OFFSET + (int) \substr($digits, 0, 4), 3),
                $this->toRadix((int) \substr($digits, 4), 2),
            ),
        };
    }

    /**
     * Convert a value to the given number of code characters, most significant first.
     *
     * @return array<int, int> Code character values
     */
    protected function toRadix(int $value, int $chars): array
    {
        $out = [];
        for ($pos = $chars - 1; $pos >= 0; --$pos) {
            $out[] = \intdiv($value, $this::NUMERIC_RADIX ** $pos) % $this::NUMERIC_RADIX;
        }

        return $out;
    }

    /**
     * Get the number of rows needed by the given number of code characters
     *
     * @throws BarcodeException if the data does not fit the largest symbol
     */
    protected function getRowCount(int $chars): int
    {
        for ($rows = $this::MIN_ROWS; $rows <= $this::MAX_ROWS; ++$rows) {
            if ($chars <= $this->getCapacity($rows)) {
                return $rows;
            }
        }

        throw new BarcodeException(
            'The code is too long: '
            . $chars
            . ' code characters (maximum '
            . $this->getCapacity($this::MAX_ROWS)
            . ')',
        );
    }

    /**
     * Get the number of data code characters of a symbol of the given number of
     * rows: seven in each row but the last one, which carries the row count and
     * mode character, the row check character and the symbol check characters.
     */
    protected function getCapacity(int $rows): int
    {
        $last = $rows <= $this::TWO_CHECKS_ROWS ? 2 : 0;
        return (($rows - 1) * ($this::ROW_CHARS - 1)) + $last;
    }

    /**
     * Get the symbol characters of every row of the symbol.
     *
     * @param array<int, int> $data Code character values of the input
     *
     * @return array<int, array<int, int>> Symbol character values, four per row
     */
    protected function getSymbolChars(int $rows, array $data): array
    {
        $chars = $this->getCodeMatrix($rows, $data);
        $symbols = [];
        foreach ($chars as $row => $values) {
            $symbols[$row] = $this->toSymbolChars($values);
        }

        $last = $rows - 1;
        $checks = $this->getSymbolChecks($rows, $symbols, $chars[$last][$this::ROW_CHARS - 2] ?? 0);
        foreach ($checks as $col => $value) {
            $symbols[$last][$col] = $value;
            $chars[$last][2 * $col] = \intdiv($value, $this::MODULUS);
            $chars[$last][(2 * $col) + 1] = $value % $this::MODULUS;
        }

        // the row check character of the last row covers the symbol check characters
        $chars[$last][$this::ROW_CHARS - 1] = $this->getRowCheck($chars[$last] ?? []);
        $symbols[$last] = $this->toSymbolChars($chars[$last]);

        return $symbols;
    }

    /**
     * Get the code characters of every row of the symbol, with the data padded
     * out by numeric shift characters, the row count and mode character, and
     * the row check character of every row but the last one.
     *
     * @param array<int, int> $data Code character values of the input
     *
     * @return array<int, array<int, int>> Code character values, eight per row
     */
    protected function getCodeMatrix(int $rows, array $data): array
    {
        $data = \array_pad($data, $this->getCapacity($rows), Data::NUMERIC_SHIFT);
        $chars = [];
        for ($row = 0; $row < $rows; ++$row) {
            $chars[$row] = \array_fill(0, $this::ROW_CHARS, 0);
        }

        $pos = 0;
        for ($row = 0; $row < ($rows - 1); ++$row) {
            for ($col = 0; $col < ($this::ROW_CHARS - 1); ++$col) {
                $chars[$row][$col] = $data[$pos++] ?? 0;
            }

            $chars[$row][$this::ROW_CHARS - 1] = $this->getRowCheck($chars[$row] ?? []);
        }

        for ($col = 0; $pos < \count($data); ++$col) {
            $chars[$rows - 1][$col] = $data[$pos++] ?? 0;
        }

        $chars[$rows - 1][$this::ROW_CHARS - 2] = ($this::MODES * ($rows - $this::MIN_ROWS)) + $this->mode;

        return $chars;
    }

    /**
     * Get the row check character, the modulo 49 sum of the other code
     * characters of the row.
     *
     * @param array<int, int> $chars Code character values of the row
     */
    protected function getRowCheck(array $chars): int
    {
        $sum = 0;
        for ($col = 0; $col < ($this::ROW_CHARS - 1); ++$col) {
            $sum += $chars[$col] ?? 0;
        }

        return $sum % $this::MODULUS;
    }

    /**
     * Get the symbol characters of a row, each one the value of its first code
     * character times 49 plus the value of its second one.
     *
     * @param array<int, int> $chars Code character values of the row
     *
     * @return array<int, int> Symbol character values of the row
     */
    protected function toSymbolChars(array $chars): array
    {
        $symbols = [];
        for ($col = 0; $col < $this::ROW_SYMBOLS; ++$col) {
            $symbols[] = ($this::MODULUS * ($chars[2 * $col] ?? 0)) + ($chars[(2 * $col) + 1] ?? 0);
        }

        return $symbols;
    }

    /**
     * Get the symbol check characters of the last row, keyed by their position
     * in the row. They are weighted sums over the row count and mode character
     * and every symbol character that precedes them.
     *
     * @param array<int, array<int, int>> $symbols Symbol character values, four per row
     * @param int                         $mode    Row count and mode character
     *
     * @return array<int, int> Symbol check character values
     */
    protected function getSymbolChecks(int $rows, array $symbols, int $mode): array
    {
        $checks = [];
        $last = $rows - 1;
        // the first symbol character of the last row is a check character only
        // in a symbol of seven or eight rows, otherwise it carries data
        $first = $rows <= $this::TWO_CHECKS_ROWS ? 1 : 0;
        for ($col = $first; $col < $this::CHECK_SYMBOLS; ++$col) {
            // the Z, Y and X weights of Table 5, in that order
            $shift = $this::CHECK_SYMBOLS - 1 - $col;
            $sum = (Data::MODE_WEIGHT[$shift] ?? 0) * $mode;
            for ($row = 0; $row < $last; ++$row) {
                foreach ($symbols[$row] ?? [] as $pos => $value) {
                    $sum += (Data::WEIGHT[($this::ROW_SYMBOLS * $row) + $pos + $shift] ?? 0) * $value;
                }
            }

            // every symbol character of the last row that precedes the check character
            for ($pos = 0; $pos < $col; ++$pos) {
                $sum +=
                    (Data::WEIGHT[($this::ROW_SYMBOLS * $last) + $pos + $shift] ?? 0) * ($symbols[$last][$pos] ?? 0);
            }

            $checks[$col] = $sum % $this::SYMBOL_MODULUS;
            $symbols[$last][$col] = $checks[$col];
        }

        return $checks;
    }

    /**
     * Get the modules of a row: the start pattern, four symbol characters in
     * the parity of the row and the stop pattern, alternating bars and spaces
     * from the leading bar of the start pattern.
     *
     * @param array<int, int> $values Symbol character values of the row
     *
     * @return array<int, int> One entry per module, 1 for a bar and 0 for a space
     */
    protected function getRowModules(int $row, int $rows, array $values): array
    {
        // every row but the last one is identified by its own parity pattern
        $parity = Data::ROW_PARITY[$row < ($rows - 1) ? $row : \count(Data::ROW_PARITY) - 1] ?? [];
        $widths = $this::START_PATTERN;
        foreach ($values as $col => $value) {
            $pattern = Data::PATTERNS[$value] ?? '';
            $widths .= \substr($pattern, ($parity[$col] ?? 0) * $this::PATTERN_LENGTH, $this::PATTERN_LENGTH);
        }

        $widths .= $this::STOP_PATTERN;

        $modules = [];
        $wlen = \strlen($widths);
        for ($pos = 0; $pos < $wlen; ++$pos) {
            $modules = \array_merge($modules, \array_fill(0, \max(0, (int) $widths[$pos]), ($pos % 2) === 0 ? 1 : 0));
        }

        return $modules;
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
