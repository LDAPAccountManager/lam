<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\ClaimChecker\AuthTimeChecker;
use Facile\JoseVerifier\ClaimChecker\AzpChecker;
use Facile\JoseVerifier\ClaimChecker\NonceChecker;
use Facile\JoseVerifier\Decrypter\TokenDecrypterInterface;
use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Facile\JoseVerifier\Exception\InvalidTokenException;
use Facile\JoseVerifier\Exception\RuntimeException;
use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\JoseVerifier\JWK\MemoryJwksProvider;
use Facile\JoseVerifier\Validate\Validate;
use function is_array;
use Jose\Component\Core\JWK;
use Jose\Component\Core\JWKSet;
use Jose\Component\Core\Util\JsonConverter;
use Jose\Component\Signature\Serializer\CompactSerializer;
use function str_replace;
use function strpos;
use Throwable;

/**
 * @psalm-import-type JWTHeaderObject from Psalm\PsalmTypes
 * @psalm-import-type JWTPayloadObject from Psalm\PsalmTypes
 */
abstract class AbstractTokenVerifier implements TokenVerifierInterface
{
    /** @var string */
    protected $issuer;

    /** @var string */
    protected $clientId;

    /** @var JwksProviderInterface */
    protected $jwksProvider;

    /** @var string|null */
    protected $clientSecret;

    /** @var string|null */
    protected $azp;

    /** @var null|string */
    protected $expectedAlg;

    /** @var string|null */
    protected $nonce;

    /** @var int|null */
    protected $maxAge;

    /** @var int */
    protected $clockTolerance = 0;

    /** @var bool */
    protected $authTimeRequired = false;

    /** @var bool */
    protected $aadIssValidation = false;

    /** @var TokenDecrypterInterface|null */
    protected $decrypter;

    public function __construct(string $issuer, string $clientId, ?TokenDecrypterInterface $decrypter = null)
    {
        $this->issuer = $issuer;
        $this->clientId = $clientId;
        $this->jwksProvider = new MemoryJwksProvider();
        $this->decrypter = $decrypter;
    }

    /**
     * @return static
     */
    public function withJwksProvider(JwksProviderInterface $jwksProvider): self
    {
        $new = clone $this;
        $new->jwksProvider = $jwksProvider;

        return $new;
    }

    /**
     * @return static
     */
    public function withClientSecret(?string $clientSecret): self
    {
        $new = clone $this;
        $new->clientSecret = $clientSecret;

        return $new;
    }

    /**
     * @return static
     */
    public function withAzp(?string $azp): self
    {
        $new = clone $this;
        $new->azp = $azp;

        return $new;
    }

    /**
     * @return static
     */
    public function withExpectedAlg(?string $expectedAlg): self
    {
        $new = clone $this;
        $new->expectedAlg = $expectedAlg;

        return $new;
    }

    /**
     * @return static
     */
    public function withNonce(?string $nonce): self
    {
        $new = clone $this;
        $new->nonce = $nonce;

        return $new;
    }

    /**
     * @return static
     */
    public function withMaxAge(?int $maxAge): self
    {
        $new = clone $this;
        $new->maxAge = $maxAge;

        return $new;
    }

    /**
     * @return static
     */
    public function withClockTolerance(int $clockTolerance): self
    {
        $new = clone $this;
        $new->clockTolerance = $clockTolerance;

        return $new;
    }

    /**
     * @return static
     */
    public function withAuthTimeRequired(bool $authTimeRequired): self
    {
        $new = clone $this;
        $new->authTimeRequired = $authTimeRequired;

        return $new;
    }

    /**
     * @return static
     */
    public function withAadIssValidation(bool $aadIssValidation): self
    {
        $new = clone $this;
        $new->aadIssValidation = $aadIssValidation;

        return $new;
    }

    protected function decrypt(string $jwt): string
    {
        if (null === $this->decrypter) {
            return $jwt;
        }

        return $this->decrypter->decrypt($jwt) ?? '{}';
    }

    protected function create(string $jwt): Validate
    {
        $mandatoryClaims = [];

        $expectedIssuer = $this->issuer;

        if ($this->aadIssValidation) {
            $payload = $this->getPayload($jwt);
            $expectedIssuer = str_replace('{tenantid}', $payload['tid'] ?? '', $expectedIssuer);
        }

        $validator = Validate::token($jwt)
            ->keyset($this->buildJwks($jwt))
            ->iss($expectedIssuer)
            ->iat($this->clockTolerance)
            ->aud($this->clientId)
            ->exp($this->clockTolerance)
            ->nbf($this->clockTolerance);

        if (null !== $this->azp) {
            $validator = $validator->claim('azp', new AzpChecker($this->azp));
        }

        if (null !== $this->expectedAlg) {
            $validator = $validator->alg($this->expectedAlg);
        }

        if (null !== $this->nonce) {
            $validator = $validator->claim('nonce', new NonceChecker($this->nonce));
        }

        if (null !== $this->maxAge) {
            $validator = $validator->claim('auth_time', new AuthTimeChecker($this->maxAge, $this->clockTolerance));
        }

        if ((int) $this->maxAge > 0 || null !== $this->maxAge) {
            $mandatoryClaims[] = 'auth_time';
        }

        /** @var Validate $validator */
        $validator = $validator->mandatory($mandatoryClaims);

        return $validator;
    }

    /**
     * @return array<string, mixed>
     *
     * @psalm-return JWTPayloadObject
     */
    protected function getPayload(string $jwt): array
    {
        try {
            $jws = (new CompactSerializer())->unserialize($jwt);
        } catch (\InvalidArgumentException $e) {
            throw new InvalidTokenException('Invalid JWT provided', 0, $e);
        }

        try {
            $payload = JsonConverter::decode($jws->getPayload() ?? '{}');
        } catch (\RuntimeException $e) {
            throw new InvalidTokenException('Unable to decode JWT payload', 0, $e);
        }

        if (! is_array($payload)) {
            throw new InvalidTokenException('Invalid token provided');
        }

        /** @var JWTPayloadObject $payload */
        return $payload;
    }

    private function buildJwks(string $jwt): JWKSet
    {
        try {
            $jws = (new CompactSerializer())->unserialize($jwt);
        } catch (\InvalidArgumentException $e) {
            throw new InvalidTokenException('Invalid JWT provided', 0, $e);
        }

        $header = $jws->getSignature(0)->getProtectedHeader();

        $alg = $header['alg'] ?? '';

        /** @var string|null $kid */
        $kid = $header['kid'] ?? null;

        return $this->getSigningJWKSet($alg, $kid);
    }

    private function getSigningJWKSet(string $alg, ?string $kid = null): JWKSet
    {
        if (0 !== strpos($alg, 'HS')) {
            // not symmetric key
            return null !== $kid
                ? new JWKSet([$this->getJWKFromKid($kid)])
                : JWKSet::createFromKeyData($this->jwksProvider->getJwks());
        }

        if (null === $this->clientSecret) {
            throw new RuntimeException('Unable to verify token without client_secret');
        }

        return new JWKSet([jose_secret_key($this->clientSecret)]);
    }

    private function getJWKFromKid(string $kid): JWK
    {
        $jwks = JWKSet::createFromKeyData($this->jwksProvider->getJwks());
        $jwk = $jwks->selectKey('sig', null, ['kid' => $kid]);

        if (null === $jwk) {
            $jwks = JWKSet::createFromKeyData($this->jwksProvider->reload()->getJwks());
            $jwk = $jwks->selectKey('sig', null, ['kid' => $kid]);
        }

        if (null === $jwk) {
            throw new InvalidTokenException('Unable to find the jwk with the provided kid: ' . $kid);
        }

        return $jwk;
    }

    protected function processException(Throwable $e): Throwable
    {
        if ($e instanceof \InvalidArgumentException) {
            return new InvalidArgumentException($e->getMessage(), 0, $e);
        }

        return new InvalidTokenException('Invalid token provided', 0, $e);
    }
}
