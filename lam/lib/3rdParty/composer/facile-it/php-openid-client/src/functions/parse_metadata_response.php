<?php

declare(strict_types=1);

namespace Facile\OpenIDClient;

use Facile\OpenIDClient\Exception\InvalidArgumentException;
use function is_array;
use function json_decode;
use JsonException;
use Psr\Http\Message\ResponseInterface;

/**
 * @internal
 *
 * @return array<string, mixed>
 *
 * @psalm-return array<string, mixed>
 */
function parse_metadata_response(ResponseInterface $response, ?int $expectedCode = null): array
{
    check_server_response($response, $expectedCode);

    try {
        /** @var null|(array{}&array{error?: string, error_description?: string, error_uri?: string, response?: string}) $data */
        $data = json_decode((string) $response->getBody(), true, 512, JSON_THROW_ON_ERROR);
    } catch (JsonException $e) {
        throw new InvalidArgumentException('Invalid metadata content', 0, $e);
    }

    if (! is_array($data)) {
        throw new InvalidArgumentException('Invalid metadata content');
    }

    return $data;
}
