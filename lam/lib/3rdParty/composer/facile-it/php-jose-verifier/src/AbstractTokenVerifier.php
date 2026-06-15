<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\Internal\InternalClock;
use Facile\JoseVerifier\Decrypter\NullTokenDecrypter;
use Facile\JoseVerifier\Decrypter\TokenDecrypterInterface;
use Facile\JoseVerifier\Exception\InvalidTokenException;
use Facile\JoseVerifier\Internal\Checker\AuthTimeChecker;
use Facile\JoseVerifier\Internal\Checker\AzpChecker;
use Facile\JoseVerifier\Internal\Checker\NonceChecker;
use Facile\JoseVerifier\Internal\Validate;
use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\JoseVerifier\JWK\MemoryJwksProvider;
use InvalidArgumentException;
use Jose\Component\Checker\AlgorithmChecker;
use Jose\Component\Checker\AudienceChecker;
use Jose\Component\Checker\ExpirationTimeChecker;
use Jose\Component\Checker\IssuedAtChecker;
use Jose\Component\Checker\IssuerChecker;
use Jose\Component\Checker\NotBeforeChecker;
use Jose\Component\Core\JWK;
use Jose\Component\Core\JWKSet;
use Jose\Component\Core\Util\JsonConverter;
use Jose\Component\Signature\Serializer\CompactSerializer;
use RuntimeException;
use Psr\Clock\ClockInterface;

use function is_array;
use function str_replace;

/**
 * @psalm-api
 *
 * @psalm-import-type JWTPayloadType from TokenVerifierInterface
 */
abstract class AbstractTokenVerifier implements TokenVerifierInterface
{
    protected JwksProviderInterface $jwksProvider;

    protected TokenDecrypterInterface $decrypter;

    protected ?string $nonce = null;

    protected ?int $maxAge = null;

    protected ClockInterface $clock;

    /**
     * @internal Use the builder
     *
     * @psalm-internal \Facile\JoseVerifier
     */
    final public function __construct(
        protected string $issuer,
        protected string $clientId,
        protected ?string $clientSecret = null,
        protected bool $authTimeRequired = false,
        protected int $clockTolerance = 0,
        protected bool $aadIssValidation = false,
        protected ?string $expectedAzp = null,
        protected ?string $expectedAlg = null,
        ?JwksProviderInterface $jwksProvider = null,
        ?TokenDecrypterInterface $decrypter = null,
        ?ClockInterface $clock = null,
    ) {
        $this->jwksProvider = $jwksProvider ?? new MemoryJwksProvider();
        $this->decrypter = $decrypter ?? new NullTokenDecrypter();
        $this->clock = $clock ?? new InternalClock();
    }

    public function withNonce(?string $nonce): static
    {
        $new = clone $this;
        $new->nonce = $nonce;

        return $new;
    }

    public function withMaxAge(?int $maxAge): static
    {
        $new = clone $this;
        $new->maxAge = $maxAge;

        return $new;
    }

    protected function decrypt(string $jwt): string
    {
        return $this->decrypter->decrypt($jwt) ?? '{}';
    }

    /**
     * @throws InvalidTokenException When unable to decode JWT or client_secret is necessary
     */
    protected function create(string $jwt): Validate
    {
        $mandatoryClaims = [];

        $expectedIssuer = $this->issuer;

        if ($this->aadIssValidation) {
            $payload = $this->getPayload($jwt);
            $expectedIssuer = str_replace('{tenantid}', (string) ($payload['tid'] ?? ''), $expectedIssuer);
        }

        $validator = Validate::withToken($jwt)
            ->withJWKSet($this->buildJwks($jwt))
            ->withClaim(new IssuerChecker([$expectedIssuer], true))
            ->withClaim(new IssuedAtChecker($this->clock, $this->clockTolerance, true))
            ->withClaim(new AudienceChecker($this->clientId, true))
            ->withClaim(new ExpirationTimeChecker($this->clock, $this->clockTolerance, false))
            ->withClaim(new NotBeforeChecker($this->clock, $this->clockTolerance, true));

        if (null !== $this->expectedAzp) {
            $validator = $validator->withClaim(new AzpChecker($this->expectedAzp));
        }

        if (null !== $this->expectedAlg) {
            $validator = $validator->withHeader(new AlgorithmChecker([$this->expectedAlg], true));
        }

        if (null !== $this->nonce) {
            $validator = $validator->withClaim(new NonceChecker($this->nonce));
        }

        if (null !== $this->maxAge) {
            $validator = $validator->withClaim(new AuthTimeChecker($this->maxAge, $this->clockTolerance, $this->clock));
        }

        if ($this->authTimeRequired || (int) $this->maxAge > 0 || null !== $this->maxAge) {
            $mandatoryClaims[] = 'auth_time';
        }

        return $validator->withMandatory($mandatoryClaims);
    }

    /**
     * @throws InvalidTokenException When unable to decode JWT payload
     *
     * @return array<string, mixed>
     *
     * @psalm-return JWTPayloadType
     */
    protected function getPayload(string $jwt): array
    {
        try {
            $jws = (new CompactSerializer())->unserialize($jwt);
        } catch (InvalidArgumentException $e) {
            throw new InvalidTokenException('Invalid JWT provided', 0, $e);
        }

        try {
            $payload = JsonConverter::decode($jws->getPayload() ?? '{}');
        } catch (RuntimeException $e) {
            throw new InvalidTokenException('Unable to decode JWT payload', 0, $e);
        }

        if (! is_array($payload)) {
            throw new InvalidTokenException('Invalid token provided');
        }

        /** @var JWTPayloadType $payload */
        return $payload;
    }

    /**
     * @throws InvalidTokenException When unable to decode JWT or client_secret is necessary
     */
    private function buildJwks(string $jwt): JWKSet
    {
        try {
            $jws = (new CompactSerializer())->unserialize($jwt);
        } catch (InvalidArgumentException $e) {
            throw new InvalidTokenException('Invalid JWT provided', 0, $e);
        }

        $header = $jws->getSignature(0)->getProtectedHeader();

        $alg = $header['alg'] ?? '';

        /** @var string|null $kid */
        $kid = $header['kid'] ?? null;

        return $this->getSigningJWKSet($alg, $kid);
    }

    /**
     * @throws InvalidTokenException When a client_secret is necessary to verify signature
     */
    private function getSigningJWKSet(string $alg, ?string $kid = null): JWKSet
    {
        if (! str_starts_with($alg, 'HS')) {
            // not symmetric key
            return null !== $kid
                ? new JWKSet([$this->getJWKFromKid($kid)])
                : JWKSet::createFromKeyData($this->jwksProvider->getJwks());
        }

        if (null === $this->clientSecret) {
            throw new InvalidTokenException('Signature requires client_secret to be verified');
        }

        return new JWKSet([jose_secret_key($this->clientSecret)]);
    }

    /**
     * @throws InvalidTokenException
     */
    private function getJWKFromKid(string $kid): JWK
    {
        $jwks = JWKSet::createFromKeyData($this->jwksProvider->getJwks());
        $jwk = $jwks->selectKey('sig', null, ['kid' => $kid]);

        if (! $jwk instanceof JWK) {
            $jwks = JWKSet::createFromKeyData($this->jwksProvider->reload()->getJwks());
            $jwk = $jwks->selectKey('sig', null, ['kid' => $kid]);
        }

        if (! $jwk instanceof JWK) {
            throw new InvalidTokenException('Unable to find the jwk with the provided kid: ' . $kid);
        }

        return $jwk;
    }
}
