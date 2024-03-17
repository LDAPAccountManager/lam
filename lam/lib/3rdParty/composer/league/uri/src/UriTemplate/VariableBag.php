<?php

/**
 * League.Uri (https://uri.thephpleague.com)
 *
 * (c) Ignace Nyamagana Butera <nyamsprod@gmail.com>
 *
 * For the full copyright and license information, please view the LICENSE
 * file that was distributed with this source code.
 */

declare(strict_types=1);

namespace League\Uri\UriTemplate;

use ArrayAccess;
use Countable;
use League\Uri\Exceptions\TemplateCanNotBeExpanded;
use Stringable;
use function is_bool;
use function is_object;
use function is_scalar;

/**
 * @implements ArrayAccess<string, string|bool|int|float|array<string|bool|int|float>>
 */
final class VariableBag implements ArrayAccess, Countable
{
    /**
     * @var array<string,string|array<string>>
     */
    private array $variables = [];

    /**
     * @param iterable<string,string|bool|int|float|array<string|bool|int|float>> $variables
     */
    public function __construct(iterable $variables = [])
    {
        foreach ($variables as $name => $value) {
            $this->assign($name, $value);
        }
    }

    public function count(): int
    {
        return count($this->variables);
    }

    /**
     * @param array{variables: array<string,string|array<string>>} $properties
     */
    public static function __set_state(array $properties): self
    {
        return new self($properties['variables']);
    }

    public function offsetExists(mixed $offset): bool
    {
        return array_key_exists($offset, $this->variables);
    }

    public function offsetUnset(mixed $offset): void
    {
        unset($this->variables[$offset]);
    }

    public function offsetSet(mixed $offset, mixed $value): void
    {
        $this->assign($offset, $value); /* @phpstan-ignore-line */
    }

    public function offsetGet(mixed $offset): mixed
    {
        return $this->fetch($offset);
    }

    /**
     * @return array<string,string|array<string>>
     */
    public function all(): array
    {
        return $this->variables;
    }

    /**
     * Tells whether the bag is empty or not.
     */
    public function isEmpty(): bool
    {
        return [] === $this->variables;
    }

    /**
     * Fetches the variable value if none found returns null.
     *
     * @return null|string|array<string>
     */
    public function fetch(string $name): null|string|array
    {
        return $this->variables[$name] ?? null;
    }

    /**
     * @param string|bool|int|float|null|array<string|bool|int|float> $value
     */
    public function assign(string $name, string|bool|int|float|array|null $value): void
    {
        $this->variables[$name] = $this->normalizeValue($value, $name, true);
    }

    /**
     * @param Stringable|string|float|int|bool|null $value the value to be expanded
     *
     * @throws TemplateCanNotBeExpanded if the value contains nested list
     */
    private function normalizeValue(Stringable|array|string|float|int|bool|null $value, string $name, bool $isNestedListAllowed): array|string
    {
        return match (true) {
            is_bool($value) => true === $value ? '1' : '0',
            (null === $value || is_scalar($value) || is_object($value)) => (string) $value,
            !$isNestedListAllowed => throw TemplateCanNotBeExpanded::dueToNestedListOfValue($name),
            default => array_map(fn ($var): array|string => self::normalizeValue($var, $name, false), $value),
        };
    }

    /**
     * Replaces elements from passed variables into the current instance.
     */
    public function replace(VariableBag $variables): self
    {
        return new self($this->variables + $variables->variables);
    }
}
