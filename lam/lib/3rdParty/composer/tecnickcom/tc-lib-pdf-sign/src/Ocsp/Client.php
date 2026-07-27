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
use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Ocsp\Client
 *
 * RFC 6960 OCSP request builder. Extracts the subject Name and public key from
 * the issuer certificate and the serial number from the target certificate,
 * then assembles an OCSPRequest with a SHA-1 CertID. HTTP transport is injected
 * into fetch() so the codec stays pure and testable while the host controls
 * networking (and SSRF protection).
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
    private Asn1 $asn1;

    public function __construct(?Asn1 $asn1 = null)
    {
        $this->asn1 = $asn1 ?? new Asn1();
    }

    /**
     * Build a DER-encoded RFC 6960 OCSPRequest for a single certificate.
     *
     * @param string $issuerDer DER of the issuing certificate.
     * @param string $leafDer   DER of the certificate whose status is queried.
     *
     * @throws Exception If either certificate cannot be parsed or encoded.
     */
    public function build(string $issuerDer, string $leafDer): string
    {
        $issuer = $this->extractSubjectAndPublicKey($issuerDer);
        $issuerNameHash = \hash('sha1', $issuer['subject'], true);
        $issuerKeyHash = \hash('sha1', $issuer['public_key'], true);
        $serial = $this->extractSerialNumber($leafDer);

        $algId = $this->asn1->encodeSequence(
            $this->asn1->encodeObjectIdentifier('1.3.14.3.2.26') . $this->asn1->encodeNull(),
        );
        $certId = $this->asn1->encodeSequence(
            $algId . $this->asn1->encodeOctetString($issuerNameHash) . $this->asn1->encodeOctetString($issuerKeyHash)
                . $this->asn1->encodeIntegerBytes($serial),
        );
        $requestList = $this->asn1->encodeSequence($this->asn1->encodeSequence($certId));

        return $this->asn1->encodeSequence($this->asn1->encodeSequence($requestList));
    }

    /**
     * Build the request and submit it through the given transport.
     *
     * @param string   $url       OCSP responder URL.
     * @param string   $issuerDer DER of the issuing certificate.
     * @param string   $leafDer   DER of the target certificate.
     * @param callable $transport Receives (url, DER request) and must return the
     *                            DER response string.
     *
     * @throws Exception If building, transport, or the response type fails.
     */
    public function fetch(string $url, string $issuerDer, string $leafDer, callable $transport): string
    {
        $request = $this->build($issuerDer, $leafDer);

        /** @var mixed $response */
        $response = $transport($url, $request);
        if (!\is_string($response)) {
            throw new Exception('Invalid OCSP transport response');
        }

        return $response;
    }

    /**
     * Extract the raw DER of the subject Name and the public-key bytes from a
     * DER-encoded X.509 certificate.
     *
     * The subject bytes are the full DER of the subject Name SEQUENCE. The
     * public-key bytes are the subjectPublicKey BIT STRING value without the
     * leading unused-bits octet. For an OCSP CertID the issuerNameHash and
     * issuerKeyHash are computed over the SUBJECT of the issuing certificate,
     * so this reads the subject field (not the issuer field).
     *
     * @return array{subject: string, public_key: string}
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    public function extractSubjectAndPublicKey(string $certDer): array
    {
        $tbs = $this->tbsCertificate($certDer);

        $off = 0;
        $this->skipOptionalVersion($tbs, $off);
        $this->asn1->readTlv($tbs, $off); // serialNumber
        $this->asn1->readTlv($tbs, $off); // signature AlgorithmIdentifier
        $this->asn1->readTlv($tbs, $off); // issuer Name
        $this->asn1->readTlv($tbs, $off); // validity

        $subjectStart = $off;
        $this->asn1->readTlv($tbs, $off); // subject Name
        $subjectDer = \substr($tbs, $subjectStart, $off - $subjectStart);

        $spki = $this->asn1->readTlv($tbs, $off); // subjectPublicKeyInfo
        $spkiOff = 0;
        $this->asn1->readTlv($spki['value'], $spkiOff); // algorithm
        $bitStr = $this->asn1->readTlv($spki['value'], $spkiOff); // subjectPublicKey BIT STRING

        return [
            'subject' => $subjectDer,
            'public_key' => \substr($bitStr['value'], 1),
        ];
    }

    /**
     * Extract the raw serialNumber INTEGER content octets from a DER-encoded
     * X.509 certificate.
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    public function extractSerialNumber(string $certDer): string
    {
        $tbs = $this->tbsCertificate($certDer);

        $off = 0;
        $this->skipOptionalVersion($tbs, $off);
        $serial = $this->asn1->readTlv($tbs, $off); // serialNumber

        return $serial['value'];
    }

    /**
     * Return the TBSCertificate content octets of a DER-encoded certificate.
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    private function tbsCertificate(string $certDer): string
    {
        $certOff = 0;
        $certTlv = $this->asn1->readTlv($certDer, $certOff);

        $tbsOff = 0;
        $tbsTlv = $this->asn1->readTlv($certTlv['value'], $tbsOff);

        return $tbsTlv['value'];
    }

    /**
     * Skip the optional [0] EXPLICIT version field if present.
     *
     * @param int $off Read cursor; advanced past the version when present.
     *
     * @throws Exception If the version field is malformed.
     */
    private function skipOptionalVersion(string $tbs, int &$off): void
    {
        if ($off < \strlen($tbs) && (\ord($tbs[$off]) & 0xE0) === 0xA0) {
            $this->asn1->readTlv($tbs, $off);
        }
    }
}
