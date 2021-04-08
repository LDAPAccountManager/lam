<?php

declare(strict_types=1);

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2018-2020 Spomky-Labs
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

namespace CBOR;

use Brick\Math\BigInteger;

/**
 * @internal
 */
abstract class Utils
{
    public static function binToInt(string $value): int
    {
        return self::binToBigInteger($value)->toInt();
    }

    public static function binToBigInteger(string $value): BigInteger
    {
        return self::hexToBigInteger(bin2hex($value));
    }

    public static function hexToInt(string $value): int
    {
        return self::hexToBigInteger($value)->toInt();
    }

    public static function hexToBigInteger(string $value): BigInteger
    {
        return BigInteger::fromBase($value, 16);
    }

    public static function hexToString(string $value): string
    {
        return BigInteger::fromBase(bin2hex($value), 16)->toBase(10);
    }

    public static function intToHex(int $value): string
    {
        return BigInteger::of($value)->toBase(16);
    }
}
