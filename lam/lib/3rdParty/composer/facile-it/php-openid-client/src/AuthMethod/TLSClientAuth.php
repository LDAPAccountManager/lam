<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

final class TLSClientAuth extends AbstractTLS
{
    public function getSupportedMethod(): string
    {
        return 'tls_client_auth';
    }
}
