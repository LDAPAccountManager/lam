<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Decrypter;

use Facile\JoseVerifier\Exception\InvalidTokenExceptionInterface;

interface TokenDecrypterInterface
{
    /**
     * @throws InvalidTokenExceptionInterface
     */
    public function decrypt(string $jwt): ?string;
}
