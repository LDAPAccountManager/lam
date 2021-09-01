<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service\Builder;

use Facile\OpenIDClient\Service\RevocationService;

final class RevocationServiceBuilder extends AbstractServiceBuilder
{
    public function build(): RevocationService
    {
        return new RevocationService(
            $this->getHttpClient(),
            $this->getRequestFactory()
        );
    }
}
