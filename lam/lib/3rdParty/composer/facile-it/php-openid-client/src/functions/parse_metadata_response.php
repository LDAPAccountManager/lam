<?php

declare(strict_types=1);

namespace Facile\OpenIDClient;

use Facile\OpenIDClient\Exception\InvalidArgumentException;
use function is_array;
use function json_decode;
use Psr\Http\Message\ResponseInterface;

/**
 * @return array<string, mixed>
 *
 * @template P as array{error?: string, error_description?: string, error_uri?: string, response?: string}&array<string, mixed>
 * @psalm-return array<string, mixed>
 */
function parse_metadata_response(ResponseInterface $response, ?int $expectedCode = null): array
{
    check_server_response($response, $expectedCode);

    /** @var bool|P $data */
    $data = json_decode((string) $response->getBody(), true);

    if (! is_array($data)) {
        throw new InvalidArgumentException('Invalid metadata content');
    }

    return $data;
}
