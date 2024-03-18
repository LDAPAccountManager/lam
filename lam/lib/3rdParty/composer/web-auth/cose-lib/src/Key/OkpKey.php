<?php

declare(strict_types=1);

namespace Cose\Key;

use function array_key_exists;
use function in_array;
use InvalidArgumentException;

/**
 * @final
 */
class OkpKey extends Key
{
    final public const CURVE_X25519 = 4;

    final public const CURVE_X448 = 5;

    final public const CURVE_ED25519 = 6;

    final public const CURVE_ED448 = 7;

    final public const DATA_CURVE = -1;

    final public const DATA_X = -2;

    final public const DATA_D = -4;

    private const SUPPORTED_CURVES = [
        self::CURVE_X25519,
        self::CURVE_X448,
        self::CURVE_ED25519,
        self::CURVE_ED448,
    ];

    /**
     * @param array<int|string, mixed> $data
     */
    public function __construct(array $data)
    {
        parent::__construct($data);
        if (! isset($data[self::TYPE]) || (int) $data[self::TYPE] !== self::TYPE_OKP) {
            throw new InvalidArgumentException('Invalid OKP key. The key type does not correspond to an OKP key');
        }
        if (! isset($data[self::DATA_CURVE], $data[self::DATA_X])) {
            throw new InvalidArgumentException('Invalid EC2 key. The curve or the "x" coordinate is missing');
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

    public function curve(): int
    {
        return (int) $this->get(self::DATA_CURVE);
    }
}
