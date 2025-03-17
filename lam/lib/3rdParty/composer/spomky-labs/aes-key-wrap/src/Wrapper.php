<?php

declare(strict_types=1);

namespace AESKW;

interface Wrapper
{
    /**
     * @param string $kek             The Key Encryption Key
     * @param string $key             The key to wrap
     * @param bool   $padding_enabled If false, the key to wrap must be a sequence of one or more 64-bit blocks (RFC3394 compliant), else the key size must be at least one octet (RFC5649 compliant)
     *
     * @return string The wrapped key
     */
    public static function wrap(string $kek, string $key, bool $padding_enabled = false): string;

    /**
     * @param string $kek             The Key Encryption Key
     * @param string $key             The key to unwrap
     * @param bool   $padding_enabled If false, the AIV check must be RFC3394 compliant, else it must be RFC5649 or RFC3394 compliant
     *
     * @return string The key unwrapped
     */
    public static function unwrap(string $kek, string $key, bool $padding_enabled = false): string;
}
