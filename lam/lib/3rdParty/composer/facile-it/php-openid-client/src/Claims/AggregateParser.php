<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use function array_filter;
use Facile\OpenIDClient\Client\ClientInterface;
use function is_array;

final class AggregateParser extends AbstractClaims implements AggregatedParserInterface
{
    public function unpack(ClientInterface $client, array $claims): array
    {
        $claimSources = $claims['_claim_sources'] ?? null;
        $claimNames = $claims['_claim_names'] ?? null;

        if (! is_array($claimSources)) {
            return $claims;
        }

        if (! is_array($claimNames)) {
            return $claims;
        }

        $aggregatedSources = array_filter($claimSources, fn ($value): bool => $this->isAggregateSource($value));

        $claimPayloads = [];
        foreach ($aggregatedSources as $sourceName => $source) {
            $claimPayloads[$sourceName] = $this->claimJWT($client, $source['JWT']);
            unset($claims['_claim_sources'][$sourceName]);
        }

        return $this->cleanClaims($this->assignClaims($claims, $claimNames, $claimPayloads));
    }
}
