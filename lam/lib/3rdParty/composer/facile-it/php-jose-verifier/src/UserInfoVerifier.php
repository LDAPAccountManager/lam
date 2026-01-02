<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

final class UserInfoVerifier extends AbstractTokenVerifier
{
    public function verify(string $jwt): array
    {
        $jwt = $this->decrypt($jwt);

        return $this->create($jwt)->withMandatory(['sub'])->run();
    }
}
