<?php

declare(strict_types=1);

namespace Cose\Encryption;

use CBOR\ByteStringObject;
use CBOR\CBORObject;
use CBOR\Decoder;
use CBOR\ListObject;
use CBOR\MapObject;
use CBOR\OtherObject\OtherObjectManager;
use CBOR\StringStream;
use CBOR\Tag;
use CBOR\Tag\TagManager;
use InvalidArgumentException;

final class CoseEncrypt0Tag extends Tag
{
    private const TAG_ID = 16;

    private readonly ByteStringObject $protectedHeader;

    private readonly MapObject $unprotectedHeader;

    private readonly ByteStringObject $ciphertext;

    public function __construct(int $additionalInformation, ?string $data, CBORObject $object)
    {
        if (! $object instanceof ListObject) {
            throw new InvalidArgumentException('Not a valid CoseEncrypt0 object. No list.');
        }
        if ($object->count() !== 3) {
            throw new InvalidArgumentException('Not a valid CoseEncrypt0 object. The list shall have 3 items.');
        }

        $protectedHeader = $object->get(0);
        $unprotectedHeader = $object->get(1);
        $ciphertext = $object->get(2);

        if (! $protectedHeader instanceof ByteStringObject) {
            throw new InvalidArgumentException(
                'Not a valid CoseEncrypt0 object. The item 1 shall be a ByteString object.'
            );
        }
        if (! $unprotectedHeader instanceof MapObject) {
            throw new InvalidArgumentException('Not a valid CoseEncrypt0 object. The item 2 shall be a Map object.');
        }
        if (! $ciphertext instanceof ByteStringObject) {
            throw new InvalidArgumentException(
                'Not a valid CoseEncrypt0 object. The item 3 shall be a ByteString object.'
            );
        }

        parent::__construct($additionalInformation, $data, $object);
        $this->protectedHeader = $protectedHeader;
        $this->unprotectedHeader = $unprotectedHeader;
        $this->ciphertext = $ciphertext;
    }

    public static function getTagId(): int
    {
        return self::TAG_ID;
    }

    public static function createFromLoadedData(int $additionalInformation, ?string $data, CBORObject $object): self
    {
        return new self($additionalInformation, $data, $object);
    }

    public static function create(
        MapObject $protectedHeader,
        MapObject $unprotectedHeader,
        ByteStringObject $ciphertext
    ): self {
        $protectedHeaderAsBytesString = ByteStringObject::create((string) $protectedHeader);
        $object = ListObject::create([$protectedHeaderAsBytesString, $unprotectedHeader, $ciphertext]);

        [$additionalInformation, $data] = self::determineComponents(self::TAG_ID);

        return new self($additionalInformation, $data, $object);
    }

    public function getProtectedHeader(): ByteStringObject
    {
        return $this->protectedHeader;
    }

    public function getProtectedHeaderAsMap(?Decoder $decoder = null): MapObject
    {
        $stream = new StringStream($this->protectedHeader->getValue());
        $decoder ??= Decoder::create(TagManager::create(), OtherObjectManager::create());
        $decoded = $decoder->decode($stream);

        if (! $decoded instanceof MapObject) {
            throw new InvalidArgumentException('Protected header is not a valid Map object.');
        }

        return $decoded;
    }

    public function getUnprotectedHeader(): MapObject
    {
        return $this->unprotectedHeader;
    }

    public function getCiphertext(): ByteStringObject
    {
        return $this->ciphertext;
    }
}
