<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

final class JWTVerifier extends AbstractTokenVerifier
{
    public function verify(string $jwt): array
    {
        $jwt = $this->decrypt($jwt);
        $validator = $this->create($jwt)
            ->withMandatory(['iss', 'sub', 'aud', 'exp', 'iat']);

        return $validator->run();
    }
}
