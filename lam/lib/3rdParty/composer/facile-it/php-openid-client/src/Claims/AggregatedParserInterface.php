<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use Facile\OpenIDClient\Client\ClientInterface;

/**
 * @psalm-import-type TokenSetClaimsType from \Facile\OpenIDClient\Token\TokenSetInterface
 */
interface AggregatedParserInterface
{
    /**
     * @param array<string, mixed> $claims
     *
     * @return array<string, mixed>
     *
     * @psalm-param TokenSetClaimsType $claims
     * @psalm-return TokenSetClaimsType
     */
    public function unpack(ClientInterface $client, array $claims): array;
}
