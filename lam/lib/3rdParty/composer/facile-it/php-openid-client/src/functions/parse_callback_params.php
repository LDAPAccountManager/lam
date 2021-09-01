<?php

declare(strict_types=1);

namespace Facile\OpenIDClient;

use Facile\OpenIDClient\Exception\RuntimeException;
use function in_array;
use function parse_str;
use Psr\Http\Message\ServerRequestInterface;
use function strtoupper;

/**
 * @return array<string, mixed>
 *
 * @template P as array{error?: string, error_description?: string, error_uri?: string, response?: string}&array<string, mixed>
 * @psalm-return P
 */
function parse_callback_params(ServerRequestInterface $serverRequest): array
{
    $method = strtoupper($serverRequest->getMethod());

    if (! in_array($method, ['GET', 'POST'], true)) {
        throw new RuntimeException('Invalid callback method');
    }

    if ('POST' === $method) {
        parse_str((string) $serverRequest->getBody(), $params);
    } elseif ('' !== $serverRequest->getUri()->getFragment()) {
        parse_str($serverRequest->getUri()->getFragment(), $params);
    } else {
        parse_str($serverRequest->getUri()->getQuery(), $params);
    }

    /** @var P $params */
    return $params;
}
