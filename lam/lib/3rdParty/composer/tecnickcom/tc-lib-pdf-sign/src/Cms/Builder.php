<?php

declare(strict_types=1);

/**
 * Builder.php
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
use OpenSSLAsymmetricKey;

/**
 * Com\Tecnick\Pdf\Sign\Cms\Builder
 *
 * Native builder for a detached CAdES-BES CMS SignedData, suitable for a
 * PAdES B-B signature (/SubFilter /ETSI.CAdES.detached). It assembles the
 * SignerInfo with the mandatory signed attributes (content-type,
 * message-digest, signing-time, and the ESS signing-certificate-v2 that plain
 * openssl_pkcs7_sign() cannot add), signs the DER SET OF signed attributes with
 * openssl_sign(), and encodes the ContentInfo. RSA and ECDSA keys are
 * supported with SHA-256/384/512.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Builder
{
    private const OID_SIGNED_DATA = '1.2.840.113549.1.7.2';

    private const OID_DATA = '1.2.840.113549.1.7.1';

    private const OID_CONTENT_TYPE = '1.2.840.113549.1.9.3';

    private const OID_MESSAGE_DIGEST = '1.2.840.113549.1.9.4';

    private const OID_SIGNING_TIME = '1.2.840.113549.1.9.5';

    private const OID_SIGNING_CERTIFICATE_V2 = '1.2.840.113549.1.9.16.2.47';

    private const OID_SIGNATURE_TIMESTAMP = '1.2.840.113549.1.9.16.2.14';

    private const OID_RSA_ENCRYPTION = '1.2.840.113549.1.1.1';

    /**
     * Digest name to [digest OID, openssl algo constant, ecdsa-with-* OID].
     *
     * @var array<string, array{string, int, string}>
     */
    private const DIGESTS = [
        'sha256' => ['2.16.840.1.101.3.4.2.1', OPENSSL_ALGO_SHA256, '1.2.840.10045.4.3.2'],
        'sha384' => ['2.16.840.1.101.3.4.2.2', OPENSSL_ALGO_SHA384, '1.2.840.10045.4.3.3'],
        'sha512' => ['2.16.840.1.101.3.4.2.3', OPENSSL_ALGO_SHA512, '1.2.840.10045.4.3.4'],
    ];

    private Asn1 $asn1;

    public function __construct(?Asn1 $asn1 = null)
    {
        $this->asn1 = $asn1 ?? new Asn1();
    }

    /**
     * Produce a detached CAdES-BES CMS SignedData over the given content.
     *
     * @param string               $data            Detached content bytes (the signed data).
     * @param string               $signerCertDer   DER of the signing certificate.
     * @param OpenSSLAsymmetricKey  $privateKey      Signing private key (RSA or EC).
     * @param list<string>         $chainCertsDer   Additional certificates (DER) to embed.
     * @param string               $digestAlgorithm One of the DIGESTS keys.
     * @param int                  $signingTime     Unix timestamp for the signing-time attribute.
     * @param (callable(string): string)|null $signatureTimestamp Optional provider that receives the
     *                            raw SignerInfo signature bytes and returns a DER-encoded RFC 3161
     *                            timestamp token (ContentInfo). When supplied, the token is embedded as
     *                            the id-aa-signatureTimeStampToken unsigned attribute (PAdES B-T).
     * @param bool                 $includeSigningTime Whether to add the CMS signing-time signed
     *                            attribute. The legacy (ISO 32000-1) profile includes it; PAdES-BASELINE
     *                            forbids it (ETSI EN 319 142-1) and carries the time in the /M signature
     *                            dictionary entry instead.
     *
     * @return string DER-encoded CMS ContentInfo.
     *
     * @throws Exception If the digest or key is unsupported, or signing fails.
     */
    public function sign(
        string $data,
        string $signerCertDer,
        OpenSSLAsymmetricKey $privateKey,
        array $chainCertsDer,
        string $digestAlgorithm,
        int $signingTime,
        ?callable $signatureTimestamp = null,
        bool $includeSigningTime = true,
    ): string {
        [$digestOid, $opensslAlgo, $ecdsaOid] = $this->algorithms($digestAlgorithm);
        [$signatureOid, $signatureHasNullParams] = $this->signatureAlgorithm($privateKey, $ecdsaOid);

        $messageDigest = \hash($digestAlgorithm, $data, true);
        $certHash = \hash($digestAlgorithm, $signerCertDer, true);

        $signedAttributes = $this->signedAttributes(
            $messageDigest,
            $certHash,
            $digestAlgorithm,
            $digestOid,
            $signingTime,
            $includeSigningTime,
        );
        $signedAttributesForSigning = $this->asn1->encodeSet($signedAttributes);

        $signature = '';
        if (!\openssl_sign($signedAttributesForSigning, $signature, $privateKey, $opensslAlgo)) {
            throw new Exception('Unable to sign the CMS signed attributes');
        }

        $unsignedAttributes = $signatureTimestamp === null
            ? ''
            : $this->signatureTimestampAttributes($signatureTimestamp, $signature);

        $signerInfo = $this->asn1->encodeSequence(
            $this->asn1->encodeInteger(1)
            . $this->issuerAndSerialNumber($signerCertDer)
            . $this->algorithmIdentifier($digestOid, false)
            . $this->asn1->encodeContext(0, $signedAttributes)
            . $this->algorithmIdentifier($signatureOid, $signatureHasNullParams)
            . $this->asn1->encodeOctetString($signature)
            . $unsignedAttributes,
        );

        $certificates = $this->asn1->encodeContext(0, $signerCertDer . \implode('', $chainCertsDer));

        $signedData = $this->asn1->encodeSequence(
            $this->asn1->encodeInteger(1)
                . $this->asn1->encodeSet($this->algorithmIdentifier($digestOid, false))
                . $this->asn1->encodeSequence($this->asn1->encodeObjectIdentifier(self::OID_DATA))
                . $certificates
                . $this->asn1->encodeSet($signerInfo),
        );

        return $this->asn1->encodeSequence(
            $this->asn1->encodeObjectIdentifier(self::OID_SIGNED_DATA) . $this->asn1->encodeContext(0, $signedData),
        );
    }

    /**
     * Resolve the OIDs and openssl constant for a digest name.
     *
     * @return array{string, int, string} [digest OID, openssl algo, ecdsa OID]
     *
     * @throws Exception If the digest is unsupported.
     */
    private function algorithms(string $digestAlgorithm): array
    {
        if (!isset(self::DIGESTS[$digestAlgorithm])) {
            throw new Exception('Unsupported digest algorithm: ' . $digestAlgorithm);
        }

        return self::DIGESTS[$digestAlgorithm];
    }

    /**
     * Resolve the signature AlgorithmIdentifier for the signing key.
     *
     * @return array{string, bool} [signature OID, whether NULL parameters are emitted]
     *
     * @throws Exception If the key type is unsupported.
     */
    private function signatureAlgorithm(OpenSSLAsymmetricKey $privateKey, string $ecdsaOid): array
    {
        $details = \openssl_pkey_get_details($privateKey);
        $type = $details !== false ? $details['type'] ?? -1 : -1;

        if ($type === OPENSSL_KEYTYPE_RSA) {
            return [self::OID_RSA_ENCRYPTION, true];
        }

        if ($type === OPENSSL_KEYTYPE_EC) {
            return [$ecdsaOid, false];
        }

        throw new Exception('Unsupported signing key type');
    }

    /**
     * Build the sorted DER SET OF signed attributes content (without the tag).
     *
     * @throws Exception If encoding fails.
     */
    private function signedAttributes(
        string $messageDigest,
        string $certHash,
        string $digestAlgorithm,
        string $digestOid,
        int $signingTime,
        bool $includeSigningTime,
    ): string {
        $attributes = [
            $this->attribute(self::OID_CONTENT_TYPE, $this->asn1->encodeObjectIdentifier(self::OID_DATA)),
            $this->attribute(self::OID_MESSAGE_DIGEST, $this->asn1->encodeOctetString($messageDigest)),
            $this->attribute(self::OID_SIGNING_CERTIFICATE_V2, $this->signingCertificateV2(
                $certHash,
                $digestAlgorithm,
                $digestOid,
            )),
        ];

        // The CMS signing-time attribute belongs to the legacy (ISO 32000-1) profile.
        // PAdES-BASELINE forbids it (ETSI EN 319 142-1): the signing time is carried by
        // the /M entry of the PDF signature dictionary, so validators demote a signature
        // that carries signing-time from PAdES-BASELINE-B to the older PAdES-BES format.
        if ($includeSigningTime) {
            $attributes[] = $this->attribute(self::OID_SIGNING_TIME, $this->encodeTime($signingTime));
        }

        // DER requires the members of a SET OF to be sorted by their encoding,
        // compared as octet strings padded with trailing zero octets.
        \usort($attributes, static function (string $one, string $two): int {
            $length = \max(\strlen($one), \strlen($two));
            return \strcmp(\str_pad($one, $length, "\x00"), \str_pad($two, $length, "\x00"));
        });

        return \implode('', $attributes);
    }

    /**
     * Build the SignerInfo [1] IMPLICIT unsignedAttrs carrying the signature
     * timestamp.
     *
     * The provider computes an RFC 3161 token over the raw signature bytes
     * (CAdES id-aa-signatureTimeStampToken), which is then wrapped as a single
     * unsigned Attribute value.
     *
     * @param callable(string): string $provider  Maps the signature bytes to a DER token.
     * @param string                   $signature Raw SignerInfo signature bytes.
     *
     * @throws Exception If the provider yields an empty or non-string token, or encoding fails.
     */
    private function signatureTimestampAttributes(callable $provider, string $signature): string
    {
        /** @var mixed $token */
        $token = $provider($signature);
        if (!\is_string($token) || $token === '') {
            throw new Exception('Invalid signature timestamp token');
        }

        return $this->asn1->encodeContext(1, $this->attribute(self::OID_SIGNATURE_TIMESTAMP, $token));
    }

    /**
     * Encode a single Attribute (type plus a one-element value SET).
     *
     * @throws Exception If encoding fails.
     */
    private function attribute(string $oid, string $value): string
    {
        return $this->asn1->encodeSequence($this->asn1->encodeObjectIdentifier($oid) . $this->asn1->encodeSet($value));
    }

    /**
     * Encode the SigningCertificateV2 attribute value.
     *
     * The ESSCertIDv2 hashAlgorithm defaults to SHA-256, so it is omitted when
     * the digest is SHA-256 and included otherwise (DER default handling).
     *
     * @throws Exception If encoding fails.
     */
    private function signingCertificateV2(string $certHash, string $digestAlgorithm, string $digestOid): string
    {
        $essCertId = '';
        if ($digestAlgorithm !== 'sha256') {
            $essCertId .= $this->algorithmIdentifier($digestOid, false);
        }

        $essCertId .= $this->asn1->encodeOctetString($certHash);

        return $this->asn1->encodeSequence($this->asn1->encodeSequence($this->asn1->encodeSequence($essCertId)));
    }

    /**
     * Encode an AlgorithmIdentifier, with optional NULL parameters.
     *
     * @throws Exception If encoding fails.
     */
    private function algorithmIdentifier(string $oid, bool $nullParameters): string
    {
        $parameters = $nullParameters ? $this->asn1->encodeNull() : '';
        return $this->asn1->encodeSequence($this->asn1->encodeObjectIdentifier($oid) . $parameters);
    }

    /**
     * Encode the signing-time value as UTCTime (1950-2049) or GeneralizedTime.
     *
     * @throws Exception If encoding fails.
     */
    private function encodeTime(int $signingTime): string
    {
        $year = (int) \gmdate('Y', $signingTime);
        if ($year >= 1950 && $year < 2050) {
            $value = \gmdate('ymdHis', $signingTime) . 'Z';
            return "\x17" . $this->asn1->encodeLength(\strlen($value)) . $value;
        }

        $value = \gmdate('YmdHis', $signingTime) . 'Z';
        return "\x18" . $this->asn1->encodeLength(\strlen($value)) . $value;
    }

    /**
     * Build the IssuerAndSerialNumber from the signer certificate.
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    private function issuerAndSerialNumber(string $certDer): string
    {
        $certOff = 0;
        $certTlv = $this->asn1->readTlv($certDer, $certOff);
        $tbsOff = 0;
        $tbsTlv = $this->asn1->readTlv($certTlv['value'], $tbsOff);
        $tbs = $tbsTlv['value'];

        $off = 0;
        if ($off < \strlen($tbs) && (\ord($tbs[$off]) & 0xE0) === 0xA0) {
            $this->asn1->readTlv($tbs, $off); // version [0]
        }

        $serial = $this->asn1->readTlv($tbs, $off); // serialNumber
        $this->asn1->readTlv($tbs, $off); // signature AlgorithmIdentifier
        $issuer = $this->asn1->readTlv($tbs, $off); // issuer Name

        return $this->asn1->encodeSequence($issuer['raw'] . $serial['raw']);
    }
}
