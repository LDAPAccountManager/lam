<?php

declare(strict_types=1);

namespace Cose\Key;

use function array_key_exists;
use FG\ASN1\ExplicitlyTaggedObject;
use FG\ASN1\Universal\BitString;
use FG\ASN1\Universal\Integer;
use FG\ASN1\Universal\ObjectIdentifier;
use FG\ASN1\Universal\OctetString;
use FG\ASN1\Universal\Sequence;
use function in_array;
use InvalidArgumentException;

/**
 * @final
 */
class Ec2Key extends Key
{
    final public const CURVE_P256 = 1;

    final public const CURVE_P256K = 8;

    final public const CURVE_P384 = 2;

    final public const CURVE_P521 = 3;

    final public const DATA_CURVE = -1;

    final public const DATA_X = -2;

    final public const DATA_Y = -3;

    final public const DATA_D = -4;

    private const SUPPORTED_CURVES = [self::CURVE_P256, self::CURVE_P256K, self::CURVE_P384, self::CURVE_P521];

    private const NAMED_CURVE_OID = [
        self::CURVE_P256 => '1.2.840.10045.3.1.7',
        // NIST P-256 / secp256r1
        self::CURVE_P256K => '1.3.132.0.10',
        // NIST P-256K / secp256k1
        self::CURVE_P384 => '1.3.132.0.34',
        // NIST P-384 / secp384r1
        self::CURVE_P521 => '1.3.132.0.35',
        // NIST P-521 / secp521r1
    ];

    private const CURVE_KEY_LENGTH = [
        self::CURVE_P256 => 32,
        self::CURVE_P256K => 32,
        self::CURVE_P384 => 48,
        self::CURVE_P521 => 66,
    ];

    /**
     * @param array<int|string, mixed> $data
     */
    public function __construct(array $data)
    {
        parent::__construct($data);
        if (! isset($data[self::TYPE]) || (int) $data[self::TYPE] !== self::TYPE_EC2) {
            throw new InvalidArgumentException('Invalid EC2 key. The key type does not correspond to an EC2 key');
        }
        if (! isset($data[self::DATA_CURVE], $data[self::DATA_X], $data[self::DATA_Y])) {
            throw new InvalidArgumentException('Invalid EC2 key. The curve or the "x/y" coordinates are missing');
        }
        if (mb_strlen((string) $data[self::DATA_X], '8bit') !== self::CURVE_KEY_LENGTH[(int) $data[self::DATA_CURVE]]) {
            throw new InvalidArgumentException('Invalid length for x coordinate');
        }
        if (mb_strlen((string) $data[self::DATA_Y], '8bit') !== self::CURVE_KEY_LENGTH[(int) $data[self::DATA_CURVE]]) {
            throw new InvalidArgumentException('Invalid length for y coordinate');
        }
        if (! in_array((int) $data[self::DATA_CURVE], self::SUPPORTED_CURVES, true)) {
            throw new InvalidArgumentException('The curve is not supported');
        }
    }

    /**
     * @param array<int|string, mixed> $data
     */
    public static function create(array $data): self
    {
        return new self($data);
    }

    public function toPublic(): self
    {
        $data = $this->getData();
        unset($data[self::DATA_D]);

        return new self($data);
    }

    public function x(): string
    {
        return $this->get(self::DATA_X);
    }

    public function y(): string
    {
        return $this->get(self::DATA_Y);
    }

    public function isPrivate(): bool
    {
        return array_key_exists(self::DATA_D, $this->getData());
    }

    public function d(): string
    {
        if (! $this->isPrivate()) {
            throw new InvalidArgumentException('The key is not private.');
        }
        return $this->get(self::DATA_D);
    }

    public function curve(): int
    {
        return (int) $this->get(self::DATA_CURVE);
    }

    public function asPEM(): string
    {
        if ($this->isPrivate()) {
            $der = new Sequence(
                new Integer(1),
                new OctetString(bin2hex($this->d())),
                new ExplicitlyTaggedObject(0, new ObjectIdentifier($this->getCurveOid())),
                new ExplicitlyTaggedObject(1, new BitString(bin2hex($this->getUncompressedCoordinates())))
            );

            return $this->pem('EC PRIVATE KEY', $der->getBinary());
        }

        $der = new Sequence(
            new Sequence(new ObjectIdentifier('1.2.840.10045.2.1'), new ObjectIdentifier($this->getCurveOid())),
            new BitString(bin2hex($this->getUncompressedCoordinates()))
        );

        return $this->pem('PUBLIC KEY', $der->getBinary());
    }

    public function getUncompressedCoordinates(): string
    {
        return "\x04" . $this->x() . $this->y();
    }

    private function getCurveOid(): string
    {
        return self::NAMED_CURVE_OID[$this->curve()];
    }

    private function pem(string $type, string $der): string
    {
        return sprintf("-----BEGIN %s-----\n", mb_strtoupper($type)) .
            chunk_split(base64_encode($der), 64, "\n") .
            sprintf("-----END %s-----\n", mb_strtoupper($type));
    }
}
