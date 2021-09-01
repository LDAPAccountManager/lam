<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service;

use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\RuntimeException;
use function Facile\OpenIDClient\get_endpoint_uri;
use function Facile\OpenIDClient\parse_metadata_response;
use Psr\Http\Client\ClientExceptionInterface;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;

/**
 * RFC 7662 Token Introspection
 *
 * @link https://tools.ietf.org/html/rfc7662 RFC 7662
 */
final class IntrospectionService
{
    /** @var ClientInterface */
    private $client;

    /** @var RequestFactoryInterface */
    private $requestFactory;

    public function __construct(
        ClientInterface $client,
        RequestFactoryInterface $requestFactory
    ) {
        $this->client = $client;
        $this->requestFactory = $requestFactory;
    }

    /**
     * @param array<string, mixed> $params
     *
     * @return array<string, mixed>
     */
    public function introspect(OpenIDClient $client, string $token, array $params = []): array
    {
        $endpointUri = get_endpoint_uri($client, 'introspection_endpoint');

        $authMethod = $client->getAuthMethodFactory()
            ->create($client->getMetadata()->getIntrospectionEndpointAuthMethod());

        $tokenRequest = $this->requestFactory->createRequest('POST', $endpointUri)
            ->withHeader('content-type', 'application/x-www-form-urlencoded');

        $params += [
            'token' => $token,
            'aud' => $client->getIssuer()->getMetadata()->getIntrospectionEndpoint(),
        ];
        $tokenRequest = $authMethod->createRequest($tokenRequest, $client, $params);

        $httpClient = $client->getHttpClient() ?? $this->client;

        try {
            $response = $httpClient->sendRequest($tokenRequest);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to get introspection response', 0, $e);
        }

        return parse_metadata_response($response, 200);
    }
}
