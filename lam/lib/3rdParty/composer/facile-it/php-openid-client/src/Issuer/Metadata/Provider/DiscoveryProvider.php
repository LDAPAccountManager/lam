<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use Facile\JoseVerifier\TokenVerifierInterface;
use Facile\OpenIDClient\Exception\RuntimeException;
use Psr\Http\Client\ClientExceptionInterface;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use Psr\Http\Message\UriFactoryInterface;
use Override;

use function array_key_exists;
use function Facile\OpenIDClient\parse_metadata_response;
use function preg_match;
use function rtrim;

/**
 * @psalm-import-type IssuerRemoteMetadataType from TokenVerifierInterface
 */
final class DiscoveryProvider implements DiscoveryProviderInterface
{
    private const OIDC_DISCOVERY = '/.well-known/openid-configuration';

    private const OAUTH2_DISCOVERY = '/.well-known/oauth-authorization-server';

    private ClientInterface $client;

    private RequestFactoryInterface $requestFactory;

    private UriFactoryInterface $uriFactory;

    public function __construct(
        ClientInterface $client,
        RequestFactoryInterface $requestFactory,
        UriFactoryInterface $uriFactory
    ) {
        $this->client = $client;
        $this->requestFactory = $requestFactory;
        $this->uriFactory = $uriFactory;
    }

    #[Override]
    public function isAllowedUri(string $uri): bool
    {
        return (int) preg_match('/https?:\/\//', $uri) > 0;
    }

    /**
     * @return array<string, mixed>
     *
     * @psalm-return IssuerRemoteMetadataType
     *
     * @psalm-suppress MixedReturnTypeCoercion
     */
    #[Override]
    public function discovery(string $url): array
    {
        $uri = $this->uriFactory->createUri($url);
        $uriPath = $uri->getPath() ?: '/';

        if (str_contains($uriPath, '/.well-known/')) {
            return $this->fetchOpenIdConfiguration((string) $uri);
        }

        $uris = [
            $uri->withPath(rtrim($uriPath, '/') . self::OIDC_DISCOVERY),
            $uri->withPath('/' === $uriPath
                ? self::OAUTH2_DISCOVERY
                : rtrim($uriPath, '/') . self::OAUTH2_DISCOVERY),
        ];

        foreach ($uris as $wellKnownUri) {
            try {
                return $this->fetchOpenIdConfiguration((string) $wellKnownUri);
            } catch (RuntimeException $e) {
            }
        }

        throw new RuntimeException('Unable to fetch provider metadata');
    }

    /**
     * @return array<mixed, string>
     *
     * @psalm-return IssuerRemoteMetadataType
     */
    private function fetchOpenIdConfiguration(string $uri): array
    {
        $request = $this->requestFactory->createRequest('GET', $uri)
            ->withHeader('accept', 'application/json');

        try {
            /** @psalm-var IssuerRemoteMetadataType $data */
            $data = parse_metadata_response($this->client->sendRequest($request));
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to fetch provider metadata', 0, $e);
        }

        if (! array_key_exists('issuer', $data)) {
            throw new RuntimeException('Invalid metadata content, no "issuer" key found');
        }

        return $data;
    }

    #[Override]
    public function fetch(string $uri): array
    {
        return $this->discovery($uri);
    }
}
