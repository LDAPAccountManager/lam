<?php

declare(strict_types=1);

/**
 * SignerTest.php
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

namespace Test;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Config;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Signer;
use Com\Tecnick\Pdf\Sign\Timestamp\Client as TimestampClient;
use Com\Tecnick\Pdf\Sign\Timestamp\Config as TimestampConfig;
use OpenSSLAsymmetricKey;
use PHPUnit\Framework\TestCase;

/**
 * Signer Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class SignerTest extends TestCase
{
    private const SIGNING_TIME = 1_700_000_000;

    private const OID_SIGNATURE_TIMESTAMP = '1.2.840.113549.1.9.16.2.14';

    private const OID_SIGNING_TIME = '1.2.840.113549.1.9.5';

    private Asn1 $asn1;

    protected function setUp(): void
    {
        $this->asn1 = new Asn1();
    }

    public function testSignLegacyProfileHasNoSignatureTimestamp(): void
    {
        $cred = $this->makeCredential();
        $signer = new Signer();

        $cms = $signer->sign(
            'document bytes',
            $cred['cert_der'],
            $cred['key'],
            [],
            new Config(Config::PROFILE_LEGACY),
            self::SIGNING_TIME,
        );

        $this->assertStringNotContainsString($this->timestampOidDer(), $cms);
        // The legacy (ISO 32000-1) profile keeps the CMS signing-time attribute.
        $this->assertStringContainsString($this->signingTimeOidDer(), $cms);
    }

    public function testSignBbProfileHasNoSignatureTimestamp(): void
    {
        $cred = $this->makeCredential();
        $signer = new Signer();

        $cms = $signer->sign(
            'document bytes',
            $cred['cert_der'],
            $cred['key'],
            [],
            new Config(Config::PROFILE_PADES_B_B),
            self::SIGNING_TIME,
        );

        $this->assertStringNotContainsString($this->timestampOidDer(), $cms);
        // PAdES-BASELINE forbids the CMS signing-time attribute (ETSI EN 319 142-1);
        // the signing time is carried by the /M signature dictionary entry instead.
        $this->assertStringNotContainsString($this->signingTimeOidDer(), $cms);
    }

    public function testSignBtProfileEmbedsSignatureTimestamp(): void
    {
        $cred = $this->makeCredential();
        $token = $this->asn1->encodeSequence($this->asn1->encodeOctetString('rfc3161-token-body'));

        $captured = null;
        $transport = function (string $request) use (&$captured, $token): string {
            $captured = $request;
            return $this->timestampResponse($token);
        };

        $signer = new Signer();
        $cms = $signer->sign(
            'document bytes',
            $cred['cert_der'],
            $cred['key'],
            [],
            new Config(Config::PROFILE_PADES_B_T),
            self::SIGNING_TIME,
            new TimestampClient(new TimestampConfig('https://tsa.example.org')),
            $transport,
        );

        // The transport received a DER TimeStampReq (SEQUENCE).
        $this->assertIsString($captured);
        $this->assertSame("\x30", $captured[0]);

        // The CMS carries the signature-timestamp attribute and the returned token bytes.
        $this->assertStringContainsString($this->timestampOidDer(), $cms);
        $this->assertStringContainsString($token, $cms);
    }

    public function testSignBtProfileRequiresTimestampClient(): void
    {
        $cred = $this->makeCredential();
        $signer = new Signer();

        $this->expectException(Exception::class);
        $signer->sign(
            'document bytes',
            $cred['cert_der'],
            $cred['key'],
            [],
            new Config(Config::PROFILE_PADES_B_T),
            self::SIGNING_TIME,
        );
    }

    public function testSignBtProfileRequiresTransport(): void
    {
        $cred = $this->makeCredential();
        $signer = new Signer();

        $this->expectException(Exception::class);
        $signer->sign(
            'document bytes',
            $cred['cert_der'],
            $cred['key'],
            [],
            new Config(Config::PROFILE_PADES_B_LTA),
            self::SIGNING_TIME,
            new TimestampClient(new TimestampConfig('https://tsa.example.org')),
            null,
        );
    }

    public function testCollectValidationMaterialGathersCertsOcspAndCrls(): void
    {
        $ltvPem = (string) \file_get_contents(__DIR__ . '/data/ltv_cert.pem');
        $caPem = (string) \file_get_contents(__DIR__ . '/data/ocsp_ca.pem');

        $ocspCalls = [];
        $ocspTransport = static function (string $url, string $request) use (&$ocspCalls): string {
            $ocspCalls[] = ['url' => $url, 'request' => $request];
            return 'OCSP-RESPONSE';
        };
        $crlTransport = static fn(string $url): string => 'CRL-' . $url;

        $signer = new Signer();
        $material = $signer->collectValidationMaterial([$ltvPem, $caPem], $ocspTransport, $crlTransport);

        // Both certificates are collected as DER.
        $this->assertCount(2, $material['certs']);

        // The leaf's single OCSP responder was queried; the CA has none.
        $this->assertCount(1, $ocspCalls);
        $firstOcspCall = $ocspCalls[0] ?? null;
        if (!\is_array($firstOcspCall)) {
            $this->fail('Expected a captured OCSP call');
        }

        $this->assertSame('http://ocsp.example.org/r', $firstOcspCall['url']);
        $this->assertSame("\x30", $firstOcspCall['request'][0]);
        $this->assertSame(['OCSP-RESPONSE'], $material['ocsp']);

        // The leaf carries two distinct CRL distribution points.
        $this->assertSame(
            ['CRL-http://crl.example.org/root.crl', 'CRL-http://crl2.example.org/root.crl'],
            $material['crls'],
        );
    }

    public function testCollectValidationMaterialSkipsRevocationWithoutTransports(): void
    {
        $ltvPem = (string) \file_get_contents(__DIR__ . '/data/ltv_cert.pem');
        $caPem = (string) \file_get_contents(__DIR__ . '/data/ocsp_ca.pem');

        $signer = new Signer();
        $material = $signer->collectValidationMaterial([$ltvPem, $caPem]);

        $this->assertCount(2, $material['certs']);
        $this->assertSame([], $material['ocsp']);
        $this->assertSame([], $material['crls']);
    }

    public function testCollectValidationMaterialDeduplicatesCertificates(): void
    {
        $ltvPem = (string) \file_get_contents(__DIR__ . '/data/ltv_cert.pem');

        $signer = new Signer();
        $material = $signer->collectValidationMaterial([$ltvPem, $ltvPem]);

        $this->assertCount(1, $material['certs']);
    }

    public function testCollectValidationMaterialRejectsInvalidPem(): void
    {
        $signer = new Signer();
        $this->expectException(Exception::class);
        $signer->collectValidationMaterial(['-----BEGIN CERTIFICATE-----@@-----END CERTIFICATE-----']);
    }

    /**
     * DER of the id-aa-signatureTimeStampToken OID, used as a presence probe.
     */
    private function timestampOidDer(): string
    {
        return $this->asn1->encodeObjectIdentifier(self::OID_SIGNATURE_TIMESTAMP);
    }

    /**
     * DER of the CMS signing-time OID, used as a presence probe.
     */
    private function signingTimeOidDer(): string
    {
        return $this->asn1->encodeObjectIdentifier(self::OID_SIGNING_TIME);
    }

    /**
     * Build a minimal DER RFC 3161 TimeStampResp wrapping the given token.
     */
    private function timestampResponse(string $tstDer): string
    {
        $status = $this->asn1->encodeSequence($this->asn1->encodeInteger(0));
        return $this->asn1->encodeSequence($status . $tstDer);
    }

    /**
     * Generate an RSA private key and a matching self-signed certificate.
     *
     * @return array{key: OpenSSLAsymmetricKey, cert_pem: string, cert_der: string}
     */
    private function makeCredential(): array
    {
        $config = [
            'config' => __DIR__ . '/../openssl.cnf',
            'digest_alg' => 'sha256',
            'private_key_bits' => 2048,
            'private_key_type' => OPENSSL_KEYTYPE_RSA,
        ];

        $key = \openssl_pkey_new($config);
        if (!$key instanceof OpenSSLAsymmetricKey) {
            $this->markTestSkipped('RSA key generation is not available');
        }

        $csr = \openssl_csr_new(['commonName' => 'tc-lib-pdf-sign signer'], $key, $config);
        if (!$csr instanceof \OpenSSLCertificateSigningRequest) {
            $this->markTestSkipped('CSR generation failed');
        }

        $cert = \openssl_csr_sign($csr, null, $key, 365, $config);
        if (!$cert instanceof \OpenSSLCertificate) {
            $this->markTestSkipped('Certificate signing failed');
        }

        $certPem = '';
        \openssl_x509_export($cert, $certPem);
        $stripped = (string) \preg_replace('/-----[^-]+-----|\s+/', '', $certPem);
        $der = \base64_decode($stripped, true);
        if ($der === false) {
            $this->fail('Invalid PEM');
        }

        return ['key' => $key, 'cert_pem' => $certPem, 'cert_der' => $der];
    }
}
