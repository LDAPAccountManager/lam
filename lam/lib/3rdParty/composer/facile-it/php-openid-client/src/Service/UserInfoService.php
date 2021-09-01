<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service;

use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Facile\OpenIDClient\Exception\OAuth2Exception;
use Facile\OpenIDClient\Exception\RuntimeException;
use Facile\OpenIDClient\Token\TokenSetInterface;
use Facile\OpenIDClient\Token\TokenVerifierBuilderInterface;
use function http_build_query;
use function is_array;
use function json_decode;
use Psr\Http\Client\ClientExceptionInterface;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use function sprintf;

/**
 * @psalm-import-type TokenSetClaimsType from TokenSetInterface
 */
final class UserInfoService
{
    /** @var ClientInterface */
    private $client;

    /** @var RequestFactoryInterface */
    private $requestFactory;

    /** @var TokenVerifierBuilderInterface */
    private $userInfoVerifierBuilder;

    public function __construct(
        TokenVerifierBuilderInterface $userInfoVerifierBuilder,
        ClientInterface $client,
        RequestFactoryInterface $requestFactory
    ) {
        $this->userInfoVerifierBuilder = $userInfoVerifierBuilder;
        $this->client = $client;
        $this->requestFactory = $requestFactory;
    }

    /**
     * @return array<string, mixed>
     */
    public function getUserInfo(OpenIDClient $client, TokenSetInterface $tokenSet, bool $useBody = false): array
    {
        $accessToken = $tokenSet->getAccessToken();

        if (null === $accessToken) {
            throw new RuntimeException('Unable to get an access token from the token set');
        }

        $clientMetadata = $client->getMetadata();
        $issuerMetadata = $client->getIssuer()->getMetadata();

        $mTLS = true === $clientMetadata->get('tls_client_certificate_bound_access_tokens');

        $endpointUri = $issuerMetadata->getUserinfoEndpoint();

        if ($mTLS) {
            $endpointUri = $issuerMetadata->getMtlsEndpointAliases()['userinfo_endpoint'] ?? $endpointUri;
        }

        if (null === $endpointUri) {
            throw new InvalidArgumentException('Invalid issuer userinfo endpoint');
        }

        $expectJwt = null !== $clientMetadata->getUserinfoSignedResponseAlg()
            || null !== $clientMetadata->getUserinfoEncryptedResponseAlg()
            || null !== $clientMetadata->getUserinfoEncryptedResponseEnc();

        if ($useBody) {
            $request = $this->requestFactory->createRequest('POST', $endpointUri)
                ->withHeader('accept', $expectJwt ? 'application/jwt' : 'application/json')
                ->withHeader('content-type', 'application/x-www-form-urlencoded');
            $request->getBody()->write(http_build_query(['access_token' => $accessToken]));
        } else {
            $request = $this->requestFactory->createRequest('GET', $endpointUri)
                ->withHeader('accept', $expectJwt ? 'application/jwt' : 'application/json')
                ->withHeader('authorization', ($tokenSet->getTokenType() ?: 'Bearer') . ' ' . $accessToken);
        }

        $httpClient = $client->getHttpClient() ?? $this->client;

        try {
            $response = $httpClient->sendRequest($request);
        } catch (ClientExceptionInterface $e) {
            throw new RuntimeException('Unable to get userinfo', 0, $e);
        }

        if (200 !== $response->getStatusCode()) {
            throw OAuth2Exception::fromResponse($response);
        }

        if ($expectJwt) {
            /** @var TokenSetClaimsType $payload */
            $payload = $this->userInfoVerifierBuilder->build($client)
                ->verify((string) $response->getBody());
        } else {
            /** @var false|TokenSetClaimsType $payload */
            $payload = json_decode((string) $response->getBody(), true);
        }

        if (! is_array($payload)) {
            throw new RuntimeException('Unable to parse userinfo claims');
        }

        $idToken = $tokenSet->getIdToken();

        if (null === $idToken) {
            return $payload;
        }

        // check expected sub
        $expectedSub = $tokenSet->claims()['sub'] ?? null;

        if (null === $expectedSub) {
            throw new RuntimeException('Unable to get sub claim from id_token');
        }

        if ($expectedSub !== ($payload['sub'] ?? null)) {
            throw new RuntimeException(
                sprintf('Userinfo sub mismatch, expected %s, got: %s', $expectedSub, $payload['sub'] ?? '')
            );
        }

        return $payload;
    }
}
