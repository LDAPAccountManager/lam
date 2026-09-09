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

namespace Com\Tecnick\Pdf\Sign\Timestamp;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Cms\Certificate;
use Com\Tecnick\Pdf\Sign\Cms\Oid;
use Com\Tecnick\Pdf\Sign\Cms\SignedDataVerifier;
use Com\Tecnick\Pdf\Sign\DigestAlgorithm;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Ocsp\Client as OcspClient;

/**
 * Com\Tecnick\Pdf\Sign\Timestamp\Client
 *
 * RFC 3161 timestamp codec. Builds a TimeStampReq for a signature, parses a
 * TimeStampResp to extract the timestamp token, and maps digest algorithms to
 * their OIDs. HTTP transport is injected into requestToken(), so the codec
 * performs no network access and the host controls networking and SSRF
 * protection.
 *
 * A returned token is verified before it is embedded: its SignerInfo signature
 * must check out against the TSA certificate the token carries, and its TSTInfo
 * must answer the request that was sent, per RFC 3161 section 2.4.2. That means
 * the same message imprint, the same policy when one was requested, the nonce
 * echoed unchanged, and a genTime near the moment of the request.
 *
 * The certificate that signed it must be a TSA certificate reserved for
 * timestamping (RFC 3161 section 2.3), and must have been inside its validity
 * period at the instant the token attests.
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
     * id-kp-timeStamping, the purpose a TSA certificate must carry.
     */
    private const OID_TIME_STAMPING = '1.3.6.1.5.5.7.3.8';

    /**
     * Default clock skew tolerated between the token's genTime and the moment of
     * use, in seconds. Ltv\Crl reads the same value.
     */
    public const CLOCK_SKEW = OcspClient::CLOCK_SKEW;

    private Asn1 $asn1;

    private Certificate $certificate;

    private SignedDataVerifier $verifier;

    /**
     * @param int $clockSkew Skew tolerated between the token's genTime and the moment
     *                       of the request, in seconds.
     *
     * @throws Exception If the skew is negative.
     */
    public function __construct(
        private readonly Config $config,
        ?Asn1 $asn1 = null,
        ?Certificate $certificate = null,
        ?SignedDataVerifier $verifier = null,
        private readonly int $clockSkew = self::CLOCK_SKEW,
    ) {
        if ($clockSkew < 0) {
            throw new Exception('Invalid TSA clock skew: ' . $clockSkew);
        }

        $this->asn1 = $asn1 ?? new Asn1();
        $this->certificate = $certificate ?? new Certificate($this->asn1);
        // RFC 3161 section 2.4.2 requires a token to carry the ESS
        // signing-certificate attribute, the only signed field naming the
        // certificate the checks below run against.
        $this->verifier = $verifier ?? new SignedDataVerifier(
            $this->asn1,
            $this->certificate,
            requireSigningCertificate: true,
        );
    }

    /**
     * Build an RFC 3161 TimeStampReq for the given signature bytes.
     *
     * @param string $signature Signature (or any bytes) to be timestamped.
     *
     * @return Request The DER request and the imprint and nonce a token must match.
     *
     * @throws Exception If encoding fails or a nonce cannot be generated.
     */
    public function buildRequest(string $signature): Request
    {
        $hashAlgo = $this->config->hashAlgorithm;
        $hash = \hash($hashAlgo, $signature, true);

        $oid = $this->hashAlgorithmOid($hashAlgo);
        $messageImprint = $this->asn1->encodeSequence(
            $this->algorithmIdentifier($oid) . $this->asn1->encodeOctetString($hash),
        );

        $body = $this->asn1->encodeInteger(1) . $messageImprint;
        if ($this->config->policyOid !== '') {
            $body .= $this->asn1->encodeObjectIdentifier($this->config->policyOid);
        }

        $nonce = '';
        if ($this->config->nonceEnabled) {
            try {
                // Eight random octets rather than random_int(1, PHP_INT_MAX): RFC 3161
                // section 2.4.1 asks for at least 64 bits, and PHP_INT_MAX yields 63 on
                // a 64-bit build and 31 on a 32-bit one.
                $nonce = $this->asn1->encodeIntegerBytes(\random_bytes(8));
            } catch (\Random\RandomException $e) {
                throw new Exception('Unable to generate random nonce: ' . $e->getMessage(), 0, $e);
            }

            $body .= $nonce;
        }

        $body .= $this->asn1->encodeBoolean(true);

        return new Request($this->asn1->encodeSequence($body), $hash, $oid, $nonce, $this->config->policyOid);
    }

    /**
     * Extract and validate the timestamp token of a DER-encoded TimeStampResp.
     *
     * The status is checked first, then the token's TSTInfo is matched against the
     * request: the messageImprint must be the digest that was sent under the same
     * algorithm, and the nonce must come back unchanged.
     *
     * @param string   $response DER-encoded timestamp response.
     * @param Request  $request  The request this response answers.
     * @param int|null $now      Unix time the token's genTime is checked against;
     *                           defaults to the current time.
     *
     * @return string DER-encoded timestamp token (ContentInfo).
     *
     * @throws Exception If the response is empty, malformed, rejected, or does not
     *                   match the request.
     */
    public function parseResponse(string $response, Request $request, ?int $now = null): string
    {
        if ($response === '') {
            throw new Exception('Empty TSA response');
        }

        $offset = 0;
        $root = $this->asn1->readTlv($response, $offset);
        if ($root['tag'] !== 0x30 || $offset !== \strlen($response)) {
            throw new Exception('Invalid TSA response');
        }

        $inner = 0;
        $statusSeq = $this->asn1->readTlv($root['value'], $inner);
        if ($statusSeq['tag'] !== 0x30) {
            throw new Exception('Invalid TSA status response');
        }

        $statusOffset = 0;
        $status = $this->asn1->readTlv($statusSeq['value'], $statusOffset);
        if ($status['tag'] !== 0x02) {
            throw new Exception('Invalid TSA status code');
        }

        $statusCode = $this->asn1->decodeInteger($status['value']);
        if ($statusCode !== 0 && $statusCode !== 1) {
            throw new Exception('TSA request rejected');
        }

        if ($inner >= \strlen($root['value'])) {
            throw new Exception('Missing TSA token');
        }

        $token = $this->asn1->readTlv($root['value'], $inner);
        if ($token['tag'] !== 0x30) {
            throw new Exception('Invalid TSA token structure');
        }

        if ($inner !== \strlen($root['value'])) {
            throw new Exception('Trailing bytes after the TSA token');
        }

        // Matched before verified, so a token answering a different request is
        // reported as that rather than as a signature mismatch.
        $stamped = $this->checkTokenMatchesRequest($token['raw'], $request, $now ?? \time());
        $this->assertTsaCertificate($this->verifier->verify($token['raw']), $stamped);

        // The token is returned unchanged and Builder embeds it verbatim, so its own
        // CertificateSet is held to the strict reading: every member has to parse as
        // a certificate, and the rest of the SignedData is bounded.
        $this->certificate->fromSignedData($token['raw'], true);

        return $token['raw'];
    }

    /**
     * Build the request, submit it through the given transport, and validate the
     * returned token.
     *
     * @param string   $signature Signature bytes to timestamp.
     * @param callable $transport Receives the DER request string and must
     *                            return the DER response string.
     * @param int|null $now       Unix time the token's genTime is checked against;
     *                            defaults to the current time.
     *
     * @throws Exception If encoding, transport, or validation fails.
     */
    public function requestToken(string $signature, callable $transport, ?int $now = null): string
    {
        $request = $this->buildRequest($signature);

        /** @var mixed $response */
        $response = $transport($request->der);
        if (!\is_string($response)) {
            throw new Exception('Invalid TSA transport response');
        }

        return $this->parseResponse($response, $request, $now);
    }

    /**
     * Extract the certificates a timestamp token embeds.
     *
     * The TSA certificate chain is validation material a PAdES B-LT document needs
     * in its Document Security Store, alongside the signer's own chain. An entry
     * that is not a certificate is dropped by Cms\Certificate::fromSignedData().
     *
     * @return list<string> DER certificates, empty when the token embeds none.
     *
     * @throws Exception If the token cannot be parsed.
     */
    public function tokenCertificates(string $tokenDer): array
    {
        return $this->certificate->fromSignedData($tokenDer);
    }

    /**
     * Verify that a token answers the request that was sent.
     *
     * @return int The instant the token attests, as a Unix time.
     *
     * @throws Exception If the imprint, the genTime, or the nonce does not match.
     */
    private function checkTokenMatchesRequest(string $tokenDer, Request $request, int $now): int
    {
        $tstInfo = $this->tstInfo($tokenDer);

        // RFC 3161 section 2.4.2 types version an INTEGER. Decoded but not
        // range-checked, as Cms\Certificate does with a SignedData version.
        $offset = 0;
        $version = $this->asn1->readTlv($tstInfo, $offset);
        if ($version['tag'] !== 0x02) {
            throw new Exception('Invalid TSA token version');
        }

        $this->asn1->decodeInteger($version['value']);

        $policy = $this->asn1->readTlv($tstInfo, $offset);
        if ($policy['tag'] !== 0x06) {
            throw new Exception('Invalid TSA token policy');
        }

        // Decoded whether or not a policy was requested, which bounds the content
        // octets: the comparison below runs only when the request named one. Which
        // policy a TSA issues under is the host's question.
        $this->asn1->decodeObjectIdentifier($policy['value']);

        // RFC 3161 section 2.4.2: a token answering a request that named a policy has
        // to be issued under it.
        if ($request->policyOid !== '' && $policy['raw'] !== $this->asn1->encodeObjectIdentifier($request->policyOid)) {
            throw new Exception('The TSA token policy does not match the request');
        }

        $imprint = $this->asn1->readTlv($tstInfo, $offset);
        if ($imprint['tag'] !== 0x30) {
            throw new Exception('Invalid TSA messageImprint');
        }

        // MessageImprint ::= SEQUENCE { hashAlgorithm AlgorithmIdentifier,
        // hashedMessage OCTET STRING }, with nothing after the two.
        $imprintOff = 0;
        $algorithm = $this->asn1->readTlv($imprint['value'], $imprintOff);
        $hashed = $this->asn1->readTlv($imprint['value'], $imprintOff);
        if ($hashed['tag'] !== 0x04 || $imprintOff !== \strlen($imprint['value'])) {
            throw new Exception('Invalid TSA messageImprint');
        }

        $algorithmOid = $this->asn1->decodeAlgorithmIdentifier($algorithm['raw'], 'TSA messageImprint');
        if ($algorithmOid !== $request->hashOid) {
            throw new Exception('The TSA token digest algorithm does not match the request');
        }

        if (!\hash_equals($request->imprint, $hashed['value'])) {
            throw new Exception('The TSA token does not cover the requested bytes');
        }

        $stamped = $this->checkGenTime($tstInfo, $offset, $now);

        if ($request->nonce !== '' && !\hash_equals($request->nonce, $this->tstInfoNonce($tstInfo, $offset))) {
            throw new Exception('The TSA token nonce does not match the request');
        }

        $this->assertTstInfoTail($tstInfo, $offset);

        return $stamped;
    }

    /**
     * Walk the TSTInfo fields after genTime, and refuse an extension marked critical.
     *
     * RFC 3161 section 2.4.2 puts accuracy, ordering, nonce and tsa [0] there, each
     * OPTIONAL, each admissible once, in that order, and extensions [1] last. The
     * walk is held to that ordering, and each field is read as well as ranked.
     *
     * This codec reads no extension, so a critical one is refused, as the OCSP and
     * CRL readers refuse theirs (RFC 6960 section 4.4, RFC 5280 section 6.3.3
     * (a)(2)).
     *
     * @param int $offset Read cursor positioned just after genTime.
     *
     * @throws Exception If the structure is malformed, carries a field out of order,
     *                   or carries a critical extension.
     */
    private function assertTstInfoTail(string $tstInfo, int $offset): void
    {
        // accuracy SEQUENCE, ordering BOOLEAN, nonce INTEGER, tsa [0], in that order.
        $order = [0x30 => 0, 0x01 => 1, 0x02 => 2, 0xA0 => 3];

        $seen = -1;
        $extensions = null;
        while ($offset < \strlen($tstInfo)) {
            $field = $this->asn1->readTlv($tstInfo, $offset);
            if ($field['tag'] === 0xA1) {
                $extensions = $field['value'];
                break;
            }

            // A tag the grammar does not have ranks below every field, so it is refused
            // here along with a repeat and with a field out of order.
            $rank = $order[$field['tag']] ?? -1;
            if ($rank <= $seen) {
                throw new Exception('Invalid TSA token field after the genTime: ' . $field['tag']);
            }

            $this->assertTstInfoField($field);

            $seen = $rank;
        }

        // extensions [1] is the tail field of a TSTInfo (RFC 3161 section 2.4.2), so
        // nothing may follow it.
        if ($offset !== \strlen($tstInfo)) {
            throw new Exception('Trailing bytes after the TSA token extensions');
        }

        if ($extensions === null) {
            return;
        }

        // extensions [1] is IMPLICIT, so the field carries the members of the
        // SEQUENCE OF directly and the tag has to be restored to read them.
        foreach ($this->asn1->decodeExtensions(
            $this->asn1->encodeSequence($extensions),
            'TSA token extension',
        ) as $oid => $extension) {
            if ($extension['critical']) {
                throw new Exception('Unsupported critical TSA token extension: ' . $oid);
            }
        }
    }

    /**
     * Hold one TSTInfo tail field to the shape RFC 3161 section 2.4.2 gives it.
     *
     * Covers the four fields the walk ranks. nonce is held here rather than where it
     * is compared, tstInfoNonce() being reached only when the request carried one.
     *
     * @param array{tag: int, value: string, raw: string} $field Parsed tail field.
     *
     * @throws Exception If the field is not the one its tag claims.
     */
    private function assertTstInfoField(array $field): void
    {
        // nonce INTEGER. A nonce runs wider than a PHP integer, so only its
        // minimality is checked (X.690 section 8.3.2), as for serialNumber.
        if ($field['tag'] === 0x02) {
            $this->asn1->assertMinimalInteger($field['value']);
        }

        // ordering BOOLEAN DEFAULT FALSE. A BOOLEAN is one content octet (X.690
        // section 8.2.1), as Asn1::decodeExtensions() holds a criticality flag.
        if ($field['tag'] === 0x01 && \strlen($field['value']) !== 1) {
            throw new Exception('Invalid TSA token ordering');
        }

        if ($field['tag'] === 0x30) {
            $this->assertAccuracy($field['value']);
        }

        // tsa [0] EXPLICIT GeneralName, and an EXPLICIT tag holds exactly one element
        // (X.690 section 8.14). GeneralName is a CHOICE, so the inner tag is not
        // constrained.
        if ($field['tag'] === 0xA0) {
            if ($field['value'] === '') {
                throw new Exception('Empty TSA token tsa field');
            }

            $inner = 0;
            $this->asn1->readTlv($field['value'], $inner);
            if ($inner !== \strlen($field['value'])) {
                throw new Exception('Invalid TSA token tsa field');
            }
        }
    }

    /**
     * Hold an Accuracy to its RFC 3161 section 2.4.2 grammar.
     *
     * Accuracy ::= SEQUENCE { seconds INTEGER OPTIONAL, millis [0] INTEGER (1..999)
     * OPTIONAL, micros [1] INTEGER (1..999) OPTIONAL }, each admissible once and in
     * that order. The ranges are not enforced, as the TSTInfo version's is not.
     *
     * @param string $accuracy Content octets of the accuracy SEQUENCE.
     *
     * @throws Exception If the structure carries a field the grammar does not have.
     */
    private function assertAccuracy(string $accuracy): void
    {
        $order = [0x02 => 0, 0x80 => 1, 0x81 => 2];

        $seen = -1;
        $offset = 0;
        while ($offset < \strlen($accuracy)) {
            $field = $this->asn1->readTlv($accuracy, $offset);

            $rank = $order[$field['tag']] ?? -1;
            if ($rank <= $seen) {
                throw new Exception('Invalid TSA token accuracy field: ' . $field['tag']);
            }

            $this->asn1->decodeInteger($field['value']);

            $seen = $rank;
        }
    }

    /**
     * Check that the certificate a token was verified against is a TSA certificate.
     *
     * RFC 3161 section 2.3 requires the TSA to sign with a key reserved for
     * timestamping, marked by the id-kp-timeStamping extended key usage as the sole
     * purpose, and requires that extension to be critical. Both are checked here.
     *
     * The validity period is checked against genTime rather than the moment of use,
     * the question being whether the TSA could have signed at the instant it claims.
     *
     * @param int $stamped The instant the token attests.
     *
     * @throws Exception If the certificate is not reserved for timestamping, or did
     *                   not cover $stamped.
     */
    private function assertTsaCertificate(string $certDer, int $stamped): void
    {
        // Both rules are read off one decode of the extension.
        [$purposes, $critical] = $this->certificate->extendedKeyUsageWithCriticality($certDer);
        if ($purposes !== [self::OID_TIME_STAMPING]) {
            throw new Exception(
                'The TSA certificate is not reserved for timestamping: '
                . ($purposes === null ? 'no extended key usage' : \implode(', ', $purposes)),
            );
        }

        if (!$critical) {
            throw new Exception('The TSA certificate extended key usage is not critical');
        }

        // RFC 5280 section 4.2.1.3: the purpose above says what the key is for, the
        // key usage says whether it may sign at all, and a token is a signature.
        $this->certificate->assertUsableForSigning($certDer);

        try {
            $this->certificate->assertValidAt($certDer, $stamped, $this->clockSkew);
        } catch (Exception $e) {
            throw new Exception('The TSA certificate did not cover the token genTime', 0, $e);
        }
    }

    /**
     * Check the TSTInfo genTime against the moment the token was requested.
     *
     * The field is asserted to be a GeneralizedTime and decoded with the
     * fraction-of-second part RFC 3161 section 2.4.2 admits, which an
     * OpenSSL-backed TSA emits whenever its clock_precision_digits is set.
     *
     * @param int &$offset Read cursor positioned just after messageImprint; advanced
     *                     past serialNumber and genTime.
     *
     * @return int The instant the token attests, as a Unix time.
     *
     * @throws Exception If genTime is not a DER GeneralizedTime, or is outside the
     *                   tolerated window around $now.
     */
    private function checkGenTime(string $tstInfo, int &$offset, int $now): int
    {
        // RFC 3161 section 2.4.2 types serialNumber an INTEGER. Its value is not
        // weighed, and it runs wider than a PHP integer, so only its minimality is
        // checked (X.690 section 8.3.2).
        $serialNumber = $this->asn1->readTlv($tstInfo, $offset);
        if ($serialNumber['tag'] !== 0x02) {
            throw new Exception('Invalid TSA token serialNumber');
        }

        $this->asn1->assertMinimalInteger($serialNumber['value']);

        $genTime = $this->asn1->readTlv($tstInfo, $offset);
        if ($genTime['tag'] !== 0x18) {
            throw new Exception('Invalid TSA token genTime');
        }

        $stamped = $this->asn1->decodeGeneralizedTime($genTime['value'], true);
        if ($stamped > ($now + $this->clockSkew) || $stamped < ($now - $this->clockSkew)) {
            throw new Exception('The TSA token genTime is not near the time of the request: ' . $genTime['value']);
        }

        return $stamped;
    }

    /**
     * Read the optional TSTInfo nonce, skipping the optional fields before it.
     *
     * @param int $offset Read cursor positioned just after genTime.
     *
     * @return string DER INTEGER of the nonce, or '' when absent.
     *
     * @throws Exception If the structure is malformed.
     */
    private function tstInfoNonce(string $tstInfo, int $offset): string
    {
        while ($offset < \strlen($tstInfo)) {
            $field = $this->asn1->readTlv($tstInfo, $offset);
            if ($field['tag'] === 0x02) {
                return $field['raw'];
            }

            // accuracy (SEQUENCE) and ordering (BOOLEAN) may precede the nonce;
            // tsa [0] and extensions [1] follow it, so nothing is left to find.
            if ($field['tag'] !== 0x30 && $field['tag'] !== 0x01) {
                break;
            }
        }

        return '';
    }

    /**
     * Unwrap a timestamp token to the content octets of its TSTInfo.
     *
     * @throws Exception If the token is not a SignedData carrying a TSTInfo.
     */
    private function tstInfo(string $tokenDer): string
    {
        $offset = 0;
        [$type, $octets] = $this->certificate->encapsulatedContent(
            $this->certificate->signedDataContent($tokenDer),
            $offset,
        );

        if ($type !== $this->asn1->encodeObjectIdentifier(Oid::TST_INFO)) {
            throw new Exception('The TSA token does not carry a TSTInfo');
        }

        $infoOff = 0;
        $info = $this->asn1->readTlv($octets, $infoOff);
        if ($info['tag'] !== 0x30 || $infoOff !== \strlen($octets)) {
            throw new Exception('Invalid TSTInfo structure');
        }

        return $info['value'];
    }

    /**
     * Encode a digest AlgorithmIdentifier.
     *
     * RFC 5754 section 2 requires the SHA-2 identifiers to be generated with the
     * parameters absent, which is what the CMS side of this library emits.
     *
     * @throws Exception If encoding fails.
     */
    private function algorithmIdentifier(string $oid): string
    {
        return $this->asn1->encodeSequence($this->asn1->encodeObjectIdentifier($oid));
    }

    /**
     * Map a digest algorithm name to its OID.
     *
     * @throws Exception If the algorithm is not supported.
     */
    public function hashAlgorithmOid(string $algorithm): string
    {
        $digest = DigestAlgorithm::tryFrom($algorithm);
        if ($digest === null) {
            throw new Exception('Unsupported TSA hash algorithm: ' . $algorithm);
        }

        return $digest->oid();
    }
}
