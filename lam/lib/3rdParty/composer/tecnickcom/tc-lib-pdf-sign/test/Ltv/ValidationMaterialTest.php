<?php

declare(strict_types=1);

/**
 * ValidationMaterialTest.php
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

namespace Test\Ltv;

use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Ltv\ValidationMaterial;
use PHPUnit\Framework\TestCase;

/**
 * ValidationMaterial Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class ValidationMaterialTest extends TestCase
{
    private ValidationMaterial $material;

    private string $ltvPem = '';

    private string $caPem = '';

    private string $leafDer = '';

    private string $caDer = '';

    protected function setUp(): void
    {
        $this->material = new ValidationMaterial();
        $this->ltvPem = (string) \file_get_contents(__DIR__ . '/../data/ltv_cert.pem');
        $this->caPem = (string) \file_get_contents(__DIR__ . '/../data/ocsp_ca.pem');
        $leafPem = (string) \file_get_contents(__DIR__ . '/../data/ocsp_leaf.pem');
        $this->leafDer = $this->pemToDer($leafPem);
        $this->caDer = $this->pemToDer($this->caPem);
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

    public function testCertificateOcspUrlsExtractsOnlyOcsp(): void
    {
        $urls = $this->material->certificateOcspUrls($this->ltvPem);
        $this->assertSame(['http://ocsp.example.org/r'], $urls);
    }

    public function testCertificateCrlUrlsExtractsAll(): void
    {
        $urls = $this->material->certificateCrlUrls($this->ltvPem);
        $this->assertSame(['http://crl.example.org/root.crl', 'http://crl2.example.org/root.crl'], $urls);
    }

    public function testUrlsEmptyWhenExtensionAbsent(): void
    {
        // The OCSP CA fixture carries no AIA or CRL distribution point extensions.
        $this->assertSame([], $this->material->certificateOcspUrls($this->caPem));
        $this->assertSame([], $this->material->certificateCrlUrls($this->caPem));
    }

    public function testCertificateUrlsEmptyForUnparseableCertificate(): void
    {
        // LTV collection is best-effort: a certificate that cannot be parsed yields no
        // OCSP/CRL URLs rather than aborting the whole signing operation. The certificate
        // is still embeddable because its DER bytes are obtained separately.
        \set_error_handler(static fn(): bool => true);
        try {
            $this->assertSame([], $this->material->certificateOcspUrls('not-a-certificate'));
            $this->assertSame([], $this->material->certificateCrlUrls('not-a-certificate'));
        } finally {
            \restore_error_handler();
        }
    }

    public function testCertificatesDeduplicates(): void
    {
        $leafPem = (string) \file_get_contents(__DIR__ . '/../data/ocsp_leaf.pem');
        $ders = $this->material->certificates([$leafPem, $leafPem, $this->caPem]);
        $this->assertCount(2, $ders);
        $this->assertSame([$this->leafDer, $this->caDer], $ders);
    }

    public function testCertificatesRejectsInvalidPem(): void
    {
        $this->expectException(Exception::class);
        $this->material->certificates(["-----BEGIN CERTIFICATE-----\n@@@@\n-----END CERTIFICATE-----"]);
    }

    public function testFetchOcspBuildsRequestAndDeduplicates(): void
    {
        $captured = [];
        $transport = static function (string $url, string $request) use (&$captured): string {
            $captured[] = ['url' => $url, 'request' => $request];
            return 'OCSP-RESPONSE';
        };

        $responses = $this->material->fetchOcsp(
            $this->caDer,
            $this->leafDer,
            ['http://ocsp.a.example', 'http://ocsp.b.example'],
            $transport,
        );

        // Two URLs, identical responses collapse to one.
        $this->assertSame(['OCSP-RESPONSE'], $responses);
        $this->assertCount(2, $captured);
        // The transport received a DER OCSP request (SEQUENCE).
        $firstCapture = $captured[0] ?? null;
        if (!\is_array($firstCapture)) {
            $this->fail('Expected a captured OCSP request');
        }

        $this->assertSame("\x30", $firstCapture['request'][0]);
    }

    public function testFetchOcspSkipsFailingUrl(): void
    {
        $transport = static function (string $url): string {
            if (\str_contains($url, 'bad')) {
                throw new \RuntimeException('boom');
            }

            return 'RESP-' . $url;
        };

        $responses = $this->material->fetchOcsp(
            $this->caDer,
            $this->leafDer,
            ['http://bad.example', 'http://good.example'],
            $transport,
        );

        $this->assertSame(['RESP-http://good.example'], $responses);
    }

    public function testFetchOcspReturnsEmptyWhenNoUrls(): void
    {
        $calls = 0;
        $transport = static function () use (&$calls): string {
            ++$calls;
            return 'X';
        };

        $this->assertSame([], $this->material->fetchOcsp($this->caDer, $this->leafDer, [], $transport));
        $this->assertSame(0, $calls);
    }

    public function testFetchCrlDeduplicatesAndSkipsEmpty(): void
    {
        $transport = static function (string $url): string {
            if (\str_contains($url, 'empty')) {
                return '';
            }

            return 'CRL-DATA';
        };

        $responses = $this->material->fetchCrl(
            ['http://empty.example', 'http://a.example', 'http://b.example'],
            $transport,
        );

        // Empty response skipped; the two identical CRLs collapse to one.
        $this->assertSame(['CRL-DATA'], $responses);
    }
}
