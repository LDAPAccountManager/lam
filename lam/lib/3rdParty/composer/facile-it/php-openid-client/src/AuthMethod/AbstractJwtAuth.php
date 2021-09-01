<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use function array_merge;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use function http_build_query;
use Psr\Http\Message\RequestInterface;

abstract class AbstractJwtAuth implements AuthMethodInterface
{
    /**
     * @param array<string, mixed> $claims
     */
    abstract protected function createAuthJwt(OpenIDClient $client, array $claims = []): string;

    public function createRequest(
        RequestInterface $request,
        OpenIDClient $client,
        array $claims
    ): RequestInterface {
        $clientId = $client->getMetadata()->getClientId();

        $claims = array_merge([
            'client_id' => $clientId,
            'client_assertion_type' => 'urn:ietf:params:oauth:client-assertion-type:jwt-bearer',
            'client_assertion' => $this->createAuthJwt($client, $claims),
        ], $claims);

        $request->getBody()->write(http_build_query($claims));

        return $request;
    }
}
