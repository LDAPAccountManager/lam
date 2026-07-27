<?php

declare(strict_types=1);

/**
 * DssTest.php
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

namespace Test\Output;

use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Output\Dss;
use PHPUnit\Framework\TestCase;

/**
 * DSS Output Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class DssTest extends TestCase
{
    private Dss $dss;

    protected function setUp(): void
    {
        $this->dss = new Dss();
    }

    public function testEmitReturnsNothingForEmptyMaterial(): void
    {
        $pon = 7;
        $result = $this->dss->emit(['certs' => [], 'ocsp' => [], 'crls' => []], 'SIG', $pon);
        $this->assertSame([], $result['objects']);
        $this->assertSame(0, $result['object_id']);
        $this->assertSame(7, $pon);
    }

    public function testEmitProducesStreamsVriAndDss(): void
    {
        $pon = 10;
        $contents = 'CMS-SIGNATURE-BYTES';
        $result = $this->dss->emit(
            ['certs' => ['CERT-DER'], 'ocsp' => ['OCSP-RESP'], 'crls' => ['CRL-DATA']],
            $contents,
            $pon,
        );

        // 3 streams (11,12,13), VRI (14), DSS (15).
        $this->assertSame(15, $pon);
        $this->assertSame(15, $result['object_id']);

        // The whole map is keyed by object number, ready for an incremental xref.
        $vriKey = \strtoupper(\sha1($contents));
        $this->assertSame(
            [
                11 => "11 0 obj\n<< /Length 8 >>\nstream\nCERT-DER\nendstream\nendobj\n",
                12 => "12 0 obj\n<< /Length 9 >>\nstream\nOCSP-RESP\nendstream\nendobj\n",
                13 => "13 0 obj\n<< /Length 8 >>\nstream\nCRL-DATA\nendstream\nendobj\n",
                14 => "14 0 obj\n<< /Type /VRI /Cert [ 11 0 R ] /OCSP [ 12 0 R ] /CRL [ 13 0 R ] >>\nendobj\n",
                15 =>
                    "15 0 obj\n<< /Type /DSS /VRI << /"
                        . $vriKey
                        . ' 14 0 R >>'
                        . ' /Certs [ 11 0 R ] /OCSPs [ 12 0 R ] /CRLs [ 13 0 R ]'
                        . " >>\nendobj\n",
            ],
            $result['objects'],
        );
    }

    public function testEmitOmitsEmptyCategories(): void
    {
        $pon = 0;
        $result = $this->dss->emit(['certs' => ['A', 'B'], 'ocsp' => [], 'crls' => []], 'SIG', $pon);

        // 2 cert streams (1,2), VRI (3), DSS (4).
        $this->assertSame(4, $result['object_id']);
        $objects = \implode('', $result['objects']);

        $this->assertStringContainsString('<< /Type /VRI /Cert [ 1 0 R 2 0 R ] >>', $objects);
        $this->assertStringNotContainsString('/OCSP ', $objects);
        $this->assertStringNotContainsString('/CRL ', $objects);
        $this->assertStringContainsString('/Certs [ 1 0 R 2 0 R ]', $objects);
        $this->assertStringNotContainsString('/OCSPs', $objects);
        $this->assertStringNotContainsString('/CRLs', $objects);
    }

    public function testEmitEncryptsStreams(): void
    {
        $pon = 0;
        $encryptor = static fn(string $data, int $objectId): string => 'E' . $objectId . ':' . $data;
        $result = $this->dss->emit(['certs' => ['X'], 'ocsp' => [], 'crls' => []], 'SIG', $pon, $encryptor);

        // Stream 1 carries the encrypted payload "E1:X" (length 4).
        $objects = \implode('', $result['objects']);
        $this->assertStringContainsString("1 0 obj\n<< /Length 4 >>\nstream\nE1:X\nendstream\nendobj\n", $objects);
    }

    public function testEmitRejectsNonStringEncryptorResult(): void
    {
        $pon = 0;
        $encryptor = static fn(string $_data, int $objectId): int => $objectId;
        $this->expectException(Exception::class);
        $this->dss->emit(['certs' => ['X'], 'ocsp' => [], 'crls' => []], 'SIG', $pon, $encryptor);
    }
}
