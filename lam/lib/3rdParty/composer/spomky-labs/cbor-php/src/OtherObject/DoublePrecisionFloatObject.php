<?php

declare(strict_types=1);

namespace CBOR\OtherObject;

use Brick\Math\BigInteger;
use CBOR\Normalizable;
use CBOR\OtherObject as Base;
use CBOR\Utils;
use InvalidArgumentException;
use function strlen;
use const INF;
use const NAN;

final class DoublePrecisionFloatObject extends Base implements Normalizable
{
    public static function supportedAdditionalInformation(): array
    {
        return [self::OBJECT_DOUBLE_PRECISION_FLOAT];
    }

    public static function createFromFloat(float $number): self
    {
        $value = match (true) {
            is_nan($number) => hex2bin('7FF8000000000000'),
            is_infinite($number) && $number > 0 => hex2bin('7FF0000000000000'),
            is_infinite($number) && $number < 0 => hex2bin('FFF0000000000000'),
            default => (fn (): string => unpack('S', "\x01\x00")[1] === 1 ? strrev(pack('d', $number)) : pack(
                'd',
                $number
            ))(),
        };

        return new self(self::OBJECT_DOUBLE_PRECISION_FLOAT, $value);
    }

    public static function createFromLoadedData(int $additionalInformation, ?string $data): Base
    {
        return new self($additionalInformation, $data);
    }

    public static function create(string $value): self
    {
        if (strlen($value) !== 8) {
            throw new InvalidArgumentException('The value is not a valid double precision floating point');
        }

        return new self(self::OBJECT_DOUBLE_PRECISION_FLOAT, $value);
    }

    public function normalize(): float|int
    {
        $exponent = $this->getExponent();
        $mantissa = $this->getMantissa();
        $sign = $this->getSign();

        if ($exponent === 0) {
            $val = $mantissa * 2 ** (-(1022 + 52));
        } elseif ($exponent !== 0b11111111111) {
            $val = ($mantissa + (1 << 52)) * 2 ** ($exponent - (1023 + 52));
        } else {
            $val = $mantissa === 0 ? INF : NAN;
        }

        return $sign * $val;
    }

    public function getExponent(): int
    {
        $data = $this->data;
        Utils::assertString($data, 'Invalid data');

        return Utils::binToBigInteger($data)->shiftedRight(52)->and(Utils::hexToBigInteger('7ff'))->toInt();
    }

    public function getMantissa(): int
    {
        $data = $this->data;
        Utils::assertString($data, 'Invalid data');

        return Utils::binToBigInteger($data)->and(Utils::hexToBigInteger('fffffffffffff'))->toInt();
    }

    public function getSign(): int
    {
        $data = $this->data;
        Utils::assertString($data, 'Invalid data');
        $sign = Utils::binToBigInteger($data)->shiftedRight(63);

        return $sign->isEqualTo(BigInteger::one()) ? -1 : 1;
    }
}
