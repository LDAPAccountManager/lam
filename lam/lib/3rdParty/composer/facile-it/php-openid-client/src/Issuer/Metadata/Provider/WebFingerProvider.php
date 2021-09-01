<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer\Metadata\Provider;

use function array_key_exists;
use function array_pop;
use function explode;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Facile\OpenIDClient\Exception\RuntimeException;
use function Facile\OpenIDClient\parse_metadata_response;
use function http_build_query;
use function is_array;
use function is_string;
use function parse_url;
use function preg_match;
use function preg_replace;
use Psr\Http\Client\ClientExceptionInterface;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use Psr\Http\Message\UriFactoryInterface;
use function strpos;
use function substr;

/**
 * @psalm-import-type IssuerMetadataObject from \Facile\JoseVerifier\Psalm\PsalmTypes
 */
final class WebFingerProvider implements RemoteProviderInterface, WebFingerProviderInterface
{
    private const OIDC_DISCOVERY = '/.well-known/openid-configuration';

    private const WEBFINGER = '/.well-known/webfinger';

    private const REL = 'http://openid.net/specs/connect/1.0/issuer';

    private const AAD_MULTITENANT_DISCOVERY = 'https://login.microsoftonline.com/common/v2.0$' . self::OIDC_DISCOVERY;

    /** @var ClientInterface */
    private $client;

    /** @var RequestFactoryInterface */
    private $requestFactory;

    /** @var UriFactoryInterface */
    private $uriFactory;

    /** @var DiscoveryProviderInterface */
    private $discoveryProvider;

    public function __construct(
        ClientInterface $client,
        RequestFactoryInterface $requestFactory,
        UriFactoryInterface $uriFactory,
        DiscoveryProviderInterface $discoveryProvider
    ) {
        $this->client = $client;
        $this->requestFactory = $requestFactory;
        $this->uriFactory = $uriFactory;
        $this->discoveryProvider = $discoveryProvider;
    }

    public function isAllowedUri(string $uri): bool
    {
        return true;
    }

    public function fetch(string $resource): array
    {
        $resource = $this->normalizeWebfinger($resource);
        $parsedUrl = parse_url(
            false !== strpos($resource, '@')
                ? 'https://' . explode('@', $resource)[1]
                : $resource
        );

        if (! is_array($parsedUrl) || ! array_key_exists('host', $parsedUrl)) {
            throw new RuntimeException('Unable to parse resource');
        }

        $host = $parsedUrl['host'];

        /** @var string|int|null $port */
        $port = $parsedUrl['port'] ?? null;

        if (((int) $port) > 0) {
            $host .= ':' . ((int) $port);
        }

        $webFingerUrl = $this->uriFactory->createUri('https://' . $host . self::WEBFINGER)
            ->withQuery(http_build_query(['resource' => $resource, 'rel' => self::REL]));

        $request = $this->requestFactory->createRequest('GET', $webFingerUrl)
            ->withHeader('accept', 'application/json');

        try {
            $data = parse_metadata_response($this->client->sendRequest($request));
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to fetch provider metadata', 0, $e);
        }

        /** @var array<array-key, null|array{rel?: string, href?: string}> $links */
        $links = $data['links'] ?? [];
        $href = null;
        foreach ($links as $link) {
            if (! is_array($link)) {
                continue;
            }

            if (($link['rel'] ?? null) !== self::REL) {
                continue;
            }

            if (! array_key_exists('href', $link)) {
                continue;
            }

            $href = $link['href'];
        }

        if (! is_string($href) || 0 !== strpos($href, 'https://')) {
            throw new InvalidArgumentException('Invalid issuer location');
        }

        $metadata = $this->discoveryProvider->discovery($href);

        if (($metadata['issuer'] ?? null) !== $href) {
            throw new RuntimeException('Discovered issuer mismatch');
        }

        /** @var IssuerMetadataObject $metadata */
        return $metadata;
    }

    private function normalizeWebfinger(string $input): string
    {
        $hasScheme = static function (string $resource): bool {
            if (false !== strpos($resource, '://')) {
                return true;
            }

            $authority = explode('#', (string) preg_replace('/(\/|\?)/', '#', $resource))[0];

            if (false === ($index = strpos($authority, ':'))) {
                return false;
            }

            $hostOrPort = substr($resource, $index + 1);

            return ! (bool) preg_match('/^\d+$/', $hostOrPort);
        };

        $acctSchemeAssumed = static function (string $input): bool {
            if (false === strpos($input, '@')) {
                return false;
            }

            $parts = explode('@', $input);
            $host = array_pop($parts);

            return ! (bool) preg_match('/[:\/?]+/', $host);
        };

        if ($hasScheme($input)) {
            $output = $input;
        } elseif ($acctSchemeAssumed($input)) {
            $output = 'acct:' . $input;
        } else {
            $output = 'https://' . $input;
        }

        return explode('#', $output)[0];
    }
}
