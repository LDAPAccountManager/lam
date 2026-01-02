<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Psr\Http\Message\RequestInterface;
use Override;

use function array_merge;
use function http_build_query;

final class ClientSecretPost implements AuthMethodInterface
{
    #[Override]
    public function getSupportedMethod(): string
    {
        return 'client_secret_post';
    }

    #[Override]
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
