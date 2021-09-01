<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use function array_merge;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use function http_build_query;
use Psr\Http\Message\RequestInterface;

final class None implements AuthMethodInterface
{
    public function getSupportedMethod(): string
    {
        return 'none';
    }

    public function createRequest(
        RequestInterface $request,
        OpenIDClient $client,
        array $claims
    ): RequestInterface {
        $params = array_merge(['client_id' => $client->getMetadata()->getClientId()], $claims);
        $request->getBody()->write(http_build_query($params));

        return $request;
    }
}
