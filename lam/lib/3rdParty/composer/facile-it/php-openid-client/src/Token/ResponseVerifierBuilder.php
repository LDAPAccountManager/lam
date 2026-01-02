<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

use Facile\JoseVerifier\Builder\AuthorizationResponseVerifierBuilder;
use Facile\JoseVerifier\TokenVerifierInterface;
use Facile\OpenIDClient\Client\ClientInterface;
use Override;

/**
 * @psalm-api
 */
final class ResponseVerifierBuilder implements TokenVerifierBuilderInterface
{
    private bool $aadIssValidation = false;

    private int $clockTolerance = 0;

    public function setAadIssValidation(bool $aadIssValidation): self
    {
        $this->aadIssValidation = $aadIssValidation;

        return $this;
    }

    public function setClockTolerance(int $clockTolerance): self
    {
        $this->clockTolerance = $clockTolerance;

        return $this;
    }

    #[Override]
    public function build(ClientInterface $client): TokenVerifierInterface
    {
        return AuthorizationResponseVerifierBuilder::create(
            $client->getIssuer()->getMetadata()->toArray(),
            $client->getMetadata()->toArray(),
        )
            ->withJwksProvider($client->getIssuer()->getJwksProvider())
            ->withClientJwksProvider($client->getJwksProvider())
            ->withClockTolerance($this->clockTolerance)
            ->withAadIssValidation($this->aadIssValidation)
            ->build();
    }
}
