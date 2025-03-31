<?php

declare(strict_types=1);

namespace AESKW;

final class A192KW implements Wrapper
{
    use AESKW;

    protected static function getMethod(): string
    {
        return 'aes-192-ecb';
    }
}
