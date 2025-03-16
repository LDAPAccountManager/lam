<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

use Throwable;

final class JWTVerifier extends AbstractTokenVerifier
{
    /**
     * @inheritDoc
     *
     * @psalm-suppress MixedReturnTypeCoercion
     */
    public function verify(string $jwt): array
    {
        $jwt = $this->decrypt($jwt);
        $validator = $this->create($jwt)
            ->mandatory(['iss', 'sub', 'aud', 'exp', 'iat']);

        try {
            return $validator->run();
        } catch (Throwable $e) {
            throw $this->processException($e);
        }
    }
}
