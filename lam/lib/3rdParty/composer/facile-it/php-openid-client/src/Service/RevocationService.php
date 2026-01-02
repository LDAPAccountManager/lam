<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service;

use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\RuntimeException;
use Psr\Http\Client\ClientExceptionInterface;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;

use function Facile\OpenIDClient\check_server_response;
use function Facile\OpenIDClient\get_endpoint_uri;

/**
 * RFC 7009 Token Revocation.
 *
 * @see https://tools.ietf.org/html/rfc7009 RFC 7009
 *
 * @psalm-api
 */
final class RevocationService
{
    private ClientInterface $client;

    private RequestFactoryInterface $requestFactory;

    public function __construct(
        ClientInterface $client,
        RequestFactoryInterface $requestFactory
    ) {
        $this->client = $client;
        $this->requestFactory = $requestFactory;
    }

    /**
     * @param array<string, mixed> $params
     */
    public function revoke(OpenIDClient $client, string $token, array $params = []): void
    {
        $endpointUri = get_endpoint_uri($client, 'revocation_endpoint');

        $authMethod = $client->getAuthMethodFactory()
            ->create($client->getMetadata()->getRevocationEndpointAuthMethod());

        $tokenRequest = $this->requestFactory->createRequest('POST', $endpointUri)
            ->withHeader('content-type', 'application/x-www-form-urlencoded');

        $params['token'] = $token;
        $tokenRequest = $authMethod->createRequest($tokenRequest, $client, $params);

        $httpClient = $client->getHttpClient() ?? $this->client;

        try {
            $response = $httpClient->sendRequest($tokenRequest);
            check_server_response($response, 200);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to get revocation response', 0, $e);
        }
    }
}
