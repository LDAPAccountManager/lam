<?php

declare(strict_types=1);

/**
 * Certificate.php
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

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Cms\Certificate
 *
 * Reads the TBSCertificate fields that CMS and OCSP structures quote verbatim:
 * the issuer and subject Names, the serial number, and the public key bits. The
 * raw DER of each field is preserved, because IssuerAndSerialNumber and the OCSP
 * CertID must carry the certificate's own encoding rather than a re-encoding of
 * the decoded value.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Certificate
{
    /**
     * The PEM armour a certificate body sits between.
     */
    private const PEM_BEGIN = '-----BEGIN CERTIFICATE-----';

    private const PEM_END = '-----END CERTIFICATE-----';

    /**
     * Most certificates accepted in an unauthenticated CMS CertificateSet.
     *
     * Ocsp\Client::MAX_RESPONDER_CERTIFICATES and Signer::MAX_PATH_CERTIFICATES
     * read it, so every certificate bag is held to the same bound.
     */
    public const MAX_EMBEDDED_CERTIFICATES = 32;

    /**
     * id-ce-subjectKeyIdentifier (RFC 5280 section 4.2.1.2).
     */
    private const OID_SUBJECT_KEY_IDENTIFIER = '2.5.29.14';

    /**
     * id-ce-keyUsage (RFC 5280 section 4.2.1.3).
     */
    private const OID_KEY_USAGE = '2.5.29.15';

    /**
     * id-ce-basicConstraints (RFC 5280 section 4.2.1.9).
     */
    private const OID_BASIC_CONSTRAINTS = '2.5.29.19';

    /**
     * id-ce-extKeyUsage (RFC 5280 section 4.2.1.12).
     */
    private const OID_EXTENDED_KEY_USAGE = '2.5.29.37';

    /**
     * KeyUsage bit names by position, in the order RFC 5280 section 4.2.1.3
     * declares them.
     *
     * @var list<string>
     */
    private const KEY_USAGE_BITS = [
        'digitalSignature',
        'nonRepudiation',
        'keyEncipherment',
        'dataEncipherment',
        'keyAgreement',
        'keyCertSign',
        'cRLSign',
        'encipherOnly',
        'decipherOnly',
    ];

    private Asn1 $asn1;

    public function __construct(?Asn1 $asn1 = null)
    {
        $this->asn1 = $asn1 ?? new Asn1();
    }

    /**
     * Read the quoted TBSCertificate fields of a DER-encoded X.509 certificate.
     *
     * The serial and Name entries are the complete TLV as the certificate carries
     * them. The public key is the subjectPublicKey BIT STRING value without its
     * leading unused-bits octet, which is what an OCSP issuerKeyHash covers. The
     * validity bounds are Unix times decoded from the Time CHOICE the issuer used.
     *
     * Every field is tag-checked and the whole structure is walked. The input must
     * be exactly one certificate, with nothing following any of its fields.
     *
     * @return array{serial: string, issuer: string, subject: string, public_key: string,
     *           not_before: int, not_after: int}
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    public function fields(string $certDer): array
    {
        $parsed = $this->parseTbs($certDer);
        unset($parsed['extensions']);

        return $parsed;
    }

    /**
     * The DER is decoded directly rather than through OpenSSL's rendering, which
     * flattens each extension to a string with no structure left in it.
     *
     * @return array<string, array{critical: bool, value: string}> Keyed by extension
     *         OID, empty when the certificate carries none.
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    public function extensions(string $certDer): array
    {
        return $this->asn1->decodeExtensions($this->parseTbs($certDer)['extensions'], 'certificate extension');
    }

    /**
     * Read the quoted TBSCertificate fields, along with its extensions.
     *
     * RFC 5280 section 4.1 shapes a Certificate as SEQUENCE { tbsCertificate,
     * signatureAlgorithm AlgorithmIdentifier, signatureValue BIT STRING }, and puts
     * nothing after the three. Every field of both layers is read and bounded here,
     * since this is the parse that decides whether a value is a certificate at all.
     *
     * @return array{serial: string, issuer: string, subject: string, public_key: string,
     *           not_before: int, not_after: int, extensions: string}
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    private function parseTbs(string $certDer): array
    {
        [$tbs, $outerAlgorithm] = $this->tbsCertificate($certDer);
        $off = 0;

        // version [0] EXPLICIT is absent in a v1 certificate. The tag is matched
        // exactly, being the only context tag admissible here.
        $version = 0;
        if ($off < \strlen($tbs) && \ord($tbs[$off]) === 0xA0) {
            $version = $this->version($this->asn1->readTlv($tbs, $off));
        }

        $serial = $this->asn1->readTlv($tbs, $off);
        if ($serial['tag'] !== 0x02) {
            throw new Exception('Invalid certificate serial number');
        }

        // A serial runs to 20 octets (RFC 5280 section 4.1.2.2), too wide to decode,
        // so only its minimality is checked (X.690 section 8.3.2).
        $this->asn1->assertMinimalInteger($serial['value']);

        $algorithm = $this->asn1->readTlv($tbs, $off);
        if ($algorithm['tag'] !== 0x30) {
            throw new Exception('Invalid TBSCertificate signature algorithm');
        }

        // RFC 5280 section 4.1.1.2: the outer signatureAlgorithm must carry the same
        // identifier as the TBSCertificate signature field, only the inner one being
        // covered by the signature.
        if ($outerAlgorithm !== $algorithm['raw']) {
            throw new Exception('The certificate signature algorithms differ');
        }

        $issuer = $this->asn1->readTlv($tbs, $off);
        if ($issuer['tag'] !== 0x30) {
            throw new Exception('Invalid certificate issuer');
        }

        $validity = $this->validity($this->asn1->readTlv($tbs, $off));

        $subject = $this->asn1->readTlv($tbs, $off);
        if ($subject['tag'] !== 0x30) {
            throw new Exception('Invalid certificate subject');
        }

        $spki = $this->asn1->readTlv($tbs, $off);
        if ($spki['tag'] !== 0x30) {
            throw new Exception('Invalid subjectPublicKeyInfo');
        }

        // SubjectPublicKeyInfo ::= SEQUENCE { algorithm AlgorithmIdentifier,
        // subjectPublicKey BIT STRING }, so the field before the key is a SEQUENCE.
        $spkiOff = 0;
        $spkiAlgorithm = $this->asn1->readTlv($spki['value'], $spkiOff);
        if ($spkiAlgorithm['tag'] !== 0x30) {
            throw new Exception('Invalid subjectPublicKeyInfo algorithm');
        }

        $publicKey = $this->asn1->decodeBitString($this->asn1->readTlv($spki['value'], $spkiOff));
        if ($spkiOff !== \strlen($spki['value'])) {
            throw new Exception('Trailing bytes in the subjectPublicKeyInfo');
        }

        return [
            'serial' => $serial['raw'],
            'issuer' => $issuer['raw'],
            'subject' => $subject['raw'],
            'public_key' => $publicKey,
            'not_before' => $validity[0],
            'not_after' => $validity[1],
            'extensions' => $this->extensionsDer($tbs, $off, $version),
        ];
    }

    /**
     * Decode a TBSCertificate version [0] EXPLICIT field.
     *
     * RFC 5280 section 4.1.2.1 shapes it as an INTEGER of v1(0), v2(1), or v3(2),
     * and the EXPLICIT wrapper holds that one element and nothing else.
     *
     * @param array{tag: int, value: string, raw: string} $element Parsed version TLV.
     *
     * @throws Exception If the field does not hold exactly one version INTEGER.
     */
    private function version(array $element): int
    {
        $offset = 0;
        $integer = $this->asn1->readTlv($element['value'], $offset);
        if ($integer['tag'] !== 0x02 || $offset !== \strlen($element['value'])) {
            throw new Exception('Invalid certificate version');
        }

        $version = $this->asn1->decodeInteger($integer['value']);
        if ($version < 0 || $version > 2) {
            throw new Exception('Unsupported certificate version: ' . ($version + 1));
        }

        return $version;
    }

    /**
     * Unwrap a Certificate to the content octets of its TBSCertificate.
     *
     * RFC 5280 section 4.1 shapes a Certificate as SEQUENCE { tbsCertificate,
     * signatureAlgorithm AlgorithmIdentifier, signatureValue BIT STRING }, and puts
     * nothing after the three.
     *
     * @return array{string, string} [TBSCertificate content octets, complete DER of
     *         the outer signatureAlgorithm]
     *
     * @throws Exception If the input is not exactly one Certificate.
     */
    private function tbsCertificate(string $certDer): array
    {
        $certOff = 0;
        $certTlv = $this->asn1->readTlv($certDer, $certOff);
        if ($certTlv['tag'] !== 0x30 || $certOff !== \strlen($certDer)) {
            throw new Exception('Invalid certificate structure');
        }

        $offset = 0;
        $tbs = $this->asn1->readTlv($certTlv['value'], $offset);
        if ($tbs['tag'] !== 0x30) {
            throw new Exception('Invalid TBSCertificate structure');
        }

        $algorithm = $this->asn1->readTlv($certTlv['value'], $offset);
        $signature = $this->asn1->readTlv($certTlv['value'], $offset);
        if ($algorithm['tag'] !== 0x30 || $signature['tag'] !== 0x03) {
            throw new Exception('Invalid certificate signature');
        }

        if ($offset !== \strlen($certTlv['value'])) {
            throw new Exception('Trailing bytes after the certificate signature');
        }

        return [$tbs['value'], $algorithm['raw']];
    }

    /**
     * Read the extensions [3] EXPLICIT field of a TBSCertificate.
     *
     * RFC 5280 section 4.1 puts issuerUniqueID [1] IMPLICIT and subjectUniqueID [2]
     * IMPLICIT before it, each admissible once and in that order, and nothing after
     * it. The tags are matched exactly and dispatched positionally.
     *
     * @param int $offset  Read cursor positioned just after subjectPublicKeyInfo.
     * @param int $version Decoded version field: 0 for v1, 1 for v2, 2 for v3.
     *
     * @return string Complete DER of the Extensions SEQUENCE, or '' when absent.
     *
     * @throws Exception If the structure is malformed.
     */
    private function extensionsDer(string $tbs, int $offset, int $version): string
    {
        $field = $this->asn1->readOptionalTlv($tbs, $offset);
        if ($field !== null && $field['tag'] === 0x81) {
            $field = $this->asn1->readOptionalTlv($tbs, $offset);
        }

        if ($field !== null && $field['tag'] === 0x82) {
            $field = $this->asn1->readOptionalTlv($tbs, $offset);
        }

        if ($field === null) {
            return '';
        }

        if ($field['tag'] !== 0xA3) {
            throw new Exception('Invalid TBSCertificate field after the subjectPublicKeyInfo');
        }

        if ($offset !== \strlen($tbs)) {
            throw new Exception('Trailing bytes after the TBSCertificate extensions');
        }

        // The [3] is EXPLICIT, so it wraps exactly one Extensions SEQUENCE.
        $this->asn1->assertSingleElement($field['value'], 0x30, 'certificate extensions');

        // RFC 5280 section 4.1.2.9: extensions appear only in a v3 certificate.
        if ($version !== 2) {
            throw new Exception('Extensions in a certificate of version ' . ($version + 1));
        }

        return $field['value'];
    }

    /**
     * Decode the two bounds of a TBSCertificate Validity SEQUENCE.
     *
     * @param array{tag: int, value: string, raw: string} $element Parsed Validity TLV.
     *
     * @return array{int, int} [notBefore, notAfter] as Unix times.
     *
     * @throws Exception If the structure is not a Validity of two Time values.
     */
    private function validity(array $element): array
    {
        if ($element['tag'] !== 0x30) {
            throw new Exception('Invalid certificate validity');
        }

        $offset = 0;
        $notBefore = $this->asn1->decodeTime($this->asn1->readTlv($element['value'], $offset));
        $notAfter = $this->asn1->decodeTime($this->asn1->readTlv($element['value'], $offset));
        if ($offset !== \strlen($element['value'])) {
            throw new Exception('Trailing bytes in the certificate validity');
        }

        return [$notBefore, $notAfter];
    }

    /**
     * Assert that a certificate is inside its validity period at a given time.
     *
     * Not called by Builder, since a host may deliberately re-sign historical
     * content; it is for a host that wants the check before it commits.
     *
     * @param int $time      Unix time the certificate must cover.
     * @param int $tolerance Clock skew tolerated on either bound, in seconds.
     *
     * @throws Exception If the certificate cannot be parsed or does not cover $time.
     */
    public function assertValidAt(string $certDer, int $time, int $tolerance = 0): void
    {
        $fields = $this->fields($certDer);

        if ($time < ($fields['not_before'] - $tolerance)) {
            throw new Exception('The certificate is not yet valid at ' . \gmdate('c', $time));
        }

        if ($time > ($fields['not_after'] + $tolerance)) {
            throw new Exception('The certificate has expired at ' . \gmdate('c', $time));
        }
    }

    /**
     * Assert that a certificate's key usage admits signing.
     *
     * RFC 5280 section 4.2.1.3: a KeyUsage extension that carries neither
     * digitalSignature nor nonRepudiation (contentCommitment) forbids the
     * signature a CAdES SignerInfo carries. A certificate without the extension
     * is unrestricted and passes.
     *
     * @throws Exception If the extension is present and admits neither purpose.
     */
    public function assertUsableForSigning(string $certDer): void
    {
        $usage = $this->keyUsage($certDer);
        if ($usage === null || \array_intersect($usage, ['digitalSignature', 'nonRepudiation']) !== []) {
            return;
        }

        throw new Exception('The certificate key usage does not admit signing: ' . \implode(', ', $usage));
    }

    /**
     * Assert that a certificate may have issued a CRL.
     *
     * RFC 5280 section 6.3.3 (f) requires the basicConstraints CA flag plus a
     * keyUsage admitting cRLSign (section 4.2.1.3). A certificate without the
     * keyUsage extension is unrestricted for that purpose and passes; one that is
     * not a CA never does.
     *
     * @throws Exception If the certificate is not a CA or its key usage forbids cRLSign.
     */
    public function assertUsableForCrlSigning(string $certDer): void
    {
        // Both rules are read off one decode of the certificate extensions.
        $extensions = $this->extensions($certDer);

        if (!self::basicConstraintsCa($this->asn1, $extensions)) {
            throw new Exception('The CRL issuer certificate is not a certification authority');
        }

        $usage = self::decodedKeyUsage($this->asn1, $extensions);
        if ($usage === null || \in_array('cRLSign', $usage, true)) {
            return;
        }

        throw new Exception('The certificate key usage does not admit CRL signing: ' . \implode(', ', $usage));
    }

    /**
     * Discard whatever the last OpenSSL call left in the thread's error queue.
     *
     * The queue is process-wide and never drained by PHP, so entries left by a
     * failed verification would surface in the host's next openssl_error_string().
     */
    public static function clearOpenSslErrors(): void
    {
        // Discarded: a failure here is reported as an Exception, not through the queue.
        $entry = \openssl_error_string();
        while ($entry !== false) {
            $entry = \openssl_error_string();
        }
    }

    /**
     * Read a certificate's extendedKeyUsage purposes.
     *
     * RFC 5280 section 4.2.1.12: the extension states the purposes the key may be
     * used for, and a purpose that is not listed is one the key may not serve.
     * The purposes are returned as the OIDs the extension names, not as rendered
     * names.
     *
     * @return list<string>|null Purpose OIDs in dotted form, or null when the
     *                           extension is absent and every purpose is admitted.
     *
     * @throws Exception If the certificate or the extension cannot be read.
     */
    public function extendedKeyUsage(string $certDer): ?array
    {
        return $this->extendedKeyUsageWithCriticality($certDer)[0];
    }

    /**
     * True when a certificate's extendedKeyUsage extension is marked critical.
     *
     * RFC 3161 section 2.3 requires it of a TSA certificate.
     *
     * @throws Exception If the certificate cannot be read.
     */
    public function extendedKeyUsageIsCritical(string $certDer): bool
    {
        return $this->extendedKeyUsageWithCriticality($certDer)[1];
    }

    /**
     * Read a certificate's extendedKeyUsage purposes along with its criticality.
     *
     * Both values are read off one decode, which is what RFC 3161 section 2.3 asks
     * of a TSA certificate.
     *
     * @return array{list<string>|null, bool} [purpose OIDs in dotted form, or null when
     *         the extension is absent and every purpose is admitted; whether it is critical]
     *
     * @throws Exception If the certificate or the extension cannot be read.
     */
    public function extendedKeyUsageWithCriticality(string $certDer): array
    {
        $extension = $this->extensions($certDer)[self::OID_EXTENDED_KEY_USAGE] ?? null;
        if ($extension === null) {
            return [null, false];
        }

        $offset = 0;
        $sequence = $this->asn1->readTlv($extension['value'], $offset);
        if ($sequence['tag'] !== 0x30 || $offset !== \strlen($extension['value']) || $sequence['value'] === '') {
            // ExtKeyUsageSyntax ::= SEQUENCE SIZE (1..MAX) OF KeyPurposeId, so an
            // empty one states no purpose rather than admitting every purpose.
            throw new Exception('Invalid certificate extendedKeyUsage');
        }

        $purposes = [];
        $inner = 0;
        while ($inner < \strlen($sequence['value'])) {
            $purpose = $this->asn1->readTlv($sequence['value'], $inner);
            if ($purpose['tag'] !== 0x06) {
                throw new Exception('Invalid certificate extendedKeyUsage');
            }

            $oid = $this->asn1->decodeObjectIdentifier($purpose['value']);

            // A purpose stated twice is refused rather than collapsed, as
            // Asn1::decodeExtensions() refuses an extension type stated twice.
            if (\in_array($oid, $purposes, true)) {
                throw new Exception('Duplicate certificate extendedKeyUsage purpose: ' . $oid);
            }

            $purposes[] = $oid;
        }

        return [$purposes, $extension['critical']];
    }

    /**
     * True when a certificate's basicConstraints marks it as a CA.
     *
     * RFC 5280 section 4.2.1.9. An absent extension reads as not a CA.
     *
     * @throws Exception If the certificate or the extension cannot be read.
     */
    public function isCertificateAuthority(string $certDer): bool
    {
        return self::basicConstraintsCa($this->asn1, $this->extensions($certDer));
    }

    /**
     * Read a certificate's subjectKeyIdentifier.
     *
     * RFC 5280 section 4.2.1.2 shapes the extension value as
     * KeyIdentifier ::= OCTET STRING. It is what a CMS SignerIdentifier names when
     * it does not name an IssuerAndSerialNumber.
     *
     * @return string Raw identifier octets, or '' when the extension is absent or
     *                cannot be read, which can never equal a non-empty identifier.
     */
    public function subjectKeyIdentifier(string $certDer): string
    {
        try {
            $value = $this->extensions($certDer)[self::OID_SUBJECT_KEY_IDENTIFIER]['value'] ?? null;
            if ($value === null) {
                return '';
            }

            $offset = 0;
            $identifier = $this->asn1->readTlv($value, $offset);
            if ($identifier['tag'] !== 0x04 || $offset !== \strlen($value)) {
                return '';
            }

            return $identifier['value'];
        } catch (Exception) {
            // An identifier that cannot be read names nothing. The caller searches
            // an unauthenticated bag, so such a member is passed over.
            return '';
        }
    }

    /**
     * Read the basicConstraints CA flag off a decoded extension map.
     *
     * BasicConstraints ::= SEQUENCE { cA BOOLEAN DEFAULT FALSE,
     * pathLenConstraint INTEGER (0..MAX) OPTIONAL }, so an extension whose first
     * field is not the BOOLEAN is one that leaves cA at its default, and the two
     * fields tile the SEQUENCE exactly.
     *
     * The pathLenConstraint is bounded but not weighed: this class does not enforce
     * path lengths.
     *
     * @param array<string, array{critical: bool, value: string}> $extensions
     *
     * @throws Exception If the extension is present and malformed.
     */
    private static function basicConstraintsCa(Asn1 $asn1, array $extensions): bool
    {
        if (!\array_key_exists(self::OID_BASIC_CONSTRAINTS, $extensions)) {
            return false;
        }

        $value = $extensions[self::OID_BASIC_CONSTRAINTS]['value'];

        $offset = 0;
        $sequence = $asn1->readTlv($value, $offset);
        if ($sequence['tag'] !== 0x30 || $offset !== \strlen($value)) {
            throw new Exception('Invalid certificate basicConstraints');
        }

        $inner = 0;
        $field = $asn1->readOptionalTlv($sequence['value'], $inner);

        $certificateAuthority = false;
        if ($field !== null && $field['tag'] === 0x01) {
            // A BOOLEAN is one content octet (X.690 section 8.2.1). Any non-zero octet
            // reads as TRUE, as Asn1::decodeExtensions() reads a criticality flag.
            if (\strlen($field['value']) !== 1) {
                throw new Exception('Invalid certificate basicConstraints');
            }

            $certificateAuthority = $field['value'] !== "\x00";
            $field = $asn1->readOptionalTlv($sequence['value'], $inner);
        }

        if ($field !== null && $field['tag'] !== 0x02) {
            throw new Exception('Invalid certificate basicConstraints');
        }

        if ($inner !== \strlen($sequence['value'])) {
            throw new Exception('Trailing bytes in the certificate basicConstraints');
        }

        return $certificateAuthority;
    }

    /**
     * Read the keyUsage bits of a certificate.
     *
     * @return list<string>|null Bit names set, or null when the extension is absent
     *                           and every use is admitted.
     *
     * @throws Exception If the certificate or the extension cannot be read.
     */
    private function keyUsage(string $certDer): ?array
    {
        return self::decodedKeyUsage($this->asn1, $this->extensions($certDer));
    }

    /**
     * Read the keyUsage bits off a decoded extension map.
     *
     * KeyUsage ::= BIT STRING (RFC 5280 section 4.2.1.3). DER drops the trailing
     * zero bits, so the unused-bits count is honoured and a bit past the last
     * significant one reads as unset. Asn1::decodeBitString() does not serve here,
     * being for the BIT STRINGs that carry whole octets.
     *
     * @param array<string, array{critical: bool, value: string}> $extensions
     *
     * @return list<string>|null Bit names set, empty when the extension states none,
     *                           null when it is absent.
     *
     * @throws Exception If the extension is present and malformed.
     */
    private static function decodedKeyUsage(Asn1 $asn1, array $extensions): ?array
    {
        if (!\array_key_exists(self::OID_KEY_USAGE, $extensions)) {
            return null;
        }

        $value = $extensions[self::OID_KEY_USAGE]['value'];

        $offset = 0;
        $bitString = $asn1->readTlv($value, $offset);
        if ($bitString['tag'] !== 0x03 || $offset !== \strlen($value) || $bitString['value'] === '') {
            throw new Exception('Invalid certificate keyUsage');
        }

        $unused = \ord($bitString['value'][0]);
        $bits = \substr($bitString['value'], 1);
        if ($unused > 7 || $bits === '' && $unused !== 0) {
            throw new Exception('Invalid certificate keyUsage');
        }

        $significant = (\strlen($bits) * 8) - $unused;

        $usage = [];
        foreach (self::KEY_USAGE_BITS as $bit => $name) {
            // A bit past the last significant one was dropped by DER, so it is unset.
            if ($bit >= $significant || (\ord($bits[\intdiv($bit, 8)]) & (0x80 >> ($bit % 8))) === 0) {
                continue;
            }

            $usage[] = $name;
        }

        return $usage;
    }

    /**
     * True when the subject Name of the issuer certificate equals the issuer Name
     * of the subject certificate.
     *
     * Compares the DER of the two Names, which is the match rule CMS and OCSP
     * apply. It establishes the naming link only, not the signature.
     *
     * @throws Exception If either certificate cannot be parsed.
     */
    public function isIssuerOf(string $issuerDer, string $subjectDer): bool
    {
        return $this->fields($issuerDer)['subject'] === $this->fields($subjectDer)['issuer'];
    }

    /**
     * Unwrap a CMS ContentInfo to the content octets of its SignedData.
     *
     * The input must be exactly one ContentInfo, and each layer of it exactly one
     * element, with no trailing bytes.
     *
     * @throws Exception If the input is not a CMS SignedData.
     */
    public function signedDataContent(string $cmsDer): string
    {
        $offset = 0;
        $root = $this->asn1->readTlv($cmsDer, $offset);
        if ($root['tag'] !== 0x30 || $offset !== \strlen($cmsDer)) {
            throw new Exception('Invalid CMS structure');
        }

        $rootOff = 0;
        $type = $this->asn1->readTlv($root['value'], $rootOff);
        if ($type['raw'] !== $this->asn1->encodeObjectIdentifier(Oid::SIGNED_DATA)) {
            throw new Exception('The CMS is not a SignedData');
        }

        $content = $this->asn1->readTlv($root['value'], $rootOff);
        if ($content['tag'] !== 0xA0 || $rootOff !== \strlen($root['value'])) {
            throw new Exception('Missing CMS SignedData');
        }

        $contentOff = 0;
        $signedData = $this->asn1->readTlv($content['value'], $contentOff);
        if ($signedData['tag'] !== 0x30 || $contentOff !== \strlen($content['value'])) {
            throw new Exception('Invalid CMS SignedData');
        }

        return $signedData['value'];
    }

    /**
     * Read the eContentType and the eContent octets of a SignedData.
     *
     * RFC 5652 section 5.1 puts encapContentInfo third, after version and
     * digestAlgorithms, and section 5.2 shapes it as SEQUENCE { eContentType
     * OBJECT IDENTIFIER, eContent [0] EXPLICIT OCTET STRING OPTIONAL }. Nothing
     * follows either field. The two fields ahead of it are held to the shape
     * assertSignedDataHead() states.
     *
     * eContent is OPTIONAL and absent in a detached signature, which is what this
     * library emits for a PDF. Under $detached the field has to be absent rather
     * than merely unread.
     *
     * @param int  $offset   Read cursor; advanced past encapContentInfo.
     * @param bool $detached Expect a signature with no eContent of its own.
     *
     * @return array{string, string} [complete eContentType OID element, eContent
     *         octets, empty when $detached]
     *
     * @throws Exception If the content is malformed, or is absent and was expected,
     *                   or is present and was not.
     */
    public function encapsulatedContent(string $signedData, int &$offset, bool $detached = false): array
    {
        $version = $this->asn1->readTlv($signedData, $offset);
        $digestAlgorithms = $this->asn1->readTlv($signedData, $offset);
        $encap = $this->asn1->readTlv($signedData, $offset);

        $this->assertSignedDataHead($version, $digestAlgorithms, $encap);

        $encapOffset = 0;
        $contentType = $this->asn1->readTlv($encap['value'], $encapOffset);
        if ($contentType['tag'] !== 0x06) {
            throw new Exception('Invalid eContentType');
        }

        if ($detached) {
            if ($encapOffset !== \strlen($encap['value'])) {
                throw new Exception('The SignedData carries its own encapsulated content');
            }

            return [$contentType['raw'], ''];
        }

        // A caller that did not ask for the detached reading requires eContent.
        if ($encapOffset >= \strlen($encap['value'])) {
            throw new Exception('Missing encapsulated content');
        }

        $content = $this->asn1->readTlv($encap['value'], $encapOffset);
        if ($content['tag'] !== 0xA0 || $encapOffset !== \strlen($encap['value'])) {
            throw new Exception('Missing encapsulated content');
        }

        $contentOffset = 0;
        $octets = $this->asn1->readTlv($content['value'], $contentOffset);
        if ($octets['tag'] !== 0x04 || $contentOffset !== \strlen($content['value'])) {
            throw new Exception('Invalid encapsulated content');
        }

        return [$contentType['raw'], $octets['value']];
    }

    /**
     * Extract the X.509 certificates a CMS SignedData embeds.
     *
     * A CertificateChoices alternative that is not a plain certificate is tagged
     * and skipped, and a member that does not parse as a certificate is dropped,
     * so the result is always a list of DER certificates.
     *
     * The field sits outside signedAttrs and is covered by no signature, so each
     * member is parsed as a certificate before it is kept.
     *
     * Under $strict a tagged member is refused along with a SEQUENCE that is not a
     * certificate, and the rest of the SignedData is bounded: the crls [1] field,
     * the signerInfos SET, and the tail. RFC 5652 section 10.2.2 types the field as
     * a set of CertificateChoices.
     *
     * @param bool $strict Refuse any member that is not a certificate, rather than
     *                     dropping it, and bound the rest of the SignedData.
     *
     * @return list<string> DER certificates, empty when the CMS embeds none.
     *
     * @throws Exception If the CMS cannot be parsed, or $strict and a member is not
     *                   a certificate.
     */
    public function fromSignedData(string $cmsDer, bool $strict = false): array
    {
        $signedData = $this->signedDataContent($cmsDer);

        $offset = 0;
        $version = $this->asn1->readTlv($signedData, $offset);
        $digestAlgorithms = $this->asn1->readTlv($signedData, $offset);
        $encap = $this->asn1->readTlv($signedData, $offset);
        $afterEncap = $offset;

        if ($strict) {
            $this->assertSignedDataHead($version, $digestAlgorithms, $encap);
        }

        $certs = [];
        $field = $this->asn1->readOptionalTlv($signedData, $offset);
        if ($field !== null && $field['tag'] === 0xA0) {
            $set = $field['value'];
            $certOff = 0;
            while ($certOff < \strlen($set)) {
                try {
                    $cert = $this->asn1->readTlv($set, $certOff);
                } catch (Exception $e) {
                    if ($strict) {
                        throw $e;
                    }

                    // A member whose length octets overrun the field ends its tiling,
                    // so there is no next member to move to and what was read before
                    // it stands.
                    break;
                }

                if ($cert['tag'] !== 0x30) {
                    if ($strict) {
                        throw new Exception('The CertificateSet holds a member that is not a certificate');
                    }

                    continue;
                }

                try {
                    $this->fields($cert['raw']);
                } catch (Exception $e) {
                    if ($strict) {
                        throw new Exception('The CertificateSet holds a member that is not a certificate', 0, $e);
                    }

                    continue;
                }

                if ($strict && \count($certs) >= self::MAX_EMBEDDED_CERTIFICATES) {
                    throw new Exception(
                        'The CertificateSet holds more than ' . self::MAX_EMBEDDED_CERTIFICATES . ' certificates',
                    );
                }

                $certs[] = $cert['raw'];
            }

            $field = $this->asn1->readOptionalTlv($signedData, $offset);
        }

        if ($strict) {
            if ($field !== null && $field['tag'] === 0xA1) {
                $this->assertRevocationInfoChoices($field['value']);
            }

            // Bounds the certificates [0] and crls [1] fields, the signerInfos SET,
            // and the tail.
            $this->signerInfos($signedData, $afterEncap);
        }

        return $certs;
    }

    /**
     * Assert that the three fields ahead of the certificates are the ones RFC 5652
     * section 5.1 puts there.
     *
     * version is an INTEGER, digestAlgorithms a SET, and encapContentInfo a
     * SEQUENCE. The version is decoded as well as tag-checked; its range is not
     * checked, a version outside the set RFC 5652 names still being an INTEGER.
     *
     * @param array{tag: int, value: string, raw: string} $version
     * @param array{tag: int, value: string, raw: string} $digestAlgorithms
     * @param array{tag: int, value: string, raw: string} $encap
     *
     * @throws Exception If a field is of another tag or is not well formed.
     */
    private function assertSignedDataHead(array $version, array $digestAlgorithms, array $encap): void
    {
        if ($version['tag'] !== 0x02 || $digestAlgorithms['tag'] !== 0x31 || $encap['tag'] !== 0x30) {
            throw new Exception('Invalid SignedData field before the certificates');
        }

        $this->asn1->decodeInteger($version['value']);

        // DigestAlgorithmIdentifiers ::= SET OF DigestAlgorithmIdentifier, whose
        // members tile the field exactly and are each bounded in turn.
        $memberOffset = 0;
        while ($memberOffset < \strlen($digestAlgorithms['value'])) {
            $member = $this->asn1->readTlv($digestAlgorithms['value'], $memberOffset);
            $this->asn1->decodeAlgorithmIdentifier($member['raw'], 'SignedData digest');
        }
    }

    /**
     * Assert that a SignedData crls [1] field holds only RevocationInfoChoice members.
     *
     * RFC 5652 section 10.2.1: RevocationInfoChoice ::= CHOICE { crl CertificateList,
     * other [1] OtherRevocationInfoFormat }, so a member is a SEQUENCE or a [1] and
     * the members tile the field exactly.
     *
     * @param string $choices Content octets of the crls [1] field.
     *
     * @throws Exception If the field holds anything else.
     */
    private function assertRevocationInfoChoices(string $choices): void
    {
        $offset = 0;
        while ($offset < \strlen($choices)) {
            $choice = $this->asn1->readTlv($choices, $offset);
            if ($choice['tag'] !== 0x30 && $choice['tag'] !== 0xA1) {
                throw new Exception('The SignedData crls field holds a member that is not revocation information');
            }
        }
    }

    /**
     * Extract the RFC 3161 tokens a CMS carries as signature timestamps.
     *
     * Reads the id-aa-signatureTimeStampToken unsigned attribute of each SignerInfo
     * (CAdES, ETSI EN 319 122-1 section 5.3), which sign() embeds for a PAdES B-T
     * signature and Signer::collectValidationMaterial() takes as input.
     *
     * unsignedAttrs sits outside the signature, so a member that cannot be read as a
     * CMS SignedData is passed over rather than returned or thrown on.
     *
     * @return list<string> DER tokens, empty when the CMS carries none.
     *
     * @throws Exception If the CMS cannot be parsed.
     */
    public function signatureTimestampTokens(string $cmsDer): array
    {
        $signedData = $this->signedDataContent($cmsDer);

        $offset = 0;
        $this->asn1->readTlv($signedData, $offset); // version
        $this->asn1->readTlv($signedData, $offset); // digestAlgorithms
        $this->asn1->readTlv($signedData, $offset); // encapContentInfo

        $wanted = $this->asn1->encodeObjectIdentifier(Oid::SIGNATURE_TIMESTAMP);

        $tokens = [];
        foreach ($this->signerInfos($signedData, $offset) as $signerInfo) {
            foreach ($this->unsignedAttributes($signerInfo) as $attribute) {
                \array_push($tokens, ...$this->signatureTimestampValues($attribute, $wanted));
            }
        }

        return self::deduplicate($tokens);
    }

    /**
     * Read the tokens one unsigned Attribute carries, or none.
     *
     * @param string $attribute Attribute content octets.
     * @param string $wanted    Complete DER of the attribute type looked for.
     *
     * @return list<string> DER tokens, empty when the attribute is of another type or
     *                      cannot be read.
     */
    private function signatureTimestampValues(string $attribute, string $wanted): array
    {
        try {
            // Attribute ::= SEQUENCE { attrType, attrValues SET OF }, with nothing
            // after the two, as the signed attributes are read elsewhere.
            $inner = 0;
            $type = $this->asn1->readTlv($attribute, $inner);
            $values = $this->asn1->readTlv($attribute, $inner);
            if ($type['tag'] !== 0x06 || $inner !== \strlen($attribute)) {
                return [];
            }

            if ($type['raw'] !== $wanted || $values['tag'] !== 0x31) {
                return [];
            }

            $tokens = [];
            $valueOffset = 0;
            while ($valueOffset < \strlen($values['value'])) {
                try {
                    $token = $this->asn1->readTlv($values['value'], $valueOffset);
                } catch (Exception) {
                    // A value whose length octets overrun the SET ends its tiling, so
                    // there is no next value to move to and what was read before it
                    // stands, as in unsignedAttributes() one level up.
                    break;
                }

                try {
                    $this->signedDataContent($token['raw']);
                } catch (Exception) {
                    continue;
                }

                $tokens[] = $token['raw'];
            }

            return $tokens;
        } catch (Exception) {
            // An attribute whose own type or value SET cannot be read names no
            // token. Only this attribute is given up on.
            return [];
        }
    }

    /**
     * Read the content octets of each SignerInfo of a SignedData.
     *
     * RFC 5652 section 5.1 puts certificates [0] and crls [1] between
     * encapContentInfo and signerInfos, each OPTIONAL, each admissible once, and in
     * that order. Nothing may follow signerInfos.
     *
     * @param int $offset Read cursor positioned just after encapContentInfo.
     *
     * @return list<string> SignerInfo content octets, one per member of the SET.
     *
     * @throws Exception If the structure is malformed or carries no SignerInfo.
     */
    public function signerInfos(string $signedData, int $offset): array
    {
        $field = $this->asn1->readOptionalTlv($signedData, $offset);
        if ($field !== null && $field['tag'] === 0xA0) {
            $field = $this->asn1->readOptionalTlv($signedData, $offset);
        }

        if ($field !== null && $field['tag'] === 0xA1) {
            $field = $this->asn1->readOptionalTlv($signedData, $offset);
        }

        if ($field === null || $field['tag'] !== 0x31 || $field['value'] === '') {
            throw new Exception('The SignedData carries no SignerInfo');
        }

        if ($offset !== \strlen($signedData)) {
            throw new Exception('Trailing bytes after the SignedData signerInfos');
        }

        $infos = [];
        $setOffset = 0;
        while ($setOffset < \strlen($field['value'])) {
            $info = $this->asn1->readTlv($field['value'], $setOffset);
            if ($info['tag'] !== 0x30) {
                throw new Exception('Invalid SignerInfo');
            }

            $infos[] = $info['value'];
        }

        return $infos;
    }

    /**
     * Read the members of a SignerInfo unsignedAttrs [1] field.
     *
     * The field is covered by no signature, so a member that is not an Attribute is
     * passed over and one whose encoding cannot be read ends the walk. The fields
     * ahead of it are the signed structure and stay fatal.
     *
     * @param string $signerInfo SignerInfo content octets.
     *
     * @return list<string> Attribute content octets, empty when the field is absent.
     *
     * @throws Exception If the SignerInfo itself is malformed.
     */
    private function unsignedAttributes(string $signerInfo): array
    {
        $offset = 0;
        $this->asn1->readTlv($signerInfo, $offset); // version
        $this->asn1->readTlv($signerInfo, $offset); // sid
        $this->asn1->readTlv($signerInfo, $offset); // digestAlgorithm

        $field = $this->asn1->readTlv($signerInfo, $offset);
        if ($field['tag'] === 0xA0) {
            $field = $this->asn1->readTlv($signerInfo, $offset); // signatureAlgorithm
        }

        $this->asn1->readTlv($signerInfo, $offset); // signature

        if ($offset >= \strlen($signerInfo)) {
            return [];
        }

        $unsigned = $this->asn1->readTlv($signerInfo, $offset);
        if ($unsigned['tag'] !== 0xA1) {
            return [];
        }

        $attributes = [];
        $attributeOffset = 0;
        while ($attributeOffset < \strlen($unsigned['value'])) {
            try {
                $attribute = $this->asn1->readTlv($unsigned['value'], $attributeOffset);
            } catch (Exception) {
                // A member whose length octets overrun the field ends the tiling of
                // the SET, so there is no next member to move to. What was read
                // before it stands.
                break;
            }

            if ($attribute['tag'] !== 0x30) {
                continue;
            }

            $attributes[] = $attribute['value'];
        }

        return $attributes;
    }

    /**
     * Decode a PEM certificate to DER.
     *
     * Exactly one certificate is decoded; a file holding more than one, such as a
     * fullchain.pem, is refused. The armour has to say CERTIFICATE, and the decoded
     * bytes have to parse as one. A body with no armour is accepted as base64 and
     * held to the same parse.
     *
     * @throws Exception If the PEM holds no certificate, more than one, or something
     *                   that is not one.
     */
    public static function pemToDer(string $pem): string
    {
        if (\substr_count($pem, '-----BEGIN ') > 1) {
            throw new Exception('The PEM holds more than one certificate');
        }

        // Located with strpos() rather than matched: a lazily quantified pattern
        // over the body exhausts pcre.backtrack_limit above roughly a megabyte.
        $body = $pem;
        if (\str_contains($pem, '-----BEGIN ')) {
            $begin = \strpos($pem, self::PEM_BEGIN);
            $end = $begin === false ? false : \strpos($pem, self::PEM_END, $begin + \strlen(self::PEM_BEGIN));
            if ($begin === false || $end === false) {
                throw new Exception('The PEM does not hold a certificate');
            }

            $body = \substr($pem, $begin + \strlen(self::PEM_BEGIN), $end - $begin - \strlen(self::PEM_BEGIN));
        }

        $der = \base64_decode((string) \preg_replace('/\s+/', '', $body), true);
        if ($der === false || $der === '') {
            throw new Exception('Invalid PEM certificate');
        }

        try {
            (new self())->fields($der);
        } catch (Exception $e) {
            throw new Exception('Invalid PEM certificate', 0, $e);
        }

        return $der;
    }

    /**
     * Decode a certificate given as either PEM or DER. Both encodings are parsed.
     *
     * A DER certificate begins with the SEQUENCE tag 0x30 and a long-form length,
     * whose first octet has the high bit set. A PEM body cannot: it is ASCII, so its
     * second octet is always below 0x80.
     *
     * @throws Exception If the value is neither a PEM nor a DER certificate.
     */
    public static function toDer(string $certificate): string
    {
        if (\strlen($certificate) > 1 && $certificate[0] === "\x30" && (\ord($certificate[1]) & 0x80) !== 0) {
            (new self())->fields($certificate);

            return $certificate;
        }

        return self::pemToDer($certificate);
    }

    /**
     * Wrap DER certificate bytes as PEM.
     */
    public static function derToPem(string $der): string
    {
        return self::PEM_BEGIN . "\n" . \chunk_split(\base64_encode($der), 64, "\n") . self::PEM_END . "\n";
    }

    /**
     * Deduplicate a list of binary blobs by content, preserving first-seen order.
     *
     * @param list<string> $items
     *
     * @return list<string>
     */
    public static function deduplicate(array $items): array
    {
        $seen = [];
        $result = [];
        foreach ($items as $item) {
            // Keyed on the blob itself: a PHP array takes binary string keys, and a
            // DER structure never reads as an integer-like key that would be coerced.
            if (isset($seen[$item])) {
                continue;
            }

            $seen[$item] = true;
            $result[] = $item;
        }

        return $result;
    }
}
