<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

/**
 * @psalm-import-type TokenSetMixedType from TokenSetInterface
 */
interface TokenSetFactoryInterface
{
    /**
     * @param array<string, mixed> $array
     *
     * @psalm-param TokenSetMixedType $array
     */
    public function fromArray(array $array): TokenSetInterface;
}
