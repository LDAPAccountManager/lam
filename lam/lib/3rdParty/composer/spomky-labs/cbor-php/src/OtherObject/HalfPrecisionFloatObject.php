<?php

declare(strict_types=1);

namespace CBOR\OtherObject;

use Brick\Math\BigInteger;
use CBOR\Normalizable;
use CBOR\OtherObject as Base;
use CBOR\Utils;
use const INF;
use InvalidArgumentException;
use const NAN;
use function strlen;

final class HalfPrecisionFloatObject extends Base implements Normalizable
{
    public static function supportedAdditionalInformation(): array
    {
        return [self::OBJECT_HALF_PRECISION_FLOAT];
    }

    public static function createFromFloat(float $number): self
    {
        // IEEE 754 binary16 (half-precision) conversion
        // Format: 1 sign bit, 5 exponent bits (bias 15), 10 mantissa bits

        // Handle special cases: NaN
        if (is_nan($number)) {
            // RFC 8949: canonical NaN is 0xf97e00 (quiet NaN with zero payload)
            return new self(self::OBJECT_HALF_PRECISION_FLOAT, self::hex2binSafe('7E00'));
        }

        // Handle special cases: Infinity
        if (is_infinite($number)) {
            $value = $number > 0 ? self::hex2binSafe('7C00') : self::hex2binSafe('FC00');
            return new self(self::OBJECT_HALF_PRECISION_FLOAT, $value);
        }

        // Extract sign
        $sign = $number < 0 ? 1 : 0;
        $absNumber = abs($number);

        // Handle zero (positive and negative zero)
        if ($absNumber === 0.0) {
            $value = pack('n', $sign << 15);
            return new self(self::OBJECT_HALF_PRECISION_FLOAT, $value);
        }

        // Convert via single precision to simplify extraction
        // Pack as float, then convert to big-endian bytes
        $packed = pack('f', $absNumber);
        if (unpack('S', "\x01\x00")[1] === 1) {
            $packed = strrev($packed); // Little-endian system
        }
        $singleBits = unpack('N', $packed)[1];

        // Extract single precision components
        $singleExponent = ($singleBits >> 23) & 0xFF;
        $singleMantissa = $singleBits & 0x7FFFFF;

        // Convert exponent: single (bias 127) to half (bias 15)
        $halfExponent = $singleExponent - 127 + 15;

        // Handle overflow: value too large for half precision
        if ($halfExponent >= 0x1F) {
            // Overflow to infinity
            $value = pack('n', ($sign << 15) | 0x7C00);
            return new self(self::OBJECT_HALF_PRECISION_FLOAT, $value);
        }

        // Handle underflow and subnormal numbers
        if ($halfExponent <= 0) {
            // Check if we can represent as subnormal
            if ($halfExponent < -10) {
                // Too small, flush to zero
                $value = pack('n', $sign << 15);
                return new self(self::OBJECT_HALF_PRECISION_FLOAT, $value);
            }

            // Subnormal: shift mantissa right and set exponent to 0
            $halfMantissa = ($singleMantissa | 0x800000) >> (1 - $halfExponent + 13);
            $halfExponent = 0;
        } else {
            // Normal number: convert mantissa from 23 bits to 10 bits
            // Truncate (simple rounding) - could be improved with round-to-nearest
            $halfMantissa = $singleMantissa >> 13;
        }

        // Assemble the 16-bit half-precision value
        $halfBits = ($sign << 15) | ($halfExponent << 10) | ($halfMantissa & 0x3FF);
        $value = pack('n', $halfBits);

        return new self(self::OBJECT_HALF_PRECISION_FLOAT, $value);
    }

    public static function createFromLoadedData(int $additionalInformation, ?string $data): Base
    {
        return new self($additionalInformation, $data);
    }

    public static function create(string $value): self
    {
        if (strlen($value) !== 2) {
            throw new InvalidArgumentException('The value is not a valid half precision floating point');
        }

        return new self(self::OBJECT_HALF_PRECISION_FLOAT, $value);
    }

    public function normalize(): float|int
    {
        $exponent = $this->getExponent();
        $mantissa = $this->getMantissa();
        $sign = $this->getSign();

        if ($exponent === 0) {
            $val = $mantissa * 2 ** (-24);
        } elseif ($exponent !== 0b11111) {
            $val = ($mantissa + (1 << 10)) * 2 ** ($exponent - 25);
        } else {
            $val = $mantissa === 0 ? INF : NAN;
        }

        return $sign * $val;
    }

    public function getExponent(): int
    {
        $data = $this->data;
        Utils::assertString($data, 'Invalid data');

        return Utils::binToBigInteger($data)->shiftedRight(10)->and(Utils::hexToBigInteger('1f'))->toInt();
    }

    public function getMantissa(): int
    {
        $data = $this->data;
        Utils::assertString($data, 'Invalid data');

        return Utils::binToBigInteger($data)->and(Utils::hexToBigInteger('3ff'))->toInt();
    }

    public function getSign(): int
    {
        $data = $this->data;
        Utils::assertString($data, 'Invalid data');
        $sign = Utils::binToBigInteger($data)->shiftedRight(15);

        return $sign->isEqualTo(BigInteger::one()) ? -1 : 1;
    }

    private static function hex2binSafe(string $hex): string
    {
        $result = hex2bin($hex);
        if ($result === false) {
            throw new InvalidArgumentException('Invalid hex string');
        }
        return $result;
    }
}
