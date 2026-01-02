<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Decrypter;

final class NullTokenDecrypter implements TokenDecrypterInterface
{
    public function decrypt(string $jwt): string
    {
        return $jwt;
    }
}
