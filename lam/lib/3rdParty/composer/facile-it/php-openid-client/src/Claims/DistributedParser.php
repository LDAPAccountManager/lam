<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\RuntimeException;
use Facile\OpenIDClient\Issuer\IssuerBuilderInterface;
use Http\Discovery\Psr17FactoryDiscovery;
use Http\Discovery\Psr18ClientDiscovery;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Signature\JWSVerifier;
use Jose\Component\Signature\Serializer\JWSSerializer;
use Psr\Http\Client\ClientInterface;
use Psr\Http\Message\RequestFactoryInterface;
use Psr\Http\Message\ResponseInterface;
use Throwable;
use Override;

use function array_filter;
use function array_key_exists;
use function Facile\OpenIDClient\check_server_response;

/**
 * @psalm-api
 */
final class DistributedParser extends AbstractClaims implements DistributedParserInterface
{
    private ClientInterface $client;

    private RequestFactoryInterface $requestFactory;

    public function __construct(
        ?IssuerBuilderInterface $issuerBuilder = null,
        ?ClientInterface $client = null,
        ?RequestFactoryInterface $requestFactory = null,
        ?AlgorithmManager $algorithmManager = null,
        ?JWSVerifier $JWSVerifier = null,
        ?JWSSerializer $serializer = null
    ) {
        parent::__construct($issuerBuilder, $algorithmManager, $JWSVerifier, $serializer);

        $this->client = $client ?? Psr18ClientDiscovery::find();
        $this->requestFactory = $requestFactory ?? Psr17FactoryDiscovery::findRequestFactory();
    }

    #[Override]
    public function fetch(OpenIDClient $client, array $claims, array $accessTokens = []): array
    {
        if (! array_key_exists('_claim_sources', $claims)) {
            return $claims;
        }

        if (! array_key_exists('_claim_names', $claims)) {
            return $claims;
        }

        $distributedSources = array_filter($claims['_claim_sources'], fn($value): bool => $this->isDistributedSource($value));

        /** @var array<string, ResponseInterface> $responses */
        $responses = [];
        foreach ($distributedSources as $sourceName => $source) {
            $request = $this->requestFactory->createRequest('GET', $source['endpoint'])
                ->withHeader('accept', 'application/jwt');

            $accessToken = $source['access_token'] ?? ($accessTokens[$sourceName] ?? null);
            if ($accessToken !== null) {
                $request = $request->withHeader('authorization', 'Bearer ' . $accessToken);
            }

            try {
                $responses[$sourceName] = $this->client->sendRequest($request);
            } catch (Throwable $e) {
                throw new RuntimeException("Unable to fetch distributed claim for \"{$sourceName}\"", 0, $e);
            }
        }

        $claimPayloads = [];
        foreach ($responses as $sourceName => $response) {
            try {
                check_server_response($response);
                $claimPayloads[$sourceName] = $this->claimJWT($client, (string) $response->getBody());
                unset($claims['_claim_sources'][$sourceName]);
            } catch (Throwable $e) {
                throw new RuntimeException("Unable to fetch distributed claim for \"{$sourceName}\"", 0, $e);
            }
        }

        return $this->cleanClaims($this->assignClaims($claims, $claims['_claim_names'], $claimPayloads));
    }
}
