<?php

declare(strict_types=1);

namespace CBOR\OtherObject;

use InvalidArgumentException;
use function array_key_exists;

final class OtherObjectManager implements OtherObjectManagerInterface
{
    /**
     * @param  class-string<OtherObjectInterface>[] $classes
     */
    public function __construct(
        private array $classes = [],
    ) {
    }

    /**
     * @param  class-string<OtherObjectInterface>[] $classes
     */
    public static function create(array $classes = []): self
    {
        return new self($classes);
    }

    /**
     * @param class-string<OtherObjectInterface> $class
     */
    public function add(string $class): self
    {
        foreach ($class::supportedAdditionalInformation() as $ai) {
            if ($ai < 0) {
                throw new InvalidArgumentException('Invalid additional information.');
            }
            $this->classes[$ai] = $class;
        }

        return $this;
    }

    /**
     * @return class-string<OtherObjectInterface>
     */
    public function getClassForValue(int $value): string
    {
        return array_key_exists($value, $this->classes) ? $this->classes[$value] : GenericObject::class;
    }

    public function createObjectForValue(int $value, ?string $data): OtherObjectInterface
    {
        $class = $this->getClassForValue($value);

        return $class::createFromLoadedData($value, $data);
    }
}
