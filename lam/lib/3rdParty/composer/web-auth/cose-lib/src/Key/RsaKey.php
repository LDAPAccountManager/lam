<?php

declare(strict_types=1);

namespace Cose\Key;

use function array_key_exists;
use Brick\Math\BigInteger;
use FG\ASN1\Universal\BitString;
use FG\ASN1\Universal\Integer;
use FG\ASN1\Universal\NullObject;
use FG\ASN1\Universal\ObjectIdentifier;
use FG\ASN1\Universal\Sequence;
use InvalidArgumentException;
use LogicException;
use function unpack;

/**
 * @final
 */
class RsaKey extends Key
{
    final public const DATA_N = -1;

    final public const DATA_E = -2;

    final public const DATA_D = -3;

    final public const DATA_P = -4;

    final public const DATA_Q = -5;

    final public const DATA_DP = -6;

    final public const DATA_DQ = -7;

    final public const DATA_QI = -8;

    final public const DATA_OTHER = -9;

    final public const DATA_RI = -10;

    final public const DATA_DI = -11;

    final public const DATA_TI = -12;

    /**
     * @param array<int|string, mixed> $data
     */
    public function __construct(array $data)
    {
        parent::__construct($data);
        if (! isset($data[self::TYPE]) || (int) $data[self::TYPE] !== self::TYPE_RSA) {
            throw new InvalidArgumentException('Invalid RSA key. The key type does not correspond to a RSA key');
        }
        if (! isset($data[self::DATA_N], $data[self::DATA_E])) {
            throw new InvalidArgumentException('Invalid RSA key. The modulus or the exponent is missing');
        }
    }

    /**
     * @param array<int|string, mixed> $data
     */
    public static function create(array $data): self
    {
        return new self($data);
    }

    public function n(): string
    {
        return $this->get(self::DATA_N);
    }

    public function e(): string
    {
        return $this->get(self::DATA_E);
    }

    public function d(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_D);
    }

    public function p(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_P);
    }

    public function q(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_Q);
    }

    public function dP(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_DP);
    }

    public function dQ(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_DQ);
    }

    public function QInv(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_QI);
    }

    /**
     * @return array<mixed>
     */
    public function other(): array
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_OTHER);
    }

    public function rI(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_RI);
    }

    public function dI(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_DI);
    }

    public function tI(): string
    {
        $this->checkKeyIsPrivate();

        return $this->get(self::DATA_TI);
    }

    public function hasPrimes(): bool
    {
        return $this->has(self::DATA_P) && $this->has(self::DATA_Q);
    }

    /**
     * @return string[]
     */
    public function primes(): array
    {
        return [$this->p(), $this->q()];
    }

    public function hasExponents(): bool
    {
        return $this->has(self::DATA_DP) && $this->has(self::DATA_DQ);
    }

    /**
     * @return string[]
     */
    public function exponents(): array
    {
        return [$this->dP(), $this->dQ()];
    }

    public function hasCoefficient(): bool
    {
        return $this->has(self::DATA_QI);
    }

    public function isPublic(): bool
    {
        return ! $this->isPrivate();
    }

    public function isPrivate(): bool
    {
        return array_key_exists(self::DATA_D, $this->getData());
    }

    public function asPem(): string
    {
        if ($this->isPrivate()) {
            throw new LogicException('Unsupported for private keys.');
        }
        $bitSring = new Sequence(
            new Integer($this->fromBase64ToInteger($this->n())),
            new Integer($this->fromBase64ToInteger($this->e()))
        );

        $der = new Sequence(
            new Sequence(new ObjectIdentifier('1.2.840.113549.1.1.1'), new NullObject()),
            new BitString(bin2hex($bitSring->getBinary()))
        );

        return $this->pem('PUBLIC KEY', $der->getBinary());
    }

    private function fromBase64ToInteger(string $value): string
    {
        $data = unpack('H*', $value);
        $hex = current($data);

        return BigInteger::fromBase($hex, 16)->toBase(10);
    }

    private function pem(string $type, string $der): string
    {
        return sprintf("-----BEGIN %s-----\n", mb_strtoupper($type)) .
            chunk_split(base64_encode($der), 64, "\n") .
            sprintf("-----END %s-----\n", mb_strtoupper($type));
    }

    private function checkKeyIsPrivate(): void
    {
        if (! $this->isPrivate()) {
            throw new InvalidArgumentException('The key is not private.');
        }
    }
}
