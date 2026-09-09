<?php

declare(strict_types=1);

/**
 * SigningRequest.php
 *
 * @since     2026-08-24
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

use Com\Tecnick\Pdf\Sign\DigestAlgorithm;
use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Cms\SigningRequest
 *
 * Validated, immutable record of everything the CMS signed attributes are derived
 * from. It is what crosses the boundary of a two-phase signature:
 * Builder::signaturePayload() turns it into the bytes a signer has to sign, and
 * Builder::buildFromSignature() rebuilds the same attributes from it once the
 * signature comes back.
 *
 * Every invariant is enforced by the constructor, and toArray()/fromArray()
 * round-trip through it, so a request rehydrated from a session, a queue, or a
 * second HTTP request is validated again. Validation is not authentication: pass a
 * key to that pair, or protect the channel, to catch a request edited into a
 * different valid one.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class SigningRequest
{
    /**
     * Latest signing time a DER Time value can carry: 9999-12-31T23:59:59Z.
     */
    public const MAX_SIGNING_TIME = 253_402_300_799;

    /**
     * Selected CMS digest algorithm (one of the DigestAlgorithm backing values).
     */
    public readonly string $digestAlgorithm;

    /**
     * Additional signed attributes, keyed by attribute type OID.
     *
     * @var array<string, string>
     */
    public readonly array $extraSignedAttributes;

    /**
     * @param string       $messageDigest      Digest of the detached content, raw bytes, computed
     *                                         with $digestAlgorithm. A caller that cannot hold the
     *                                         content in memory computes it with hash_update_stream().
     * @param string       $signerCertDer      DER of the signing certificate.
     * @param string|DigestAlgorithm $digestAlgorithm Digest algorithm name or enum case.
     * @param int          $signingTime        Unix timestamp for the signing-time attribute.
     * @param bool         $includeSigningTime Whether to add the CMS signing-time signed attribute.
     *                                         The legacy (ISO 32000-1) profile includes it;
     *                                         PAdES-BASELINE forbids it (ETSI EN 319 142-1).
     * @param array<array-key, string> $extraSignedAttributes Additional signed attributes as
     *                                         OID => DER-encoded attribute value, for a profile that
     *                                         requires one such as the CAdES
     *                                         signature-policy-identifier.
     *
     * @throws Exception If the digest, the certificate, the signing time, or an extra attribute is invalid.
     */
    public function __construct(
        public readonly string $messageDigest,
        public readonly string $signerCertDer,
        string|DigestAlgorithm $digestAlgorithm = 'sha256',
        public readonly int $signingTime = 0,
        public readonly bool $includeSigningTime = true,
        array $extraSignedAttributes = [],
    ) {
        $algorithm = DigestAlgorithm::fromLoose($digestAlgorithm);
        $this->digestAlgorithm = $algorithm->value;

        // A digest of the wrong length is a digest of something else.
        if (\strlen($messageDigest) !== $algorithm->digestLength()) {
            throw new Exception('The message digest length does not match ' . $algorithm->value);
        }

        $asn1 = new Asn1();

        // openssl_pkey_get_public() reads the first DER element and ignores whatever
        // follows it, so trailing bytes are refused here.
        $asn1->assertSingleElement($signerCertDer, 0x30, 'signer certificate');

        if (\openssl_pkey_get_public($this->signerCertPem()) === false) {
            // Reported as an Exception, so the OpenSSL queue entries are discarded
            // rather than left for the host.
            Certificate::clearOpenSslErrors();
            throw new Exception('Unreadable signer certificate');
        }

        // The parse every chain certificate goes through in Certificate::toDer(), run
        // at construction rather than at Builder::buildFromSignature(), so a refusal
        // lands before an external signer has spent a signature on the payload. It
        // applies the RFC 5280 section 4.1.1.2 rule that the inner and outer signature
        // algorithms agree, which OpenSSL does not weigh at read time.
        try {
            (new Certificate($asn1))->fields($signerCertDer);
        } catch (Exception $e) {
            throw new Exception('Invalid signer certificate', 0, $e);
        }

        // The upper bound is 9999-12-31T23:59:59Z, the last instant a DER
        // GeneralizedTime can express (X.690 section 11.7 fixes the year at four
        // digits). Past it gmdate() emits a five-digit year.
        if ($signingTime < 0 || $signingTime > self::MAX_SIGNING_TIME) {
            throw new Exception('Invalid signing time: ' . $signingTime);
        }

        $this->extraSignedAttributes = $this->validExtraAttributes($asn1, $extraSignedAttributes);
    }

    /**
     * The signing certificate wrapped as PEM, which is what the OpenSSL functions read.
     */
    public function signerCertPem(): string
    {
        return Certificate::derToPem($this->signerCertDer);
    }

    /**
     * Export the request as a JSON-safe array, with the binary fields base64-encoded.
     *
     * Pass $key to add a keyed MAC over the exported fields. Without one the export
     * carries no integrity protection: the constructor re-runs on the way back in,
     * so a malformed payload is rejected, but a payload edited into another
     * well-formed one is not.
     *
     * @param string|null $key Secret for the HMAC-SHA256, or null to export unprotected.
     *
     * @return array{
     *           message_digest: string,
     *           signer_cert: string,
     *           digest_algorithm: string,
     *           signing_time: int,
     *           include_signing_time: bool,
     *           extra_signed_attributes: array<string, string>,
     *           mac?: string,
     *         }
     *
     * @throws Exception If the key is empty.
     */
    public function toArray(#[\SensitiveParameter] ?string $key = null): array
    {
        $extra = [];
        foreach ($this->extraSignedAttributes as $oid => $value) {
            $extra[$oid] = \base64_encode($value);
        }

        $state = [
            'message_digest' => \base64_encode($this->messageDigest),
            'signer_cert' => \base64_encode($this->signerCertDer),
            'digest_algorithm' => $this->digestAlgorithm,
            'signing_time' => $this->signingTime,
            'include_signing_time' => $this->includeSigningTime,
            'extra_signed_attributes' => $extra,
        ];

        if ($key === null) {
            return $state;
        }

        return [...$state, 'mac' => self::mac($state, $key)];
    }

    /**
     * Rebuild a request from the array produced by toArray().
     *
     * The full constructor runs again, so a payload that is not a valid request is
     * rejected. Pass the $key given to toArray() and the MAC is verified
     * first, which rejects a payload edited into a different valid request.
     *
     * @param array<array-key, mixed> $state Exported state.
     * @param string|null             $key   The secret passed to toArray(), or null when
     *                                       the state was exported unprotected.
     *
     * @throws Exception If the MAC is missing or wrong, or a field is missing, malformed,
     *                   or fails validation.
     */
    public static function fromArray(array $state, #[\SensitiveParameter] ?string $key = null): self
    {
        if ($key !== null) {
            self::verifyMac($state, $key);
        }

        return new self(
            self::binaryField($state, 'message_digest'),
            self::binaryField($state, 'signer_cert'),
            self::stringField($state, 'digest_algorithm'),
            self::intField($state, 'signing_time'),
            self::boolField($state, 'include_signing_time'),
            self::extraAttributesField($state),
        );
    }

    /**
     * Compute the MAC over the exported fields.
     *
     * The fields are sorted and JSON-encoded so the same request always yields the
     * same input, whatever order the transport put the keys in. The attribute map is
     * sorted too.
     *
     * Both sorts are SORT_STRING. Under the default SORT_REGULAR, PHP compares two
     * numeric strings as numbers, and a two-arc OID key is one: "1.2" and "1.20"
     * would compare equal and leave the order to insertion.
     *
     * @param array<array-key, mixed> $state Exported state, without the mac field.
     *
     * @throws Exception If the key is empty or the state cannot be encoded.
     */
    private static function mac(array $state, #[\SensitiveParameter] string $key): string
    {
        if ($key === '') {
            throw new Exception('The signing request MAC key is empty');
        }

        unset($state['mac']);

        /** @var mixed $extra */
        $extra = $state['extra_signed_attributes'] ?? null;
        if (\is_array($extra)) {
            \ksort($extra, SORT_STRING);
            $state['extra_signed_attributes'] = $extra;
        }

        \ksort($state, SORT_STRING);

        // verifyMac() computes the MAC over the payload as received, so every value
        // here is the transport's. json_encode() refuses a string that is not UTF-8
        // and a float that is NAN or INF, and a JsonException is translated rather
        // than left to escape.
        try {
            $encoded = \json_encode($state, JSON_THROW_ON_ERROR | JSON_UNESCAPED_SLASHES);
        } catch (\JsonException $e) {
            throw new Exception('The signing request state cannot be encoded', 0, $e);
        }

        return \hash_hmac('sha256', $encoded, $key);
    }

    /**
     * Check the MAC a protected export carries.
     *
     * @param array<array-key, mixed> $state Exported state.
     *
     * @throws Exception If the MAC is absent or does not match.
     */
    private static function verifyMac(array $state, #[\SensitiveParameter] string $key): void
    {
        /** @var mixed $mac */
        $mac = $state['mac'] ?? null;
        if (!\is_string($mac) || !\hash_equals(self::mac($state, $key), $mac)) {
            throw new Exception('The signing request was altered or signed with another key');
        }
    }

    /**
     * Validate the caller-supplied signed attributes.
     *
     * The OID is validated by encoding it, so an arc the encoder rejects is refused
     * at construction rather than at the first signaturePayload() call.
     *
     * @param array<array-key, string> $extraSignedAttributes
     *
     * @return array<string, string>
     *
     * @throws Exception If an OID is malformed or reserved, or a value is not a single DER element.
     */
    private function validExtraAttributes(Asn1 $asn1, array $extraSignedAttributes): array
    {
        $validated = [];
        /** @var mixed $value */
        foreach ($extraSignedAttributes as $key => $value) {
            $oid = (string) $key;

            // Held here as well as in fromArray(): a value that is not a string would
            // reach readTlv() as a TypeError rather than an Exception.
            if (!\is_string($value)) {
                throw new Exception('Invalid signed attribute value: ' . $oid);
            }

            try {
                $asn1->encodeObjectIdentifier($oid);
            } catch (Exception $e) {
                throw new Exception('Invalid signed attribute OID: ' . $oid, 0, $e);
            }

            // RFC 5652 section 5.3 allows at most one instance of each attribute type
            // in SignedAttributes, so the types the builder emits are reserved.
            if (\in_array($oid, Oid::BUILDER_ATTRIBUTES, true)) {
                throw new Exception('Reserved signed attribute OID: ' . $oid);
            }

            $offset = 0;
            $asn1->readTlv($value, $offset);
            if ($offset !== \strlen($value)) {
                throw new Exception('Signed attribute value is not a single DER element: ' . $oid);
            }

            $validated[$oid] = $value;
        }

        return $validated;
    }

    /**
     * Read a base64-encoded binary field.
     *
     * @param array<array-key, mixed> $state
     *
     * @throws Exception If the field is missing or not valid base64.
     */
    private static function binaryField(array $state, string $key): string
    {
        $decoded = \base64_decode(self::stringField($state, $key), true);
        if ($decoded === false) {
            throw new Exception('Field is not valid base64: ' . $key);
        }

        return $decoded;
    }

    /**
     * Read a string field.
     *
     * @param array<array-key, mixed> $state
     *
     * @throws Exception If the field is missing or not a string.
     */
    private static function stringField(array $state, string $key): string
    {
        /** @var mixed $value */
        $value = $state[$key] ?? null;
        if (!\is_string($value)) {
            throw new Exception('Missing or invalid string field: ' . $key);
        }

        return $value;
    }

    /**
     * Read an integer field.
     *
     * @param array<array-key, mixed> $state
     *
     * @throws Exception If the field is missing or not an integer.
     */
    private static function intField(array $state, string $key): int
    {
        /** @var mixed $value */
        $value = $state[$key] ?? null;
        if (!\is_int($value)) {
            throw new Exception('Missing or invalid integer field: ' . $key);
        }

        return $value;
    }

    /**
     * Read a boolean field.
     *
     * @param array<array-key, mixed> $state
     *
     * @throws Exception If the field is missing or not a boolean.
     */
    private static function boolField(array $state, string $key): bool
    {
        /** @var mixed $value */
        $value = $state[$key] ?? null;
        if (!\is_bool($value)) {
            throw new Exception('Missing or invalid boolean field: ' . $key);
        }

        return $value;
    }

    /**
     * Read and decode the extra signed attributes map.
     *
     * @param array<array-key, mixed> $state
     *
     * @return array<string, string>
     *
     * @throws Exception If the map or one of its values is malformed.
     */
    private static function extraAttributesField(array $state): array
    {
        /** @var mixed $value */
        $value = $state['extra_signed_attributes'] ?? [];
        if (!\is_array($value)) {
            throw new Exception('Missing or invalid extra_signed_attributes field');
        }

        $extra = [];
        /** @var mixed $encoded */
        foreach ($value as $key => $encoded) {
            $oid = (string) $key;
            if (!\is_string($encoded)) {
                throw new Exception('Invalid signed attribute value: ' . $oid);
            }

            $decoded = \base64_decode($encoded, true);
            if ($decoded === false) {
                throw new Exception('Signed attribute value is not valid base64: ' . $oid);
            }

            $extra[$oid] = $decoded;
        }

        return $extra;
    }
}
