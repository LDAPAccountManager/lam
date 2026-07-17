<?php

declare(strict_types=1);

/**
 * Asn1.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Cms;

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Cms\Asn1
 *
 * Minimal DER ASN.1 encoder/decoder used to assemble and inspect CMS/CAdES
 * structures, RFC 3161 timestamp messages, and OCSP requests. Only the subset
 * of ASN.1 needed by PDF signatures is implemented.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class Asn1
{
    /**
     * Encode a DER length octet sequence.
     *
     * @param int<0, max> $length Number of content octets.
     *
     * @throws Exception If the length is too large to encode.
     */
    public function encodeLength(int $length): string
    {
        if ($length < 128) {
            return \chr($length);
        }

        $encoded = '';
        $value = $length;
        while ($value > 0) {
            $encoded = \chr((int) ($value & 0xFF)) . $encoded;
            $value = (int) ($value / 256);
        }

        $encodedLength = \strlen($encoded);
        if ($encodedLength > 0x7F) {
            // Defensive: unreachable, as this needs content larger than 2^1016
            // bytes, which is unrepresentable and unallocatable.
            throw new Exception('ASN.1 length encoding overflow');
        }

        return \chr(0x80 | $encodedLength) . $encoded;
    }

    /**
     * Encode a non-negative integer as a DER INTEGER.
     *
     * @param int<0, max> $value Integer value.
     *
     * @throws Exception If the length cannot be encoded.
     */
    public function encodeInteger(int $value): string
    {
        $data = '';
        $num = $value;
        while ($num > 0) {
            $data = \chr((int) ($num & 0xFF)) . $data;
            $num = (int) ($num / 256);
        }

        if ($data === '') {
            $data = "\x00";
        }

        if ((\ord($data[0]) & 0x80) !== 0) {
            $data = "\x00" . $data;
        }

        return "\x02" . $this->encodeLength(\strlen($data)) . $data;
    }

    /**
     * Encode a big-endian magnitude byte string as a DER INTEGER.
     *
     * Trims superfluous leading zero octets and prepends a zero octet when the
     * most significant bit is set, so the value stays non-negative. Useful for
     * certificate serial numbers.
     *
     * @throws Exception If the length cannot be encoded.
     */
    public function encodeIntegerBytes(string $bytes): string
    {
        $len = \strlen($bytes);
        $start = 0;
        while ($start < ($len - 1) && $bytes[$start] === "\x00" && (\ord($bytes[$start + 1]) & 0x80) === 0) {
            ++$start;
        }

        $magnitude = \substr($bytes, $start);
        if ($magnitude === '') {
            $magnitude = "\x00";
        }

        if ((\ord($magnitude[0]) & 0x80) !== 0) {
            $magnitude = "\x00" . $magnitude;
        }

        return "\x02" . $this->encodeLength(\strlen($magnitude)) . $magnitude;
    }

    /**
     * Encode a DER BOOLEAN.
     */
    public function encodeBoolean(bool $value): string
    {
        return "\x01\x01" . ($value ? "\xFF" : "\x00");
    }

    /**
     * Encode a DER NULL.
     */
    public function encodeNull(): string
    {
        return "\x05\x00";
    }

    /**
     * Encode a DER OCTET STRING.
     *
     * @throws Exception If the length cannot be encoded.
     */
    public function encodeOctetString(string $value): string
    {
        return "\x04" . $this->encodeLength(\strlen($value)) . $value;
    }

    /**
     * Wrap pre-encoded content in a DER SEQUENCE.
     *
     * @throws Exception If the length cannot be encoded.
     */
    public function encodeSequence(string $value): string
    {
        return "\x30" . $this->encodeLength(\strlen($value)) . $value;
    }

    /**
     * Wrap pre-encoded content in a DER SET.
     *
     * @throws Exception If the length cannot be encoded.
     */
    public function encodeSet(string $value): string
    {
        return "\x31" . $this->encodeLength(\strlen($value)) . $value;
    }

    /**
     * Wrap pre-encoded content in a context-specific constructed tag [n].
     *
     * @param int<0, 30> $number Context tag number.
     *
     * @throws Exception If the length cannot be encoded.
     */
    public function encodeContext(int $number, string $value): string
    {
        return \chr(0xA0 | ($number & 0x1F)) . $this->encodeLength(\strlen($value)) . $value;
    }

    /**
     * Encode a dotted OID string as a DER OBJECT IDENTIFIER.
     *
     * @throws Exception If the OID is malformed or the length cannot be encoded.
     */
    public function encodeObjectIdentifier(string $oid): string
    {
        $parts = \array_map('intval', \explode('.', $oid));
        if (\count($parts) < 2) {
            throw new Exception('Invalid OID');
        }

        $data = \chr((int) ((($parts[0] * 40) + ($parts[1] ?? 0)) & 0xFF));
        $count = \count($parts);
        for ($idx = 2; $idx < $count; ++$idx) {
            $part = (int) ($parts[$idx] ?? 0);
            if ($part < 0) {
                $part = 0;
            }

            $data .= $this->encodeBase128Int($part);
        }

        return "\x06" . $this->encodeLength(\strlen($data)) . $data;
    }

    /**
     * Encode a non-negative integer in base-128 with continuation bits.
     *
     * @param int<0, max> $value Integer value.
     */
    public function encodeBase128Int(int $value): string
    {
        $bytes = [$value & 0x7F];
        $value = (int) ($value / 128);
        while ($value > 0) {
            \array_unshift($bytes, ($value & 0x7F) | 0x80);
            $value = (int) ($value / 128);
        }

        $out = '';
        foreach ($bytes as $byte) {
            $out .= \chr($byte);
        }

        return $out;
    }

    /**
     * Read one DER TLV triplet starting at the given offset.
     *
     * @param int $offset Read cursor; advanced past the parsed element.
     *
     * @return array{tag: int, value: string, raw: string}
     *
     * @throws Exception If the structure or length is malformed.
     */
    public function readTlv(string $data, int &$offset): array
    {
        if ($offset >= \strlen($data)) {
            throw new Exception('Malformed ASN.1 structure');
        }

        $start = $offset;
        $tag = \ord($data[$offset]);
        ++$offset;

        $length = $this->readLength($data, $offset);
        if (($offset + $length) > \strlen($data)) {
            throw new Exception('Malformed ASN.1 length');
        }

        $value = \substr($data, $offset, $length);
        $offset += $length;
        $raw = \substr($data, $start, $offset - $start);

        return ['tag' => $tag, 'value' => $value, 'raw' => $raw];
    }

    /**
     * Read a DER length starting at the given offset.
     *
     * @param int $offset Read cursor; advanced past the length octets.
     *
     * @throws Exception If the length is malformed or unsupported.
     */
    public function readLength(string $data, int &$offset): int
    {
        if ($offset >= \strlen($data)) {
            throw new Exception('Malformed ASN.1 length');
        }

        $first = \ord($data[$offset]);
        ++$offset;
        if (($first & 0x80) === 0) {
            return $first;
        }

        $numBytes = $first & 0x7F;
        if ($numBytes < 1 || $numBytes > 4 || ($offset + $numBytes) > \strlen($data)) {
            throw new Exception('Unsupported ASN.1 length');
        }

        $length = 0;
        for ($idx = 0; $idx < $numBytes; ++$idx) {
            $length = ($length * 256) + \ord($data[$offset + $idx]);
        }

        $offset += $numBytes;
        return $length;
    }

    /**
     * Decode a DER INTEGER content string to a PHP integer.
     *
     * @param string $value Content octets (without tag/length).
     *
     * @throws Exception If the value is empty.
     */
    public function decodeInteger(string $value): int
    {
        if ($value === '') {
            throw new Exception('Invalid ASN.1 integer');
        }

        $int = 0;
        $len = \strlen($value);
        for ($idx = 0; $idx < $len; ++$idx) {
            $int = ($int * 256) + \ord($value[$idx]);
        }

        return $int;
    }
}
