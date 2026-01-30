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

final class CoseEncryptTag extends Tag
{
    private const TAG_ID = 96;

    private readonly ByteStringObject $protectedHeader;

    private readonly MapObject $unprotectedHeader;

    private readonly ByteStringObject $ciphertext;

    private readonly ListObject $recipients;

    public function __construct(int $additionalInformation, ?string $data, CBORObject $object)
    {
        if (! $object instanceof ListObject) {
            throw new InvalidArgumentException('Not a valid CoseEncrypt object. No list.');
        }
        if ($object->count() !== 4) {
            throw new InvalidArgumentException('Not a valid CoseEncrypt object. The list shall have 4 items.');
        }

        $protectedHeader = $object->get(0);
        $unprotectedHeader = $object->get(1);
        $ciphertext = $object->get(2);
        $recipients = $object->get(3);

        if (! $protectedHeader instanceof ByteStringObject) {
            throw new InvalidArgumentException(
                'Not a valid CoseEncrypt object. The item 1 shall be a ByteString object.'
            );
        }
        if (! $unprotectedHeader instanceof MapObject) {
            throw new InvalidArgumentException('Not a valid CoseEncrypt object. The item 2 shall be a Map object.');
        }
        if (! $ciphertext instanceof ByteStringObject) {
            throw new InvalidArgumentException(
                'Not a valid CoseEncrypt object. The item 3 shall be a ByteString object.'
            );
        }
        if (! $recipients instanceof ListObject) {
            throw new InvalidArgumentException('Not a valid CoseEncrypt object. The item 4 shall be a List object.');
        }

        parent::__construct($additionalInformation, $data, $object);
        $this->protectedHeader = $protectedHeader;
        $this->unprotectedHeader = $unprotectedHeader;
        $this->ciphertext = $ciphertext;
        $this->recipients = $recipients;
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
        ByteStringObject $ciphertext,
        ListObject $recipients
    ): self {
        $protectedHeaderAsBytesString = ByteStringObject::create((string) $protectedHeader);
        $object = ListObject::create([
            $protectedHeaderAsBytesString,
            $unprotectedHeader,
            $ciphertext,
            $recipients,
        ]);

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

    public function getRecipients(): ListObject
    {
        return $this->recipients;
    }
}
