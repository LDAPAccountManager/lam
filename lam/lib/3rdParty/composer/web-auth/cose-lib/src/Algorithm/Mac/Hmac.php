<?php

declare(strict_types=1);

namespace Cose\Algorithm\Mac;

use Cose\Key\Key;
use InvalidArgumentException;

abstract class Hmac implements Mac
{
    public function hash(string $data, Key $key): string
    {
        $this->checKey($key);
        $signature = hash_hmac($this->getHashAlgorithm(), $data, (string) $key->get(-1), true);

        return mb_substr($signature, 0, intdiv($this->getSignatureLength(), 8), '8bit');
    }

    public function verify(string $data, Key $key, string $signature): bool
    {
        return hash_equals($this->hash($data, $key), $signature);
    }

    abstract protected function getHashAlgorithm(): string;

    abstract protected function getSignatureLength(): int;

    private function checKey(Key $key): void
    {
        if ($key->type() !== 4) {
            throw new InvalidArgumentException('Invalid key. Must be of type symmetric');
        }

        if (! $key->has(-1)) {
            throw new InvalidArgumentException('Invalid key. The value of the key is missing');
        }
    }
}
