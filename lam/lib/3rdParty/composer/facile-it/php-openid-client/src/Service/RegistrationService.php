<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service;

use function array_diff_key;
use function array_flip;
use function array_intersect_key;
use function array_key_exists;
use function array_merge;
use function Facile\OpenIDClient\check_server_response;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Facile\OpenIDClient\Exception\RuntimeException;
use Facile\OpenIDClient\Issuer\IssuerInterface;
use function Facile\OpenIDClient\parse_metadata_response;
use function json_encode;
use Psr\Http\Client\ClientExceptionInterface;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;

/**
 * Dynamic Client Registration Protocol
 *
 * @link https://tools.ietf.org/html/rfc7591
 * @link https://openid.net/specs/openid-connect-registration-1_0.html
 */
final class RegistrationService
{
    /** @var ClientInterface */
    private $client;

    /** @var RequestFactoryInterface */
    private $requestFactory;

    /** @var string[] */
    private static $registrationClaims = [
        'registration_access_token',
        'registration_client_uri',
        'client_secret_expires_at',
        'client_id_issued_at',
    ];

    public function __construct(
        ClientInterface $client,
        RequestFactoryInterface $requestFactory
    ) {
        $this->client = $client;
        $this->requestFactory = $requestFactory;
    }

    /**
     * @param array<string, mixed> $metadata
     *
     * @return array<string, mixed>
     */
    public function register(
        IssuerInterface $issuer,
        array $metadata,
        ?string $initialToken = null
    ): array {
        $registrationEndpoint = $issuer->getMetadata()->getRegistrationEndpoint();

        if (null === $registrationEndpoint) {
            throw new InvalidArgumentException('Issuer does not support dynamic client registration');
        }

        $encodedMetadata = json_encode($metadata);

        if (false === $encodedMetadata) {
            throw new RuntimeException('Unable to encode client metadata');
        }

        $request = $this->requestFactory->createRequest('POST', $registrationEndpoint)
            ->withHeader('content-type', 'application/json')
            ->withHeader('accept', 'application/json');

        $request->getBody()->write($encodedMetadata);

        if (null !== $initialToken) {
            $request = $request->withHeader('authorization', 'Bearer ' . $initialToken);
        }

        try {
            $response = $this->client->sendRequest($request);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to register OpenID client', 0, $e);
        }

        $data = parse_metadata_response($response, 201);

        if (! array_key_exists('client_id', $data)) {
            throw new RuntimeException('Registration response did not return a client_id field');
        }

        return $data;
    }

    /**
     * @return array<string, mixed>
     */
    public function read(string $clientUri, string $accessToken): array
    {
        $request = $this->requestFactory->createRequest('GET', $clientUri)
            ->withHeader('accept', 'application/json')
            ->withHeader('authorization', 'Bearer ' . $accessToken);

        try {
            $response = $this->client->sendRequest($request);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to read OpenID client', 0, $e);
        }

        $claims = parse_metadata_response($response, 200);

        if (! array_key_exists('client_id', $claims)) {
            throw new RuntimeException('Registration response did not return a client_id field');
        }

        return $claims;
    }

    /**
     * @param array<string, mixed> $metadata
     *
     * @return array<string, mixed>
     */
    public function update(
        string $clientUri,
        string $accessToken,
        array $metadata
    ): array {
        $clientRegistrationMetadata = array_intersect_key($metadata, array_flip(self::$registrationClaims));
        $metadata = array_diff_key($metadata, $clientRegistrationMetadata);

        $encodedMetadata = json_encode($metadata);

        if (false === $encodedMetadata) {
            throw new RuntimeException('Unable to encode client metadata');
        }

        $request = $this->requestFactory->createRequest('PUT', $clientUri)
            ->withHeader('accept', 'application/json')
            ->withHeader('content-type', 'application/json')
            ->withHeader('authorization', 'Bearer ' . $accessToken);

        $request->getBody()->write($encodedMetadata);

        try {
            $response = $this->client->sendRequest($request);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to update OpenID client', 0, $e);
        }

        $data = parse_metadata_response($response, 200);

        if (! array_key_exists('client_id', $data)) {
            throw new RuntimeException('Registration response did not return a client_id field');
        }

        /** @var array<string, mixed> $merged */
        $merged = array_merge($clientRegistrationMetadata, $data);

        return $merged;
    }

    public function delete(string $clientUri, string $accessToken): void
    {
        $request = $this->requestFactory->createRequest('DELETE', $clientUri)
            ->withHeader('authorization', 'Bearer ' . $accessToken);

        try {
            $response = $this->client->sendRequest($request);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to delete OpenID client', 0, $e);
        }

        check_server_response($response, 204);
    }
}
