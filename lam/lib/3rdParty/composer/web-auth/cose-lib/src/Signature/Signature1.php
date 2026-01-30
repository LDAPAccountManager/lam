<?php

declare(strict_types=1);

namespace Cose\Signature;

use CBOR\ByteStringObject;
use CBOR\ListObject;
use CBOR\TextStringObject;
use Stringable;

final class Signature1 implements Stringable
{
    public function __construct(
        private readonly ByteStringObject $protectedHeader,
        private readonly ByteStringObject $payload
    ) {
    }

    public function __toString(): string
    {
        $structure = new ListObject();
        $structure->add(new TextStringObject('Signature1'));
        $structure->add($this->protectedHeader);
        $structure->add(new ByteStringObject(''));
        $structure->add($this->payload);

        return (string) $structure;
    }

    public static function create(ByteStringObject $protectedHeader, ByteStringObject $payload): self
    {
        return new self($protectedHeader, $payload);
    }

    public function getProtectedHeader(): ByteStringObject
    {
        return $this->protectedHeader;
    }

    public function getPayload(): ByteStringObject
    {
        return $this->payload;
    }
}
