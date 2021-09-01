<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use Facile\OpenIDClient\Client\ClientInterface;

/**
 * @psalm-import-type TokenSetClaimsType from \Facile\OpenIDClient\Token\TokenSetInterface
 */
interface DistributedParserInterface
{
    /**
     * @param array<string, mixed> $claims
     * @param string[] $accessTokens
     *
     * @return array<string, mixed>
     *
     * @psalm-param TokenSetClaimsType $claims
     * @psalm-return TokenSetClaimsType
     */
    public function fetch(ClientInterface $client, array $claims, array $accessTokens = []): array;
}
