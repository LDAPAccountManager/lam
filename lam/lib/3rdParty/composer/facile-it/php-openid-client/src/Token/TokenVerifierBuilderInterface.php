<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Token;

use Facile\JoseVerifier\TokenVerifierInterface;
use Facile\OpenIDClient\Client\ClientInterface;

interface TokenVerifierBuilderInterface
{
    public function build(ClientInterface $client): TokenVerifierInterface;
}
