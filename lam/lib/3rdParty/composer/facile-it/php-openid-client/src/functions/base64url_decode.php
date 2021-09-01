<?php

declare(strict_types=1);

namespace Facile\OpenIDClient;

use function base64_decode;
use Facile\OpenIDClient\Exception\RuntimeException;
use function str_pad;
use function strlen;
use function strtr;

function base64url_decode(string $data): string
{
    $decoded = base64_decode(str_pad(strtr($data, '-_', '+/'), strlen($data) % 4, '='), true);

    if (false === $decoded) {
        throw new RuntimeException('Unable to base64url_decode');
    }

    return $decoded;
}
