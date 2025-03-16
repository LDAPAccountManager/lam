<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Validate;

use Facile\JoseVerifier\Exception\RuntimeException;
use Jose\Component\Checker;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Core\JWKSet;
use Jose\Component\Core\Util\JsonConverter;
use Jose\Component\Signature\Algorithm;
use Jose\Component\Signature\JWSTokenSupport;
use Jose\Component\Signature\JWSVerifier;
use Jose\Component\Signature\Serializer\CompactSerializer;
use Throwable;

/**
 * @internal
 */
final class Validate
{
    /** @var string */
    protected $token;

    /** @var JWKSet */
    protected $jwkset;

    /** @var Checker\HeaderChecker[] */
    protected $headerCheckers = [];

    /** @var Checker\ClaimChecker[] */
    protected $claimCheckers = [];

    /** @var \Jose\Component\Core\Algorithm[] */
    protected $algorithms = [];

    /** @var string[] */
    protected $mandatoryClaims = [];

    protected function __construct(string $token)
    {
        $this->token = $token;
        $this->jwkset = new JWKSet([]);
        $this->claimCheckers = [];

        foreach ($this->getAlgorithmMap() as $algorithmClass) {
            if (class_exists($algorithmClass)) {
                try {
                    $this->algorithms[] = new $algorithmClass();
                } catch (Throwable $throwable) {
                    //does nothing
                }
            }
        }
    }

    public static function token(string $token): self
    {
        return new self($token);
    }

    /**
     * @throws Checker\InvalidClaimException
     * @throws Checker\MissingMandatoryClaimException
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
            throw new RuntimeException('Invalid signature');
        }

        /** @var array<string, mixed> $claims */
        $claims = JsonConverter::decode($jws->getPayload() ?? '{}');

        $claimChecker = new Checker\ClaimCheckerManager($this->claimCheckers);
        $claimChecker->check($claims, $this->mandatoryClaims);

        return $claims;
    }

    /**
     * @return string[]
     *
     * @psalm-return list<class-string<Algorithm\SignatureAlgorithm>>
     *
     * @psalm-suppress UndefinedClass
     * @psalm-suppress InvalidReturnStatement
     * @psalm-suppress InvalidReturnType
     */
    protected function getAlgorithmMap(): array
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
    public function mandatory(array $mandatoryClaims): self
    {
        $clone = clone $this;
        $clone->mandatoryClaims = $mandatoryClaims;

        return $clone;
    }

    public function claim(Checker\ClaimChecker $checker): self
    {
        $clone = clone $this;

        $clone->claimCheckers[] = $checker;

        return $clone;
    }

    public function header(Checker\HeaderChecker $checker): self
    {
        $clone = clone $this;

        $clone->headerCheckers[] = $checker;

        return $clone;
    }

    public function keyset(JWKSet $jwkset): self
    {
        $clone = clone $this;
        $clone->jwkset = $jwkset;

        return $clone;
    }
}
