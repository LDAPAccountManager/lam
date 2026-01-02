<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

use Override;

final class TokenSetFactory implements TokenSetFactoryInterface
{
    #[Override]
    public function fromArray(array $array): TokenSetInterface
    {
        return TokenSet::fromParams($array);
    }
}
