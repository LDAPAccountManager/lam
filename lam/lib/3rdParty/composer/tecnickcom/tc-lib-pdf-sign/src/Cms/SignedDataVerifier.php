<?php

declare(strict_types=1);

/**
 * SignedDataVerifier.php
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
 * Com\Tecnick\Pdf\Sign\Cms\SignedDataVerifier
 *
 * Verifies a CMS SignedData that carries its own content, which is the shape of
 * an RFC 3161 timestamp token. It resolves the signer certificate from the ones
 * the message embeds, checks the content-type and message-digest signed
 * attributes against the encapsulated content, and verifies the signature over
 * the DER SET OF signed attributes (RFC 5652 section 5.4).
 *
 * eContentType sits outside the signature, so it is compared with the signed
 * content-type attribute (RFC 5652 sections 5.3 and 11.1). An ESS
 * signing-certificate attribute is checked when present, which binds the
 * signature to that certificate rather than to its key alone (RFC 5035); a
 * caller whose profile requires the attribute passes $requireSigningCertificate.
 *
 * The result establishes that the message was signed by the key in the
 * certificate it names and has not been altered since. Whether that certificate
 * is trusted stays the host's question.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class SignedDataVerifier
{
    /**
     * SHA-1, accepted only when the caller opts in. See SignatureVerifier.
     *
     * @var array<string, string>
     */
    public const LEGACY_DIGESTS = ['1.3.14.3.2.26' => 'sha1'];

    private Asn1 $asn1;

    private Certificate $certificate;

    private SignatureVerifier $verifier;

    /**
     * @param bool $allowSha1 Accept SHA-1 for the message-digest attribute, the ESS
     *                        certificate hash, and the signature algorithm.
     * @param bool $requireSigningCertificate Refuse a SignerInfo that carries no ESS
     *                        signing-certificate attribute. Off by default, the
     *                        attribute being optional in CMS at large; the codecs
     *                        reading a timestamp token turn it on, as RFC 3161
     *                        section 2.4.2 requires it there.
     */
    public function __construct(
        ?Asn1 $asn1 = null,
        ?Certificate $certificate = null,
        ?SignatureVerifier $verifier = null,
        private readonly bool $allowSha1 = false,
        private readonly bool $requireSigningCertificate = false,
    ) {
        $this->asn1 = $asn1 ?? new Asn1();
        $this->certificate = $certificate ?? new Certificate($this->asn1);
        $this->verifier = $verifier ?? new SignatureVerifier($this->asn1, $allowSha1);
    }

    /**
     * Verify the single SignerInfo of a content-carrying SignedData.
     *
     * @param string      $cmsDer           DER-encoded CMS ContentInfo.
     * @param string|null $detachedContent  Content a detached signature covers, which is
     *                            the ByteRange-covered document bytes for the CMS
     *                            Builder emits. Given one, the message must carry no
     *                            eContent of its own and these octets are what the
     *                            message-digest attribute is checked against. Omitted,
     *                            the message has to carry its own content, which is the
     *                            shape of an RFC 3161 timestamp token.
     *
     * @return string DER of the certificate the signature was verified against.
     *
     * @throws Exception If the message is malformed, embeds no usable signer certificate,
     *                   or the signature or the message digest does not check out.
     */
    public function verify(string $cmsDer, ?string $detachedContent = null): string
    {
        $signedData = $this->certificate->signedDataContent($cmsDer);

        $offset = 0;
        [$contentType, $content] = $this->certificate->encapsulatedContent(
            $signedData,
            $offset,
            $detachedContent !== null,
        );

        $content = $detachedContent ?? $content;

        $signerInfo = $this->signerInfo($signedData, $offset);
        $this->checkContentType($signerInfo, $contentType);

        // Resolved once for both the digest comparison below and the signature check
        // in the loop, being a field of the message rather than of the candidate.
        $digestName = $this->digestName($signerInfo['digestAlgorithm']);
        $this->checkMessageDigest($signerInfo, $content, $digestName);

        $candidates = $this->signerCertificates($signerInfo['sid'], $this->certificate->fromSignedData($cmsDer));

        // Every candidate carries the SignerIdentifier, and the CertificateSet is
        // covered by no signature, so which of them signed is settled by trying each
        // rather than by position in the bag. Each still has to pass the ESS
        // certificate hash and the signature.
        $rejection = null;
        foreach ($candidates as $signerCert) {
            try {
                $this->checkSigningCertificate($signerInfo, $signerCert);

                $this->verifier->verify(
                    // signedAttrs is carried as [0] IMPLICIT but signed as the DER SET OF.
                    "\x31"
                    . $this->asn1->encodeLength(\strlen($signerInfo['signedAttrs']))
                    . $signerInfo['signedAttrs'],
                    $signerInfo['signatureAlgorithm'],
                    $signerInfo['signature'],
                    $signerCert,
                    // RFC 3370 section 3.2: an RSA signature value may be identified
                    // by rsaEncryption, which names no digest, so the digest comes
                    // from the SignerInfo digestAlgorithm. It is the same field
                    // checkMessageDigest() compared against the signed message-digest
                    // attribute above.
                    $digestName,
                );

                return $signerCert;
            } catch (Exception $e) {
                $rejection ??= $e;
            }
        }

        throw $rejection ?? new Exception('The SignedData does not embed the signer certificate');
    }

    /**
     * Read the fields of the single SignerInfo a timestamp token carries.
     *
     * RFC 3161 section 2.4.2 allows a timestamp token exactly one member: "The
     * time-stamp token MUST NOT contain any signatures other than the signature of
     * the TSA." Cms\Certificate::signerInfos() owns the walk to the SET.
     *
     * @param int $offset Read cursor positioned just after encapContentInfo.
     *
     * @return array{sid: array{tag: int, value: string, raw: string}, digestAlgorithm: string,
     *           signedAttrs: string, signatureAlgorithm: string, signature: string}
     *
     * @throws Exception If the structure is malformed, carries no signed attributes,
     *                   or carries more than one SignerInfo.
     */
    private function signerInfo(string $signedData, int $offset): array
    {
        $infos = $this->certificate->signerInfos($signedData, $offset);
        if (\count($infos) !== 1) {
            throw new Exception('The SignedData carries more than one SignerInfo');
        }

        $info = $infos[0];

        $inner = 0;

        // Decoded rather than stepped over, as Cms\Certificate::assertSignedDataHead()
        // decodes the SignedData version. The range is not checked.
        $version = $this->asn1->readTlv($info, $inner);
        if ($version['tag'] !== 0x02) {
            throw new Exception('Invalid SignerInfo version');
        }

        $this->asn1->decodeInteger($version['value']);

        $sid = $this->asn1->readTlv($info, $inner);
        $digestAlgorithm = $this->asn1->readTlv($info, $inner);

        $signedAttrs = $this->asn1->readTlv($info, $inner);
        if ($signedAttrs['tag'] !== 0xA0) {
            throw new Exception('The SignerInfo carries no signed attributes');
        }

        $signatureAlgorithm = $this->asn1->readTlv($info, $inner);
        $signature = $this->asn1->readTlv($info, $inner);
        if ($signature['tag'] !== 0x04) {
            throw new Exception('Invalid SignerInfo signature');
        }

        // RFC 5652 section 5.3 puts unsignedAttrs [1] last and nothing after it.
        $unsigned = $this->asn1->readOptionalTlv($info, $inner);
        if ($unsigned !== null && $unsigned['tag'] !== 0xA1) {
            throw new Exception('Invalid SignerInfo field after the signature');
        }

        if ($inner !== \strlen($info)) {
            throw new Exception('Trailing bytes after the SignerInfo unsigned attributes');
        }

        return [
            'sid' => $sid,
            'digestAlgorithm' => $digestAlgorithm['raw'],
            'signedAttrs' => $signedAttrs['value'],
            'signatureAlgorithm' => $signatureAlgorithm['raw'],
            'signature' => $signature['value'],
        ];
    }

    /**
     * Check the content-type signed attribute against the eContentType.
     *
     * RFC 5652 section 5.3 requires the attribute whenever signedAttrs is present,
     * and section 11.1 requires its value to equal eContentType.
     *
     * @param array{sid: array{tag: int, value: string, raw: string}, digestAlgorithm: string,
     *           signedAttrs: string, signatureAlgorithm: string, signature: string} $signerInfo
     *
     * @throws Exception If the attribute is missing or names another content type.
     */
    private function checkContentType(array $signerInfo, string $eContentType): void
    {
        $attribute = $this->attributeValue($signerInfo['signedAttrs'], Oid::CONTENT_TYPE);
        if ($attribute === null) {
            throw new Exception('The SignerInfo carries no content-type attribute');
        }

        if ($attribute['raw'] !== $eContentType) {
            throw new Exception('The signed content-type does not match the encapsulated content');
        }
    }

    /**
     * Check the ESS signing-certificate attribute against the resolved signer.
     *
     * RFC 5035 binds the signature to one certificate rather than to its key. It is
     * the only signed field that names the signing certificate, SignerIdentifier and
     * the CertificateSet both sitting outside signedAttrs. The attribute is optional
     * in CMS at large, so it is checked when present and demanded only when the
     * caller asked for it.
     *
     * @param array{sid: array{tag: int, value: string, raw: string}, digestAlgorithm: string,
     *           signedAttrs: string, signatureAlgorithm: string, signature: string} $signerInfo
     *
     * @throws Exception If the attribute names another certificate, or is absent and
     *                   the caller required it.
     */
    private function checkSigningCertificate(array $signerInfo, string $signerCert): void
    {
        // SigningCertificate (v1) hashes with SHA-1 by definition; SigningCertificateV2
        // carries an optional AlgorithmIdentifier defaulting to SHA-256.
        $version2 = $this->attributeValue($signerInfo['signedAttrs'], Oid::SIGNING_CERTIFICATE_V2);
        $attribute = $version2 ?? $this->attributeValue($signerInfo['signedAttrs'], Oid::SIGNING_CERTIFICATE);
        if ($attribute === null) {
            if ($this->requireSigningCertificate) {
                throw new Exception('The SignerInfo carries no signing-certificate attribute');
            }

            return;
        }

        [$name, $certHash] = $this->essCertId($attribute, $version2 === null);
        if (!\hash_equals(\hash($name, $signerCert, true), $certHash)) {
            throw new Exception('The signing-certificate attribute names another certificate');
        }
    }

    /**
     * Read the hash algorithm and the certificate hash of the first ESSCertID.
     *
     * @param array{tag: int, value: string, raw: string} $attribute Attribute value TLV.
     * @param bool                                        $legacy    True for the v1 attribute.
     *
     * @return array{string, string} [hash name, certHash octets]
     *
     * @throws Exception If the structure is malformed or names an unsupported digest.
     */
    private function essCertId(array $attribute, bool $legacy): array
    {
        if ($attribute['tag'] !== 0x30) {
            throw new Exception('Invalid signing-certificate attribute');
        }

        $certsOffset = 0;
        $certs = $this->asn1->readTlv($attribute['value'], $certsOffset);
        if ($certs['tag'] !== 0x30 || $certs['value'] === '') {
            throw new Exception('Invalid signing-certificate attribute');
        }

        $idOffset = 0;
        $essCertId = $this->asn1->readTlv($certs['value'], $idOffset);
        if ($essCertId['tag'] !== 0x30) {
            throw new Exception('Invalid signing-certificate attribute');
        }

        $fieldOffset = 0;
        $field = $this->asn1->readTlv($essCertId['value'], $fieldOffset);

        // ESSCertIDv2 hashAlgorithm is OPTIONAL and DEFAULT SHA-256, so an ESSCertID
        // that starts with the OCTET STRING is using the default. ESSCertID (v1) has
        // no algorithm field at all.
        $name = $legacy ? 'sha1' : 'sha256';
        if (!$legacy && $field['tag'] === 0x30) {
            $name = $this->digestName($field['raw']);
            $field = $this->asn1->readTlv($essCertId['value'], $fieldOffset);
        }

        if ($field['tag'] !== 0x04) {
            throw new Exception('Invalid signing-certificate attribute');
        }

        if ($legacy && !$this->allowSha1) {
            throw new Exception('Refusing the SHA-1 signing-certificate attribute');
        }

        return [$name, $field['value']];
    }

    /**
     * Check the message-digest signed attribute against the encapsulated content.
     *
     * @param array{sid: array{tag: int, value: string, raw: string}, digestAlgorithm: string,
     *           signedAttrs: string, signatureAlgorithm: string, signature: string} $signerInfo
     * @param string $name Digest the SignerInfo digestAlgorithm names.
     *
     * @throws Exception If the attribute is missing or does not match.
     */
    private function checkMessageDigest(array $signerInfo, string $content, string $name): void
    {
        $digest = $this->attributeValue($signerInfo['signedAttrs'], Oid::MESSAGE_DIGEST);
        if ($digest === null || $digest['tag'] !== 0x04) {
            throw new Exception('The SignerInfo carries no message-digest attribute');
        }

        if (!\hash_equals(\hash($name, $content, true), $digest['value'])) {
            throw new Exception('The message digest does not cover the encapsulated content');
        }
    }

    /**
     * Resolve the hash name of a digest AlgorithmIdentifier.
     *
     * Asn1::decodeAlgorithmIdentifier() owns the walk, which bounds both layers of
     * the field.
     *
     * @param string $algorithmIdDer Complete DER of the AlgorithmIdentifier.
     *
     * @throws Exception If the identifier is malformed or names a refused algorithm.
     */
    private function digestName(string $algorithmIdDer): string
    {
        $oid = $this->asn1->decodeAlgorithmIdentifier($algorithmIdDer, 'digest');

        $algorithm = DigestAlgorithm::tryFromOid($oid);
        if ($algorithm !== null) {
            return $algorithm->value;
        }

        if (isset(self::LEGACY_DIGESTS[$oid])) {
            if (!$this->allowSha1) {
                throw new Exception('Refusing the SHA-1 digest algorithm: ' . $oid);
            }

            return self::LEGACY_DIGESTS[$oid];
        }

        throw new Exception('Unsupported digest algorithm: ' . $oid);
    }

    /**
     * Find the single value of a signed attribute by type OID.
     *
     * @param string $attributes SignedAttributes content octets.
     *
     * @return array{tag: int, value: string, raw: string}|null Null when the type is absent.
     *
     * @throws Exception If the structure is malformed.
     */
    private function attributeValue(string $attributes, string $oid): ?array
    {
        $wanted = $this->asn1->encodeObjectIdentifier($oid);

        $found = null;
        $offset = 0;
        while ($offset < \strlen($attributes)) {
            $attribute = $this->asn1->readTlv($attributes, $offset);
            if ($attribute['tag'] !== 0x30) {
                throw new Exception('Invalid signed attribute');
            }

            // Attribute ::= SEQUENCE { attrType OBJECT IDENTIFIER, attrValues SET OF }
            // (RFC 5652 section 5.3), with nothing after the two.
            $inner = 0;
            $type = $this->asn1->readTlv($attribute['value'], $inner);
            $values = $this->asn1->readTlv($attribute['value'], $inner);
            if ($type['tag'] !== 0x06 || $inner !== \strlen($attribute['value'])) {
                throw new Exception('Invalid signed attribute');
            }

            if ($type['raw'] !== $wanted) {
                continue;
            }

            // RFC 5652 section 5.3: an attribute type appears at most once, and the
            // types read here carry exactly one value.
            if ($found !== null) {
                throw new Exception('Duplicate signed attribute: ' . $oid);
            }

            if ($values['tag'] !== 0x31) {
                throw new Exception('Invalid signed attribute values: ' . $oid);
            }

            $valueOffset = 0;
            $found = $this->asn1->readTlv($values['value'], $valueOffset);
            if ($valueOffset !== \strlen($values['value'])) {
                throw new Exception('Signed attribute carries more than one value: ' . $oid);
            }
        }

        return $found;
    }

    /**
     * Resolve the SignerIdentifier to the embedded certificates that carry it.
     *
     * All of them are returned rather than the first. The CertificateSet is
     * unauthenticated, so a bag may hold several members answering to one
     * identifier, and only the signature says which one signed.
     *
     * @param array{tag: int, value: string, raw: string} $sid
     * @param list<string>                                $certs
     *
     * @return list<string> DER certificates naming $sid, in the order given.
     *
     * @throws Exception If the identifier is unsupported or names no embedded certificate.
     */
    private function signerCertificates(array $sid, array $certs): array
    {
        // An empty identifier names nothing; subjectKeyIdentifier() answers '' for a
        // certificate carrying no such extension, so the two would compare equal.
        if ($sid['value'] === '') {
            throw new Exception('Invalid SignerIdentifier');
        }

        $matches = [];
        foreach ($certs as $cert) {
            $fields = $this->certificate->fields($cert);

            // SignerIdentifier ::= CHOICE { issuerAndSerialNumber, subjectKeyIdentifier [0] }.
            if ($sid['tag'] === 0x30) {
                if ($sid['value'] === $fields['issuer'] . $fields['serial']) {
                    $matches[] = $cert;
                }

                continue;
            }

            if ($sid['tag'] === 0x80 && $this->certificate->subjectKeyIdentifier($cert) === $sid['value']) {
                $matches[] = $cert;
            }
        }

        if ($matches === []) {
            throw new Exception('The SignedData does not embed the signer certificate');
        }

        return $matches;
    }
}
