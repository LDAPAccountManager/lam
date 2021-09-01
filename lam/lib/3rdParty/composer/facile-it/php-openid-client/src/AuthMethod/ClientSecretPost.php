<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use function array_merge;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use function http_build_query;
use Psr\Http\Message\RequestInterface;

final class ClientSecretPost implements AuthMethodInterface
{
    public function getSupportedMethod(): string
    {
        return 'client_secret_post';
    }

    public function createRequest(
        RequestInterface $request,
        OpenIDClient $client,
        array $claims
    ): RequestInterface {
        $clientSecret = $client->getMetadata()->getClientSecret();

        if (null === $clientSecret) {
            throw new InvalidArgumentException($this->getSupportedMethod() . ' cannot be used without client_secret metadata');
        }

        $clientId = $client->getMetadata()->getClientId();

        $claims = array_merge($claims, [
            'client_id' => $clientId,
            'client_secret' => $clientSecret,
        ]);

        $request->getBody()->write(http_build_query($claims));

        return $request;
    }
}
