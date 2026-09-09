<?php

declare(strict_types=1);

namespace CBOR\Tag;

use Brick\Math\BigInteger;
use CBOR\CBORObject;
use CBOR\ListObject;
use CBOR\NegativeIntegerObject;
use CBOR\Normalizable;
use CBOR\Tag;
use CBOR\UnsignedIntegerObject;
use function count;
use function extension_loaded;
use InvalidArgumentException;
use RuntimeException;
use function sprintf;

final class BigFloatTag extends Tag implements Normalizable
{
    /**
     * The maximum absolute value accepted for the exponent.
     *
     * The result of 2^e grows exponentially with e, so an exponent taken from untrusted input is a denial of
     * service vector: a document of a handful of bytes can otherwise ask bcpow() for a number of several billion
     * digits. The bound is far above any legitimate use of this tag -- 2^8192 already has more than 2400 digits,
     * whereas an IEEE 754 double covers exponents within +/-1074.
     */
    public const MAX_ABSOLUTE_EXPONENT = 8192;

    public function __construct(int $additionalInformation, ?string $data, CBORObject $object)
    {
        if (! extension_loaded('bcmath')) {
            throw new RuntimeException('The extension "bcmath" is required to use this tag');
        }

        if (! $object instanceof ListObject || count($object) !== 2) {
            throw new InvalidArgumentException(
                'This tag only accepts a ListObject object that contains an exponent and a mantissa.'
            );
        }
        $e = $object->get(0);
        if (! $e instanceof UnsignedIntegerObject && ! $e instanceof NegativeIntegerObject) {
            throw new InvalidArgumentException('The exponent must be a Signed Integer or an Unsigned Integer object.');
        }
        $m = $object->get(1);
        if (! $m instanceof UnsignedIntegerObject && ! $m instanceof NegativeIntegerObject && ! $m instanceof NegativeBigIntegerTag && ! $m instanceof UnsignedBigIntegerTag) {
            throw new InvalidArgumentException(
                'The mantissa must be a Positive or Negative Signed Integer or an Unsigned Integer object.'
            );
        }

        parent::__construct($additionalInformation, $data, $object);
    }

    public static function getTagId(): int
    {
        return self::TAG_BIG_FLOAT;
    }

    public static function createFromLoadedData(int $additionalInformation, ?string $data, CBORObject $object): self
    {
        return new self($additionalInformation, $data, $object);
    }

    public static function create(CBORObject $object): self
    {
        [$ai, $data] = self::determineComponents(self::TAG_BIG_FLOAT);

        return new self($ai, $data, $object);
    }

    public static function createFromExponentAndMantissa(CBORObject $e, CBORObject $m): self
    {
        $object = ListObject::create()
            ->add($e)
            ->add($m)
        ;

        return self::create($object);
    }

    /**
     * Create a BigFloat from a PHP float value.
     * BigFloat represents: mantissa × 2^exponent
     * This method converts a float to the most compact BigFloat representation.
     *
     * @param float $value The float value to convert
     * @return self The BigFloat tag object
     */
    public static function createFromFloat(float $value): self
    {
        if (! extension_loaded('bcmath')) {
            throw new RuntimeException('The extension "bcmath" is required to use this method');
        }

        // Handle special cases
        if (is_nan($value) || is_infinite($value)) {
            throw new InvalidArgumentException('BigFloat cannot represent NaN or Infinity');
        }

        if ($value === 0.0) {
            // 0 = 0 × 2^0
            return self::createFromExponentAndMantissa(
                UnsignedIntegerObject::create(0),
                UnsignedIntegerObject::create(0)
            );
        }

        // Get the binary representation
        $negative = $value < 0;
        $absValue = abs($value);

        // Convert to binary string representation
        // We'll use the fact that PHP floats are IEEE 754 double precision
        // Extract mantissa and exponent from the float
        $packed = pack('d', $absValue);
        if (unpack('S', "\x01\x00")[1] === 1) {
            $packed = strrev($packed); // Little-endian system
        }
        $bits = unpack('J', $packed)[1];

        $biasedExponent = ($bits >> 52) & 0x7FF;
        $fraction = $bits & 0xFFFFFFFFFFFFF;

        if ($biasedExponent === 0) {
            // Subnormal number
            $exponent = -1022 - 52;
            $mantissa = $fraction;
        } else {
            // Normal number
            $exponent = $biasedExponent - 1023 - 52;
            $mantissa = $fraction | (1 << 52); // Add implicit leading 1
        }

        // Apply sign
        if ($negative) {
            $mantissa = -$mantissa;
        }

        // Normalize: remove trailing zeros from mantissa by adjusting exponent
        while ($mantissa !== 0 && ($mantissa & 1) === 0) {
            $mantissa >>= 1;
            $exponent++;
        }

        // Create exponent object
        if ($exponent >= 0) {
            $exponentObj = UnsignedIntegerObject::create($exponent);
        } else {
            $exponentObj = NegativeIntegerObject::create($exponent);
        }

        // Create mantissa object
        if ($mantissa >= 0) {
            $mantissaObj = UnsignedIntegerObject::createFromString((string) $mantissa);
        } else {
            $mantissaObj = NegativeIntegerObject::createFromString((string) $mantissa);
        }

        return self::createFromExponentAndMantissa($exponentObj, $mantissaObj);
    }

    public function normalize()
    {
        /** @var ListObject $object */
        $object = $this->object;
        /** @var UnsignedIntegerObject|NegativeIntegerObject $e */
        $e = $object->get(0);
        /** @var UnsignedIntegerObject|NegativeIntegerObject|NegativeBigIntegerTag|UnsignedBigIntegerTag $m */
        $m = $object->get(1);

        $exponent = (string) $e->normalize();
        self::assertExponentIsWithinBounds($exponent);

        return rtrim(bcmul((string) $m->normalize(), bcpow('2', $exponent, 100), 100), '0');
    }

    private static function assertExponentIsWithinBounds(string $exponent): void
    {
        if (BigInteger::of($exponent)->abs()->isGreaterThan(BigInteger::of(self::MAX_ABSOLUTE_EXPONENT))) {
            throw new InvalidArgumentException(sprintf(
                'The exponent is out of range. Its absolute value shall not exceed %d, got "%s".',
                self::MAX_ABSOLUTE_EXPONENT,
                $exponent
            ));
        }
    }
}
