<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service\Builder;

use Facile\OpenIDClient\Service\RegistrationService;

final class RegistrationServiceBuilder extends AbstractServiceBuilder
{
    public function build(): RegistrationService
    {
        return new RegistrationService(
            $this->getHttpClient(),
            $this->getRequestFactory()
        );
    }
}
