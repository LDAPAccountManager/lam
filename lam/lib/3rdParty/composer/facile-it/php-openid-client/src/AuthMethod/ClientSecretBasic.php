<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Psr\Http\Message\RequestInterface;
use Override;

use function base64_encode;
use function http_build_query;
use function urlencode;

final class ClientSecretBasic implements AuthMethodInterface
{
    #[Override]
    public function getSupportedMethod(): string
    {
        return 'client_secret_basic';
    }

    #[Override]
    public function createRequest(
        RequestInterface $request,
        OpenIDClient $client,
        array $claims
    ): RequestInterface {
        $clientId = $client->getMetadata()->getClientId();
        $clientSecret = $client->getMetadata()->getClientSecret();

        if (null === $clientSecret) {
            throw new InvalidArgumentException($this->getSupportedMethod() . ' cannot be used without client_secret metadata');
        }

        $request = $request->withHeader(
            'Authorization',
            'Basic ' . base64_encode(urlencode($clientId) . ':' . urlencode($clientSecret))
        );

        $request->getBody()->write(http_build_query($claims));

        return $request;
    }
}
