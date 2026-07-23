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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Timestamp;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Timestamp\Client
 *
 * RFC 3161 timestamp codec. Builds a TimeStampReq for a signature, parses a
 * TimeStampResp to extract the timestamp token, and maps digest algorithms to
 * their OIDs. HTTP transport is intentionally not part of this class: pass a
 * transport callable to requestToken() so the codec stays pure and testable
 * while the host controls networking (and SSRF protection).
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Client
{
    private Asn1 $asn1;

    public function __construct(
        private readonly Config $config,
        ?Asn1 $asn1 = null,
    ) {
        $this->asn1 = $asn1 ?? new Asn1();
    }

    /**
     * Build a DER-encoded RFC 3161 TimeStampReq for the given signature bytes.
     *
     * @param string $signature Signature (or any bytes) to be timestamped.
     *
     * @throws Exception If encoding fails or a nonce cannot be generated.
     */
    public function buildRequest(string $signature): string
    {
        $hashAlgo = $this->config->hashAlgorithm;
        $hash = \hash($hashAlgo, $signature, true);

        $oid = $this->hashAlgorithmOid($hashAlgo);
        $messageImprint = $this->asn1->encodeSequence(
            $this->asn1->encodeSequence($this->asn1->encodeObjectIdentifier($oid) . $this->asn1->encodeNull())
                . $this->asn1->encodeOctetString($hash),
        );

        $body = $this->asn1->encodeInteger(1) . $messageImprint;
        if ($this->config->policyOid !== '') {
            $body .= $this->asn1->encodeObjectIdentifier($this->config->policyOid);
        }

        if ($this->config->nonceEnabled) {
            try {
                $nonce = \random_int(1, PHP_INT_MAX);
            } catch (\Random\RandomException $e) {
                // Defensive: the CSPRNG failing is not reproducible in a unit test.
                throw new Exception('Unable to generate random nonce: ' . $e->getMessage(), 0, $e);
            }

            $body .= $this->asn1->encodeInteger($nonce);
        }

        $body .= $this->asn1->encodeBoolean(true);
        return $this->asn1->encodeSequence($body);
    }

    /**
     * Extract the timestamp token from a DER-encoded RFC 3161 TimeStampResp.
     *
     * @param string $response DER-encoded timestamp response.
     *
     * @return string DER-encoded timestamp token (ContentInfo).
     *
     * @throws Exception If the response is empty, malformed, or rejected.
     */
    public function parseResponse(string $response): string
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

        return $token['raw'];
    }

    /**
     * Build the request, submit it through the given transport, and parse the
     * returned token.
     *
     * @param string   $signature Signature bytes to timestamp.
     * @param callable $transport Receives the DER request string and must
     *                            return the DER response string.
     *
     * @throws Exception If encoding, transport, or parsing fails.
     */
    public function requestToken(string $signature, callable $transport): string
    {
        $request = $this->buildRequest($signature);

        /** @var mixed $response */
        $response = $transport($request);
        if (!\is_string($response)) {
            throw new Exception('Invalid TSA transport response');
        }

        return $this->parseResponse($response);
    }

    /**
     * Map a digest algorithm name to its OID.
     *
     * @throws Exception If the algorithm is not supported.
     */
    public function hashAlgorithmOid(string $algorithm): string
    {
        return match ($algorithm) {
            'sha256' => '2.16.840.1.101.3.4.2.1',
            'sha384' => '2.16.840.1.101.3.4.2.2',
            'sha512' => '2.16.840.1.101.3.4.2.3',
            default => throw new Exception('Unsupported TSA hash algorithm: ' . $algorithm),
        };
    }
}
