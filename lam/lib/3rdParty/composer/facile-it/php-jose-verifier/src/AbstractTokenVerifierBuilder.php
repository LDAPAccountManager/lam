<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Facile\JoseVerifier\Decrypter\TokenDecrypter;
use Facile\JoseVerifier\Decrypter\TokenDecrypterInterface;
use Facile\JoseVerifier\Exception\InvalidArgumentException;
use Facile\JoseVerifier\JWK\JwksProviderBuilder;
use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\JoseVerifier\JWK\MemoryJwksProvider;

/**
 * @psalm-import-type ClientMetadataObject from Psalm\PsalmTypes
 * @psalm-import-type IssuerMetadataObject from Psalm\PsalmTypes
 * @psalm-import-type JWKSetObject from Psalm\PsalmTypes
 * @template TVerifier of AbstractTokenVerifier
 * @template-implements TokenVerifierBuilderInterface<TVerifier>
 */
abstract class AbstractTokenVerifierBuilder implements TokenVerifierBuilderInterface
{
    /**
     * @var null|array<string, mixed>
     * @psalm-var null|ClientMetadataObject
     */
    protected $clientMetadata;

    /**
     * @var null|array<string, mixed>
     * @psalm-var null|IssuerMetadataObject
     */
    protected $issuerMetadata;

    /** @var int */
    protected $clockTolerance = 0;

    /** @var bool */
    protected $aadIssValidation = false;

    /** @var JwksProviderInterface|null */
    protected $clientJwksProvider;

    /** @var JwksProviderInterface|null */
    protected $jwksProvider;

    /** @var JwksProviderBuilder|null */
    protected $jwksProviderBuilder;

    /**
     * @param array<string, mixed> $clientMetadata
     * @psalm-param ClientMetadataObject $clientMetadata
     */
    public function setClientMetadata(array $clientMetadata): void
    {
        $this->clientMetadata = $clientMetadata;
    }

    /**
     * @param array<string, mixed> $issuerMetadata
     * @psalm-param IssuerMetadataObject $issuerMetadata
     */
    public function setIssuerMetadata(array $issuerMetadata): void
    {
        $this->issuerMetadata = $issuerMetadata;
    }

    public function setClockTolerance(int $clockTolerance): void
    {
        $this->clockTolerance = $clockTolerance;
    }

    public function setAadIssValidation(bool $aadIssValidation): void
    {
        $this->aadIssValidation = $aadIssValidation;
    }

    public function setJwksProvider(?JwksProviderInterface $jwksProvider): void
    {
        $this->jwksProvider = $jwksProvider;
    }

    public function setClientJwksProvider(?JwksProviderInterface $clientJwksProvider): void
    {
        $this->clientJwksProvider = $clientJwksProvider;
    }

    protected function buildJwksProvider(): JwksProviderInterface
    {
        if (null !== $this->jwksProvider) {
            return $this->jwksProvider;
        }

        $jwksUri = $this->getIssuerMetadata()['jwks_uri'] ?? null;

        $jwksBuilder = $this->jwksProviderBuilder ?? new JwksProviderBuilder();
        $jwksBuilder->setJwksUri($jwksUri ?: null);

        return $jwksBuilder->build();
    }

    protected function buildClientJwksProvider(): JwksProviderInterface
    {
        if (null !== $this->clientJwksProvider) {
            return $this->clientJwksProvider;
        }

        /** @var JWKSetObject $jwks */
        $jwks = ['keys' => []];

        if ($this->clientMetadata) {
            $jwks = $this->clientMetadata['jwks'] ?? $jwks;
        }

        return new MemoryJwksProvider($jwks);
    }

    public function setJwksProviderBuilder(?JwksProviderBuilder $jwksProviderBuilder): void
    {
        $this->jwksProviderBuilder = $jwksProviderBuilder;
    }

    /**
     * @psalm-return TVerifier
     */
    abstract protected function getVerifier(string $issuer, string $clientId): AbstractTokenVerifier;

    abstract protected function getExpectedAlg(): ?string;

    abstract protected function getExpectedEncAlg(): ?string;

    abstract protected function getExpectedEnc(): ?string;

    /**
     * @return array<string, mixed>
     * @psalm-return ClientMetadataObject
     */
    protected function getClientMetadata(): array
    {
        if (! $this->clientMetadata) {
            throw new InvalidArgumentException('No client metadata provided');
        }

        return $this->clientMetadata;
    }

    /**
     * @return array<string, mixed>
     * @psalm-return IssuerMetadataObject
     */
    protected function getIssuerMetadata(): array
    {
        if (! $this->issuerMetadata) {
            throw new InvalidArgumentException('No issuer metadata provided');
        }

        return $this->issuerMetadata;
    }

    /**
     * @psalm-return TVerifier
     */
    public function build(): TokenVerifierInterface
    {
        $issuer = $this->getIssuerMetadata()['issuer'] ?? null;
        $clientId = $this->getClientMetadata()['client_id'] ?? null;

        if (empty($issuer)) {
            throw new InvalidArgumentException('Invalid "issuer" from issuer metadata');
        }

        if (empty($clientId)) {
            throw new InvalidArgumentException('Invalid "client_id" from client metadata');
        }

        $verifier = $this->getVerifier($issuer, $clientId)
            ->withJwksProvider($this->buildJwksProvider())
            ->withClientSecret($this->getClientMetadata()['client_secret'] ?? null)
            ->withAuthTimeRequired($this->getClientMetadata()['require_auth_time'] ?? false)
            ->withClockTolerance($this->clockTolerance)
            ->withAadIssValidation($this->aadIssValidation)
            ->withExpectedAlg($this->getExpectedAlg());

        return $verifier;
    }

    protected function buildDecrypter(): ?TokenDecrypterInterface
    {
        $alg = $this->getExpectedEncAlg();
        $enc = $this->getExpectedEnc();

        if ((null !== $alg) xor (null !== $enc)) {
            throw new InvalidArgumentException('Invalid values received for id_token_encrypted* values');
        }

        if (null === $alg) {
            return null;
        }

        return (new TokenDecrypter())
            ->withExpectedAlg($alg)
            ->withExpectedEnc($enc)
            ->withClientSecret($this->getClientMetadata()['client_secret'] ?? null)
            ->withJwksProvider($this->buildClientJwksProvider());
    }
}
