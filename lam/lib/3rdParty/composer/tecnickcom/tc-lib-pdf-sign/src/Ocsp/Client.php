<?php

declare(strict_types=1);

/**
 * Client.php
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

namespace Com\Tecnick\Pdf\Sign\Ocsp;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Cms\Certificate;
use Com\Tecnick\Pdf\Sign\Cms\SignatureVerifier;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\RevokedException;

/**
 * Com\Tecnick\Pdf\Sign\Ocsp\Client
 *
 * RFC 6960 OCSP codec. build() assembles an OCSPRequest with a SHA-1 CertID over
 * the issuer's subject Name and public key and the target's serial number.
 * parseResponse() applies the RFC 6960 section 3.2 acceptance rules before a
 * response is used: successful status, a basic response type, a signature that
 * verifies against a responder the issuer authorised and that is itself inside
 * its validity period, a CertID matching the request, a good certificate status,
 * and a validity interval that covers the moment of use.
 *
 * Rule 5, that thisUpdate is sufficiently recent, is applied whether or not the
 * response carries a nextUpdate: a response is accepted only while it is younger
 * than $maxAge.
 *
 * HTTP transport is injected into fetch(), so the codec performs no network
 * access and the host controls networking and SSRF protection.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Client
{
    /**
     * id-pkix-ocsp-basic, the only response type this codec accepts.
     */
    private const OID_BASIC_RESPONSE = '1.3.6.1.5.5.7.48.1.1';

    /**
     * id-sha1, the CertID digest RFC 6960 responders are required to support.
     */
    private const OID_SHA1 = '1.3.14.3.2.26';

    /**
     * id-kp-OCSPSigning, the purpose a delegated responder certificate must carry.
     */
    private const OID_OCSP_SIGNING = '1.3.6.1.5.5.7.3.9';

    /**
     * Extension types this codec understands well enough to accept when critical.
     *
     * RFC 6960 section 4.4: id-pkix-ocsp-nonce ties a response to a request, and
     * id-pkix-ocsp-archive-cutoff states how far back the responder keeps records.
     * Neither narrows what a good status means. Anything else marked critical is
     * refused.
     *
     * @var list<string>
     */
    private const KNOWN_EXTENSIONS = [
        '1.3.6.1.5.5.7.48.1.2', // id-pkix-ocsp-nonce, { id-pkix-ocsp 2 }
        '1.3.6.1.5.5.7.48.1.6', // id-pkix-ocsp-archive-cutoff, { id-pkix-ocsp 6 }
    ];

    /**
     * OCSPResponseStatus values, for the rejection message.
     *
     * @var array<int, string>
     */
    private const RESPONSE_STATUS = [
        1 => 'malformedRequest',
        2 => 'internalError',
        3 => 'tryLater',
        5 => 'sigRequired',
        6 => 'unauthorized',
    ];

    /**
     * Default clock skew tolerated when checking the response validity interval,
     * in seconds. Timestamp\Client and Ltv\Crl read the same value.
     */
    public const CLOCK_SKEW = 300;

    /**
     * Default age limit applied to thisUpdate, in seconds.
     */
    public const DEFAULT_MAX_AGE = 604_800;

    /**
     * Most certificates accepted in the certs [0] bag of a response.
     *
     * Read from Cms\Certificate, as Signer::MAX_PATH_CERTIFICATES is, so every
     * unauthenticated certificate bag is held to the same bound.
     */
    public const MAX_RESPONDER_CERTIFICATES = Certificate::MAX_EMBEDDED_CERTIFICATES;

    private Asn1 $asn1;

    private Certificate $certificate;

    private SignatureVerifier $verifier;

    /**
     * @param int $maxAge Age limit applied to thisUpdate, in seconds. Zero disables
     *                    the bound.
     * @param int $clockSkew Skew tolerated between the response validity interval and
     *                    the moment of use, in seconds.
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
            throw new Exception('Invalid OCSP response age limit: ' . $maxAge);
        }

        if ($clockSkew < 0) {
            throw new Exception('Invalid OCSP clock skew: ' . $clockSkew);
        }

        $this->asn1 = $asn1 ?? new Asn1();
        $this->certificate = $certificate ?? new Certificate($this->asn1);
        $this->verifier = $verifier ?? new SignatureVerifier($this->asn1);
    }

    /**
     * Build an RFC 6960 OCSPRequest for a single certificate.
     *
     * No nonce extension is sent: RFC 6960 section 4.4.1 makes it optional and many
     * responders pre-sign and reject it. The response age is bounded by $maxAge
     * instead.
     *
     * @param string $issuerDer DER of the issuing certificate.
     * @param string $leafDer   DER of the certificate whose status is queried.
     *
     * @return Request The DER request and the CertID a response has to quote back.
     *
     * @throws Exception If either certificate cannot be parsed or encoded, or the
     *                   issuer did not issue the leaf.
     */
    public function build(string $issuerDer, string $leafDer): Request
    {
        $issuer = $this->certificate->fields($issuerDer);
        $leaf = $this->certificate->fields($leafDer);

        $this->assertIssued($issuerDer, $leafDer);

        $algId = $this->asn1->encodeSequence(
            $this->asn1->encodeObjectIdentifier(self::OID_SHA1) . $this->asn1->encodeNull(),
        );

        // RFC 6960 section 4.1.1: the name hash covers "the issuer's name field in
        // the certificate being checked", which is the leaf's own issuer field
        // rather than the issuer certificate's subject.
        //
        // The serial is spliced in as the certificate encodes it, so a negative one
        // is quoted unchanged (RFC 5280 section 4.1.2.2).
        $certId = $this->asn1->encodeSequence(
            $algId
            . $this->asn1->encodeOctetString(\hash('sha1', $leaf['issuer'], true))
            . $this->asn1->encodeOctetString(\hash('sha1', $issuer['public_key'], true))
            . $leaf['serial'],
        );

        $requestList = $this->asn1->encodeSequence($this->asn1->encodeSequence($certId));

        return new Request($this->asn1->encodeSequence($this->asn1->encodeSequence($requestList)), $certId, $issuerDer);
    }

    /**
     * Assert that the issuer's key signed the certificate being asked about.
     *
     * Established by signature rather than by Name, as Signer::assertOrderedChain()
     * establishes the same link.
     *
     * @throws Exception If the issuer's key did not sign the leaf.
     */
    private function assertIssued(string $issuerDer, string $leafDer): void
    {
        $issuerKey = \openssl_pkey_get_public(Certificate::derToPem($issuerDer));
        $issued = $issuerKey !== false && \openssl_x509_verify(Certificate::derToPem($leafDer), $issuerKey) === 1;

        // A refusal here is reported as an Exception, so the OpenSSL queue entries
        // are discarded rather than left for the host.
        Certificate::clearOpenSslErrors();

        if (!$issued) {
            throw new Exception('The OCSP request issuer did not issue the certificate');
        }
    }

    /**
     * Build the request, submit it through the transport, and validate the response.
     *
     * @param string   $url       OCSP responder URL.
     * @param string   $issuerDer DER of the issuing certificate.
     * @param string   $leafDer   DER of the target certificate.
     * @param callable $transport Receives (url, DER request) and must return the
     *                            DER response string.
     * @param int|null $now       Unix time the validity interval is checked against;
     *                            defaults to the current time.
     *
     * @return string The DER response bytes, once accepted.
     *
     * @throws Exception If building, transport, or validation fails.
     */
    public function fetch(
        string $url,
        string $issuerDer,
        string $leafDer,
        callable $transport,
        ?int $now = null,
    ): string {
        $request = $this->build($issuerDer, $leafDer);

        /** @var mixed $response */
        $response = $transport($url, $request->der);
        if (!\is_string($response)) {
            throw new Exception('Invalid OCSP transport response');
        }

        return $this->parseResponse($response, $request, $now);
    }

    /**
     * Validate a DER OCSPResponse against the request it answers.
     *
     * Applies the RFC 6960 section 3.2 acceptance rules. A response that fails any
     * of them is rejected rather than returned.
     *
     * @param string   $response DER-encoded OCSPResponse.
     * @param Request  $request  The request this response answers.
     * @param int|null $now      Unix time the validity interval is checked against;
     *                           defaults to the current time.
     *
     * @return string The response bytes unchanged, once accepted.
     *
     * @throws RevokedException If the responder states that the certificate is revoked.
     * @throws Exception If the response is malformed, unsuccessful, unmatched, not
     *                   good, or outside its validity interval.
     */
    public function parseResponse(string $response, Request $request, ?int $now = null): string
    {
        if ($response === '') {
            throw new Exception('Empty OCSP response');
        }

        $moment = $now ?? \time();

        $parts = $this->responseData($this->basicResponse($response));
        $this->verifySignature($parts, $request->issuerDer, $moment);
        $this->assertNoUnknownCritical($parts['extensions'], 'responseExtension');

        [$matches, $fault] = $this->matchingSingleResponses($parts['responses'], $request->certId);

        // Decoded once and read by each rule below. The passes stay separate, their
        // order being what gives revoked its precedence.
        $answers = [];
        foreach ($matches as $candidate) {
            [$status, $extensionsOffset] = $this->certStatus($candidate);
            $answers[] = ['single' => $candidate, 'status' => $status, 'extensions' => $extensionsOffset];
        }

        // Among several entries for one CertID, revoked wins wherever it sits: it is
        // the only verdict that says the certificate must not be used.
        foreach ($answers as $answer) {
            if ($answer['status'] === 1) {
                throw new RevokedException('The certificate is revoked');
            }
        }

        // A structural fault is reported after the revoked scan, so a revoked verdict
        // among the matches already found outranks it. A fault ahead of the match
        // leaves nothing found and is reported as malformed.
        if ($fault !== null) {
            throw $fault;
        }

        if ($matches === []) {
            throw new Exception('The OCSP response does not match the request');
        }

        // Every match is held to the validity interval and to the RFC 6960 section
        // 4.4 criticality rule, wherever it sits.
        foreach ($answers as $answer) {
            $this->assertNoUnknownCritical(
                $this->optionalExtensions(
                    $answer['single'],
                    $this->checkValidity($answer['single'], $answer['extensions'], $moment),
                    0xA1,
                    'singleExtension',
                ),
                'singleExtension',
            );
        }

        // Held to every match as well, so the order of two entries for one CertID
        // does not decide the outcome.
        foreach ($answers as $answer) {
            if ($answer['status'] !== 0) {
                throw new Exception('The certificate status is unknown');
            }
        }

        return $response;
    }

    /**
     * Read the certStatus of a SingleResponse.
     *
     * CertStatus ::= CHOICE { good [0] IMPLICIT NULL, revoked [1] IMPLICIT RevokedInfo,
     * unknown [2] IMPLICIT UnknownInfo } (RFC 6960 section 4.2.1), and UnknownInfo is
     * itself a NULL. A NULL has no content octets and is primitive (X.690 section
     * 8.8.2), so good and unknown are held to two primitive octets and the CHOICE has
     * no fourth alternative.
     *
     * revoked [1] is admitted whatever its encoding, a responder that emits it
     * primitively having still said revoked. An entry answering about another
     * certificate is held to the whole of the alternative by
     * assertForeignRevokedInfo().
     *
     * @param string $single SingleResponse content octets.
     *
     * @return array{int, int} [certStatus tag number, read cursor after certStatus]
     *
     * @throws Exception If the structure is malformed or names no alternative of the CHOICE.
     */
    private function certStatus(string $single): array
    {
        $offset = 0;
        $this->asn1->readTlv($single, $offset); // certID, read for its length alone

        $status = $this->asn1->readTlv($single, $offset);
        if (($status['tag'] & 0xC0) !== 0x80) {
            throw new Exception('Invalid OCSP certStatus');
        }

        // good and unknown are IMPLICIT NULL, so they are primitive as well as empty.
        $choice = $status['tag'] & 0x1F;
        if ($choice > 2 || $choice !== 1 && ($status['value'] !== '' || ($status['tag'] & 0x20) !== 0)) {
            throw new Exception('Invalid OCSP certStatus');
        }

        return [$choice, $offset];
    }

    /**
     * Read an OPTIONAL context-tagged EXPLICIT Extensions field at the cursor.
     *
     * Both fields read through here, responseExtensions [1] of a ResponseData and
     * singleExtensions [1] of a SingleResponse, are the tail field of their
     * structure (RFC 6960 section 4.2.1), so an element at the cursor carrying
     * another tag is refused rather than reported as an absent field.
     *
     * @param int $offset Read cursor.
     * @param int $tag    Identifier octet of the field.
     *
     * @return string Extensions content octets, or '' when the field is absent.
     *
     * @throws Exception If the structure is malformed or carries a field after the
     *                   extensions.
     */
    private function optionalExtensions(string $data, int $offset, int $tag, string $label): string
    {
        if ($offset >= \strlen($data)) {
            return '';
        }

        $field = $this->asn1->readTlv($data, $offset);
        if ($field['tag'] !== $tag) {
            throw new Exception('Invalid OCSP ' . $label . ' field after the extensions position');
        }

        // An EXPLICIT tag wraps exactly one element (X.690 section 8.14), so an empty
        // one is refused rather than read as an absent field.
        if ($field['value'] === '') {
            throw new Exception('Empty OCSP ' . $label . 's');
        }

        if ($offset !== \strlen($data)) {
            throw new Exception('Trailing bytes after the OCSP ' . $label . 's');
        }

        return $field['value'];
    }

    /**
     * Refuse an extension marked critical that this reader does not understand.
     *
     * RFC 6960 section 4.4: a client that cannot process a critical extension must
     * not rely on the response.
     *
     * @param string $extensions Extensions content octets, or '' when absent.
     * @param string $label      Field name, for the error message.
     *
     * @throws Exception If the structure is malformed or carries an unknown critical extension.
     */
    private function assertNoUnknownCritical(string $extensions, string $label): void
    {
        // Decoded by the shared codec the CRL and TSA readers use, an Extension being
        // the same structure whichever field carries it.
        foreach ($this->asn1->decodeExtensions($extensions, 'OCSP ' . $label) as $oid => $extension) {
            if ($extension['critical'] && !\in_array($oid, self::KNOWN_EXTENSIONS, true)) {
                throw new Exception('Unsupported critical OCSP ' . $label . ': ' . $oid);
            }
        }
    }

    /**
     * Unwrap an OCSPResponse to the content octets of its BasicOCSPResponse.
     *
     * Every layer is bounded as well as tag-checked. The responder's signature spans
     * tbsResponseData alone, and parseResponse() answers with the caller's own
     * string, so bytes elsewhere in the response are covered by nothing.
     *
     * @throws Exception If the status is not successful, the type is not basic, or
     *                   a layer carries anything besides the field read from it.
     */
    private function basicResponse(string $response): string
    {
        $offset = 0;
        $root = $this->asn1->readTlv($response, $offset);
        if ($root['tag'] !== 0x30 || $offset !== \strlen($response)) {
            throw new Exception('Invalid OCSP response');
        }

        $inner = 0;
        $status = $this->asn1->readTlv($root['value'], $inner);
        if ($status['tag'] !== 0x0A) {
            throw new Exception('Invalid OCSP response status');
        }

        $statusCode = $this->asn1->decodeInteger($status['value']);
        if ($statusCode !== 0) {
            throw new Exception(
                'OCSP responder returned ' . (self::RESPONSE_STATUS[$statusCode] ?? 'status ' . $statusCode),
            );
        }

        if ($inner >= \strlen($root['value'])) {
            throw new Exception('Missing OCSP response bytes');
        }

        $bytes = $this->asn1->readTlv($root['value'], $inner);
        if ($bytes['tag'] !== 0xA0 || $inner !== \strlen($root['value'])) {
            throw new Exception('Invalid OCSP response bytes');
        }

        $bytesOff = 0;
        $responseBytes = $this->asn1->readTlv($bytes['value'], $bytesOff);
        if ($responseBytes['tag'] !== 0x30 || $bytesOff !== \strlen($bytes['value'])) {
            throw new Exception('Invalid OCSP response bytes');
        }

        $typeOff = 0;
        $type = $this->asn1->readTlv($responseBytes['value'], $typeOff);
        if ($type['raw'] !== $this->asn1->encodeObjectIdentifier(self::OID_BASIC_RESPONSE)) {
            throw new Exception('Unsupported OCSP response type');
        }

        $payload = $this->asn1->readTlv($responseBytes['value'], $typeOff);
        if ($payload['tag'] !== 0x04 || $typeOff !== \strlen($responseBytes['value'])) {
            throw new Exception('Invalid OCSP response payload');
        }

        return $payload['value'];
    }

    /**
     * Return the content octets of every SingleResponse matching a CertID.
     *
     * A responder may answer a single-certificate request with several
     * SingleResponse entries, in any order, so the whole SEQUENCE is searched and
     * every match is returned; the caller decides which verdict wins.
     *
     * An entry whose CertID does not parse names no certificate and is skipped
     * rather than fatal.
     *
     * A structural fault in an entry ends the walk but is handed back rather than
     * thrown, so the caller reads the verdict of the matches already found first.
     *
     * @param string $responses SEQUENCE OF SingleResponse content octets.
     *
     * @return array{list<string>, Exception|null} [matching SingleResponse content
     *         octets, in the order given; the fault that ended the walk, if any]
     *
     * @throws Exception If the CertID asked about is malformed.
     */
    private function matchingSingleResponses(string $responses, string $certId): array
    {
        $wanted = $this->certIdParts($certId);

        $matches = [];
        $offset = 0;

        try {
            while ($offset < \strlen($responses)) {
                $single = $this->asn1->readTlv($responses, $offset);
                if ($single['tag'] !== 0x30) {
                    throw new Exception('Invalid OCSP SingleResponse');
                }

                $this->assertSingleResponseShape($single['value']);

                try {
                    $certIdOffset = 0;
                    $candidate = $this->asn1->readTlv($single['value'], $certIdOffset);
                    $parts = $this->certIdParts($candidate['raw']);
                } catch (Exception) {
                    continue;
                }

                if ($parts === $wanted) {
                    $matches[] = $single['value'];
                    continue;
                }

                $this->assertForeignRevokedInfo($single['value']);
            }
        } catch (Exception $e) {
            return [$matches, $e];
        }

        return [$matches, null];
    }

    /**
     * Hold the revoked [1] of an entry answering about another certificate to RevokedInfo.
     *
     * RevokedInfo ::= SEQUENCE { revocationTime GeneralizedTime, revocationReason [0]
     * EXPLICIT CRLReason OPTIONAL } (RFC 6960 section 4.2.1). certStatus() leaves the
     * alternative open for the entry answering the request, whose verdict is read; a
     * foreign entry's verdict is never read, so it is held to the whole structure.
     *
     * This completes assertSingleResponseShape(), certStatus being the one field that
     * walk reaches through without holding. good and unknown need nothing here,
     * certStatus() bounding both to two primitive octets.
     *
     * A throw here ends the walk and is handed back as the fault.
     *
     * @param string $single SingleResponse content octets.
     *
     * @throws Exception If the alternative is revoked and is not a RevokedInfo.
     */
    private function assertForeignRevokedInfo(string $single): void
    {
        $offset = 0;
        $this->asn1->readTlv($single, $offset); // certID, read for its length alone
        $status = $this->asn1->readTlv($single, $offset);

        if (($status['tag'] & 0x1F) !== 1) {
            return;
        }

        // RevokedInfo is a SEQUENCE, so the IMPLICIT tag carries the constructed bit.
        if (($status['tag'] & 0x20) === 0) {
            throw new Exception('Invalid OCSP revoked entry');
        }

        $inner = 0;
        $revocationTime = $this->asn1->readTlv($status['value'], $inner);
        if ($revocationTime['tag'] !== 0x18) {
            throw new Exception('Invalid OCSP revocationTime');
        }

        $this->asn1->decodeGeneralizedTime($revocationTime['value']);

        // revocationReason [0] EXPLICIT CRLReason, and CRLReason is an ENUMERATED, which
        // X.690 encodes as an INTEGER does. Nothing follows it.
        $reason = $this->asn1->readOptionalTlv($status['value'], $inner);
        if ($reason !== null) {
            if ($reason['tag'] !== 0xA0) {
                throw new Exception('Invalid OCSP revocationReason');
            }

            $this->asn1->decodeInteger(
                $this->asn1->readSingleElement($reason['value'], 0x0A, 'OCSP revocationReason')['value'],
            );
        }

        if ($inner !== \strlen($status['value'])) {
            throw new Exception('Trailing bytes in the OCSP revoked entry');
        }
    }

    /**
     * Hold a SingleResponse to its RFC 6960 section 4.2.1 shape, whoever it answers about.
     *
     * SingleResponse ::= SEQUENCE { certID CertID, certStatus CertStatus, thisUpdate
     * GeneralizedTime, nextUpdate [0] EXPLICIT GeneralizedTime OPTIONAL,
     * singleExtensions [1] EXPLICIT Extensions OPTIONAL }, and nothing follows the
     * last field.
     *
     * Every entry is held to the shape, whichever certificate it answers about, as
     * Ltv\Crl::checkNotRevoked() holds every revocation entry to its own.
     *
     * The certID is the one field left out: the caller parses it to compare it and
     * passes over an entry it cannot read. certStatus is held here only as far as
     * certStatus() holds it, a foreign entry's revoked alternative being completed by
     * assertForeignRevokedInfo().
     *
     * The acceptance rules stay with the entries that answer the request: the
     * validity interval and the criticality of a singleExtension.
     *
     * @param string $single SingleResponse content octets.
     *
     * @throws Exception If the structure is malformed or carries a field the shape
     *                   does not have.
     */
    private function assertSingleResponseShape(string $single): void
    {
        [, $offset] = $this->certStatus($single);

        $this->asn1->decodeExtensions(
            $this->optionalExtensions($single, $this->readInterval($single, $offset)[2], 0xA1, 'singleExtension'),
            'OCSP singleExtension',
        );
    }

    /**
     * Split a CertID into the four values that identify a certificate.
     *
     * The comparison is by value rather than by raw DER, so a responder that
     * rebuilds the CertID with the SHA-1 AlgorithmIdentifier parameters absent
     * rather than NULL still matches.
     *
     * @return array{string, string, string, string} [digest OID, nameHash, keyHash, serial]
     *
     * @throws Exception If the structure is not a CertID.
     */
    private function certIdParts(string $certIdDer): array
    {
        $offset = 0;
        $certId = $this->asn1->readTlv($certIdDer, $offset);
        if ($certId['tag'] !== 0x30) {
            throw new Exception('Invalid OCSP CertID');
        }

        // Read through the shared AlgorithmIdentifier reader, which bounds both
        // layers of the field and admits the parameters absent as well as NULL.
        $inner = 0;
        $algorithm = $this->asn1->readTlv($certId['value'], $inner);
        $digest = $this->asn1->decodeAlgorithmIdentifier($algorithm['raw'], 'OCSP CertID');

        $nameHash = $this->asn1->readTlv($certId['value'], $inner);
        $keyHash = $this->asn1->readTlv($certId['value'], $inner);
        $serial = $this->asn1->readTlv($certId['value'], $inner);
        if ($nameHash['tag'] !== 0x04 || $keyHash['tag'] !== 0x04 || $serial['tag'] !== 0x02) {
            throw new Exception('Invalid OCSP CertID');
        }

        // A serial runs to 20 octets (RFC 5280 section 4.1.2.2), too wide to decode,
        // so only its minimality is checked.
        $this->asn1->assertMinimalInteger($serial['value']);

        // serialNumber is the last field of a CertID (RFC 6960 section 4.1.1), so
        // nothing may follow it.
        if ($inner !== \strlen($certId['value'])) {
            throw new Exception('Invalid OCSP CertID');
        }

        return [$digest, $nameHash['value'], $keyHash['value'], $serial['value']];
    }

    /**
     * Decode a ResponseData version [0] EXPLICIT field.
     *
     * RFC 6960 section 4.2.1 shapes it as an INTEGER, and the EXPLICIT wrapper holds
     * exactly one element and nothing else.
     *
     * @param array{tag: int, value: string, raw: string} $element Parsed version TLV.
     *
     * @throws Exception If the field does not hold exactly one INTEGER.
     */
    private function explicitVersion(array $element): void
    {
        $offset = 0;
        $number = $this->asn1->readTlv($element['value'], $offset);
        if ($number['tag'] !== 0x02 || $offset !== \strlen($element['value'])) {
            throw new Exception('Invalid OCSP ResponseData version');
        }

        $this->asn1->decodeInteger($number['value']);
    }

    /**
     * Split a BasicOCSPResponse into the parts its acceptance rules need.
     *
     * @return array{tbs: string, responderId: array{tag: int, value: string, raw: string},
     *           responses: string, algorithmId: string, signature: string, certs: list<string>,
     *           extensions: string}
     *
     * @throws Exception If the structure is malformed or carries no certificate status.
     */
    private function responseData(string $basic): array
    {
        $offset = 0;
        $basicSeq = $this->asn1->readTlv($basic, $offset);
        if ($basicSeq['tag'] !== 0x30) {
            throw new Exception('Invalid BasicOCSPResponse');
        }

        if ($offset !== \strlen($basic)) {
            throw new Exception('Trailing bytes after the BasicOCSPResponse');
        }

        $dataOff = 0;
        $responseData = $this->asn1->readTlv($basicSeq['value'], $dataOff);
        if ($responseData['tag'] !== 0x30) {
            throw new Exception('Invalid OCSP ResponseData');
        }

        // signatureAlgorithm sits outside tbsResponseData and a BasicOCSPResponse
        // carries no inner copy to compare it with, so the field is held to its shape
        // here. Asn1::decodeAlgorithmIdentifier() bounds its contents when the
        // signature is checked.
        $algorithmId = $this->asn1->readTlv($basicSeq['value'], $dataOff);
        if ($algorithmId['tag'] !== 0x30) {
            throw new Exception('Invalid OCSP signatureAlgorithm');
        }

        $signature = $this->asn1->decodeBitString($this->asn1->readTlv($basicSeq['value'], $dataOff));

        $tbs = $responseData['value'];
        $tbsOff = 0;

        // version [0] EXPLICIT DEFAULT v1 is omitted by a conforming responder. RFC
        // 6960 section 4.2.1 types version an INTEGER and producedAt a
        // GeneralizedTime, and both are read rather than stepped over. The version is
        // decoded but not range-checked, as Cms\Certificate does with a SignedData
        // version.
        if ($tbsOff < \strlen($tbs) && \ord($tbs[$tbsOff]) === 0xA0) {
            $this->explicitVersion($this->asn1->readTlv($tbs, $tbsOff));
        }

        $responderId = $this->asn1->readTlv($tbs, $tbsOff);

        $producedAt = $this->asn1->readTlv($tbs, $tbsOff);
        if ($producedAt['tag'] !== 0x18) {
            throw new Exception('Invalid OCSP producedAt');
        }

        $this->asn1->decodeGeneralizedTime($producedAt['value']);

        $responses = $this->asn1->readTlv($tbs, $tbsOff);
        if ($responses['tag'] !== 0x30 || $responses['value'] === '') {
            throw new Exception('The OCSP response carries no certificate status');
        }

        $certs = $this->embeddedCertificates($basicSeq['value'], $dataOff);

        // certs [0] is the tail field of a BasicOCSPResponse (RFC 6960 section 4.2.1),
        // so nothing may follow it.
        if ($dataOff !== \strlen($basicSeq['value'])) {
            throw new Exception('Trailing bytes in the BasicOCSPResponse');
        }

        return [
            'tbs' => $responseData['raw'],
            'responderId' => $responderId,
            'responses' => $responses['value'],
            'algorithmId' => $algorithmId['raw'],
            'signature' => $signature,
            'certs' => $certs,
            // RFC 6960 section 4.2.1: responseExtensions is [1], the tail field of
            // ResponseData. The [1] and [2] that also appear in this structure are the
            // ResponderID CHOICE alternatives, which are mandatory and read above.
            'extensions' => $this->optionalExtensions($tbs, $tbsOff, 0xA1, 'responseExtension'),
        ];
    }

    /**
     * Read the optional certs [0] EXPLICIT SEQUENCE OF Certificate of a
     * BasicOCSPResponse.
     *
     * A bag larger than MAX_RESPONDER_CERTIFICATES is refused rather than searched,
     * every member naming the ResponderID costing a key load, two DER parses, and a
     * signature check. Every layer is bounded, the signature spanning
     * tbsResponseData alone.
     *
     * @param int &$offset Read cursor positioned just after the signature; advanced
     *                     past the field so the caller can bound what follows it.
     *
     * @return list<string> DER certificates, empty when the response embeds none.
     *
     * @throws Exception If the field is malformed or embeds more than
     *                   MAX_RESPONDER_CERTIFICATES certificates.
     */
    private function embeddedCertificates(string $basic, int &$offset): array
    {
        if ($offset >= \strlen($basic)) {
            return [];
        }

        // certs [0] is the tail field of a BasicOCSPResponse, so an element carrying
        // another tag is refused rather than reported as an absent field.
        $field = $this->asn1->readTlv($basic, $offset);
        if ($field['tag'] !== 0xA0) {
            throw new Exception('Invalid OCSP field after the responder certificates position');
        }

        $listOffset = 0;
        $list = $this->asn1->readTlv($field['value'], $listOffset);
        if ($list['tag'] !== 0x30 || $listOffset !== \strlen($field['value'])) {
            throw new Exception('Invalid OCSP responder certificates');
        }

        $certs = [];
        $certOffset = 0;
        while ($certOffset < \strlen($list['value'])) {
            $cert = $this->asn1->readTlv($list['value'], $certOffset);

            // RFC 6960 section 4.2.1 types the field SEQUENCE OF Certificate, so a
            // member has to parse as one rather than merely carry the SEQUENCE tag.
            if ($cert['tag'] !== 0x30) {
                throw new Exception('Invalid OCSP responder certificates');
            }

            // Tested before the parse, which is the work the bound is bounding, as
            // Signer::boundedTokenCertificates() is tested before verification.
            if (\count($certs) >= self::MAX_RESPONDER_CERTIFICATES) {
                throw new Exception(
                    'The OCSP response embeds more than ' . self::MAX_RESPONDER_CERTIFICATES . ' certificates',
                );
            }

            try {
                $this->certificate->fields($cert['raw']);
            } catch (Exception $e) {
                throw new Exception('Invalid OCSP responder certificates', 0, $e);
            }

            $certs[] = $cert['raw'];
        }

        return $certs;
    }

    /**
     * Verify the responder's signature over the ResponseData.
     *
     * RFC 6960 section 3.2 requires the signature to be valid and the signer to be
     * authorised for the certificate in question. The signer is either the issuer
     * itself, or a responder the issuer delegated to, which RFC 6960 section
     * 4.2.2.2 defines as a certificate issued by that same issuer and carrying the
     * id-kp-OCSPSigning extended key usage.
     *
     * @param array{tbs: string, responderId: array{tag: int, value: string, raw: string},
     *           responses: string, algorithmId: string, signature: string, certs: list<string>,
     *           extensions: string} $parts
     *
     * @throws Exception If the responder cannot be identified, is not authorised, or the
     *                   signature does not verify.
     */
    private function verifySignature(array $parts, string $issuerDer, int $now): void
    {
        $this->responderCertificate(
            $parts['responderId'],
            $parts['certs'],
            $issuerDer,
            $now,
            /** @throws Exception */
            function (string $certDer) use ($parts): void {
                $this->verifier->verify($parts['tbs'], $parts['algorithmId'], $parts['signature'], $certDer);
            },
        );
    }

    /**
     * Resolve the certificate a ResponderID names to the DER that verifies the response.
     *
     * certs [0] is outside tbsResponseData and so covered by no signature, so every
     * candidate is tried and the rejection is reported only once none is left.
     *
     * The signature is part of the search rather than a step after it: a candidate
     * can be authorised and still not be the key that signed, which is the case for
     * an authority that has rolled its responder key.
     *
     * @param array{tag: int, value: string, raw: string} $responderId
     * @param list<string>                                $certs
     * @param callable(string): void                      $verify Checks the signature
     *                             against a candidate, throwing when it does not hold.
     *
     * @throws Exception If the ResponderID is malformed, or names no certificate that
     *                   is authorised and verifies the signature.
     */
    private function responderCertificate(
        array $responderId,
        array $certs,
        string $issuerDer,
        int $now,
        callable $verify,
    ): string {
        // ResponderID ::= CHOICE { byName [1] Name, byKey [2] KeyHash }, both EXPLICIT.
        $choice = $responderId['tag'];
        if ($choice !== 0xA1 && $choice !== 0xA2) {
            throw new Exception('Invalid OCSP responderID');
        }

        $innerOffset = 0;
        $identifier = $this->asn1->readTlv($responderId['value'], $innerOffset);
        if ($innerOffset !== \strlen($responderId['value'])) {
            throw new Exception('Invalid OCSP responderID');
        }

        if ($choice === 0xA2 && $identifier['tag'] !== 0x04) {
            throw new Exception('Invalid OCSP responderID');
        }

        $rejection = null;

        // The issuer is an authorised responder in its own right (RFC 6960 section
        // 4.2.2.2), so it needs no delegation, only the signature.
        if ($this->isResponder($issuerDer, $choice, $identifier)) {
            try {
                $verify($issuerDer);

                return $issuerDer;
            } catch (Exception $e) {
                $rejection = $e;
            }
        }

        foreach ($certs as $cert) {
            try {
                if (!$this->isResponder($cert, $choice, $identifier)) {
                    continue;
                }

                $this->authorisedResponder($cert, $issuerDer, $now);
                $verify($cert);

                return $cert;
            } catch (Exception $e) {
                $rejection ??= $e;
            }
        }

        if ($rejection !== null) {
            throw $rejection;
        }

        throw new Exception('The OCSP response does not carry the responder certificate');
    }

    /**
     * True when a certificate is the one a ResponderID names.
     *
     * @param int                                         $choice     0xA1 for byName, 0xA2 for byKey.
     * @param array{tag: int, value: string, raw: string} $identifier Parsed ResponderID content.
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    private function isResponder(string $certDer, int $choice, array $identifier): bool
    {
        $fields = $this->certificate->fields($certDer);

        if ($choice === 0xA1) {
            return $fields['subject'] === $identifier['raw'];
        }

        return \hash_equals($identifier['value'], \hash('sha1', $fields['public_key'], true));
    }

    /**
     * Check that a delegated responder certificate was authorised by the issuer.
     *
     * RFC 6960 section 3.2 rule 4 asks for a responder that is currently authorised,
     * so the certificate's own validity period is part of the check;
     * openssl_x509_verify() reads the signature and nothing else.
     *
     * @throws Exception If the delegation is missing, unsigned, expired, or lacks the
     *                   OCSPSigning purpose.
     */
    private function authorisedResponder(string $certDer, string $issuerDer, int $now): void
    {
        $issuerKey = \openssl_pkey_get_public(Certificate::derToPem($issuerDer));
        $authorised =
            $issuerKey !== false
            && $this->certificate->isIssuerOf($issuerDer, $certDer)
            && \openssl_x509_verify(Certificate::derToPem($certDer), $issuerKey) === 1;

        // A candidate out of the unauthenticated certs [0] bag is expected to fail
        // this, so the queue is drained either way.
        Certificate::clearOpenSslErrors();

        if (!$authorised) {
            throw new Exception('The OCSP responder was not authorised by the certificate issuer');
        }

        if (!$this->hasOcspSigning($certDer)) {
            throw new Exception('The OCSP responder certificate lacks the OCSPSigning purpose');
        }

        // RFC 5280 section 4.2.1.3 reserves digitalSignature for a signature that is
        // not over a certificate or a CRL, which is what a response carries. Not
        // asked of the issuer signing in its own right, one frame up: RFC 6960
        // section 4.2.2.2 makes it a responder without delegation, and a CA
        // certificate commonly carries keyCertSign and cRLSign alone.
        $this->certificate->assertUsableForSigning($certDer);

        try {
            $this->certificate->assertValidAt($certDer, $now, $this->clockSkew);
        } catch (Exception $e) {
            throw new Exception('The OCSP responder certificate is outside its validity period', 0, $e);
        }
    }

    /**
     * True when a certificate carries the id-kp-OCSPSigning extended key usage.
     *
     * @throws Exception If the certificate cannot be read.
     */
    private function hasOcspSigning(string $certDer): bool
    {
        $purposes = $this->certificate->extendedKeyUsage($certDer);

        return $purposes !== null && \in_array(self::OID_OCSP_SIGNING, $purposes, true);
    }

    /**
     * Check that thisUpdate/nextUpdate of a SingleResponse cover the given time.
     *
     * @param int $offset Read cursor positioned just after certStatus.
     *
     * @return int Read cursor after nextUpdate, where singleExtensions [1] may follow.
     *
     * @throws Exception If the interval is malformed or does not cover $now.
     */
    private function checkValidity(string $single, int $offset, int $now): int
    {
        [$produced, $next, $afterNext] = $this->readInterval($single, $offset);

        if ($produced > ($now + $this->clockSkew)) {
            throw new Exception('The OCSP response is not yet valid');
        }

        // RFC 6960 section 3.2 rule 5, that thisUpdate is sufficiently recent, is an
        // acceptance rule of its own, so the age bound applies whether or not a
        // nextUpdate is present.
        if ($this->maxAge > 0 && $produced < ($now - $this->maxAge - $this->clockSkew)) {
            throw new Exception('The OCSP response is too old');
        }

        if ($next !== null && $next < ($now - $this->clockSkew)) {
            throw new Exception('The OCSP response has expired');
        }

        return $afterNext;
    }

    /**
     * Read the validity interval of a SingleResponse.
     *
     * Separate from the rules over it: the shape belongs to every entry of the
     * response, the rules to the entry that answers the request. See
     * assertSingleResponseShape().
     *
     * @param string $single SingleResponse content octets.
     * @param int    $offset Read cursor positioned just after certStatus.
     *
     * @return array{int, int|null, int} [thisUpdate, nextUpdate or null when absent,
     *         read cursor after nextUpdate, where singleExtensions [1] may follow]
     *
     * @throws Exception If the interval is malformed.
     */
    private function readInterval(string $single, int $offset): array
    {
        $thisUpdate = $this->asn1->readTlv($single, $offset);
        if ($thisUpdate['tag'] !== 0x18) {
            throw new Exception('Invalid OCSP thisUpdate');
        }

        $produced = $this->asn1->decodeGeneralizedTime($thisUpdate['value']);

        $next = null;
        $afterNext = $offset;
        if ($offset < \strlen($single)) {
            $field = $this->asn1->readTlv($single, $offset);
            if ($field['tag'] === 0xA0) {
                $next = $field;
                $afterNext = $offset;
            }
        }

        if ($next === null) {
            return [$produced, null, $afterNext];
        }

        // The [0] is EXPLICIT, so it wraps exactly one GeneralizedTime.
        $nextOff = 0;
        $nextUpdate = $this->asn1->readTlv($next['value'], $nextOff);
        if ($nextUpdate['tag'] !== 0x18 || $nextOff !== \strlen($next['value'])) {
            throw new Exception('Invalid OCSP nextUpdate');
        }

        return [$produced, $this->asn1->decodeGeneralizedTime($nextUpdate['value']), $afterNext];
    }
}
