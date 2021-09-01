<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use function array_merge;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use function http_build_query;
use Psr\Http\Message\RequestInterface;

abstract class AbstractTLS implements AuthMethodInterface
{
    public function createRequest(
        RequestInterface $request,
        OpenIDClient $client,
        array $claims
    ): RequestInterface {
        $clientId = $client->getMetadata()->getClientId();

        $claims = array_merge($claims, [
            'client_id' => $clientId,
        ]);

        $request->getBody()->write(http_build_query($claims));

        return $request;
    }
}
