<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\Builder;

use Facile\JoseVerifier\JWK\JwksProviderInterface;
use Facile\JoseVerifier\TokenVerifierInterface;

/**
 * @psalm-api
 *
 * @psalm-import-type ClientMetadataType from TokenVerifierInterface
 * @psalm-import-type IssuerMetadataType from TokenVerifierInterface
 *
 * @template T of TokenVerifierInterface
 */
interface TokenVerifierBuilderInterface
{
    /**
     * @return TokenVerifierBuilderInterface<T>
     */
    public function withClockTolerance(int $clockTolerance): TokenVerifierBuilderInterface;

    /**
     * @return TokenVerifierBuilderInterface<T>
     */
    public function withAadIssValidation(bool $aadIssValidation): TokenVerifierBuilderInterface;

    /**
     * @return TokenVerifierBuilderInterface<T>
     */
    public function withJwksProvider(JwksProviderInterface $jwksProvider): TokenVerifierBuilderInterface;

    /**
     * @return TokenVerifierBuilderInterface<T>
     */
    public function withClientJwksProvider(JwksProviderInterface $clientJwksProvider): TokenVerifierBuilderInterface;

    /**
     * @psalm-return T
     */
    public function build(): TokenVerifierInterface;
}
