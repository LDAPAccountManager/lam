<?php

declare(strict_types=1);

namespace Facile\OpenIDClient\Claims;

use function array_filter;
use Facile\OpenIDClient\Client\ClientInterface;
use function is_array;
use function is_string;
use Throwable;

/**
 * @psalm-import-type TokenSetClaimsType from \Facile\OpenIDClient\Token\TokenSetInterface
 */
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

        /** @var array<string, array{JWT: string}> $aggregatedSources */
        $aggregatedSources = array_filter($claimSources, static function ($value): bool {
            return is_string($value['JWT'] ?? null);
        });

        $claimPayloads = [];
        foreach ($aggregatedSources as $sourceName => $source) {
            try {
                $claimPayloads[$sourceName] = $this->claimJWT($client, $source['JWT']);
                unset($claims['_claim_sources'][$sourceName]);
            } catch (Throwable $e) {
                throw $e;
            }
        }

        return $this->cleanClaims($this->assignClaims($claims, $claimNames, $claimPayloads));
    }
}
