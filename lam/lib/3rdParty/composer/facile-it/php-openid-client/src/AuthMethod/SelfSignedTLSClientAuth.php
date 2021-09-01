<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

final class SelfSignedTLSClientAuth extends AbstractTLS
{
    public function getSupportedMethod(): string
    {
        return 'self_signed_tls_client_auth';
    }
}
