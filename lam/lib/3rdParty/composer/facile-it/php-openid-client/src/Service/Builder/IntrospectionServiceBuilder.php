<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Service\Builder;

use Facile\OpenIDClient\Service\IntrospectionService;

final class IntrospectionServiceBuilder extends AbstractServiceBuilder
{
    public function build(): IntrospectionService
    {
        return new IntrospectionService(
            $this->getHttpClient(),
            $this->getRequestFactory()
        );
    }
}
