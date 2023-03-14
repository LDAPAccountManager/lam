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

namespace CBOR\Tag;

use CBOR\ByteStringObject;
use CBOR\CBORObject;
use CBOR\IndefiniteLengthByteStringObject;
use CBOR\IndefiniteLengthTextStringObject;
use CBOR\Tag;
use CBOR\TextStringObject;
use InvalidArgumentException;

final class Base64EncodingTag extends Tag
{
    public static function getTagId(): int
    {
        return self::TAG_ENCODED_BASE64;
    }

    public static function createFromLoadedData(int $additionalInformation, ?string $data, CBORObject $object): Tag
    {
        return new self($additionalInformation, $data, $object);
    }

    public static function create(CBORObject $object): Tag
    {
        [$ai, $data] = self::determineComponents(self::TAG_ENCODED_BASE64);

        return new self($ai, $data, $object);
    }

    /**
     * @deprecated The method will be removed on v3.0. Please rely on the CBOR\Normalizable interface
     */
    public function getNormalizedData(bool $ignoreTags = false)
    {
        if ($ignoreTags) {
            return $this->object->getNormalizedData($ignoreTags);
        }

        if (! $this->object instanceof ByteStringObject && ! $this->object instanceof IndefiniteLengthByteStringObject && ! $this->object instanceof TextStringObject && ! $this->object instanceof IndefiniteLengthTextStringObject) {
            return $this->object->getNormalizedData($ignoreTags);
        }

        $result = base64_decode($this->object->getNormalizedData($ignoreTags), true);
        if ($result === false) {
            throw new InvalidArgumentException('Unable to decode the data');
        }

        return $result;
    }
}
