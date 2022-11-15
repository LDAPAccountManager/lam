<?php

declare(strict_types=1);

/*
 * The MIT License (MIT)
 *
 * Copyright (c) 2014-2021 Spomky-Labs
 *
 * This software may be modified and distributed under the terms
 * of the MIT license.  See the LICENSE file for details.
 */

namespace Cose\Algorithm\Signature\EdDSA;

use Cose\Key\Key;

final class ED256 extends EdDSA
{
    public const ID = -260;

    public static function identifier(): int
    {
        return self::ID;
    }

    public function sign(string $data, Key $key): string
    {
        $hashedData = hash('sha256', $data, true);

        return parent::sign($hashedData, $key);
    }

    public function verify(string $data, Key $key, string $signature): bool
    {
        $hashedData = hash('sha256', $data, true);

        return parent::verify($hashedData, $key, $signature);
    }
}
