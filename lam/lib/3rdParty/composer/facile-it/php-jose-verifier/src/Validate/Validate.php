<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Validate;

use function count;
use Facile\JoseVerifier\Exception\RuntimeException;
use Jose\Component\Checker;
use Jose\Component\Core\AlgorithmManager;
use Jose\Component\Core\Util\JsonConverter;
use Jose\Component\Signature\Algorithm;
use Jose\Component\Signature\JWSTokenSupport;
use Jose\Component\Signature\JWSVerifier;
use Jose\Component\Signature\Serializer\CompactSerializer;
use Jose\Easy\AbstractLoader;
use Jose\Easy\JWT;

class Validate extends AbstractLoader
{
    public static function token(string $token): self
    {
        return new self($token);
    }

    public function run(): JWT
    {
        if (0 !== count($this->allowedAlgorithms)) {
            $this->headerCheckers[] = new Checker\AlgorithmChecker($this->allowedAlgorithms, true);
        }
        $jws = (new CompactSerializer())->unserialize($this->token);
        $headerChecker = new Checker\HeaderCheckerManager($this->headerCheckers, [new JWSTokenSupport()]);
        $headerChecker->check($jws, 0);

        $verifier = new JWSVerifier(new AlgorithmManager($this->algorithms));
        if (! $verifier->verifyWithKeySet($jws, $this->jwkset, 0)) {
            throw new RuntimeException('Invalid signature');
        }

        $jwt = new JWT();
        $jwt->header->replace($jws->getSignature(0)->getProtectedHeader());
        /** @var array<string, mixed> $claims */
        $claims = JsonConverter::decode($jws->getPayload() ?? '{}');
        $jwt->claims->replace($claims);

        $claimChecker = new Checker\ClaimCheckerManager($this->claimCheckers);
        $claimChecker->check($jwt->claims->all(), $this->mandatoryClaims);

        return $jwt;
    }

    /**
     * @return string[]
     * @psalm-suppress UndefinedClass
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
}
