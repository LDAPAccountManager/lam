<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use Override;

final class TLSClientAuth extends AbstractTLS
{
    #[Override]
    public function getSupportedMethod(): string
    {
        return 'tls_client_auth';
    }
}
