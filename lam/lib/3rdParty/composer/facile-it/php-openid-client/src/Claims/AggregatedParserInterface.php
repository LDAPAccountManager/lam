<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use Facile\OpenIDClient\Client\ClientInterface;
use Facile\OpenIDClient\Token\TokenSetInterface;

/**
 * @psalm-api
 *
 * @psalm-import-type TokenSetClaimsType from TokenSetInterface
 */
interface AggregatedParserInterface
{
    /**
     * @param array<string, mixed> $claims
     *
     * @psalm-param TokenSetClaimsType $claims
     *
     * @return array<string, mixed>
     *
     * @psalm-return TokenSetClaimsType
     */
    public function unpack(ClientInterface $client, array $claims): array;
}
