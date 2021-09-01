<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

use Facile\JoseVerifier\TokenVerifierBuilderInterface;
use Facile\JoseVerifier\TokenVerifierInterface;
use Facile\OpenIDClient\Client\ClientInterface;

final class AccessTokenVerifierBuilder implements AccessTokenVerifierBuilderInterface
{
    /** @var bool */
    private $aadIssValidation = false;

    /** @var int */
    private $clockTolerance = 0;

    /** @var null|TokenVerifierBuilderInterface */
    private $joseBuilder;

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

    public function setJoseBuilder(?TokenVerifierBuilderInterface $joseBuilder): void
    {
        $this->joseBuilder = $joseBuilder;
    }

    private function getJoseBuilder(): TokenVerifierBuilderInterface
    {
        return $this->joseBuilder ?? new \Facile\JoseVerifier\AccessTokenVerifierBuilder();
    }

    public function build(ClientInterface $client): TokenVerifierInterface
    {
        $builder = $this->getJoseBuilder();
        $builder->setJwksProvider($client->getIssuer()->getJwksProvider());
        $builder->setClientMetadata($client->getMetadata()->toArray());
        $builder->setClientJwksProvider($client->getJwksProvider());
        $builder->setIssuerMetadata($client->getIssuer()->getMetadata()->toArray());
        $builder->setClockTolerance($this->clockTolerance);
        $builder->setAadIssValidation($this->aadIssValidation);

        return $builder->build();
    }
}
