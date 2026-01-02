<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Internal;

use Facile\JoseVerifier\Exception\InvalidTokenClaimException;
use Facile\JoseVerifier\Exception\InvalidTokenException;
use Jose\Component\Checker;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Core\JWKSet;
use Jose\Component\Core\Util\JsonConverter;
use Jose\Component\Signature\Algorithm;
use Jose\Component\Signature\JWSTokenSupport;
use Jose\Component\Signature\JWSVerifier;
use Jose\Component\Signature\Serializer\CompactSerializer;
use JsonException;
use Throwable;

/**
 * @internal
 *
 * @psalm-api
 */
final class Validate
{
    private string $token;

    private JWKSet $jwkset;

    /** @var Checker\HeaderChecker[] */
    private array $headerCheckers = [];

    /** @var Checker\ClaimChecker[] */
    private array $claimCheckers = [];

    /** @var \Jose\Component\Core\Algorithm[] */
    private array $algorithms = [];

    /** @var string[] */
    private array $mandatoryClaims = [];

    private function __construct(string $token)
    {
        $this->token = $token;
        $this->jwkset = new JWKSet([]);

        foreach ($this->getAlgorithmMap() as $algorithmClass) {
            if (class_exists($algorithmClass)) {
                try {
                    $this->algorithms[] = new $algorithmClass();
                } catch (Throwable $throwable) {
                    // does nothing
                }
            }
        }
    }

    public static function withToken(string $token): static
    {
        return new static($token);
    }

    /**
     * @throws InvalidTokenClaimException When a JWT claim is not valid
     * @throws InvalidTokenException When the JWT is not valid
     *
     * @return array<string, mixed>
     */
    public function run(): array
    {
        $jws = (new CompactSerializer())->unserialize($this->token);
        $headerChecker = new Checker\HeaderCheckerManager($this->headerCheckers, [new JWSTokenSupport()]);
        $headerChecker->check($jws, 0);

        $verifier = new JWSVerifier(new AlgorithmManager($this->algorithms));
        if (! $verifier->verifyWithKeySet($jws, $this->jwkset, 0)) {
            throw new InvalidTokenException('Invalid token signature');
        }

        try {
            /** @var array<string, mixed> $claims */
            $claims = JsonConverter::decode($jws->getPayload() ?? '{}');
        } catch (JsonException $e) {
            throw new InvalidTokenException('Unable to decode JWT payload');
        }

        $claimChecker = new Checker\ClaimCheckerManager($this->claimCheckers);
        try {
            $claimChecker->check($claims, $this->mandatoryClaims);
        } catch (Checker\InvalidHeaderException $e) {
            throw new InvalidTokenException($e->getMessage(), 0, $e);
        } catch (Checker\InvalidClaimException $e) {
            throw new InvalidTokenClaimException($e->getMessage(), $e->getClaim(), $e->getValue(), $e);
        } catch (Checker\MissingMandatoryHeaderParameterException $e) {
            throw new InvalidTokenException($e->getMessage(), 0, $e);
        } catch (Checker\MissingMandatoryClaimException $e) {
            throw new InvalidTokenException($e->getMessage(), 0, $e);
        } catch (Throwable $e) {
            throw new InvalidTokenException('An error occurred validating JWT', 0, $e);
        }

        return $claims;
    }

    /**
     * @return string[]
     *
     * @psalm-return list<class-string<Algorithm\SignatureAlgorithm|Algorithm\MacAlgorithm>>
     */
    private function getAlgorithmMap(): array
    {
        return [
            Algorithm\None::class,
            Algorithm\HS256::class,
            Algorithm\HS384::class,
            Algorithm\HS512::class,
            Algorithm\RS256::class,
            Algorithm\RS384::class,
            Algorithm\RS512::class,
            Algorithm\PS256::class,
            Algorithm\PS384::class,
            Algorithm\PS512::class,
            Algorithm\ES256::class,
            Algorithm\ES384::class,
            Algorithm\ES512::class,
            Algorithm\EdDSA::class,
        ];
    }

    /**
     * @param string[] $mandatoryClaims
     */
    public function withMandatory(array $mandatoryClaims): static
    {
        $clone = clone $this;
        $clone->mandatoryClaims = $mandatoryClaims;

        return $clone;
    }

    public function withClaim(Checker\ClaimChecker $checker): static
    {
        $clone = clone $this;

        $clone->claimCheckers[] = $checker;

        return $clone;
    }

    public function withHeader(Checker\HeaderChecker $checker): static
    {
        $clone = clone $this;

        $clone->headerCheckers[] = $checker;

        return $clone;
    }

    public function withJWKSet(JWKSet $jwkset): static
    {
        $clone = clone $this;
        $clone->jwkset = $jwkset;

        return $clone;
    }
}
