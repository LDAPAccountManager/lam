<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\AuthMethod;

interface AuthMethodFactoryInterface
{
    public function create(string $authMethod): AuthMethodInterface;
}
