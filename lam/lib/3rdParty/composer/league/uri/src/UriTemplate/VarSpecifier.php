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

use League\Uri\Exceptions\SyntaxError;
use function preg_match;

final class VarSpecifier
{
    /**
     * Variables specification regular expression pattern.
     *
     * @link https://tools.ietf.org/html/rfc6570#section-2.3
     */
    private const REGEXP_VARSPEC = '/^
        (?<name>(?:[A-z0-9_\.]|%[0-9a-fA-F]{2})+)
        (?<modifier>\:(?<position>\d+)|\*)?
    $/x';

    private function __construct(
        public readonly string $name,
        public readonly string $modifier,
        public readonly int $position
    ) {
    }

    /**
     * @param array{name: string, modifier:string, position:int} $properties
     */
    public static function __set_state(array $properties): self
    {
        return new self($properties['name'], $properties['modifier'], $properties['position']);
    }

    public static function createFromString(string $specification): self
    {
        if (1 !== preg_match(self::REGEXP_VARSPEC, $specification, $parsed)) {
            throw new SyntaxError('The variable specification "'.$specification.'" is invalid.');
        }

        $parsed += ['modifier' => '', 'position' => ''];
        if ('' !== $parsed['position']) {
            $parsed['position'] = (int) $parsed['position'];
            $parsed['modifier'] = ':';
        }

        if ('' === $parsed['position']) {
            $parsed['position'] = 0;
        }

        if (10000 <= $parsed['position']) {
            throw new SyntaxError('The variable specification "'.$specification.'" is invalid the position modifier must be lower than 10000.');
        }

        return new self($parsed['name'], $parsed['modifier'], $parsed['position']);
    }

    public function toString(): string
    {
        if (0 < $this->position) {
            return $this->name.$this->modifier.$this->position;
        }

        return $this->name.$this->modifier;
    }

    /**
     * @codeCoverageIgnore
     * @deprecated since version 6.6.0 use the readonly property instead
     */
    public function name(): string
    {
        return $this->name;
    }

    /**
     * @codeCoverageIgnore
     * @deprecated since version 6.6.0 use the readonly property instead
     */
    public function modifier(): string
    {
        return $this->modifier;
    }

    /**
     * @codeCoverageIgnore
     * @deprecated since version 6.6.0 use the readonly property instead
     */
    public function position(): int
    {
        return $this->position;
    }
}
