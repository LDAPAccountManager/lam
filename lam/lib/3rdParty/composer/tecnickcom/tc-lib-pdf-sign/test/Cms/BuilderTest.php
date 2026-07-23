<?php

declare(strict_types=1);

/**
 * BuilderTest.php
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

namespace Test\Cms;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Cms\Builder;
use Com\Tecnick\Pdf\Sign\Exception;
use OpenSSLAsymmetricKey;
use PHPUnit\Framework\TestCase;

/**
 * CMS Builder Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class BuilderTest extends TestCase
{
    private const SIGNING_TIME = 1_700_000_000;

    private Asn1 $asn1;

    protected function setUp(): void
    {
        $this->asn1 = new Asn1();
    }

    public function testSignRsaSha256ProducesVerifiableCms(): void
    {
        $cred = $this->makeCredential('rsa');
        $data = 'The quick brown fox jumps over the lazy dog.';

        $builder = new Builder($this->asn1);
        $cms = $builder->sign($data, $cred['cert_der'], $cred['key'], [], 'sha256', self::SIGNING_TIME);

        $parts = $this->parseSignerInfo($cms);
        $this->assertSame(0xA0, $parts['signed_attrs']['tag']);
        $this->assertSame(0x04, $parts['signature']['tag']);
        $this->assertSame(0xA0, $parts['certificates']['tag']);
        $this->assertStringContainsString($cred['cert_der'], $parts['certificates']['value']);

        // Cryptographically verify the signature over the DER SET OF signed attributes.
        $this->assertVerifies($parts, $cred['cert_pem'], OPENSSL_ALGO_SHA256);

        // content-type present and equal to id-data.
        $contentType = $this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.3');
        $this->assertNotNull($contentType);
        $this->assertSame($this->asn1->encodeObjectIdentifier('1.2.840.113549.1.7.1'), $contentType['raw']);

        // signing-time is a UTCTime for a 2023 timestamp.
        $signingTime = $this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.5');
        $this->assertNotNull($signingTime);
        $this->assertSame(0x17, $signingTime['tag']);

        // message-digest equals SHA-256 of the content.
        $messageDigest = $this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.4');
        $this->assertNotNull($messageDigest);
        $this->assertSame(\hash('sha256', $data, true), $messageDigest['value']);

        // signing-certificate-v2 carries the SHA-256 hash of the signer certificate;
        // for SHA-256 the ESSCertIDv2 hashAlgorithm is omitted so certHash is first.
        $certHash = $this->firstCertHash('1.2.840.113549.1.9.16.2.47', $parts['signed_attrs']['value']);
        $this->assertSame(0x04, $certHash['tag']);
        $this->assertSame(\hash('sha256', $cred['cert_der'], true), $certHash['value']);
    }

    public function testSignOmitsSigningTimeForPadesBaseline(): void
    {
        $cred = $this->makeCredential('rsa');
        $data = 'PAdES-BASELINE forbids the CMS signing-time attribute.';

        $builder = new Builder($this->asn1);
        // includeSigningTime = false: the PAdES-BASELINE case, where the signing time
        // is carried by the /M signature dictionary entry rather than the CMS.
        $cms = $builder->sign($data, $cred['cert_der'], $cred['key'], [], 'sha256', self::SIGNING_TIME, null, false);

        $parts = $this->parseSignerInfo($cms);
        // The signature still verifies over the (smaller) DER SET OF signed attributes.
        $this->assertVerifies($parts, $cred['cert_pem'], OPENSSL_ALGO_SHA256);

        // signing-time (1.2.840.113549.1.9.5) is absent.
        $this->assertNull($this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.5'));

        // The other mandatory signed attributes remain present.
        $this->assertNotNull($this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.3'));
        $this->assertNotNull($this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.4'));
        $this->assertNotNull($this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.16.2.47'));
    }

    public function testSignEcSha256ProducesVerifiableCms(): void
    {
        $cred = $this->makeCredential('ec');
        $data = 'elliptic-curve payload';

        $builder = new Builder($this->asn1);
        $cms = $builder->sign($data, $cred['cert_der'], $cred['key'], [], 'sha256', self::SIGNING_TIME);

        $parts = $this->parseSignerInfo($cms);
        $this->assertVerifies($parts, $cred['cert_pem'], OPENSSL_ALGO_SHA256);
    }

    public function testSignRsaSha384IncludesEssCertHashAlgorithm(): void
    {
        $cred = $this->makeCredential('rsa');
        $builder = new Builder($this->asn1);
        $cms = $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'sha384', self::SIGNING_TIME);

        $parts = $this->parseSignerInfo($cms);
        $this->assertVerifies($parts, $cred['cert_pem'], OPENSSL_ALGO_SHA384);

        // For a non-default digest, ESSCertIDv2 begins with the hashAlgorithm SEQUENCE.
        $scv2 = $this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.16.2.47');
        $this->assertNotNull($scv2);
        $certsOffset = 0;
        $certs = $this->asn1->readTlv($scv2['value'], $certsOffset);
        $essOffset = 0;
        $ess = $this->asn1->readTlv($certs['value'], $essOffset);
        $firstOffset = 0;
        $first = $this->asn1->readTlv($ess['value'], $firstOffset);
        $this->assertSame(0x30, $first['tag']);
    }

    public function testSignRsaSha512ProducesVerifiableCms(): void
    {
        $cred = $this->makeCredential('rsa');
        $builder = new Builder($this->asn1);
        $cms = $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'sha512', self::SIGNING_TIME);

        $parts = $this->parseSignerInfo($cms);
        $this->assertVerifies($parts, $cred['cert_pem'], OPENSSL_ALGO_SHA512);
    }

    public function testSignEmbedsChainCertificates(): void
    {
        $cred = $this->makeCredential('rsa');
        $chainDer = $this->pemToDer((string) \file_get_contents(__DIR__ . '/../data/ocsp_ca.pem'));

        $builder = new Builder($this->asn1);
        $cms = $builder->sign('data', $cred['cert_der'], $cred['key'], [$chainDer], 'sha256', self::SIGNING_TIME);

        $parts = $this->parseSignerInfo($cms);
        $this->assertStringContainsString($cred['cert_der'], $parts['certificates']['value']);
        $this->assertStringContainsString($chainDer, $parts['certificates']['value']);
    }

    public function testSignWithoutTimestampHasNoUnsignedAttributes(): void
    {
        $cred = $this->makeCredential('rsa');
        $builder = new Builder($this->asn1);
        $cms = $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'sha256', self::SIGNING_TIME);

        $parts = $this->parseSignerInfo($cms);
        $this->assertNull($parts['unsigned_attrs']);
    }

    public function testSignEmbedsSignatureTimestampUnsignedAttribute(): void
    {
        $cred = $this->makeCredential('rsa');
        $token = $this->asn1->encodeSequence($this->asn1->encodeOctetString('fake-rfc3161-token'));

        $captured = '';
        $provider = static function (string $signature) use (&$captured, $token): string {
            $captured = $signature;
            return $token;
        };

        $builder = new Builder($this->asn1);
        $cms = $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'sha256', self::SIGNING_TIME, $provider);

        $parts = $this->parseSignerInfo($cms);
        // The signature is cryptographically unchanged by the added unsigned attribute.
        $this->assertVerifies($parts, $cred['cert_pem'], OPENSSL_ALGO_SHA256);

        // The provider timestamps the raw SignerInfo signature bytes.
        $this->assertSame($parts['signature']['value'], $captured);

        // unsignedAttrs is a [1] IMPLICIT context tag carrying id-aa-signatureTimeStampToken.
        $this->assertNotNull($parts['unsigned_attrs']);
        $this->assertSame(0xA1, $parts['unsigned_attrs']['tag']);

        $tstValue = $this->attributeValue($parts['unsigned_attrs']['value'], '1.2.840.113549.1.9.16.2.14');
        $this->assertNotNull($tstValue);
        $this->assertSame($token, $tstValue['raw']);
    }

    public function testSignRejectsEmptySignatureTimestampToken(): void
    {
        $cred = $this->makeCredential('rsa');
        $provider = static fn(): string => '';

        $builder = new Builder($this->asn1);
        $this->expectException(Exception::class);
        $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'sha256', self::SIGNING_TIME, $provider);
    }

    public function testSignUsesGeneralizedTimeForFarFuture(): void
    {
        $cred = $this->makeCredential('rsa');
        $builder = new Builder($this->asn1);
        // 2100-01-01T00:00:00Z is outside the UTCTime range (1950-2049).
        $cms = $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'sha256', 4_102_444_800);

        $parts = $this->parseSignerInfo($cms);
        $signingTime = $this->attributeValue($parts['signed_attrs']['value'], '1.2.840.113549.1.9.5');
        $this->assertNotNull($signingTime);
        $this->assertSame(0x18, $signingTime['tag']);
    }

    public function testSignRejectsUnsupportedDigest(): void
    {
        $cred = $this->makeCredential('rsa');
        $builder = new Builder($this->asn1);
        $this->expectException(Exception::class);
        $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'md5', self::SIGNING_TIME);
    }

    public function testSignFailsWithNonSigningKey(): void
    {
        $cred = $this->makeCredential('rsa');
        $publicKey = \openssl_pkey_get_public($cred['cert_pem']);
        if ($publicKey === false) {
            $this->fail('Unable to load public key');
        }

        $builder = new Builder($this->asn1);
        $this->expectException(Exception::class);
        \set_error_handler(static fn(): bool => true);
        try {
            $builder->sign('data', $cred['cert_der'], $publicKey, [], 'sha256', self::SIGNING_TIME);
        } finally {
            \restore_error_handler();
        }
    }

    public function testSignRejectsUnsupportedKeyType(): void
    {
        $cred = $this->makeCredential('dsa');
        $builder = new Builder($this->asn1);
        $this->expectException(Exception::class);
        $builder->sign('data', $cred['cert_der'], $cred['key'], [], 'sha256', self::SIGNING_TIME);
    }

    /**
     * Generate a private key and a matching self-signed certificate.
     *
     * @return array{key: OpenSSLAsymmetricKey, cert_pem: string, cert_der: string}
     */
    private function makeCredential(string $keyType): array
    {
        $config = [
            'config' => __DIR__ . '/../../openssl.cnf',
            'digest_alg' => 'sha256',
            'private_key_bits' => 2048,
            'private_key_type' => OPENSSL_KEYTYPE_RSA,
        ];
        if ($keyType === 'ec') {
            $config['private_key_type'] = OPENSSL_KEYTYPE_EC;
            $config['curve_name'] = 'prime256v1';
        } elseif ($keyType === 'dsa') {
            $config['private_key_type'] = OPENSSL_KEYTYPE_DSA;
            $config['private_key_bits'] = 1024;
        }

        $key = \openssl_pkey_new($config);
        if (!$key instanceof OpenSSLAsymmetricKey) {
            $this->markTestSkipped($keyType . ' key generation is not available');
        }

        $csr = \openssl_csr_new(['commonName' => 'tc-lib-pdf-sign signer'], $key, $config);
        if (!$csr instanceof \OpenSSLCertificateSigningRequest) {
            $this->markTestSkipped('CSR generation failed for ' . $keyType);
        }

        $cert = \openssl_csr_sign($csr, null, $key, 365, $config);
        if (!$cert instanceof \OpenSSLCertificate) {
            $this->markTestSkipped('Certificate signing failed for ' . $keyType);
        }

        $certPem = '';
        \openssl_x509_export($cert, $certPem);

        return ['key' => $key, 'cert_pem' => $certPem, 'cert_der' => $this->pemToDer($certPem)];
    }

    private function pemToDer(string $pem): string
    {
        $stripped = (string) \preg_replace('/-----[^-]+-----|\s+/', '', $pem);
        $der = \base64_decode($stripped, true);
        if ($der === false) {
            $this->fail('Invalid PEM');
        }

        return $der;
    }

    /**
     * Verify the SignerInfo signature over the reconstructed DER SET OF signed attributes.
     *
     * @param array{signed_attrs: array{tag:int,value:string,raw:string}, signature: array{tag:int,value:string,raw:string}, certificates: array{tag:int,value:string,raw:string}, unsigned_attrs: array{tag:int,value:string,raw:string}|null} $parts
     */
    private function assertVerifies(array $parts, string $certPem, int $opensslAlgo): void
    {
        $publicKey = \openssl_pkey_get_public($certPem);
        if ($publicKey === false) {
            $this->fail('Unable to load public key');
        }

        $signedAttrsSet = $this->asn1->encodeSet($parts['signed_attrs']['value']);
        $result = \openssl_verify($signedAttrsSet, $parts['signature']['value'], $publicKey, $opensslAlgo);
        $this->assertSame(1, $result);
    }

    /**
     * Descend into a SigningCertificate attribute and return the ESSCertID certHash TLV.
     *
     * @return array{tag: int, value: string, raw: string}
     */
    private function firstCertHash(string $oid, string $attrsDer): array
    {
        $value = $this->attributeValue($attrsDer, $oid);
        $this->assertNotNull($value);
        $certsOffset = 0;
        $certs = $this->asn1->readTlv($value['value'], $certsOffset);
        $essOffset = 0;
        $ess = $this->asn1->readTlv($certs['value'], $essOffset);
        $hashOffset = 0;
        return $this->asn1->readTlv($ess['value'], $hashOffset);
    }

    /**
     * Find an Attribute by OID and return the first value TLV of its value SET.
     *
     * @return array{tag: int, value: string, raw: string}|null
     */
    private function attributeValue(string $attrsDer, string $oid): ?array
    {
        $oidDer = $this->asn1->encodeObjectIdentifier($oid);
        $offset = 0;
        $length = \strlen($attrsDer);
        while ($offset < $length) {
            $attribute = $this->asn1->readTlv($attrsDer, $offset);
            $inner = 0;
            $attrOid = $this->asn1->readTlv($attribute['value'], $inner);
            if ($attrOid['raw'] === $oidDer) {
                $set = $this->asn1->readTlv($attribute['value'], $inner);
                $valueOffset = 0;
                return $this->asn1->readTlv($set['value'], $valueOffset);
            }
        }

        return null;
    }

    /**
     * Navigate a CMS ContentInfo to the SignerInfo fields under test.
     *
     * @return array{signed_attrs: array{tag:int,value:string,raw:string}, signature: array{tag:int,value:string,raw:string}, certificates: array{tag:int,value:string,raw:string}, unsigned_attrs: array{tag:int,value:string,raw:string}|null}
     */
    private function parseSignerInfo(string $cms): array
    {
        $offset = 0;
        $contentInfo = $this->asn1->readTlv($cms, $offset);

        $ciOffset = 0;
        $this->asn1->readTlv($contentInfo['value'], $ciOffset); // contentType OID
        $explicit = $this->asn1->readTlv($contentInfo['value'], $ciOffset); // [0] EXPLICIT

        $sdOffset = 0;
        $signedData = $this->asn1->readTlv($explicit['value'], $sdOffset);

        $sdInner = 0;
        $this->asn1->readTlv($signedData['value'], $sdInner); // version
        $this->asn1->readTlv($signedData['value'], $sdInner); // digestAlgorithms
        $this->asn1->readTlv($signedData['value'], $sdInner); // encapContentInfo
        $certificates = $this->asn1->readTlv($signedData['value'], $sdInner); // certificates [0]
        $signerInfos = $this->asn1->readTlv($signedData['value'], $sdInner); // signerInfos SET

        $siOffset = 0;
        $signerInfo = $this->asn1->readTlv($signerInfos['value'], $siOffset);

        $siInner = 0;
        $this->asn1->readTlv($signerInfo['value'], $siInner); // version
        $this->asn1->readTlv($signerInfo['value'], $siInner); // sid
        $this->asn1->readTlv($signerInfo['value'], $siInner); // digestAlgorithm
        $signedAttrs = $this->asn1->readTlv($signerInfo['value'], $siInner); // [0] IMPLICIT
        $this->asn1->readTlv($signerInfo['value'], $siInner); // signatureAlgorithm
        $signature = $this->asn1->readTlv($signerInfo['value'], $siInner); // signature

        $unsignedAttrs = null;
        if ($siInner < \strlen($signerInfo['value'])) {
            $unsignedAttrs = $this->asn1->readTlv($signerInfo['value'], $siInner); // [1] IMPLICIT
        }

        return [
            'signed_attrs' => $signedAttrs,
            'signature' => $signature,
            'certificates' => $certificates,
            'unsigned_attrs' => $unsignedAttrs,
        ];
    }
}
