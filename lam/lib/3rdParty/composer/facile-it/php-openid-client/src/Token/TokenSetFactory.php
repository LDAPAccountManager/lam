<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

final class TokenSetFactory implements TokenSetFactoryInterface
{
    public function fromArray(array $array): TokenSetInterface
    {
        return TokenSet::fromParams($array);
    }
}
