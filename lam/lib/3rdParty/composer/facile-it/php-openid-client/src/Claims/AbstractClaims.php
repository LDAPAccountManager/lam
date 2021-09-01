<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use function array_diff_key;
use function array_flip;
use function array_key_exists;
use function count;
use function explode;
use Facile\OpenIDClient\AlgorithmManagerBuilder;
use function Facile\OpenIDClient\base64url_decode;
use Facile\OpenIDClient\Client\ClientInterface as OpenIDClient;
use Facile\OpenIDClient\Exception\InvalidArgumentException;
use Facile\OpenIDClient\Exception\RuntimeException;
use Facile\OpenIDClient\Issuer\IssuerBuilder;
use Facile\OpenIDClient\Issuer\IssuerBuilderInterface;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Core\JWKSet;
use Jose\Component\Signature\JWSVerifier;
use Jose\Component\Signature\Serializer\CompactSerializer;
use Jose\Component\Signature\Serializer\JWSSerializer;
use function json_decode;
use function sprintf;

/**
 * @psalm-import-type TokenSetClaimsType from \Facile\OpenIDClient\Token\TokenSetInterface
 */
abstract class AbstractClaims
{
    /** @var IssuerBuilderInterface */
    protected $issuerBuilder;

    /** @var AlgorithmManager */
    protected $algorithmManager;

    /** @var JWSVerifier */
    protected $JWSVerifier;

    /** @var JWSSerializer */
    protected $serializer;

    public function __construct(
        ?IssuerBuilderInterface $issuerBuilder = null,
        ?AlgorithmManager $algorithmManager = null,
        ?JWSVerifier $JWSVerifier = null,
        ?JWSSerializer $serializer = null
    ) {
        $this->issuerBuilder = $issuerBuilder ?? new IssuerBuilder();
        $this->algorithmManager = $algorithmManager ?? (new AlgorithmManagerBuilder())->build();
        $this->JWSVerifier = $JWSVerifier ?? new JWSVerifier($this->algorithmManager);
        $this->serializer = $serializer ?? new CompactSerializer();
    }

    /**
     * @return array<string, mixed>
     */
    protected function claimJWT(OpenIDClient $client, string $jwt): array
    {
        $issuer = $client->getIssuer();

        /** @var null|array<string, mixed> $header */
        $header = json_decode(base64url_decode(explode('.', $jwt)[0] ?? '{}'), true);
        /** @var array<string, mixed> $payload */
        $payload = json_decode(base64url_decode(explode('.', $jwt)[1] ?? '{}'), true);

        /** @var null|string $alg */
        $alg = $header['alg'] ?? null;
        /** @var null|string $kid */
        $kid = $header['kid'] ?? null;

        if (null === $alg) {
            throw new InvalidArgumentException('Claim source is missing JWT header alg property');
        }

        if ('none' === $alg) {
            return $payload;
        }

        /** @var null|string $iss */
        $iss = $payload['iss'] ?? null;

        if (null === $iss || $iss === $issuer->getMetadata()->getIssuer()) {
            $jwks = JWKSet::createFromKeyData($issuer->getJwksProvider()->getJwks());
        } else {
            $discovered = $this->issuerBuilder->build($iss);
            $jwks = JWKSet::createFromKeyData($discovered->getJwksProvider()->getJwks());
        }

        $jws = $this->serializer->unserialize($jwt);

        $jwk = $jwks->selectKey('sig', $this->algorithmManager->get($alg), null !== $kid ? ['kid' => $kid] : []);

        if (null === $jwk) {
            throw new RuntimeException('Unable to get a key to verify claim source JWT');
        }

        if (false === $this->JWSVerifier->verifyWithKey($jws, $jwk, 0)) {
            throw new InvalidArgumentException('Invalid claim source JWT signature');
        }

        return $payload;
    }

    /**
     * @param array<string, mixed> $claims
     * @param array<string, string> $sourceNames
     * @param array<string, array<string, mixed>> $sources
     *
     * @return array<string, mixed>
     *
     * @psalm-param TokenSetClaimsType $claims
     * @psalm-return TokenSetClaimsType
     */
    protected function assignClaims(array $claims, array $sourceNames, array $sources): array
    {
        foreach ($sourceNames as $claim => $inSource) {
            if (! array_key_exists($inSource, $sources)) {
                continue;
            }

            if (! array_key_exists($claim, $sources[$inSource])) {
                throw new RuntimeException(sprintf('Unable to find claim "%s" in source "%s"', $claim, $inSource));
            }

            /** @var scalar $value */
            $value = $sources[$inSource][$claim];
            $claims[$claim] = $value;
            /** @var TokenSetClaimsType $claims */
            $claims['_claim_names'] = array_diff_key($claims['_claim_names'] ?? [], array_flip([$claim]));
        }

        return $claims;
    }

    /**
     * @param array<string, mixed> $claims
     *
     * @return array<string, mixed>
     *
     * @psalm-param TokenSetClaimsType $claims
     * @psalm-return TokenSetClaimsType
     */
    protected function cleanClaims(array $claims): array
    {
        if (array_key_exists('_claim_names', $claims) && 0 === count($claims['_claim_names'] ?? [])) {
            /** @var TokenSetClaimsType $claims */
            $claims = array_diff_key($claims, array_flip(['_claim_names']));
        }

        if (array_key_exists('_claim_sources', $claims) && 0 === count($claims['_claim_sources'] ?? [])) {
            /** @var TokenSetClaimsType $claims */
            $claims = array_diff_key($claims, array_flip(['_claim_sources']));
        }

        return $claims;
    }
}
