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

use Com\Tecnick\Pdf\Sign\DigestAlgorithm;
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
 * sign() covers the case where the private key is available in this process.
 * When it is not, or when the content is too large to hold as a string, the two
 * halves are available on their own: signaturePayload() returns the bytes a signer
 * has to sign for a given SigningRequest, and buildFromSignature() turns those plus the
 * signature into the CMS. sign() is implemented over both.
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
    /**
     * Digest name to [openssl algo constant, ecdsa-with-* OID].
     *
     * The digest's own OID is carried by DigestAlgorithm.
     *
     * @var array<string, array{int, string}>
     */
    private const SIGNATURES = [
        'sha256' => [OPENSSL_ALGO_SHA256, '1.2.840.10045.4.3.2'],
        'sha384' => [OPENSSL_ALGO_SHA384, '1.2.840.10045.4.3.3'],
        'sha512' => [OPENSSL_ALGO_SHA512, '1.2.840.10045.4.3.4'],
    ];

    private Asn1 $asn1;

    private Certificate $certificate;

    public function __construct(?Asn1 $asn1 = null, ?Certificate $certificate = null)
    {
        $this->asn1 = $asn1 ?? new Asn1();
        $this->certificate = $certificate ?? new Certificate($this->asn1);
    }

    /**
     * Produce a detached CAdES-BES CMS SignedData over the given content.
     *
     * @param string               $data            Detached content bytes (the signed data).
     * @param string               $signerCertDer   DER of the signing certificate.
     * @param OpenSSLAsymmetricKey  $privateKey      Signing private key (RSA or EC).
     * @param list<string>         $chainCertsDer   Additional certificates to embed, each as
     *                            PEM or as DER. Every entry is parsed.
     * @param string|DigestAlgorithm $digestAlgorithm Digest algorithm name or enum case.
     * @param int                  $signingTime     Unix timestamp for the signing-time attribute.
     * @param (callable(string): string)|null $signatureTimestamp Optional provider that receives the
     *                            raw SignerInfo signature bytes and returns a DER-encoded RFC 3161
     *                            timestamp token (ContentInfo). When supplied, the token is embedded as
     *                            the id-aa-signatureTimeStampToken unsigned attribute (PAdES B-T).
     * @param bool                 $includeSigningTime Whether to add the CMS signing-time signed
     *                            attribute. The legacy (ISO 32000-1) profile includes it; PAdES-BASELINE
     *                            forbids it (ETSI EN 319 142-1) and carries the time in the /M signature
     *                            dictionary entry instead.
     * @param array<array-key, string> $extraSignedAttributes Additional signed attributes as
     *                            OID => DER-encoded attribute value, for a profile that requires one
     *                            such as the CAdES signature-policy-identifier.
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
        string|DigestAlgorithm $digestAlgorithm,
        int $signingTime,
        ?callable $signatureTimestamp = null,
        bool $includeSigningTime = true,
        array $extraSignedAttributes = [],
    ): string {
        $digest = DigestAlgorithm::fromLoose($digestAlgorithm)->value;
        [, $opensslAlgo] = $this->algorithms($digest);

        $request = new SigningRequest(
            \hash($digest, $data, true),
            $signerCertDer,
            $digest,
            $signingTime,
            $includeSigningTime,
            $extraSignedAttributes,
        );

        $signature = '';
        $signed = \openssl_sign($this->signaturePayload($request), $signature, $privateKey, $opensslAlgo);
        if (!$signed) {
            // The failure is reported as an Exception, so the OpenSSL queue entries
            // are discarded rather than left for the host.
            Certificate::clearOpenSslErrors();
            throw new Exception('Unable to sign the CMS signed attributes');
        }

        return $this->buildFromSignature($request, $signature, $chainCertsDer, $signatureTimestamp);
    }

    /**
     * Produce the bytes a signer has to sign.
     *
     * The first half of sign() on its own, for a signer whose private key this
     * process cannot reach: a hardware token, a smart card, or a remote signing
     * service. It also serves a signer that holds the key but not the content,
     * since the request carries the message digest rather than the content.
     *
     * The result is the DER SET OF signed attributes defined by RFC 5652 section
     * 5.4, which is what the signature covers. It is a pure function of the request,
     * so buildFromSignature() derives the same bytes again rather than taking them
     * from the caller.
     *
     * @param SigningRequest $request Validated inputs for the signed attributes.
     *
     * @return string DER SET OF signed attributes, ready to be signed.
     *
     * @throws Exception If the digest is unsupported or encoding fails.
     */
    public function signaturePayload(SigningRequest $request): string
    {
        return $this->asn1->encodeSet($this->signedAttributesContent($request));
    }

    /**
     * Produce the CMS from a request and the signature over its signaturePayload().
     *
     * The second half of sign(), for a signature produced elsewhere. The
     * signature AlgorithmIdentifier is read from the signing certificate rather
     * than from a private key, since there may be none in this process.
     *
     * The signature is verified against the certificate before anything is emitted,
     * so a signature over the wrong bytes, from the wrong key, or in the wrong
     * encoding fails at the call.
     *
     * @param SigningRequest $request         The same request passed to signaturePayload().
     * @param string         $signature       Signature over $this->signaturePayload($request).
     * @param list<string>   $chainCertsDer   Additional certificates to embed, each as PEM or as DER.
     * @param (callable(string): string)|null $signatureTimestamp Optional provider that receives the
     *                            raw SignerInfo signature bytes and returns a DER-encoded RFC 3161
     *                            timestamp token (ContentInfo). When supplied, the token is embedded as
     *                            the id-aa-signatureTimeStampToken unsigned attribute (PAdES B-T).
     * @param string|SignatureEncoding $signatureEncoding Encoding of $signature. An ECDSA signature
     *                            returned as the fixed-width r || s concatenation is converted to the
     *                            DER form CMS requires when this is P1363.
     *
     * @return string DER-encoded CMS ContentInfo.
     *
     * @throws Exception If the digest or certificate key type is unsupported, if the signature is
     *                   empty, malformed, or does not verify, or if encoding fails.
     */
    public function buildFromSignature(
        SigningRequest $request,
        string $signature,
        array $chainCertsDer = [],
        ?callable $signatureTimestamp = null,
        string|SignatureEncoding $signatureEncoding = SignatureEncoding::Der,
    ): string {
        [$digestOid, $opensslAlgo, $ecdsaOid] = $this->algorithms($request->digestAlgorithm);

        // Loaded once for both the identifier and the verification.
        $publicKey = $this->certificatePublicKey($request->signerCertPem());
        [$signatureOid, $signatureHasNullParams] = $this->signatureAlgorithm($publicKey, $ecdsaOid);

        $signedAttributes = $this->signedAttributesContent($request);
        $signature = $this->verifiedSignature(
            $this->asn1->encodeSet($signedAttributes),
            $signature,
            $publicKey,
            $opensslAlgo,
            $signatureEncoding,
        );

        $unsignedAttributes = $signatureTimestamp === null
            ? ''
            : $this->signatureTimestampAttributes($signatureTimestamp, $signature);

        $signerInfo = $this->asn1->encodeSequence(
            $this->asn1->encodeInteger(1)
            . $this->issuerAndSerialNumber($request->signerCertDer)
            . $this->algorithmIdentifier($digestOid, false)
            . $this->asn1->encodeContext(0, $signedAttributes)
            . $this->algorithmIdentifier($signatureOid, $signatureHasNullParams)
            . $this->asn1->encodeOctetString($signature)
            . $unsignedAttributes,
        );

        $certificates = $this->asn1->encodeContext(0, $this->certificateSet($request->signerCertDer, $chainCertsDer));

        $signedData = $this->asn1->encodeSequence(
            $this->asn1->encodeInteger(1)
                . $this->asn1->encodeSet($this->algorithmIdentifier($digestOid, false))
                . $this->asn1->encodeSequence($this->asn1->encodeObjectIdentifier(Oid::DATA))
                . $certificates
                . $this->asn1->encodeSet($signerInfo),
        );

        return $this->asn1->encodeSequence(
            $this->asn1->encodeObjectIdentifier(Oid::SIGNED_DATA) . $this->asn1->encodeContext(0, $signedData),
        );
    }

    /**
     * Build the signed attributes content for a request (without the SET tag).
     *
     * @throws Exception If the digest is unsupported or encoding fails.
     */
    private function signedAttributesContent(SigningRequest $request): string
    {
        [$digestOid] = $this->algorithms($request->digestAlgorithm);

        return $this->signedAttributes(
            $request->messageDigest,
            \hash($request->digestAlgorithm, $request->signerCertDer, true),
            $request->digestAlgorithm,
            $digestOid,
            $request->signingTime,
            $request->includeSigningTime,
            $request->extraSignedAttributes,
        );
    }

    /**
     * Normalise a signature to DER and verify it against the signing certificate.
     *
     * @param string $signaturePayload  DER SET OF signed attributes the signature must cover.
     * @param string $signature       Signature bytes as supplied by the caller.
     * @param OpenSSLAsymmetricKey $publicKey Public key of the signing certificate.
     * @param int    $opensslAlgo     openssl algorithm constant for the digest.
     * @param string|SignatureEncoding $signatureEncoding Encoding of $signature.
     *
     * @return string Signature bytes as CMS carries them.
     *
     * @throws Exception If the signature is empty, malformed, or does not verify.
     */
    private function verifiedSignature(
        string $signaturePayload,
        string $signature,
        OpenSSLAsymmetricKey $publicKey,
        int $opensslAlgo,
        string|SignatureEncoding $signatureEncoding,
    ): string {
        if ($signature === '') {
            throw new Exception('The signature is empty');
        }

        if (SignatureEncoding::fromLoose($signatureEncoding) === SignatureEncoding::P1363) {
            $signature = $this->p1363ToDer($signature);
        }

        $verified = \openssl_verify($signaturePayload, $signature, $publicKey, $opensslAlgo);

        // A refused signature is reported as an Exception, so the OpenSSL queue
        // entries are discarded rather than left for the host.
        Certificate::clearOpenSslErrors();

        if ($verified !== 1) {
            throw new Exception('The signature does not verify against the signing certificate');
        }

        return $signature;
    }

    /**
     * Convert a fixed-width ECDSA signature r || s to the DER SEQUENCE CMS carries.
     *
     * @throws Exception If the length is not a whole number of two equal halves.
     */
    private function p1363ToDer(string $signature): string
    {
        $length = \strlen($signature);
        if ($length < 2 || ($length % 2) !== 0) {
            throw new Exception('Invalid P1363 signature length: ' . $length);
        }

        $half = \intdiv($length, 2);

        return $this->asn1->encodeSequence(
            $this->asn1->encodeIntegerBytes(\substr($signature, 0, $half))
                . $this->asn1->encodeIntegerBytes(\substr($signature, $half)),
        );
    }

    /**
     * Load the public key of the signing certificate.
     *
     * @throws Exception If the certificate cannot be read.
     */
    private function certificatePublicKey(string $signerCertPem): OpenSSLAsymmetricKey
    {
        $publicKey = \openssl_pkey_get_public($signerCertPem);
        if ($publicKey === false) {
            // Unreachable: SigningRequest makes the same call on construction.
            Certificate::clearOpenSslErrors();
            throw new Exception('Unreadable signer certificate');
        }

        return $publicKey;
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
        // Unreachable through the public API, which resolves the name through
        // DigestAlgorithm first; this fires only if SIGNATURES drifts from its cases.
        $algorithm = DigestAlgorithm::tryFrom($digestAlgorithm);
        if ($algorithm === null || !isset(self::SIGNATURES[$digestAlgorithm])) {
            throw new Exception('Unsupported digest algorithm: ' . $digestAlgorithm);
        }

        [$opensslAlgo, $ecdsaOid] = self::SIGNATURES[$digestAlgorithm];

        return [$algorithm->oid(), $opensslAlgo, $ecdsaOid];
    }

    /**
     * Resolve the signature AlgorithmIdentifier for the signer's key.
     *
     * Read from the certificate rather than from the private key, since
     * buildFromSignature() has a certificate where it may have no key. For a
     * matching pair the result is the same.
     *
     * An RSA signature is identified by rsaEncryption with NULL parameters, which
     * RFC 3370 section 3.2 defines as the PKCS #1 v1.5 signature value identifier
     * whatever the digest; SignerInfo carries the digest in its own field. The OID
     * is SignatureVerifier's.
     *
     * @return array{string, bool} [signature OID, whether NULL parameters are emitted]
     *
     * @throws Exception If the key type is unsupported.
     */
    private function signatureAlgorithm(OpenSSLAsymmetricKey $publicKey, string $ecdsaOid): array
    {
        $details = \openssl_pkey_get_details($publicKey);
        $type = $details !== false ? $details['type'] ?? -1 : -1;

        if ($type === OPENSSL_KEYTYPE_RSA) {
            return [SignatureVerifier::OID_RSA_ENCRYPTION, true];
        }

        if ($type === OPENSSL_KEYTYPE_EC) {
            return [$ecdsaOid, false];
        }

        throw new Exception('Unsupported signing key type');
    }

    /**
     * Build the sorted DER SET OF signed attributes content (without the tag).
     *
     * @param array<string, string> $extraSignedAttributes Additional attributes as OID => DER value.
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
        array $extraSignedAttributes = [],
    ): string {
        $attributes = [
            $this->attribute(Oid::CONTENT_TYPE, $this->asn1->encodeObjectIdentifier(Oid::DATA)),
            $this->attribute(Oid::MESSAGE_DIGEST, $this->asn1->encodeOctetString($messageDigest)),
            $this->attribute(Oid::SIGNING_CERTIFICATE_V2, $this->signingCertificateV2(
                $certHash,
                $digestAlgorithm,
                $digestOid,
            )),
        ];

        // The CMS signing-time attribute belongs to the legacy (ISO 32000-1) profile.
        // PAdES-BASELINE forbids it (ETSI EN 319 142-1) and carries the signing time
        // in the /M entry of the PDF signature dictionary instead.
        if ($includeSigningTime) {
            $attributes[] = $this->attribute(Oid::SIGNING_TIME, $this->encodeTime($signingTime));
        }

        // Attribute types the builder controls are reserved by SigningRequest, so an
        // extra attribute cannot duplicate one of them.
        foreach ($extraSignedAttributes as $oid => $value) {
            $attributes[] = $this->attribute($oid, $value);
        }

        return \implode('', $this->sortSetOf($attributes));
    }

    /**
     * Build the CertificateSet content: the signer certificate and the chain.
     *
     * CertificateSet is a SET OF (RFC 5652 section 10.2.3), so its members follow the
     * same DER ordering rule as the signed attributes rather than the order the
     * caller supplied, and a member appears once: a chain that already carries the
     * leaf is deduplicated against the signer certificate.
     *
     * Each entry may be given as PEM or as DER, and is parsed as a certificate.
     *
     * @param list<string> $chainCertsDer Additional certificates, PEM or DER.
     *
     * @throws Exception If a chain entry is not a certificate.
     */
    private function certificateSet(string $signerCertDer, array $chainCertsDer): string
    {
        $certificates = [$signerCertDer];
        /** @var mixed $cert */
        foreach ($chainCertsDer as $index => $cert) {
            // An entry that is not a string would reach toDer() as a TypeError rather
            // than an Exception.
            if (!\is_string($cert)) {
                throw new Exception('Invalid chain certificate ' . $index);
            }

            try {
                $certificates[] = Certificate::toDer($cert);
            } catch (Exception $e) {
                throw new Exception('Invalid chain certificate ' . $index, 0, $e);
            }
        }

        return \implode('', $this->sortSetOf(Certificate::deduplicate($certificates)));
    }

    /**
     * Sort the members of a SET OF into DER order.
     *
     * X.690 section 11.6: members are ordered by their encodings, compared as
     * octet strings padded with trailing zero octets.
     *
     * @param list<string> $members
     *
     * @return list<string>
     */
    private function sortSetOf(array $members): array
    {
        \usort($members, static function (string $one, string $two): int {
            $length = \max(\strlen($one), \strlen($two));
            return \strcmp(\str_pad($one, $length, "\x00"), \str_pad($two, $length, "\x00"));
        });

        return $members;
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
        if (!\is_string($token)) {
            throw new Exception('Invalid signature timestamp token');
        }

        // Held to the strict reading Timestamp\Client applies before returning a
        // token, so a token from a provider of the host's own is bounded the same
        // way: the SignedData head, the CertificateSet, the crls [1], and the tail.
        try {
            $this->certificate->fromSignedData($token, true);
        } catch (Exception $e) {
            throw new Exception('The signature timestamp token is not a CMS SignedData', 0, $e);
        }

        return $this->asn1->encodeContext(1, $this->attribute(Oid::SIGNATURE_TIMESTAMP, $token));
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
        $fields = $this->certificate->fields($certDer);

        return $this->asn1->encodeSequence($fields['issuer'] . $fields['serial']);
    }
}
