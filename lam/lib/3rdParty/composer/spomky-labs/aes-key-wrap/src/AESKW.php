<?php

declare(strict_types=1);

namespace AESKW;

use function count;
use function hex2bin;
use InvalidArgumentException;
use function mb_str_split;
use function openssl_decrypt;
use function openssl_encrypt;
use const OPENSSL_RAW_DATA;
use const OPENSSL_ZERO_PADDING;
use const STR_PAD_LEFT;
use const STR_PAD_RIGHT;

trait AESKW
{
    /**
     * @param string $kek             The Key Encryption Key
     * @param string $key             The key to wrap
     * @param bool   $padding_enabled If false, the key to wrap must be a sequence of one or more 64-bit blocks (RFC3394 compliant), else the key size must be at least one octet (RFC5649 compliant)
     *
     * @return string The wrapped key
     */
    public static function wrap(string $kek, string $key, bool $padding_enabled = false): string
    {
        $A = self::getInitialValue($key, $padding_enabled);
        self::checkKeySize($key, $padding_enabled);
        $P = mb_str_split($key, 8, '8bit');
        $N = count($P);
        $C = [];

        if ($N === 1) {
            $B = self::encrypt($kek, $A . $P[0]);
            $C[0] = self::getMSB($B);
            $C[1] = self::getLSB($B);
        } elseif ($N > 1) {
            $R = $P;
            for ($j = 0; $j <= 5; ++$j) {
                for ($i = 1; $i <= $N; ++$i) {
                    $B = self::encrypt($kek, $A . $R[$i - 1]);
                    $t = $i + $j * $N;
                    $A = self::toXBits(64, $t) ^ self::getMSB($B);
                    $R[$i - 1] = self::getLSB($B);
                }
            }
            $C = array_merge([$A], $R);
        }

        return implode('', $C);
    }

    /**
     * @param string $kek             The Key Encryption Key
     * @param string $key             The key to unwrap
     * @param bool   $padding_enabled If false, the AIV check must be RFC3394 compliant, else it must be RFC5649 or RFC3394 compliant
     *
     * @return string The key unwrapped
     */
    public static function unwrap(string $kek, string $key, bool $padding_enabled = false): string
    {
        $P = mb_str_split($key, 8, '8bit');
        $A = $P[0];
        $N = count($P);

        if ($N < 2) {
            throw new InvalidArgumentException('Bad data');
        }
        if ($N === 2) {
            $B = self::decrypt($kek, $P[0] . $P[1]);
            $unwrapped = self::getLSB($B);
            $A = self::getMSB($B);
        } else {
            $R = $P;
            for ($j = 5; $j >= 0; --$j) {
                for ($i = $N - 1; $i >= 1; --$i) {
                    $t = $i + $j * ($N - 1);
                    $B = self::decrypt($kek, (self::toXBits(64, $t) ^ $A) . $R[$i]);
                    $A = self::getMSB($B);
                    $R[$i] = self::getLSB($B);
                }
            }
            unset($R[0]);

            $unwrapped = implode('', $R);
        }
        if (self::checkInitialValue($unwrapped, $padding_enabled, $A) === false) {
            throw new InvalidArgumentException('Integrity check failed!');
        }

        return $unwrapped;
    }

    /**
     * The initial value used to wrap the key and check the integrity when unwrapped. The RFC3394 set this value to
     * 0xA6A6A6A6A6A6A6A6 The RFC5649 set this value to 0xA65959A6XXXXXXXX (The part with XXXXXXXX is the MLI, depends
     * on the padding).
     *
     * @param string $key             The key
     * @param bool   $padding_enabled Enable padding (RFC5649)
     *
     * @see https://tools.ietf.org/html/rfc3394#section-2.2.3.1
     */
    private static function getInitialValue(string &$key, bool $padding_enabled): string
    {
        if ($padding_enabled === false) {
            $bin = hex2bin('A6A6A6A6A6A6A6A6');
            if ($bin === false) {
                throw new InvalidArgumentException('Unable to convert the data');
            }

            return $bin;
        }

        $MLI = mb_strlen($key, '8bit');
        $iv = hex2bin('A65959A6') . self::toXBits(32, $MLI);

        $n = (int) ceil($MLI / 8);
        $key = str_pad($key, 8 * $n, "\0", STR_PAD_RIGHT);

        return $iv;
    }

    private static function checkInitialValue(string &$key, bool $padding_enabled, string $iv): bool
    {
        // RFC3394 compliant
        if ($iv === hex2bin('A6A6A6A6A6A6A6A6')) {
            return true;
        }

        // The RFC3394 is required but the previous check is not satisfied => invalid
        if ($padding_enabled === false) {
            return false;
        }

        // The high-order half of the AIV according to the RFC5649
        if (hex2bin('A65959A6') !== self::getMSB($iv)) {
            return false;
        }

        $n = mb_strlen($key, '8bit') / 8;
        $MLI = (int) hexdec(bin2hex(ltrim(self::getLSB($iv), "\0")));

        if (! (8 * ($n - 1) < $MLI && $MLI <= 8 * $n)) {
            return false;
        }

        $b = 8 * $n - $MLI;
        for ($i = 0; $i < $b; ++$i) {
            if (mb_substr($key, $MLI + $i, 1, '8bit') !== "\0") {
                return false;
            }
        }
        $key = mb_substr($key, 0, $MLI, '8bit');

        return true;
    }

    private static function checkKeySize(string $key, bool $padding_enabled): void
    {
        if ($key === '') {
            throw new InvalidArgumentException('Bad key size');
        }
        if ($padding_enabled === false && mb_strlen($key, '8bit') % 8 !== 0) {
            throw new InvalidArgumentException('Bad key size');
        }
    }

    private static function toXBits(int $bits, int $value): string
    {
        $bin = hex2bin(str_pad(dechex($value), $bits / 4, '0', STR_PAD_LEFT));
        if ($bin === false) {
            throw new InvalidArgumentException('Unable to convert the data');
        }

        return $bin;
    }

    private static function getMSB(string $value): string
    {
        return mb_substr($value, 0, mb_strlen($value, '8bit') / 2, '8bit');
    }

    private static function getLSB(string $value): string
    {
        return mb_substr($value, mb_strlen($value, '8bit') / 2, null, '8bit');
    }

    private static function encrypt(string $kek, string $data): string
    {
        $result = openssl_encrypt($data, self::getMethod(), $kek, OPENSSL_ZERO_PADDING | OPENSSL_RAW_DATA);
        if ($result === false) {
            throw new InvalidArgumentException('Unable to encrypt the data');
        }

        return $result;
    }

    private static function decrypt(string $kek, string $data): string
    {
        $result = openssl_decrypt($data, self::getMethod(), $kek, OPENSSL_ZERO_PADDING | OPENSSL_RAW_DATA);
        if ($result === false) {
            throw new InvalidArgumentException('Unable to decrypt the data');
        }

        return $result;
    }
}
