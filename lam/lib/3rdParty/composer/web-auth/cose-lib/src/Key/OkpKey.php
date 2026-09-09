<?php

declare(strict_types=1);

namespace Cose\Key;

use function array_key_exists;
use function in_array;
use InvalidArgumentException;
use SpomkyLabs\Pki\ASN1\Type\Constructed\Sequence;
use SpomkyLabs\Pki\ASN1\Type\Primitive\BitString;
use SpomkyLabs\Pki\ASN1\Type\Primitive\Integer;
use SpomkyLabs\Pki\ASN1\Type\Primitive\ObjectIdentifier;
use SpomkyLabs\Pki\ASN1\Type\Primitive\OctetString;

/**
 * @final
 * @see \Cose\Tests\Key\OkpKeyTest
 */
class OkpKey extends Key
{
    final public const CURVE_X25519 = 4;

    final public const CURVE_X448 = 5;

    final public const CURVE_ED25519 = 6;

    final public const CURVE_ED448 = 7;

    final public const CURVE_NAME_X25519 = 'X25519';

    final public const CURVE_NAME_X448 = 'X448';

    final public const CURVE_NAME_ED25519 = 'Ed25519';

    final public const CURVE_NAME_ED448 = 'Ed448';

    final public const DATA_CURVE = -1;

    final public const DATA_X = -2;

    final public const DATA_D = -4;

    private const SUPPORTED_CURVES_INT = [
        self::CURVE_X25519,
        self::CURVE_X448,
        self::CURVE_ED25519,
        self::CURVE_ED448,
    ];

    private const SUPPORTED_CURVES_NAME = [
        self::CURVE_NAME_X25519,
        self::CURVE_NAME_X448,
        self::CURVE_NAME_ED25519,
        self::CURVE_NAME_ED448,
    ];

    private const CURVE_OID = [
        self::CURVE_X25519 => '1.3.101.110',
        self::CURVE_X448 => '1.3.101.111',
        self::CURVE_ED25519 => '1.3.101.112',
        self::CURVE_ED448 => '1.3.101.113',
        self::CURVE_NAME_X25519 => '1.3.101.110',
        self::CURVE_NAME_X448 => '1.3.101.111',
        self::CURVE_NAME_ED25519 => '1.3.101.112',
        self::CURVE_NAME_ED448 => '1.3.101.113',
    ];

    /**
     * @param array<int|string, mixed> $data
     */
    public function __construct(array $data)
    {
        foreach ([self::DATA_CURVE, self::TYPE] as $key) {
            if (is_numeric($data[$key])) {
                $data[$key] = (int) $data[$key];
            }
        }
        parent::__construct($data);
        if ($data[self::TYPE] !== self::TYPE_OKP && $data[self::TYPE] !== self::TYPE_NAME_OKP) {
            throw new InvalidArgumentException('Invalid OKP key. The key type does not correspond to an OKP key');
        }
        if (! isset($data[self::DATA_CURVE], $data[self::DATA_X])) {
            throw new InvalidArgumentException('Invalid EC2 key. The curve or the "x" coordinate is missing');
        }
        if (is_numeric($data[self::DATA_CURVE])) {
            if (! in_array((int) $data[self::DATA_CURVE], self::SUPPORTED_CURVES_INT, true)) {
                throw new InvalidArgumentException('The curve is not supported');
            }
        } elseif (! in_array($data[self::DATA_CURVE], self::SUPPORTED_CURVES_NAME, true)) {
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

    public function x(): string
    {
        return $this->get(self::DATA_X);
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

    public function curve(): int|string
    {
        return $this->get(self::DATA_CURVE);
    }

    public function toPublic(): self
    {
        $data = $this->getData();
        unset($data[self::DATA_D]);

        return new self($data);
    }

    /**
     * Returns the key as a PEM encoded RFC 8410 OneAsymmetricKey (private) or SubjectPublicKeyInfo (public) structure.
     */
    public function asPEM(): string
    {
        $oid = ObjectIdentifier::create(self::CURVE_OID[$this->curve()]);

        if ($this->isPrivate()) {
            $der = Sequence::create(
                Integer::create(0),
                Sequence::create($oid),
                OctetString::create(OctetString::create($this->d())->toDER())
            );

            return $this->pem('PRIVATE KEY', $der->toDER());
        }

        $der = Sequence::create(Sequence::create($oid), BitString::create($this->x()));

        return $this->pem('PUBLIC KEY', $der->toDER());
    }
}
