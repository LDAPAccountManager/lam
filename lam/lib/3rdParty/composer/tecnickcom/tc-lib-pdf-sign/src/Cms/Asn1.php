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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class Asn1
{
    /**
     * Encode a DER length octet sequence.
     *
     * @param int $length Number of content octets; must not be negative.
     *
     * @throws Exception If the length is negative or too large to encode.
     */
    public function encodeLength(int $length): string
    {
        if ($length < 0) {
            throw new Exception('Negative ASN.1 length: ' . $length);
        }

        if ($length < 128) {
            return \chr($length);
        }

        // Shifted rather than divided: / is float division in PHP and loses the
        // low bits above 2^53.
        $encoded = '';
        $value = $length;
        while ($value > 0) {
            $encoded = \chr($value & 0xFF) . $encoded;
            $value >>= 8;
        }

        $encodedLength = \strlen($encoded);
        if ($encodedLength > 0x7F) {
            // Unreachable: requires content larger than 2^1016 bytes.
            throw new Exception('ASN.1 length encoding overflow');
        }

        return \chr(0x80 | $encodedLength) . $encoded;
    }

    /**
     * Encode a non-negative integer as a DER INTEGER.
     *
     * @param int $value Integer value; must not be negative.
     *
     * @throws Exception If the value is negative or the length cannot be encoded.
     */
    public function encodeInteger(int $value): string
    {
        if ($value < 0) {
            throw new Exception('Negative ASN.1 integer: ' . $value);
        }

        // Shifted rather than divided, as in encodeLength().
        $data = '';
        $num = $value;
        while ($num > 0) {
            $data = \chr($num & 0xFF) . $data;
            $num >>= 8;
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
     * most significant bit is set, so the value stays non-negative.
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
     * The multi-octet tag form (X.690 section 8.1.2.4) is not emitted, so tag
     * numbers of 31 and above are rejected.
     *
     * @param int $number Context tag number; must be 0..30.
     *
     * @throws Exception If the tag number is out of range or the length cannot be encoded.
     */
    public function encodeContext(int $number, string $value): string
    {
        if ($number < 0 || $number > 30) {
            throw new Exception('Unsupported context tag number: ' . $number);
        }

        return \chr(0xA0 | $number) . $this->encodeLength(\strlen($value)) . $value;
    }

    /**
     * Encode a dotted OID string as a DER OBJECT IDENTIFIER.
     *
     * The first two arcs share one subidentifier with the value 40*arc0 + arc1,
     * itself base-128 encoded (X.690 sections 8.19.2 and 8.19.4). The root arc is
     * limited to 0..2, and the second arc to 0..39 under roots 0 and 1.
     *
     * @throws Exception If the OID is malformed or the length cannot be encoded.
     */
    public function encodeObjectIdentifier(string $oid): string
    {
        $parts = \explode('.', $oid);
        if (\count($parts) < 2) {
            throw new Exception('Invalid OID: ' . $oid);
        }

        $arcs = [];
        foreach ($parts as $part) {
            if (\preg_match('/^(?:0|[1-9][0-9]*)$/', $part) !== 1) {
                throw new Exception('Invalid OID arc "' . $part . '" in: ' . $oid);
            }

            $arc = (int) $part;
            if ((string) $arc !== $part) {
                throw new Exception('OID arc out of range "' . $part . '" in: ' . $oid);
            }

            $arcs[] = $arc;
        }

        // The count check above guarantees both arcs are present.
        $root = (int) \array_shift($arcs);
        $second = (int) \array_shift($arcs);

        if ($root > 2) {
            throw new Exception('Invalid OID root arc in: ' . $oid);
        }

        if ($root < 2 && $second > 39) {
            throw new Exception('Invalid OID second arc in: ' . $oid);
        }

        // X.690 section 8.19.4 combines the first two arcs into one subidentifier.
        // Under root 2 the second arc is unbounded, so the sum is checked against
        // PHP_INT_MAX before it overflows to a float.
        if ($second > (PHP_INT_MAX - ($root * 40))) {
            throw new Exception('OID second arc out of range "' . $second . '" in: ' . $oid);
        }

        $data = $this->encodeBase128Int(($root * 40) + $second);
        foreach ($arcs as $arc) {
            $data .= $this->encodeBase128Int($arc);
        }

        return "\x06" . $this->encodeLength(\strlen($data)) . $data;
    }

    /**
     * Encode a non-negative integer in base-128 with continuation bits.
     *
     * @param int $value Integer value; must not be negative.
     *
     * @throws Exception If the value is negative.
     */
    public function encodeBase128Int(int $value): string
    {
        if ($value < 0) {
            throw new Exception('Negative base-128 integer: ' . $value);
        }

        // Shifted rather than divided, as in encodeLength().
        $bytes = [$value & 0x7F];
        $value >>= 7;
        while ($value > 0) {
            \array_unshift($bytes, ($value & 0x7F) | 0x80);
            $value >>= 7;
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

        // X.690 section 8.1.2.4: a low tag number of 31 means the tag continues in
        // the following octets. The multi-octet form is not supported.
        if (($tag & 0x1F) === 0x1F) {
            throw new Exception('Unsupported ASN.1 high tag number form');
        }

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
     * Read one DER TLV triplet, or null when the cursor is at the end of the data.
     *
     * @param int $offset Read cursor; advanced past the parsed element.
     *
     * @return array{tag: int, value: string, raw: string}|null
     *
     * @throws Exception If the structure or length is malformed.
     */
    public function readOptionalTlv(string $data, int &$offset): ?array
    {
        return $offset < \strlen($data) ? $this->readTlv($data, $offset) : null;
    }

    /**
     * Read a string that has to be exactly one complete DER element of the given tag.
     *
     * @param int    $tag   Expected identifier octet.
     * @param string $label Name of the value, for the error message.
     *
     * @return array{tag: int, value: string, raw: string}
     *
     * @throws Exception If the value is empty, truncated, trailed, or of another tag.
     */
    public function readSingleElement(string $value, int $tag, string $label): array
    {
        if ($value === '') {
            throw new Exception('Empty ' . $label);
        }

        $offset = 0;
        $element = $this->readTlv($value, $offset);
        if ($element['tag'] !== $tag || $offset !== \strlen($value)) {
            throw new Exception('Invalid DER for ' . $label);
        }

        return $element;
    }

    /**
     * Assert that a string is exactly one complete DER element of the given tag.
     *
     * @param int    $tag   Expected identifier octet.
     * @param string $label Name of the value, for the error message.
     *
     * @throws Exception If the value is empty, truncated, trailed, or of another tag.
     */
    public function assertSingleElement(string $value, int $tag, string $label): void
    {
        $this->readSingleElement($value, $tag, $label);
    }

    /**
     * Decode an X.509 Extensions SEQUENCE into an OID to value-and-criticality map.
     *
     * The shape is the one RFC 5280 section 4.1 defines: a SEQUENCE of SEQUENCE
     * { extnID OBJECT IDENTIFIER, critical BOOLEAN DEFAULT FALSE, extnValue OCTET
     * STRING }.
     *
     * The input has to be exactly one Extensions SEQUENCE with no trailing bytes.
     * An OID that appears twice is refused: RFC 5280 sections 4.2 and 5.2 admit at
     * most one instance of each type.
     *
     * @param string $extensionsDer Complete DER of the Extensions SEQUENCE, or '' when
     *                              the field is absent.
     * @param string $label         Name of the field, for the error messages.
     *
     * @return array<string, array{critical: bool, value: string}>
     *
     * @throws Exception If the structure is malformed, trailed, or an OID appears twice.
     */
    public function decodeExtensions(string $extensionsDer, string $label): array
    {
        if ($extensionsDer === '') {
            return [];
        }

        $offset = 0;
        $sequence = $this->readTlv($extensionsDer, $offset);
        if ($sequence['tag'] !== 0x30) {
            throw new Exception('Invalid ' . $label . 's');
        }

        if ($offset !== \strlen($extensionsDer)) {
            throw new Exception('Trailing bytes after the ' . $label . 's');
        }

        $found = [];
        $inner = 0;
        while ($inner < \strlen($sequence['value'])) {
            $extension = $this->readTlv($sequence['value'], $inner);
            if ($extension['tag'] !== 0x30) {
                throw new Exception('Invalid ' . $label);
            }

            $fieldOffset = 0;
            $oid = $this->readTlv($extension['value'], $fieldOffset);
            if ($oid['tag'] !== 0x06) {
                throw new Exception('Invalid ' . $label);
            }

            $field = $this->readTlv($extension['value'], $fieldOffset);

            // critical BOOLEAN DEFAULT FALSE precedes the value when present. A
            // BOOLEAN is one content octet (X.690 section 8.2.1); any other length
            // is refused. Any non-zero octet reads as TRUE, since encoders in the
            // wild emit 0x01 where DER fixes 0xFF.
            $critical = false;
            if ($field['tag'] === 0x01) {
                if (\strlen($field['value']) !== 1) {
                    throw new Exception('Invalid ' . $label . ' critical flag');
                }

                $critical = $field['value'] !== "\x00";
                $field = $this->readTlv($extension['value'], $fieldOffset);
            }

            if ($field['tag'] !== 0x04) {
                throw new Exception('Invalid ' . $label);
            }

            // extnValue is the last field, so nothing may follow it.
            if ($fieldOffset !== \strlen($extension['value'])) {
                throw new Exception('Trailing bytes in ' . $label);
            }

            $name = $this->decodeObjectIdentifier($oid['value']);
            if (isset($found[$name])) {
                throw new Exception('Duplicate ' . $label . ': ' . $name);
            }

            $found[$name] = ['critical' => $critical, 'value' => $field['value']];
        }

        return $found;
    }

    /**
     * Decode an X.509 AlgorithmIdentifier to the dotted form of its OID.
     *
     * RFC 5280 section 4.1.1.2 shapes it as SEQUENCE { algorithm OBJECT IDENTIFIER,
     * parameters ANY DEFINED BY algorithm OPTIONAL }, so one element may follow the
     * OID and nothing may follow that. Both layers are bounded here rather than in
     * each reader.
     *
     * @param string $algorithmIdDer Complete DER of the AlgorithmIdentifier.
     * @param string $label          Name of the field, for the error messages.
     *
     * @throws Exception If the structure is malformed, trailed, or names no OID.
     */
    public function decodeAlgorithmIdentifier(string $algorithmIdDer, string $label): string
    {
        $offset = 0;
        $algorithmId = $this->readTlv($algorithmIdDer, $offset);
        if ($algorithmId['tag'] !== 0x30 || $offset !== \strlen($algorithmIdDer)) {
            throw new Exception('Invalid ' . $label . ' AlgorithmIdentifier');
        }

        $inner = 0;
        $oid = $this->readTlv($algorithmId['value'], $inner);
        if ($oid['tag'] !== 0x06) {
            throw new Exception('Invalid ' . $label . ' AlgorithmIdentifier');
        }

        // parameters is the one field that may follow, and every algorithm accepted
        // here takes it absent or NULL: RFC 3279 section 2.2.1 fixes NULL for the
        // PKCS #1 v1.5 identifiers, RFC 5758 section 3.2 requires it absent for the
        // ECDSA ones, and RFC 5754 section 2 admits either for a digest.
        $parameters = $this->readOptionalTlv($algorithmId['value'], $inner);
        if ($parameters !== null && $parameters['raw'] !== $this->encodeNull()) {
            throw new Exception('Unsupported ' . $label . ' AlgorithmIdentifier parameters');
        }

        if ($inner !== \strlen($algorithmId['value'])) {
            throw new Exception('Trailing bytes in the ' . $label . ' AlgorithmIdentifier');
        }

        return $this->decodeObjectIdentifier($oid['value']);
    }

    /**
     * Read a DER length starting at the given offset.
     *
     * The indefinite form and non-minimal long forms are rejected: DER requires
     * the definite form with the fewest possible octets (X.690 section 10.1). The
     * octet count is also capped so the accumulated length always fits a PHP
     * integer, which on a 32-bit build is narrower than the 4-octet DER maximum.
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

        // 4 octets is the supported cap. The accumulated value is bounded as it is
        // built, since on a 32-bit build a 4-octet length can exceed PHP_INT_MAX.
        $numBytes = $first & 0x7F;
        if ($numBytes < 1 || $numBytes > 4 || ($offset + $numBytes) > \strlen($data)) {
            throw new Exception('Unsupported ASN.1 length');
        }

        if (\ord($data[$offset]) === 0) {
            throw new Exception('Non-minimal ASN.1 length encoding');
        }

        $length = 0;
        for ($idx = 0; $idx < $numBytes; ++$idx) {
            // Unreachable on a 64-bit build; on a 32-bit one a 4-octet length can
            // exceed PHP_INT_MAX and promote to a float.
            if ($length > (PHP_INT_MAX >> 8)) {
                throw new Exception('Unsupported ASN.1 length');
            }

            $length = ($length << 8) + \ord($data[$offset + $idx]);
        }

        if ($length < 128) {
            throw new Exception('Non-minimal ASN.1 length encoding');
        }

        $offset += $numBytes;
        return $length;
    }

    /**
     * Assert that a DER INTEGER content string is minimally encoded.
     *
     * The minimality half of decodeInteger(), for fields carrying an integer too
     * wide to decode, such as a certificate serial number of up to 20 octets
     * (RFC 5280 section 4.1.2.2).
     *
     * @param string $value Content octets (without tag/length).
     *
     * @throws Exception If the value is empty or non-minimally encoded.
     */
    public function assertMinimalInteger(string $value): void
    {
        $len = \strlen($value);
        if ($len === 0) {
            throw new Exception('Invalid ASN.1 integer');
        }

        // X.690 section 8.3.2: the first nine bits must not be all zeros or all ones.
        if ($len === 1) {
            return;
        }

        $lead = \ord($value[0]);
        $next = \ord($value[1]);
        if ($lead === 0x00 && ($next & 0x80) === 0 || $lead === 0xFF && ($next & 0x80) !== 0) {
            throw new Exception('Non-minimal ASN.1 integer encoding');
        }
    }

    /**
     * Decode a DER INTEGER content string to a PHP integer.
     *
     * The content octets are two's complement (X.690 section 8.3), so the sign
     * bit is honoured. A value too wide for a PHP integer is rejected.
     *
     * @param string $value Content octets (without tag/length).
     *
     * @throws Exception If the value is empty, non-minimal, or out of range.
     */
    public function decodeInteger(string $value): int
    {
        $this->assertMinimalInteger($value);

        $len = \strlen($value);
        if ($len > PHP_INT_SIZE) {
            throw new Exception('ASN.1 integer out of range');
        }

        $int = (\ord($value[0]) & 0x80) !== 0 ? -1 : 0;
        for ($idx = 0; $idx < $len; ++$idx) {
            $int = ($int << 8) | \ord($value[$idx]);
        }

        return $int;
    }

    /**
     * Decode a DER GeneralizedTime content string to a Unix timestamp.
     *
     * The seconds must be present and the zone must be Z (X.690 section 11.7).
     * The fractional part is refused unless the caller opts in; when accepted it
     * must hold at least one digit and no trailing zero (X.690 section 11.7), and
     * is dropped once validated.
     *
     * Every field is range-checked by re-encoding the result and comparing it with
     * the input, since gmmktime() wraps an out-of-range field rather than failing.
     *
     * @param string $value         Content octets (without tag/length).
     * @param bool   $allowFraction Accept a fraction-of-second part, admitted by
     *                              RFC 3161 section 2.4.2 for a token's genTime.
     *
     * @throws Exception If the value is not a DER GeneralizedTime.
     */
    public function decodeGeneralizedTime(string $value, bool $allowFraction = false): int
    {
        $seconds = $value;
        if ($allowFraction && \preg_match('/^\d{14}\.\d*[1-9]Z\z/', $value) === 1) {
            $seconds = \substr($value, 0, 14) . 'Z';
        }

        // \z rather than $, which PCRE matches before a final newline.
        if (\preg_match('/^\d{14}Z\z/', $seconds) !== 1) {
            throw new Exception('Invalid ASN.1 GeneralizedTime');
        }

        $time = \gmmktime(
            (int) \substr($seconds, 8, 2),
            (int) \substr($seconds, 10, 2),
            (int) \substr($seconds, 12, 2),
            (int) \substr($seconds, 4, 2),
            (int) \substr($seconds, 6, 2),
            (int) \substr($seconds, 0, 4),
        );

        if ($time === false) {
            // Unreachable: the pattern constrains every field to two or four digits.
            throw new Exception('Invalid ASN.1 GeneralizedTime');
        }

        if (\gmdate('YmdHis', $time) . 'Z' !== $seconds) {
            throw new Exception('Out-of-range ASN.1 GeneralizedTime: ' . $value);
        }

        return $time;
    }

    /**
     * Decode a DER UTCTime content string to a Unix timestamp.
     *
     * DER requires the YYMMDDHHMMSSZ form (X.690 section 11.8). The two-digit
     * year is read as 1950-2049, per RFC 5280 section 4.1.2.5.1.
     *
     * @param string $value Content octets (without tag/length).
     *
     * @throws Exception If the value is not a DER UTCTime.
     */
    public function decodeUtcTime(string $value): int
    {
        // \z rather than $, as decodeGeneralizedTime() explains.
        if (\preg_match('/^\d{12}Z\z/', $value) !== 1) {
            throw new Exception('Invalid ASN.1 UTCTime');
        }

        $year = (int) \substr($value, 0, 2);
        $century = $year >= 50 ? '19' : '20';

        return $this->decodeGeneralizedTime($century . $value);
    }

    /**
     * Decode a DER Time CHOICE element to a Unix timestamp.
     *
     * X.509 carries validity and revocation instants as a CHOICE of UTCTime and
     * GeneralizedTime, so a reader has to accept whichever the issuer used.
     *
     * @param array{tag: int, value: string, raw: string} $element Parsed TLV.
     *
     * @throws Exception If the element is neither a UTCTime nor a GeneralizedTime.
     */
    public function decodeTime(array $element): int
    {
        return match ($element['tag']) {
            0x17 => $this->decodeUtcTime($element['value']),
            0x18 => $this->decodeGeneralizedTime($element['value']),
            default => throw new Exception('Invalid ASN.1 Time'),
        };
    }

    /**
     * Read the octets of a DER BIT STRING element, without the unused-bits count.
     *
     * Every BIT STRING read here holds whole octets (a signature, a public key),
     * so a non-zero unused-bits count is refused.
     *
     * @param array{tag: int, value: string, raw: string} $element Parsed TLV.
     *
     * @throws Exception If the element is not a BIT STRING of whole octets.
     */
    public function decodeBitString(array $element): string
    {
        if ($element['tag'] !== 0x03 || $element['value'] === '' || $element['value'][0] !== "\x00") {
            throw new Exception('Invalid ASN.1 BIT STRING');
        }

        return \substr($element['value'], 1);
    }

    /**
     * Decode a DER OBJECT IDENTIFIER content string to its dotted form.
     *
     * The inverse of encodeObjectIdentifier(): the first subidentifier carries
     * both leading arcs (X.690 section 8.19.4), and the rest are base-128 with
     * continuation bits.
     *
     * @param string $value Content octets (without tag/length).
     *
     * @throws Exception If the value is empty, truncated, or non-minimally encoded.
     */
    public function decodeObjectIdentifier(string $value): string
    {
        $len = \strlen($value);
        if ($len === 0) {
            throw new Exception('Invalid ASN.1 OID');
        }

        $arcs = [];
        $arc = 0;
        $started = false;
        for ($idx = 0; $idx < $len; ++$idx) {
            $byte = \ord($value[$idx]);

            // X.690 section 8.19.2: a subidentifier may not begin with 0x80, which
            // would be a leading zero in base 128.
            if (!$started && $byte === 0x80) {
                throw new Exception('Non-minimal ASN.1 OID encoding');
            }

            if ($arc > (PHP_INT_MAX >> 7)) {
                throw new Exception('ASN.1 OID arc out of range');
            }

            $arc = ($arc << 7) | ($byte & 0x7F);
            $started = true;
            if (($byte & 0x80) === 0) {
                $arcs[] = $arc;
                $arc = 0;
                $started = false;
            }
        }

        if ($started) {
            throw new Exception('Truncated ASN.1 OID');
        }

        $first = (int) \array_shift($arcs);
        $root = \intdiv($first, 40);
        $second = $first % 40;
        if ($root > 2) {
            $root = 2;
            $second = $first - 80;
        }

        return \implode('.', [$root, $second, ...$arcs]);
    }
}
