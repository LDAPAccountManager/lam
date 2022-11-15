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

namespace Webauthn\TrustPath;

use JsonSerializable;

interface TrustPath extends JsonSerializable
{
    /**
     * @param mixed[] $data
     */
    public static function createFromArray(array $data): self;
}
