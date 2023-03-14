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

namespace CBOR\OtherObject;

use CBOR\Normalizable;
use CBOR\OtherObject as Base;

final class NullObject extends Base implements Normalizable
{
    public function __construct()
    {
        parent::__construct(self::OBJECT_NULL, null);
    }

    public static function create(): self
    {
        return new self();
    }

    public static function supportedAdditionalInformation(): array
    {
        return [self::OBJECT_NULL];
    }

    public static function createFromLoadedData(int $additionalInformation, ?string $data): Base
    {
        return new self();
    }

    public function normalize(): ?string
    {
        return null;
    }

    /**
     * @deprecated The method will be removed on v3.0. Please rely on the CBOR\Normalizable interface
     */
    public function getNormalizedData(bool $ignoreTags = false)
    {
        return $this->normalize();
    }
}
