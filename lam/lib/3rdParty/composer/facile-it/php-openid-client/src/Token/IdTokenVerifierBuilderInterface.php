<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

use Facile\JoseVerifier\IdTokenVerifierInterface;
use Facile\OpenIDClient\Client\ClientInterface;

interface IdTokenVerifierBuilderInterface
{
    public function build(ClientInterface $client): IdTokenVerifierInterface;
}
