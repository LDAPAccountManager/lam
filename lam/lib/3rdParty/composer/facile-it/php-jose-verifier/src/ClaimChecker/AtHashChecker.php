<?php

declare(strict_types=1);

namespace Facile\JoseVerifier\ClaimChecker;

final class AtHashChecker extends AbstractHashChecker
{
    private const CLAIM_NAME = 'at_hash';

    public function supportedClaim(): string
    {
        return self::CLAIM_NAME;
    }
}
