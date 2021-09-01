<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Decrypter;

interface TokenDecrypterInterface
{
    public function decrypt(string $jwt): ?string;
}
