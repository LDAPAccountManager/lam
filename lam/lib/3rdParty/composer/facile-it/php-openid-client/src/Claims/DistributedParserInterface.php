<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use Facile\OpenIDClient\Client\ClientInterface;

/**
 * @psalm-api
 *
 * @psalm-import-type TokenSetClaimsType from \Facile\OpenIDClient\Token\TokenSetInterface
 */
interface DistributedParserInterface
{
    /**
     * @param array<string, mixed> $claims
     * @param string[] $accessTokens
     *
     * @psalm-param TokenSetClaimsType $claims
     *
     * @return array<string, mixed>
     *
     * @psalm-return TokenSetClaimsType
     */
    public function fetch(ClientInterface $client, array $claims, array $accessTokens = []): array;
}
