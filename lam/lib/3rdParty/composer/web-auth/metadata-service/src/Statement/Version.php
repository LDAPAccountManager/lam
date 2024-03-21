<?php

declare(strict_types=1);

namespace Webauthn\MetadataService\Statement;

use function array_key_exists;
use Assert\Assertion;
use JsonSerializable;
use LogicException;
use Webauthn\MetadataService\Utils;

class Version implements JsonSerializable
{
    private readonly ?int $major;

    private readonly ?int $minor;

    public function __construct(?int $major, ?int $minor)
    {
        if ($major === null && $minor === null) {
            throw new LogicException('Invalid data. Must contain at least one item');
        }
        Assertion::greaterOrEqualThan($major, 0, 'Invalid argument "major"');
        Assertion::greaterOrEqualThan($minor, 0, 'Invalid argument "minor"');

        $this->major = $major;
        $this->minor = $minor;
    }

    public function getMajor(): ?int
    {
        return $this->major;
    }

    public function getMinor(): ?int
    {
        return $this->minor;
    }

    /**
     * @param array<string, mixed> $data
     */
    public static function createFromArray(array $data): self
    {
        $data = Utils::filterNullValues($data);
        foreach (['major', 'minor'] as $key) {
            if (array_key_exists($key, $data)) {
                Assertion::integer($data[$key], sprintf('Invalid value for key "%s"', $key));
            }
        }

        return new self($data['major'] ?? null, $data['minor'] ?? null);
    }

    /**
     * @return array<string, int|null>
     */
    public function jsonSerialize(): array
    {
        $data = [
            'major' => $this->major,
            'minor' => $this->minor,
        ];

        return Utils::filterNullValues($data);
    }
}
