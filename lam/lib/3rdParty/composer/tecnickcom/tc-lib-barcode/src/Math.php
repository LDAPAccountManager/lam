<?php

declare(strict_types=1);

/**
 * Math.php
 *
 * @since       2026-08-06
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode;

/**
 * Com\Tecnick\Barcode\Math
 *
 * Arbitrary precision arithmetic on non-negative decimal integer strings.
 * The bcmath extension is used when available, otherwise the equivalent
 * pure-PHP implementation is used, so bcmath is an optional dependency.
 *
 * @since       2026-08-06
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
final class Math
{
    /**
     * Maximum number of digits of a divisor that can be processed as an integer
     * without overflowing the running remainder, including on 32 bit platforms.
     *
     * @var int
     */
    private const MAX_INT_DIGITS = 8;

    /**
     * Cached availability of the bcmath functions.
     */
    private static ?bool $bcmath = null;

    /**
     * Returns true if the bcmath functions are available.
     */
    public static function hasBcmath(): bool
    {
        return self::$bcmath ??=
            \function_exists('bcadd')
            && \function_exists('bcmul')
            && \function_exists('bcdiv')
            && \function_exists('bcmod');
    }

    /**
     * Add two non-negative decimal integers.
     *
     * @param string $left  First operand
     * @param string $right Second operand
     *
     * @return numeric-string
     */
    public static function add(string $left, string $right): string
    {
        if (!self::hasBcmath()) {
            return self::fallbackAdd($left, $right);
        }

        return \bcadd(self::normalize($left), self::normalize($right), 0);
    }

    /**
     * Multiply two non-negative decimal integers.
     *
     * @param string $left  First operand
     * @param string $right Second operand
     *
     * @return numeric-string
     */
    public static function mul(string $left, string $right): string
    {
        if (!self::hasBcmath()) {
            return self::fallbackMul($left, $right);
        }

        return \bcmul(self::normalize($left), self::normalize($right), 0);
    }

    /**
     * Integer division of two non-negative decimal integers.
     *
     * @param string $left  Dividend
     * @param string $right Divisor
     *
     * @return numeric-string
     */
    public static function div(string $left, string $right): string
    {
        if (!self::hasBcmath()) {
            return self::fallbackDiv($left, $right);
        }

        return \bcdiv(self::normalize($left), self::normalize($right), 0);
    }

    /**
     * Remainder of the integer division of two non-negative decimal integers.
     *
     * @param string $left  Dividend
     * @param string $right Divisor
     *
     * @return numeric-string
     */
    public static function mod(string $left, string $right): string
    {
        if (!self::hasBcmath()) {
            return self::fallbackMod($left, $right);
        }

        return \bcmod(self::normalize($left), self::normalize($right), 0);
    }

    /**
     * Add two non-negative decimal integers without bcmath.
     *
     * @param string $left  First operand
     * @param string $right Second operand
     *
     * @return numeric-string
     */
    public static function fallbackAdd(string $left, string $right): string
    {
        $lft = self::normalize($left);
        $rgt = self::normalize($right);
        $pl = \strlen($lft) - 1;
        $pr = \strlen($rgt) - 1;
        $carry = 0;
        $sum = '';
        while ($pl >= 0 || $pr >= 0 || $carry > 0) {
            $digit = $carry;
            if ($pl >= 0) {
                $digit += (int) $lft[$pl];
                --$pl;
            }

            if ($pr >= 0) {
                $digit += (int) $rgt[$pr];
                --$pr;
            }

            $sum = (string) ($digit % 10) . $sum;
            $carry = \intdiv($digit, 10);
        }

        return self::normalize($sum);
    }

    /**
     * Multiply two non-negative decimal integers without bcmath.
     *
     * @param string $left  First operand
     * @param string $right Second operand
     *
     * @return numeric-string
     */
    public static function fallbackMul(string $left, string $right): string
    {
        $lft = self::normalize($left);
        $rgt = self::normalize($right);
        if ($lft === '0' || $rgt === '0') {
            return '0';
        }

        $llen = \strlen($lft);
        $rlen = \strlen($rgt);
        // partial products, least significant digit first
        $digits = \array_fill(0, $llen + $rlen, 0);
        for ($pl = $llen - 1; $pl >= 0; --$pl) {
            $mul = (int) $lft[$pl];
            $carry = 0;
            $pos = $llen - 1 - $pl;
            for ($pr = $rlen - 1; $pr >= 0; --$pr) {
                $cur = ($digits[$pos] ?? 0) + ($mul * (int) $rgt[$pr]) + $carry;
                $digits[$pos] = $cur % 10;
                $carry = \intdiv($cur, 10);
                ++$pos;
            }

            while ($carry > 0) {
                $cur = ($digits[$pos] ?? 0) + $carry;
                $digits[$pos] = $cur % 10;
                $carry = \intdiv($cur, 10);
                ++$pos;
            }
        }

        $product = '';
        foreach ($digits as $digit) {
            $product = (string) $digit . $product;
        }

        return self::normalize($product);
    }

    /**
     * Integer division of two non-negative decimal integers without bcmath.
     *
     * @param string $left  Dividend
     * @param string $right Divisor
     *
     * @return numeric-string
     */
    public static function fallbackDiv(string $left, string $right): string
    {
        return self::fallbackDivMod($left, $right)[0];
    }

    /**
     * Remainder of the integer division of two non-negative decimal integers without bcmath.
     *
     * @param string $left  Dividend
     * @param string $right Divisor
     *
     * @return numeric-string
     */
    public static function fallbackMod(string $left, string $right): string
    {
        return self::fallbackDivMod($left, $right)[1];
    }

    /**
     * Long division of two non-negative decimal integers.
     *
     * @param string $left  Dividend
     * @param string $right Divisor
     *
     * @return array{numeric-string, numeric-string} Quotient and remainder
     */
    private static function fallbackDivMod(string $left, string $right): array
    {
        $lft = self::normalize($left);
        $rgt = self::normalize($right);
        if ($rgt === '0') {
            throw new \DivisionByZeroError('Division by zero');
        }

        // divisors small enough to keep the running remainder inside an integer
        if (\strlen($rgt) <= self::MAX_INT_DIGITS) {
            return self::fallbackDivModInt($lft, (int) $rgt);
        }

        $quotient = '';
        $remainder = '0';
        $len = \strlen($lft);
        for ($pos = 0; $pos < $len; ++$pos) {
            // shift the next dividend digit into the remainder
            $remainder = self::normalize($remainder . $lft[$pos]);
            $digit = 0;
            while (self::compare($remainder, $rgt) >= 0) {
                $remainder = self::subtract($remainder, $rgt);
                ++$digit;
            }

            $quotient .= (string) $digit;
        }

        return [self::normalize($quotient), $remainder];
    }

    /**
     * Long division of a non-negative decimal integer by an integer divisor.
     *
     * @param numeric-string $left  Normalized dividend
     * @param int            $right Divisor greater than zero
     *
     * @return array{numeric-string, numeric-string} Quotient and remainder
     */
    private static function fallbackDivModInt(string $left, int $right): array
    {
        $quotient = '';
        $remainder = 0;
        $len = \strlen($left);
        for ($pos = 0; $pos < $len; ++$pos) {
            $remainder = ($remainder * 10) + (int) $left[$pos];
            $quotient .= (string) \intdiv($remainder, $right);
            $remainder %= $right;
        }

        return [self::normalize($quotient), (string) $remainder];
    }

    /**
     * Compare two normalized non-negative decimal integers.
     *
     * @param string $left  First operand
     * @param string $right Second operand
     *
     * @return int Negative if left is lower, zero if equal, positive if left is greater
     */
    private static function compare(string $left, string $right): int
    {
        $llen = \strlen($left);
        $rlen = \strlen($right);
        if ($llen !== $rlen) {
            return $llen <=> $rlen;
        }

        return \strcmp($left, $right);
    }

    /**
     * Subtract two normalized non-negative decimal integers, where the first one is the greater.
     *
     * @param string $left  Minuend
     * @param string $right Subtrahend
     *
     * @return numeric-string
     */
    private static function subtract(string $left, string $right): string
    {
        $pl = \strlen($left) - 1;
        $pr = \strlen($right) - 1;
        $borrow = 0;
        $diff = '';
        while ($pl >= 0) {
            $digit = (int) $left[$pl] - $borrow;
            if ($pr >= 0) {
                $digit -= (int) $right[$pr];
                --$pr;
            }

            if ($digit < 0) {
                $digit += 10;
                $borrow = 1;
            } else {
                $borrow = 0;
            }

            $diff = (string) $digit . $diff;
            --$pl;
        }

        return self::normalize($diff);
    }

    /**
     * Strip the leading zeros from a non-negative decimal integer string.
     *
     * @param string $number Number to normalize
     *
     * @return numeric-string
     */
    private static function normalize(string $number): string
    {
        if ($number === '' || !\ctype_digit($number)) {
            throw new \ValueError('Expecting a non-negative decimal integer string');
        }

        $number = \ltrim($number, '0');
        if ($number === '') {
            return '0';
        }

        /** @var numeric-string */
        return $number;
    }
}
