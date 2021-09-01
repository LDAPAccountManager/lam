<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Base64Url\Base64Url;
use function hash;
use Jose\Component\Core\JWK;
use function round;
use function substr;

function derived_key(string $secret, int $length): JWK
{
    $hash = substr(hash('sha256', $secret, true), 0, (int) round($length / 8));

    return new JWK([
        'k' => Base64Url::encode($hash),
        'kty' => 'oct',
    ]);
}
