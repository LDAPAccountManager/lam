<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Psr\Http\Message\RequestInterface;
use Override;

use function array_merge;
use function http_build_query;

abstract class AbstractTLS implements AuthMethodInterface
{
    #[Override]
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
