<?php

declare(strict_types=1);

namespace AESKW;

final class A128KW implements Wrapper
{
    use AESKW;

    protected static function getMethod(): string
    {
        return 'aes-128-ecb';
    }
}
