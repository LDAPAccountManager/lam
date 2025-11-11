<?php

declare(strict_types=1);

namespace CBOR\Tag;

use CBOR\CBORObject;
use CBOR\Utils;
use InvalidArgumentException;
use function array_key_exists;

final class TagManager implements TagManagerInterface
{
    /**
     * @param class-string<TagInterface>[] $classes
     */
    public function __construct(
        private array $classes = []
    ) {
    }

    /**
     * @param array<class-string<TagInterface>> $classes
     */
    public static function create(array $classes = []): self
    {
        return new self($classes);
    }

    /**
     * @param class-string<TagInterface> $class
     */
    public function add(string $class): self
    {
        if ($class::getTagId() < 0) {
            throw new InvalidArgumentException('Invalid tag ID.');
        }
        $this->classes[$class::getTagId()] = $class;

        return $this;
    }

    /**
     * @return class-string<TagInterface>
     */
    public function getClassForValue(int $value): string
    {
        return array_key_exists($value, $this->classes) ? $this->classes[$value] : GenericTag::class;
    }

    public function createObjectForValue(int $additionalInformation, ?string $data, CBORObject $object): TagInterface
    {
        $value = $additionalInformation;
        if ($additionalInformation >= 24) {
            Utils::assertString($data, 'Invalid data');
            $value = Utils::binToInt($data);
        }
        $class = $this->getClassForValue($value);

        return $class::createFromLoadedData($additionalInformation, $data, $object);
    }
}
