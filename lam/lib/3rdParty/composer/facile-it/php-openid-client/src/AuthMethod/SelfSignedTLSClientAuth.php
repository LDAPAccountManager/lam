<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

use Override;

final class SelfSignedTLSClientAuth extends AbstractTLS
{
    #[Override]
    public function getSupportedMethod(): string
    {
        return 'self_signed_tls_client_auth';
    }
}
