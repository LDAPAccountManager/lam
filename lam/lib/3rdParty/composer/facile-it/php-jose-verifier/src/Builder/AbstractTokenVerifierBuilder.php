<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Builder;

use Facile\JoseVerifier\AbstractTokenVerifier;
use Facile\JoseVerifier\Decrypter\NullTokenDecrypter;
use Facile\JoseVerifier\Decrypter\TokenDecrypter;
use Facile\JoseVerifier\Decrypter\TokenDecrypterInterface;
use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Facile\JoseVerifier\JWK\JwksProviderBuilder;
use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\JoseVerifier\JWK\MemoryJwksProvider;
use Facile\JoseVerifier\TokenVerifierInterface;

/**
 * @psalm-api
 *
 * @psalm-type JWKSetType = array{keys: list<array<string, mixed>>}
 *
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 * @psalm-import-type IssuerMetadataType from TokenVerifierInterface
 *
 * @template TVerifier of AbstractTokenVerifier
 *
 * @template-implements TokenVerifierBuilderInterface<TVerifier>
 */
abstract class AbstractTokenVerifierBuilder implements TokenVerifierBuilderInterface
{
    /**
     * @var array<string, mixed>
     *
     * @psalm-var ClientMetadataType
     */
    protected array $clientMetadata;

    /**
     * @var array<string, mixed>
     *
     * @psalm-var IssuerMetadataType
     */
    protected array $issuerMetadata;

    protected int $clockTolerance = 0;

    protected bool $aadIssValidation = false;

    protected ?string $expectedAzp = null;

    protected ?JwksProviderInterface $clientJwksProvider = null;

    protected ?JwksProviderInterface $jwksProvider = null;

    protected ?JwksProviderBuilder $jwksProviderBuilder = null;

    /**
     * @psalm-param IssuerMetadataType $issuerMetadata
     * @psalm-param ClientMetadataType $clientMetadata
     */
    protected function __construct(array $issuerMetadata, array $clientMetadata)
    {
        $this->issuerMetadata = $issuerMetadata;
        $this->clientMetadata = $clientMetadata;
    }

    public function withClockTolerance(int $clockTolerance): static
    {
        $new = clone $this;
        $new->clockTolerance = $clockTolerance;

        return $new;
    }

    public function withAadIssValidation(bool $aadIssValidation): static
    {
        $new = clone $this;
        $new->aadIssValidation = $aadIssValidation;

        return $new;
    }

    public function withExpectedAzp(string $azp): static
    {
        $new = clone $this;
        $new->expectedAzp = $azp;

        return $new;
    }

    protected function getExpectedAzp(): ?string
    {
        return $this->expectedAzp;
    }

    public function withJwksProvider(JwksProviderInterface $jwksProvider): static
    {
        $new = clone $this;
        $new->jwksProvider = $jwksProvider;

        return $new;
    }

    public function withClientJwksProvider(JwksProviderInterface $clientJwksProvider): static
    {
        $new = clone $this;
        $new->clientJwksProvider = $clientJwksProvider;

        return $new;
    }

    protected function getJwksProvider(): JwksProviderInterface
    {
        if ($this->jwksProvider) {
            return $this->jwksProvider;
        }

        return $this->jwksProvider = $this->buildJwksProvider();
    }

    /**
     * @throws InvalidArgumentException
     */
    protected function buildJwksProvider(): JwksProviderInterface
    {
        $jwksUri = $this->issuerMetadata['jwks_uri'] ?? null;

        $jwksBuilder = $this->jwksProviderBuilder ?? new JwksProviderBuilder();

        if (null !== $jwksUri) {
            $jwksBuilder = $jwksBuilder->withJwksUri($jwksUri);
        }

        return $jwksBuilder->build();
    }

    protected function getClientJwksProvider(): JwksProviderInterface
    {
        if ($this->clientJwksProvider) {
            return $this->clientJwksProvider;
        }

        return $this->clientJwksProvider = $this->buildClientJwksProvider();
    }

    protected function buildClientJwksProvider(): JwksProviderInterface
    {
        $jwks = ['keys' => []];

        $jwks = $this->clientMetadata['jwks'] ?? $jwks;

        return new MemoryJwksProvider($jwks);
    }

    public function withJwksProviderBuilder(?JwksProviderBuilder $jwksProviderBuilder): static
    {
        $new = clone $this;
        $new->jwksProviderBuilder = $jwksProviderBuilder;

        return $new;
    }

    abstract protected function getExpectedAlg(): ?string;

    abstract protected function getExpectedEncAlg(): ?string;

    abstract protected function getExpectedEnc(): ?string;

    protected function getIssuer(): string
    {
        return $this->issuerMetadata['issuer'];
    }

    protected function getClientId(): string
    {
        return $this->clientMetadata['client_id'];
    }

    protected function getClientSecret(): ?string
    {
        return $this->clientMetadata['client_secret'] ?? null;
    }

    protected function getAuthTimeRequired(): bool
    {
        return $this->clientMetadata['require_auth_time'] ?? false;
    }

    /**
     * @throws InvalidArgumentException On invalid id_token_encrypted* values
     */
    protected function buildDecrypter(): TokenDecrypterInterface
    {
        $alg = $this->getExpectedEncAlg();
        $enc = $this->getExpectedEnc();

        if ((null !== $alg) xor (null !== $enc)) {
            throw new InvalidArgumentException('Invalid values received for id_token_encrypted* values');
        }

        if (null === $alg) {
            return new NullTokenDecrypter();
        }

        return (new TokenDecrypter())
            ->withExpectedAlg($alg)
            ->withExpectedEnc($enc)
            ->withClientSecret($this->getClientSecret())
            ->withJwksProvider($this->getClientJwksProvider());
    }
}
