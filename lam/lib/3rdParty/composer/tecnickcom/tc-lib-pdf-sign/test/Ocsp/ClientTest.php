<?php

declare(strict_types=1);

/**
 * ClientTest.php
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

namespace Test\Ocsp;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Ocsp\Client;
use PHPUnit\Framework\TestCase;

/**
 * OCSP Client Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class ClientTest extends TestCase
{
    private Asn1 $asn1;

    private string $leafPem = '';

    private string $leafDer = '';

    private string $caDer = '';

    protected function setUp(): void
    {
        $this->asn1 = new Asn1();
        $this->leafPem = (string) \file_get_contents(__DIR__ . '/../data/ocsp_leaf.pem');
        $this->leafDer = $this->pemToDer($this->leafPem);
        $this->caDer = $this->pemToDer((string) \file_get_contents(__DIR__ . '/../data/ocsp_ca.pem'));
    }

    private function pemToDer(string $pem): string
    {
        $stripped = (string) \preg_replace('/-----[^-]+-----|\s+/', '', $pem);
        $der = \base64_decode($stripped, true);
        if ($der === false) {
            $this->fail('Invalid PEM fixture');
        }

        return $der;
    }

    public function testExtractSubjectReturnsSubjectNotIssuer(): void
    {
        // The leaf subject (CN=...leaf) differs from its issuer (CN=...root CA),
        // so this proves the subject field is read, not the issuer field.
        $client = new Client($this->asn1);
        $info = $client->extractSubjectAndPublicKey($this->leafDer);
        $this->assertStringContainsString('tc-lib-pdf-sign leaf', $info['subject']);
        $this->assertStringNotContainsString('root CA', $info['subject']);
        $this->assertNotSame('', $info['public_key']);
    }

    public function testExtractSerialNumberMatchesOpenssl(): void
    {
        $parsed = \openssl_x509_parse($this->leafPem);
        if (!\is_array($parsed)) {
            $this->fail('Unable to parse leaf certificate');
        }

        $expectedHex = \strtolower($parsed['serialNumberHex']);
        $client = new Client($this->asn1);
        $serial = $client->extractSerialNumber($this->leafDer);
        $this->assertSame($expectedHex, \bin2hex($serial));
    }

    public function testBuildProducesValidOcspRequest(): void
    {
        $client = new Client($this->asn1);
        $req = $client->build($this->caDer, $this->leafDer);

        // OCSPRequest ::= SEQ { tbsRequest SEQ { requestList SEQ OF { Request SEQ { CertID SEQ } } } }
        $offset = 0;
        $ocspRequest = $this->asn1->readTlv($req, $offset);
        $this->assertSame(0x30, $ocspRequest['tag']);
        $this->assertSame(\strlen($req), $offset);

        $certId = $this->descend($ocspRequest['value'], 4); // tbsRequest, requestList, Request, CertID
        $this->assertSame(0x30, $certId['tag']);

        $inner = 0;
        $algId = $this->asn1->readTlv($certId['value'], $inner);
        $nameHash = $this->asn1->readTlv($certId['value'], $inner);
        $keyHash = $this->asn1->readTlv($certId['value'], $inner);
        $serial = $this->asn1->readTlv($certId['value'], $inner);

        // SHA-1 CertID hashes are computed over the issuer certificate's subject and key.
        $issuer = $client->extractSubjectAndPublicKey($this->caDer);
        $this->assertSame(0x04, $nameHash['tag']);
        $this->assertSame(\hash('sha1', $issuer['subject'], true), $nameHash['value']);
        $this->assertSame(20, \strlen($nameHash['value']));
        $this->assertSame(0x04, $keyHash['tag']);
        $this->assertSame(\hash('sha1', $issuer['public_key'], true), $keyHash['value']);
        $this->assertSame(20, \strlen($keyHash['value']));

        // hashAlgorithm OID is SHA-1 (1.3.14.3.2.26).
        $algOffset = 0;
        $oid = $this->asn1->readTlv($algId['value'], $algOffset);
        $this->assertSame($this->asn1->encodeObjectIdentifier('1.3.14.3.2.26'), $oid['raw']);

        // serialNumber matches the leaf certificate.
        $this->assertSame(0x02, $serial['tag']);
        $this->assertSame($client->extractSerialNumber($this->leafDer), $serial['value']);
    }

    public function testFetchUsesTransport(): void
    {
        $captured = ['url' => '', 'request' => ''];
        $transport = static function (string $url, string $request) use (&$captured): string {
            $captured['url'] = $url;
            $captured['request'] = $request;
            return 'OCSP-RESPONSE-BYTES';
        };

        $client = new Client($this->asn1);
        $result = $client->fetch('http://ocsp.example.org', $this->caDer, $this->leafDer, $transport);

        $this->assertSame('OCSP-RESPONSE-BYTES', $result);
        $this->assertSame('http://ocsp.example.org', $captured['url']);

        $offset = 0;
        $root = $this->asn1->readTlv($captured['request'], $offset);
        $this->assertSame(0x30, $root['tag']);
    }

    public function testFetchRejectsNonStringTransportResult(): void
    {
        $transport = static fn(string $url, string $request): int => \strlen($url . $request);
        $this->expectException(Exception::class);
        $client = new Client($this->asn1);
        $client->fetch('http://x', $this->caDer, $this->leafDer, $transport);
    }

    /**
     * Read the first TLV, then descend into the first child $depth times.
     *
     * @param int<0, max> $depth
     *
     * @return array{tag: int, value: string, raw: string}
     */
    private function descend(string $data, int $depth): array
    {
        $tlv = ['tag' => 0, 'value' => $data, 'raw' => $data];
        for ($i = 0; $i < $depth; ++$i) {
            $offset = 0;
            $tlv = $this->asn1->readTlv($tlv['value'], $offset);
        }

        return $tlv;
    }
}
