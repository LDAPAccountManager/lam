<?php

declare(strict_types=1);

namespace Facile\JoseVerifier;

interface IdTokenVerifierInterface extends TokenVerifierInterface
{
    /**
     * @return $this
     */
    public function withAccessToken(?string $accessToken);

    /**
     * @return $this
     */
    public function withCode(?string $code);

    /**
     * @return $this
     */
    public function withState(?string $state);
}
