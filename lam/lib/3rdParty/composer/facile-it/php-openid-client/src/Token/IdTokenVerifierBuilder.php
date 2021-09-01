<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

use Facile\JoseVerifier\IdTokenVerifierInterface;
use Facile\OpenIDClient\Client\ClientInterface;

final class IdTokenVerifierBuilder implements IdTokenVerifierBuilderInterface
{
    /** @var bool */
    private $aadIssValidation = false;

    /** @var int */
    private $clockTolerance = 0;

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

    public function build(ClientInterface $client): IdTokenVerifierInterface
    {
        $builder = new \Facile\JoseVerifier\IdTokenVerifierBuilder();

        $builder->setJwksProvider($client->getIssuer()->getJwksProvider());
        $builder->setClientMetadata($client->getMetadata()->toArray());
        $builder->setClientJwksProvider($client->getJwksProvider());
        $builder->setIssuerMetadata($client->getIssuer()->getMetadata()->toArray());
        $builder->setClockTolerance($this->clockTolerance);
        $builder->setAadIssValidation($this->aadIssValidation);

        return $builder->build();
    }
}
