<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service;

use function array_filter;
use function array_key_exists;
use function array_merge;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Facile\OpenIDClient\Exception\OAuth2Exception;
use Facile\OpenIDClient\Exception\RuntimeException;
use function Facile\OpenIDClient\get_endpoint_uri;
use function Facile\OpenIDClient\parse_callback_params;
use function Facile\OpenIDClient\parse_metadata_response;
use Facile\OpenIDClient\Session\AuthSessionInterface;
use Facile\OpenIDClient\Token\IdTokenVerifierBuilderInterface;
use Facile\OpenIDClient\Token\TokenSetFactoryInterface;
use Facile\OpenIDClient\Token\TokenSetInterface;
use Facile\OpenIDClient\Token\TokenVerifierBuilderInterface;
use function http_build_query;
use function is_array;
use function is_string;
use function json_encode;
use JsonSerializable;
use Psr\Http\Client\ClientExceptionInterface;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use Psr\Http\Message\ServerRequestInterface;

/**
 * OAuth 2.0
 *
 * @link https://tools.ietf.org/html/rfc6749 RFC 6749
 *
 * @psalm-import-type TokenSetAttributesType from TokenSetInterface
 * @psalm-import-type TokenSetClaimsType from TokenSetInterface
 * @psalm-import-type OAuth2ErrorType from OAuth2Exception
 *
 * @psalm-type AuthorizationResponseObjectType = array{}&array{
 *     response: string,
 * }
 * @psalm-type AuthorizationResponseType = array{}&array{
 *     code?: string,
 *     access_token?: string,
 *     token_type?: string,
 *     expires_in?: int,
 *     scope?: string,
 *     state?: string,
 *     code_verifier?: string,
 *     redirect_uri?: string
 * }
 * @psalm-type CallbackParamsType = array<string, mixed>
 */
final class AuthorizationService
{
    /** @var TokenSetFactoryInterface */
    private $tokenSetFactory;

    /** @var ClientInterface */
    private $client;

    /** @var RequestFactoryInterface */
    private $requestFactory;

    /** @var IdTokenVerifierBuilderInterface */
    private $idTokenVerifierBuilder;

    /** @var TokenVerifierBuilderInterface */
    private $responseVerifierBuilder;

    public function __construct(
        TokenSetFactoryInterface $tokenSetFactory,
        ClientInterface $client,
        RequestFactoryInterface $requestFactory,
        IdTokenVerifierBuilderInterface $idTokenVerifierBuilder,
        TokenVerifierBuilderInterface $responseVerifierBuilder
    ) {
        $this->tokenSetFactory = $tokenSetFactory;
        $this->client = $client;
        $this->requestFactory = $requestFactory;
        $this->idTokenVerifierBuilder = $idTokenVerifierBuilder;
        $this->responseVerifierBuilder = $responseVerifierBuilder;
    }

    /**
     * @param array<string, mixed> $params
     *
     * @psalm-param array<string, mixed> $params
     */
    public function getAuthorizationUri(OpenIDClient $client, array $params = []): string
    {
        $clientMetadata = $client->getMetadata();
        $issuerMetadata = $client->getIssuer()->getMetadata();
        $endpointUri = $issuerMetadata->getAuthorizationEndpoint();

        $params = array_merge([
            'client_id' => $clientMetadata->getClientId(),
            'scope' => 'openid',
            'response_type' => $clientMetadata->getResponseTypes()[0] ?? 'code',
            'redirect_uri' => $clientMetadata->getRedirectUris()[0] ?? null,
        ], $params);

        $params = array_filter($params, static fn ($value): bool => null !== $value);

        /**
         * @var string $key
         * @var mixed $value
         */
        foreach ($params as $key => $value) {
            if (null === $value) {
                unset($params[$key]);
            } elseif ('claims' === $key && (is_array($value) || $value instanceof JsonSerializable)) {
                $params['claims'] = json_encode($value, JSON_THROW_ON_ERROR);
            } elseif (! is_string($value)) {
                $params[$key] = (string) $value;
            }
        }

        if (! array_key_exists('nonce', $params) && 'code' !== ($params['response_type'] ?? '')) {
            throw new InvalidArgumentException('nonce MUST be provided for implicit and hybrid flows');
        }

        // Support Authorization Endpoint uris with query params
        $splitUrl = explode('?', $endpointUri, 2);
        if (isset($splitUrl[1])) {
            parse_str($splitUrl[1], $existingParameters);
            $params = array_merge($existingParameters, $params);
        }

        return $splitUrl[0] . '?' . http_build_query($params);
    }

    /**
     * @throws OAuth2Exception
     *
     * @return array<string, mixed>
     *
     * @psalm-return TokenSetAttributesType
     */
    public function getCallbackParams(ServerRequestInterface $serverRequest, OpenIDClient $client): array
    {
        return $this->processResponseParams($client, parse_callback_params($serverRequest));
    }

    /**
     * @param array<string, mixed> $params
     *
     * @psalm-param TokenSetAttributesType $params
     */
    public function callback(
        OpenIDClient $client,
        array $params,
        ?string $redirectUri = null,
        ?AuthSessionInterface $authSession = null,
        ?int $maxAge = null
    ): TokenSetInterface {
        $allowedParams = ['code', 'state', 'token_type', 'access_token', 'id_token', 'refresh_token', 'expires_in', 'code_verifier'];
        /** @psalm-var AuthorizationResponseType $safeParams */
        $safeParams = array_intersect_key(
            $params,
            array_fill_keys($allowedParams, true)
        );
        $tokenSet = $this->tokenSetFactory->fromArray($safeParams);

        $idToken = $tokenSet->getIdToken();

        if (null !== $idToken) {
            /** @psalm-var TokenSetClaimsType $claims */
            $claims = $this->idTokenVerifierBuilder->build($client)
                ->withNonce(null !== $authSession ? $authSession->getNonce() : null)
                ->withState(null !== $authSession ? $authSession->getState() : null)
                ->withCode($tokenSet->getCode())
                ->withMaxAge($maxAge)
                ->withAccessToken($tokenSet->getAccessToken())
                ->verify($idToken);
            $tokenSet = $tokenSet->withClaims($claims);
        }

        if (null === $tokenSet->getCode()) {
            return $tokenSet;
        }

        // get token
        return $this->fetchToken($client, $tokenSet, $redirectUri, $authSession, $maxAge);
    }

    /**
     * @throws OAuth2Exception
     */
    public function fetchToken(
        OpenIDClient $client,
        TokenSetInterface $tokenSet,
        ?string $redirectUri = null,
        ?AuthSessionInterface $authSession = null,
        ?int $maxAge = null
    ): TokenSetInterface {
        $code = $tokenSet->getCode();

        if (null === $code) {
            throw new RuntimeException('Unable to fetch token without a code');
        }

        if (null === $redirectUri) {
            $redirectUri = $client->getMetadata()->getRedirectUris()[0] ?? null;
        }

        if (null === $redirectUri) {
            throw new InvalidArgumentException('A redirect_uri should be provided');
        }

        $params = [
            'grant_type' => 'authorization_code',
            'code' => $code,
            'redirect_uri' => $redirectUri,
        ];

        if (null !== $authSession && null !== $authSession->getCodeVerifier()) {
            $params['code_verifier'] = $authSession->getCodeVerifier();
        }

        $tokenSet = $this->grant($client, $params);

        $idToken = $tokenSet->getIdToken();

        if (null !== $idToken) {
            /** @psalm-var TokenSetClaimsType $claims */
            $claims = $this->idTokenVerifierBuilder->build($client)
                ->withNonce(null !== $authSession ? $authSession->getNonce() : null)
                ->withState(null !== $authSession ? $authSession->getState() : null)
                ->withMaxAge($maxAge)
                ->verify($idToken);
            $tokenSet = $tokenSet->withClaims($claims);
        }

        return $tokenSet;
    }

    /**
     * @param array<string, mixed> $params
     */
    public function refresh(OpenIDClient $client, string $refreshToken, array $params = []): TokenSetInterface
    {
        $tokenSet = $this->grant($client, array_merge($params, [
            'grant_type' => 'refresh_token',
            'refresh_token' => $refreshToken,
        ]));

        $idToken = $tokenSet->getIdToken();

        if (null === $idToken) {
            return $tokenSet;
        }

        $idToken = $tokenSet->getIdToken();

        if (null !== $idToken) {
            /** @psalm-var TokenSetClaimsType $claims */
            $claims = $this->idTokenVerifierBuilder->build($client)
                ->withAccessToken($tokenSet->getAccessToken())
                ->verify($idToken);
            $tokenSet = $tokenSet->withClaims($claims);
        }

        return $tokenSet;
    }

    /**
     * @param array<string, mixed> $params
     *
     * @throws OAuth2Exception
     */
    public function grant(OpenIDClient $client, array $params = []): TokenSetInterface
    {
        $authMethod = $client->getAuthMethodFactory()
            ->create($client->getMetadata()->getTokenEndpointAuthMethod());

        $endpointUri = get_endpoint_uri($client, 'token_endpoint');

        $tokenRequest = $this->requestFactory->createRequest('POST', $endpointUri)
            ->withHeader('content-type', 'application/x-www-form-urlencoded');

        $tokenRequest = $authMethod->createRequest($tokenRequest, $client, $params);

        $httpClient = $client->getHttpClient() ?? $this->client;

        try {
            $response = $httpClient->sendRequest($tokenRequest);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to get token response', 0, $e);
        }

        $data = parse_metadata_response($response);
        $params = $this->processResponseParams($client, $data);

        return $this->tokenSetFactory->fromArray($params);
    }

    /**
     * @param array<string, mixed> $params
     *
     * @psalm-param array<string, mixed> $params
     *
     * @psalm-assert-if-true AuthorizationResponseObjectType $params
     */
    private function isAuthorizationResponseObject(array $params): bool
    {
        return array_key_exists('response', $params);
    }

    /**
     * @param array<string, mixed> $params
     *
     * @throws OAuth2Exception
     *
     * @return array<string, mixed>
     *
     * @psalm-param array<string, mixed> $params
     *
     * @psalm-return TokenSetAttributesType
     */
    private function processResponseParams(OpenIDClient $client, array $params): array
    {
        if ($this->isAuthorizationResponseObject($params)) {
            $params = $this->responseVerifierBuilder->build($client)
                ->verify($params['response']);
        }

        if (OAuth2Exception::isOAuth2Error($params)) {
            throw OAuth2Exception::fromParameters($params);
        }

        /** @psalm-var TokenSetAttributesType $params */
        return $params;
    }
}
