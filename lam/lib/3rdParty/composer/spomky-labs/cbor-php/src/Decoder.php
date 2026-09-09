<?php

declare(strict_types=1);

namespace CBOR;

use CBOR\OtherObject\BreakObject;
use CBOR\OtherObject\DoublePrecisionFloatObject;
use CBOR\OtherObject\FalseObject;
use CBOR\OtherObject\HalfPrecisionFloatObject;
use CBOR\OtherObject\NullObject;
use CBOR\OtherObject\OtherObjectManager;
use CBOR\OtherObject\OtherObjectManagerInterface;
use CBOR\OtherObject\SimpleObject;
use CBOR\OtherObject\SinglePrecisionFloatObject;
use CBOR\OtherObject\TrueObject;
use CBOR\OtherObject\UndefinedObject;
use CBOR\Tag\Base16EncodingTag;
use CBOR\Tag\Base64EncodingTag;
use CBOR\Tag\Base64Tag;
use CBOR\Tag\Base64UrlEncodingTag;
use CBOR\Tag\Base64UrlTag;
use CBOR\Tag\BigFloatTag;
use CBOR\Tag\CBOREncodingTag;
use CBOR\Tag\CBORTag;
use CBOR\Tag\DatetimeTag;
use CBOR\Tag\DecimalFractionTag;
use CBOR\Tag\MimeTag;
use CBOR\Tag\NegativeBigIntegerTag;
use CBOR\Tag\TagManager;
use CBOR\Tag\TagManagerInterface;
use CBOR\Tag\TimestampTag;
use CBOR\Tag\UnsignedBigIntegerTag;
use CBOR\Tag\UriTag;
use InvalidArgumentException;
use function ord;
use RuntimeException;
use function sprintf;
use const STR_PAD_LEFT;

final class Decoder implements DecoderInterface
{
    /**
     * The maximum nesting depth allowed by default. Deeper structures are rejected as the recursive processing of the
     * data may exhaust the call stack.
     */
    public const DEFAULT_MAX_DEPTH = 1000;

    private TagManagerInterface $tagObjectManager;

    private OtherObjectManagerInterface $otherTypeManager;

    private int $maxDepth;

    public function __construct(
        ?TagManagerInterface $tagObjectManager = null,
        ?OtherObjectManagerInterface $otherTypeManager = null,
        int $maxDepth = self::DEFAULT_MAX_DEPTH
    ) {
        if ($maxDepth < 1) {
            throw new InvalidArgumentException(sprintf(
                'The maximum nesting depth shall be at least 1. Got %d.',
                $maxDepth
            ));
        }
        $this->tagObjectManager = $tagObjectManager ?? $this->generateTagManager();
        $this->otherTypeManager = $otherTypeManager ?? $this->generateOtherObjectManager();
        $this->maxDepth = $maxDepth;
    }

    public static function create(
        ?TagManagerInterface $tagObjectManager = null,
        ?OtherObjectManagerInterface $otherTypeManager = null,
        int $maxDepth = self::DEFAULT_MAX_DEPTH
    ): self {
        return new self($tagObjectManager, $otherTypeManager, $maxDepth);
    }

    public function decode(Stream $stream): CBORObject
    {
        return $this->process($stream, false, 0);
    }

    private function process(Stream $stream, bool $breakable, int $depth): CBORObject
    {
        if ($depth > $this->maxDepth) {
            throw new InvalidArgumentException(sprintf(
                'Cannot parse the data. Maximum nesting depth of %d exceeded.',
                $this->maxDepth
            ));
        }
        $ib = ord($stream->read(1));
        $mt = $ib >> 5;
        $ai = $ib & 0b00011111;
        $val = null;
        switch ($ai) {
            case CBORObject::LENGTH_1_BYTE: // 24
            case CBORObject::LENGTH_2_BYTES: // 25
            case CBORObject::LENGTH_4_BYTES: // 26
            case CBORObject::LENGTH_8_BYTES: // 27
                $val = $stream->read(2 ** ($ai & 0b00000111));
                break;
            case CBORObject::FUTURE_USE_1: // 28
            case CBORObject::FUTURE_USE_2: // 29
            case CBORObject::FUTURE_USE_3: // 30
                throw new InvalidArgumentException(sprintf(
                    'Cannot parse the data. Found invalid Additional Information "%s" (%d).',
                    str_pad(decbin($ai), 8, '0', STR_PAD_LEFT),
                    $ai
                ));
            case CBORObject::LENGTH_INDEFINITE: // 31
                return $this->processInfinite($stream, $mt, $breakable, $depth);
        }

        return $this->processFinite($stream, $mt, $ai, $val, $depth);
    }

    private function processFinite(Stream $stream, int $mt, int $ai, ?string $val, int $depth): CBORObject
    {
        switch ($mt) {
            case CBORObject::MAJOR_TYPE_UNSIGNED_INTEGER: // 0
                return UnsignedIntegerObject::createObjectForValue($ai, $val);
            case CBORObject::MAJOR_TYPE_NEGATIVE_INTEGER: // 1
                return NegativeIntegerObject::createObjectForValue($ai, $val);
            case CBORObject::MAJOR_TYPE_BYTE_STRING: // 2
                $length = $val === null ? $ai : Utils::binToInt($val);

                return ByteStringObject::create($stream->read($length));
            case CBORObject::MAJOR_TYPE_TEXT_STRING: // 3
                $length = $val === null ? $ai : Utils::binToInt($val);

                return TextStringObject::create($stream->read($length));
            case CBORObject::MAJOR_TYPE_LIST: // 4
                $object = ListObject::create();
                $nbItems = $val === null ? $ai : Utils::binToInt($val);
                for ($i = 0; $i < $nbItems; ++$i) {
                    $object->add($this->process($stream, false, $depth + 1));
                }

                return $object;
            case CBORObject::MAJOR_TYPE_MAP: // 5
                $object = MapObject::create();
                $nbItems = $val === null ? $ai : Utils::binToInt($val);
                for ($i = 0; $i < $nbItems; ++$i) {
                    $object->add(
                        $this->process($stream, false, $depth + 1),
                        $this->process($stream, false, $depth + 1)
                    );
                }

                return $object;
            case CBORObject::MAJOR_TYPE_TAG: // 6
                return $this->tagObjectManager->createObjectForValue(
                    $ai,
                    $val,
                    $this->process($stream, false, $depth + 1)
                );
            case CBORObject::MAJOR_TYPE_OTHER_TYPE: // 7
                return $this->otherTypeManager->createObjectForValue($ai, $val);
            default:
                throw new RuntimeException(sprintf(
                    'Unsupported major type "%s" (%d).',
                    str_pad(decbin($mt), 5, '0', STR_PAD_LEFT),
                    $mt
                )); // Should never append
        }
    }

    private function processInfinite(Stream $stream, int $mt, bool $breakable, int $depth): CBORObject
    {
        switch ($mt) {
            case CBORObject::MAJOR_TYPE_BYTE_STRING: // 2
                $object = IndefiniteLengthByteStringObject::create();
                while (! ($it = $this->process($stream, true, $depth + 1)) instanceof BreakObject) {
                    if (! $it instanceof ByteStringObject) {
                        throw new RuntimeException(
                            'Unable to parse the data. Infinite Byte String object can only get Byte String objects.'
                        );
                    }
                    $object->add($it);
                }

                return $object;
            case CBORObject::MAJOR_TYPE_TEXT_STRING: // 3
                $object = IndefiniteLengthTextStringObject::create();
                while (! ($it = $this->process($stream, true, $depth + 1)) instanceof BreakObject) {
                    if (! $it instanceof TextStringObject) {
                        throw new RuntimeException(
                            'Unable to parse the data. Infinite Text String object can only get Text String objects.'
                        );
                    }
                    $object->add($it);
                }

                return $object;
            case CBORObject::MAJOR_TYPE_LIST: // 4
                $object = IndefiniteLengthListObject::create();
                $it = $this->process($stream, true, $depth + 1);
                while (! $it instanceof BreakObject) {
                    $object->add($it);
                    $it = $this->process($stream, true, $depth + 1);
                }

                return $object;
            case CBORObject::MAJOR_TYPE_MAP: // 5
                $object = IndefiniteLengthMapObject::create();
                while (! ($it = $this->process($stream, true, $depth + 1)) instanceof BreakObject) {
                    $object->add($it, $this->process($stream, false, $depth + 1));
                }

                return $object;
            case CBORObject::MAJOR_TYPE_OTHER_TYPE: // 7
                if (! $breakable) {
                    throw new InvalidArgumentException('Cannot parse the data. No enclosing indefinite.');
                }

                return BreakObject::create();
            case CBORObject::MAJOR_TYPE_UNSIGNED_INTEGER: // 0
            case CBORObject::MAJOR_TYPE_NEGATIVE_INTEGER: // 1
            case CBORObject::MAJOR_TYPE_TAG: // 6
            default:
                throw new InvalidArgumentException(sprintf(
                    'Cannot parse the data. Found infinite length for Major Type "%s" (%d).',
                    str_pad(decbin($mt), 5, '0', STR_PAD_LEFT),
                    $mt
                ));
        }
    }

    private function generateTagManager(): TagManagerInterface
    {
        return TagManager::create([
            DatetimeTag::class,
            TimestampTag::class,

            UnsignedBigIntegerTag::class,
            NegativeBigIntegerTag::class,

            DecimalFractionTag::class,
            BigFloatTag::class,

            Base64UrlEncodingTag::class,
            Base64EncodingTag::class,
            Base16EncodingTag::class,
            CBOREncodingTag::class,

            UriTag::class,
            Base64UrlTag::class,
            Base64Tag::class,
            MimeTag::class,

            CBORTag::class,
        ]);
    }

    private function generateOtherObjectManager(): OtherObjectManagerInterface
    {
        return OtherObjectManager::create()
            ->add(BreakObject::class)
            ->add(SimpleObject::class)
            ->add(FalseObject::class)
            ->add(TrueObject::class)
            ->add(NullObject::class)
            ->add(UndefinedObject::class)
            ->add(HalfPrecisionFloatObject::class)
            ->add(SinglePrecisionFloatObject::class)
            ->add(DoublePrecisionFloatObject::class)
        ;
    }
}
