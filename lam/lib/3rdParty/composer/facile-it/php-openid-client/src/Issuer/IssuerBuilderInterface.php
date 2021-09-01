<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Issuer;

interface IssuerBuilderInterface
{
    public function build(string $resource): IssuerInterface;
}
