<?php

declare(strict_types=1);

/**
 * Crl.php
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

namespace Com\Tecnick\Pdf\Sign\Ltv;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Cms\Certificate;
use Com\Tecnick\Pdf\Sign\Cms\SignatureVerifier;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Ocsp\Client as OcspClient;
use Com\Tecnick\Pdf\Sign\RevokedException;

/**
 * Com\Tecnick\Pdf\Sign\Ltv\Crl
 *
 * RFC 5280 CertificateList reader. It checks what has to hold before a CRL is
 * embedded in a Document Security Store as evidence: the bytes are one complete
 * DER CertificateList, the issuer Name is the one that issued the certificate the
 * distribution point came from, the two signature AlgorithmIdentifiers agree, the
 * validity interval covers the moment of use, the list is a complete one rather
 * than a delta or a scoped partition, and the signature verifies against the
 * issuer's public key.
 *
 * The issuer certificate is required, none of the above being establishable
 * without it. Given the certificate the list is being fetched for, its serial is
 * looked up among the revoked entries as well, and a match is reported as a
 * RevokedException.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Crl
{
    /**
     * Default clock skew tolerated when checking the validity interval, in seconds.
     * Ocsp\Client and Timestamp\Client read the same value.
     */
    public const CLOCK_SKEW = OcspClient::CLOCK_SKEW;

    /**
     * Default age limit applied to thisUpdate, in seconds.
     */
    public const DEFAULT_MAX_AGE = OcspClient::DEFAULT_MAX_AGE;

    /**
     * id-ce-deltaCRLIndicator, marking a list of the changes since a base CRL.
     */
    private const OID_DELTA_CRL_INDICATOR = '2.5.29.27';

    /**
     * id-ce-issuingDistributionPoint, describing what a CRL covers.
     */
    private const OID_ISSUING_DISTRIBUTION_POINT = '2.5.29.28';

    /**
     * Extension types this reader understands well enough to accept when critical.
     *
     * id-ce-cRLNumber is a monotonic counter and id-ce-authorityKeyIdentifier names
     * the key that signed. Neither narrows what the list covers. The two scope
     * extensions are handled before this list is consulted.
     *
     * @var list<string>
     */
    private const KNOWN_EXTENSIONS = [
        '2.5.29.20', // id-ce-cRLNumber
        '2.5.29.35', // id-ce-authorityKeyIdentifier
    ];

    /**
     * Revocation entry extension types this reader understands when critical.
     *
     * id-ce-cRLReason says why an entry is there and id-ce-invalidityDate says since
     * when; neither changes that the serial is listed. id-ce-certificateIssuer is
     * not among them: it attributes an entry to another authority, and RFC 5280
     * section 5.3.3 requires it to be critical.
     *
     * @var list<string>
     */
    private const KNOWN_ENTRY_EXTENSIONS = [
        '2.5.29.21', // id-ce-cRLReason
        '2.5.29.24', // id-ce-invalidityDate
    ];

    private Asn1 $asn1;

    private Certificate $certificate;

    private SignatureVerifier $verifier;

    /**
     * @param int $maxAge Age limit applied to thisUpdate, in seconds. Zero disables
     *                    the bound.
     * @param int $clockSkew Skew tolerated between the validity interval and the moment
     *                    of use, in seconds.
     *
     * @throws Exception If the age limit or the skew is negative.
     */
    public function __construct(
        ?Asn1 $asn1 = null,
        ?Certificate $certificate = null,
        ?SignatureVerifier $verifier = null,
        private readonly int $maxAge = self::DEFAULT_MAX_AGE,
        private readonly int $clockSkew = self::CLOCK_SKEW,
    ) {
        if ($maxAge < 0) {
            throw new Exception('Invalid CRL age limit: ' . $maxAge);
        }

        if ($clockSkew < 0) {
            throw new Exception('Invalid CRL clock skew: ' . $clockSkew);
        }

        $this->asn1 = $asn1 ?? new Asn1();
        $this->certificate = $certificate ?? new Certificate($this->asn1);
        $this->verifier = $verifier ?? new SignatureVerifier($this->asn1);
    }

    /**
     * Validate a DER CertificateList against the certificate that issued it.
     *
     * @param string      $crlDer     DER-encoded CertificateList.
     * @param string      $issuerDer  DER of the issuing certificate.
     * @param string|null $subjectDer DER of the certificate the list is being fetched
     *                                for, or null to run the structural checks alone
     *                                and skip the revocation lookup.
     * @param int|null    $now        Unix time the validity interval is checked against;
     *                                defaults to the current time.
     *
     * @return string The CRL bytes unchanged, once accepted.
     *
     * @throws RevokedException If $subjectDer is listed as revoked.
     * @throws Exception If the CRL is malformed, issued by another authority or by a
     *                   certificate that may not sign CRLs, outside its validity
     *                   interval, incomplete in scope, or not correctly signed.
     */
    public function validate(string $crlDer, string $issuerDer, ?string $subjectDer, ?int $now = null): string
    {
        $root = $this->asn1->readSingleElement($crlDer, 0x30, 'CRL');

        $listOffset = 0;
        $tbs = $this->asn1->readTlv($root['value'], $listOffset);
        if ($tbs['tag'] !== 0x30) {
            throw new Exception('Invalid TBSCertList');
        }

        $algorithmId = $this->asn1->readTlv($root['value'], $listOffset);
        $signature = $this->asn1->decodeBitString($this->asn1->readTlv($root['value'], $listOffset));
        if ($listOffset !== \strlen($root['value'])) {
            throw new Exception('Trailing bytes after the CertificateList');
        }

        $fields = $this->fields($tbs['value']);

        // RFC 5280 section 5.1.1.2: the inner AlgorithmIdentifier exists so the
        // algorithm is covered by the signature, and must equal the outer one.
        if ($fields['algorithm'] !== $algorithmId['raw']) {
            throw new Exception('The CRL signature algorithms do not match');
        }

        // Read once for both the issuer rule and the serial lookup.
        $subjectFields = $subjectDer === null ? null : $this->certificate->fields($subjectDer);

        $this->checkIssuer($fields['issuer'], $issuerDer, $subjectFields);
        $this->checkScope($fields['extensions'], $subjectDer);

        $this->verifier->verify($tbs['raw'], $algorithmId['raw'], $signature, $issuerDer);

        // Read after the signature and the scope rules, which decide whether the list
        // may determine status at all (RFC 5280 section 6.3.3 (a)(2)), and before the
        // freshness rule, so a revoked verdict outranks a stale list.
        $this->checkNotRevoked($fields['revoked'], $subjectFields === null ? null : $subjectFields['serial']);

        $this->checkValidity($fields, $now ?? \time());

        return $crlDer;
    }

    /**
     * Split a TBSCertList into the fields the acceptance rules need.
     *
     * @param string $tbs TBSCertList content octets.
     *
     * @return array{algorithm: string, issuer: string, this_update: int, next_update: int|null,
     *           revoked: string, extensions: string}
     *
     * @throws Exception If the structure is malformed.
     */
    private function fields(string $tbs): array
    {
        $offset = 0;

        // version is OPTIONAL and absent in a v1 CRL; when present it precedes the
        // signature AlgorithmIdentifier. Decoded rather than stepped over, which
        // holds it to X.690 section 8.3.2. The range is not checked.
        $field = $this->asn1->readTlv($tbs, $offset);
        if ($field['tag'] === 0x02) {
            $this->asn1->decodeInteger($field['value']);
            $field = $this->asn1->readTlv($tbs, $offset);
        }

        if ($field['tag'] !== 0x30) {
            throw new Exception('Invalid CRL signature algorithm');
        }

        $algorithm = $field['raw'];

        $issuer = $this->asn1->readTlv($tbs, $offset);
        if ($issuer['tag'] !== 0x30) {
            throw new Exception('Invalid CRL issuer');
        }

        $thisUpdate = $this->asn1->decodeTime($this->asn1->readTlv($tbs, $offset));

        $nextUpdate = null;
        $revoked = '';
        $extensions = '';

        // nextUpdate, revokedCertificates and crlExtensions [0] are each OPTIONAL and
        // appear in that order, and RFC 5280 section 5.1.2 puts nothing after the last
        // of them. Each is consumed at its own position and anything else is refused.
        $field = $this->asn1->readOptionalTlv($tbs, $offset);

        if ($field !== null && ($field['tag'] === 0x17 || $field['tag'] === 0x18)) {
            $nextUpdate = $this->asn1->decodeTime($field);
            $field = $this->asn1->readOptionalTlv($tbs, $offset);
        }

        if ($field !== null && $field['tag'] === 0x30) {
            $revoked = $field['value'];
            $field = $this->asn1->readOptionalTlv($tbs, $offset);
        }

        if ($field !== null) {
            if ($field['tag'] !== 0xA0) {
                throw new Exception('Invalid TBSCertList field after thisUpdate');
            }

            // An EXPLICIT tag wraps exactly one element (X.690 section 8.14), so an
            // empty one is refused rather than read as an absent field, as
            // Ocsp\Client::optionalExtensions() refuses its own.
            if ($field['value'] === '') {
                throw new Exception('Empty CRL crlExtensions');
            }

            $extensions = $field['value'];
        }

        if ($offset !== \strlen($tbs)) {
            throw new Exception('Trailing bytes after the TBSCertList extensions');
        }

        return [
            'algorithm' => $algorithm,
            'issuer' => $issuer['raw'],
            'this_update' => $thisUpdate,
            'next_update' => $nextUpdate,
            'revoked' => $revoked,
            'extensions' => $extensions,
        ];
    }

    /**
     * Check that the CRL names the issuing certificate as its issuer, and that the
     * certificate being looked up is one that certificate issued.
     *
     * @param array{serial: string, issuer: string, subject: string, public_key: string,
     *           not_before: int, not_after: int}|null $subjectFields Fields of the
     *           certificate the lookup is about, or null when none was given.
     *
     * @throws Exception If the issuer certificate cannot be parsed or a Name differs.
     */
    private function checkIssuer(string $issuerName, string $issuerDer, ?array $subjectFields): void
    {
        if ($issuerName !== $this->certificate->fields($issuerDer)['subject']) {
            throw new Exception('The CRL was issued by another authority');
        }

        // RFC 5280 section 4.1.2.2: a serial number is unique only within an issuer,
        // so a list from an authority that did not issue the certificate indexes
        // nothing about it.
        if ($subjectFields !== null && $subjectFields['issuer'] !== $issuerName) {
            throw new Exception('The CRL was not issued by the authority that issued the certificate');
        }

        // RFC 5280 section 6.3.3 (f): the key that signed has to be one the issuer
        // authorised to sign a CRL. The counterpart of the id-kp-OCSPSigning check on
        // the OCSP side.
        $this->certificate->assertUsableForCrlSigning($issuerDer);
    }

    /**
     * Check the validity interval and the age of a CRL.
     *
     * As on the OCSP side, the age bound applies whether or not a nextUpdate
     * follows; RFC 5280 section 5.1.2.5 puts one on every conforming CRL.
     *
     * @param array{algorithm: string, issuer: string, this_update: int, next_update: int|null,
     *           revoked: string, extensions: string} $fields
     *
     * @throws Exception If the interval does not cover $now, or the list is older
     *                   than the age limit.
     */
    private function checkValidity(array $fields, int $now): void
    {
        if ($fields['this_update'] > ($now + $this->clockSkew)) {
            throw new Exception('The CRL is not yet valid');
        }

        if ($this->maxAge > 0 && $fields['this_update'] < ($now - $this->maxAge - $this->clockSkew)) {
            throw new Exception('The CRL is too old');
        }

        if ($fields['next_update'] !== null && $fields['next_update'] < ($now - $this->clockSkew)) {
            throw new Exception('The CRL has expired');
        }
    }

    /**
     * Reject a CRL that does not stand on its own for the certificate at hand.
     *
     * A delta CRL (RFC 5280 section 5.2.4) lists only the changes since a base list,
     * and an issuingDistributionPoint (section 5.2.5) may narrow the list to a
     * subset of revocation reasons, to another authority's certificates, or to
     * attribute certificates. Both are refused.
     *
     * An extension marked critical that this reader does not recognise is refused
     * too, per RFC 5280 section 6.3.3 step (a)(2).
     *
     * @param string      $extensions crlExtensions content octets.
     * @param string|null $subjectDer The certificate the list is being fetched for.
     *
     * @throws Exception If the CRL is a delta, is scoped to something else, or carries
     *                   an unknown critical extension.
     */
    private function checkScope(string $extensions, ?string $subjectDer): void
    {
        foreach ($this->asn1->decodeExtensions($extensions, 'CRL extension') as $oid => $extension) {
            if ($oid === self::OID_DELTA_CRL_INDICATOR) {
                throw new Exception('The CRL is a delta CRL, not a complete list');
            }

            if ($oid === self::OID_ISSUING_DISTRIBUTION_POINT) {
                $this->checkDistributionPoint($extension['value'], $subjectDer);
                continue;
            }

            if ($extension['critical'] && !\in_array($oid, self::KNOWN_EXTENSIONS, true)) {
                throw new Exception('Unsupported critical CRL extension: ' . $oid);
            }
        }
    }

    /**
     * Reject an issuingDistributionPoint that narrows the list.
     *
     * @param string      $value      Extension value octets (the DER IssuingDistributionPoint).
     * @param string|null $subjectDer The certificate the list is being fetched for.
     *
     * @throws Exception If the structure is malformed or the scope is narrowed.
     */
    private function checkDistributionPoint(string $value, ?string $subjectDer): void
    {
        $offset = 0;
        $point = $this->asn1->readTlv($value, $offset);

        // The extension value is one IssuingDistributionPoint and nothing else.
        if ($point['tag'] !== 0x30 || $offset !== \strlen($value)) {
            throw new Exception('Invalid CRL issuingDistributionPoint');
        }

        // IssuingDistributionPoint fields are IMPLICIT: distributionPoint [0] is a
        // constructed DistributionPointName, onlySomeReasons [3] a BIT STRING, and
        // indirectCRL [4] and onlyContainsAttributeCerts [5] BOOLEANs.
        //
        // RFC 5280 section 5.2.5 and the matching rule in section 6.3.3 (b)(1): a
        // present distributionPoint restricts the list to the certificates whose own
        // cRLDistributionPoints name that same point. Which shard answers cannot be
        // decided from the list alone, so a named point is refused as the other
        // narrowings are.
        $narrowed = [
            0xA0 => 'a named distribution point',
            0x83 => 'a subset of revocation reasons',
            0x84 => 'another authority',
            0x85 => 'attribute certificates',
        ];

        // onlyContainsUserCerts [1], onlyContainsCACerts [2], indirectCRL [4] and
        // onlyContainsAttributeCerts [5] are BOOLEAN DEFAULT FALSE, so one carrying
        // FALSE narrows nothing. onlySomeReasons [3] is a BIT STRING with no default
        // and is not exempt: an empty one covers no reason at all (RFC 5280 section
        // 6.3.3 (b)(1) intersects it into the reasons mask).
        $defaultFalse = [0x81 => true, 0x82 => true, 0x84 => true, 0x85 => true];

        $inner = 0;
        while ($inner < \strlen($point['value'])) {
            $field = $this->asn1->readTlv($point['value'], $inner);

            if (isset($defaultFalse[$field['tag']]) && $field['value'] === "\x00") {
                continue;
            }

            if (isset($narrowed[$field['tag']])) {
                throw new Exception('The CRL is scoped to ' . $narrowed[$field['tag']]);
            }

            // onlyContainsUserCerts [1] and onlyContainsCACerts [2] split the issuer's
            // certificates in two, so which half answers is decided against the
            // certificate the list was fetched for (RFC 5280 section 6.3.3 (b)(2)).
            if ($field['tag'] === 0x81 || $field['tag'] === 0x82) {
                $this->checkPartition($field['tag'] === 0x82, $subjectDer);
                continue;
            }

            // IssuingDistributionPoint has the six fields above and no others, so
            // anything else is a scope statement this reader cannot weigh. A
            // constructed [3] or [4] reaches here, being BER rather than the DER RFC
            // 5280 section 5 requires.
            throw new Exception('Unsupported CRL issuingDistributionPoint field: ' . $field['tag']);
        }
    }

    /**
     * Check that a CRL covering one class of certificate covers this one.
     *
     * @param bool        $caOnly     True for onlyContainsCACerts, false for onlyContainsUserCerts.
     * @param string|null $subjectDer The certificate the list is being fetched for.
     *
     * @throws Exception If the list covers the other class, or there is no
     *                   certificate to decide which class is wanted.
     */
    private function checkPartition(bool $caOnly, ?string $subjectDer): void
    {
        if ($subjectDer === null) {
            throw new Exception('The CRL covers one class of certificate and none was given to check it against');
        }

        if ($this->certificate->isCertificateAuthority($subjectDer) !== $caOnly) {
            throw new Exception(
                $caOnly ? 'The CRL covers CA certificates only' : 'The CRL covers end-entity certificates only',
            );
        }
    }

    /**
     * Walk the revoked entries, and look a certificate's serial up among them.
     *
     * Every entry is read, not only the ones up to a match, since a critical entry
     * extension this reader cannot process makes the whole list unusable wherever it
     * sits (RFC 5280 section 5.3). The walk runs whether or not a serial was given;
     * only the lookup is the caller's to skip.
     *
     * No entry extension is weighed. id-ce-cRLReason could invert a match through
     * removeFromCRL, but that appears only on a delta CRL, which checkScope()
     * refuses a step earlier.
     *
     * A match already found outranks a structural fault in a later entry, revoked
     * being the verdict that says the certificate must not be used.
     *
     * @param string      $revoked revokedCertificates content octets.
     * @param string|null $serial  Complete DER of the serial to look up, or null to
     *                             walk the entries without looking one up.
     *
     * @throws RevokedException If the serial is listed.
     * @throws Exception If the structure is malformed or an entry carries an unknown
     *                   critical extension.
     */
    private function checkNotRevoked(string $revoked, ?string $serial): void
    {
        $listed = false;

        try {
            $offset = 0;
            while ($offset < \strlen($revoked)) {
                $entry = $this->asn1->readTlv($revoked, $offset);
                if ($entry['tag'] !== 0x30) {
                    throw new Exception('Invalid CRL revocation entry');
                }

                $inner = 0;
                $userCertificate = $this->asn1->readTlv($entry['value'], $inner);
                if ($userCertificate['tag'] !== 0x02) {
                    throw new Exception('Invalid CRL revocation entry');
                }

                // A serial runs to 20 octets (RFC 5280 section 4.1.2.2), too wide to
                // decode, so only its minimality is checked; the comparison below is
                // against the DER the certificate carries.
                $this->asn1->assertMinimalInteger($userCertificate['value']);

                // RFC 5280 section 5.1 types revocationDate a Time.
                $this->asn1->decodeTime($this->asn1->readTlv($entry['value'], $inner));

                if ($inner < \strlen($entry['value'])) {
                    $this->checkEntryExtensions($this->asn1->readTlv($entry['value'], $inner)['raw']);
                }

                // crlEntryExtensions is the last field of an entry (RFC 5280 section
                // 5.1), so nothing may follow it.
                if ($inner !== \strlen($entry['value'])) {
                    throw new Exception('Trailing bytes in a CRL revocation entry');
                }

                $listed = $listed || $serial !== null && $userCertificate['raw'] === $serial;
            }
        } catch (Exception $e) {
            // A fault ahead of the match leaves nothing found, so the list is refused
            // as malformed rather than as revoked.
            if (!$listed) {
                throw $e;
            }

            throw new RevokedException('The certificate is revoked', 0, $e);
        }

        if ($listed) {
            throw new RevokedException('The certificate is revoked');
        }
    }

    /**
     * Refuse a revocation entry extension marked critical that this reader does not
     * understand.
     *
     * @param string $extensionsDer Complete DER of the crlEntryExtensions SEQUENCE.
     *
     * @throws Exception If the structure is malformed or an unknown type is critical.
     */
    private function checkEntryExtensions(string $extensionsDer): void
    {
        foreach ($this->asn1->decodeExtensions($extensionsDer, 'CRL entry extension') as $oid => $extension) {
            if ($extension['critical'] && !\in_array($oid, self::KNOWN_ENTRY_EXTENSIONS, true)) {
                throw new Exception('Unsupported critical CRL entry extension: ' . $oid);
            }
        }
    }
}
