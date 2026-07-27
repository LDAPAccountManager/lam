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

namespace Test\Timestamp;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Timestamp\Client;
use Com\Tecnick\Pdf\Sign\Timestamp\Config;
use PHPUnit\Framework\TestCase;

/**
 * Timestamp Client Test
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

    protected function setUp(): void
    {
        $this->asn1 = new Asn1();
    }

    private function client(bool $nonce = false, string $policyOid = '', string $hash = 'sha256'): Client
    {
        return new Client(new Config(
            host: 'https://tsa.example.org',
            hashAlgorithm: $hash,
            policyOid: $policyOid,
            nonceEnabled: $nonce,
        ));
    }

    /**
     * Build a minimal valid TimeStampResp wrapping the given content.
     *
     * @param int<0, max> $statusCode PKIStatus value.
     */
    private function response(int $statusCode, string $content): string
    {
        return $this->asn1->encodeSequence(
            $this->asn1->encodeSequence($this->asn1->encodeInteger($statusCode)) . $content,
        );
    }

    private function sampleToken(): string
    {
        return $this->asn1->encodeSequence($this->asn1->encodeObjectIdentifier('1.2.840.113549.1.7.2'));
    }

    public function testHashAlgorithmOid(): void
    {
        $client = $this->client();
        $this->assertSame('2.16.840.1.101.3.4.2.1', $client->hashAlgorithmOid('sha256'));
        $this->assertSame('2.16.840.1.101.3.4.2.2', $client->hashAlgorithmOid('sha384'));
        $this->assertSame('2.16.840.1.101.3.4.2.3', $client->hashAlgorithmOid('sha512'));
    }

    public function testHashAlgorithmOidRejectsUnknown(): void
    {
        $this->expectException(Exception::class);
        $this->client()->hashAlgorithmOid('sha1');
    }

    public function testBuildRequestStructure(): void
    {
        $req = $this->client()->buildRequest('payload');

        $offset = 0;
        $root = $this->asn1->readTlv($req, $offset);
        $this->assertSame(0x30, $root['tag']);
        $this->assertSame(\strlen($req), $offset);

        $inner = 0;
        $version = $this->asn1->readTlv($root['value'], $inner);
        $this->assertSame(0x02, $version['tag']);
        $this->assertSame(1, $this->asn1->decodeInteger($version['value']));

        $messageImprint = $this->asn1->readTlv($root['value'], $inner);
        $this->assertSame(0x30, $messageImprint['tag']);

        $certReq = $this->asn1->readTlv($root['value'], $inner);
        $this->assertSame(0x01, $certReq['tag']);
        $this->assertSame("\xFF", $certReq['value']);
        // Nothing follows certReq when no policy and no nonce are present.
        $this->assertSame(\strlen($root['value']), $inner);

        // The message imprint carries the SHA-256 digest of the input.
        $miOffset = 0;
        $algId = $this->asn1->readTlv($messageImprint['value'], $miOffset);
        $this->assertSame(0x30, $algId['tag']);
        $digest = $this->asn1->readTlv($messageImprint['value'], $miOffset);
        $this->assertSame(0x04, $digest['tag']);
        $this->assertSame(\hash('sha256', 'payload', true), $digest['value']);
    }

    public function testBuildRequestIncludesPolicyOid(): void
    {
        $req = $this->client(policyOid: '1.2.3.4')->buildRequest('x');

        $offset = 0;
        $root = $this->asn1->readTlv($req, $offset);
        $inner = 0;
        $this->asn1->readTlv($root['value'], $inner); // version
        $this->asn1->readTlv($root['value'], $inner); // messageImprint

        $policy = $this->asn1->readTlv($root['value'], $inner);
        $this->assertSame(0x06, $policy['tag']);
        $this->assertSame($this->asn1->encodeObjectIdentifier('1.2.3.4'), $policy['raw']);
    }

    public function testBuildRequestIncludesNonce(): void
    {
        $req = $this->client(nonce: true)->buildRequest('x');

        $offset = 0;
        $root = $this->asn1->readTlv($req, $offset);
        $inner = 0;
        $this->asn1->readTlv($root['value'], $inner); // version
        $this->asn1->readTlv($root['value'], $inner); // messageImprint

        $nonce = $this->asn1->readTlv($root['value'], $inner);
        $this->assertSame(0x02, $nonce['tag']);

        $certReq = $this->asn1->readTlv($root['value'], $inner);
        $this->assertSame(0x01, $certReq['tag']);
    }

    public function testParseResponseReturnsToken(): void
    {
        $token = $this->sampleToken();
        $this->assertSame($token, $this->client()->parseResponse($this->response(0, $token)));
        // status 1 (granted with mods) is also accepted
        $this->assertSame($token, $this->client()->parseResponse($this->response(1, $token)));
    }

    public function testParseResponseRejectsEmpty(): void
    {
        $this->expectException(Exception::class);
        $this->client()->parseResponse('');
    }

    public function testParseResponseRejectsNonSequenceRoot(): void
    {
        $this->expectException(Exception::class);
        $this->client()->parseResponse($this->asn1->encodeInteger(0));
    }

    public function testParseResponseRejectsInvalidStatusStructure(): void
    {
        $bad = $this->asn1->encodeSequence($this->asn1->encodeInteger(0) . $this->sampleToken());
        $this->expectException(Exception::class);
        $this->client()->parseResponse($bad);
    }

    public function testParseResponseRejectsNonIntegerStatus(): void
    {
        $bad = $this->asn1->encodeSequence(
            $this->asn1->encodeSequence($this->asn1->encodeOctetString('x')) . $this->sampleToken(),
        );
        $this->expectException(Exception::class);
        $this->client()->parseResponse($bad);
    }

    public function testParseResponseRejectsRejectedStatus(): void
    {
        $this->expectException(Exception::class);
        $this->client()->parseResponse($this->response(2, $this->sampleToken()));
    }

    public function testParseResponseRejectsMissingToken(): void
    {
        $noToken = $this->asn1->encodeSequence($this->asn1->encodeSequence($this->asn1->encodeInteger(0)));
        $this->expectException(Exception::class);
        $this->client()->parseResponse($noToken);
    }

    public function testParseResponseRejectsNonSequenceToken(): void
    {
        $bad = $this->asn1->encodeSequence(
            $this->asn1->encodeSequence($this->asn1->encodeInteger(0)) . $this->asn1->encodeInteger(5),
        );
        $this->expectException(Exception::class);
        $this->client()->parseResponse($bad);
    }

    public function testRequestTokenUsesTransport(): void
    {
        $token = $this->sampleToken();
        $response = $this->response(0, $token);

        $captured = '';
        $transport = static function (string $request) use (&$captured, $response): string {
            $captured = $request;
            return $response;
        };

        $result = $this->client()->requestToken('payload', $transport);
        $this->assertSame($token, $result);

        // The transport received a well-formed DER request.
        $offset = 0;
        $root = $this->asn1->readTlv($captured, $offset);
        $this->assertSame(0x30, $root['tag']);
    }

    public function testRequestTokenRejectsNonStringTransportResult(): void
    {
        $transport = static fn(string $request): int => \strlen($request);
        $this->expectException(Exception::class);
        $this->client()->requestToken('payload', $transport);
    }
}
