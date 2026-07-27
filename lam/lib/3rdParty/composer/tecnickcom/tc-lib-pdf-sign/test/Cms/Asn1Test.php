<?php

declare(strict_types=1);

/**
 * Asn1Test.php
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

namespace Test\Cms;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Exception;
use PHPUnit\Framework\TestCase;

/**
 * Asn1 Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class Asn1Test extends TestCase
{
    private Asn1 $asn1;

    protected function setUp(): void
    {
        $this->asn1 = new Asn1();
    }

    public function testEncodeLengthShortForm(): void
    {
        $this->assertSame("\x05", $this->asn1->encodeLength(5));
        $this->assertSame("\x7F", $this->asn1->encodeLength(127));
    }

    public function testEncodeLengthLongForm(): void
    {
        $this->assertSame("\x81\x80", $this->asn1->encodeLength(128));
        $this->assertSame("\x82\x01\x00", $this->asn1->encodeLength(256));
    }

    public function testEncodeInteger(): void
    {
        $this->assertSame("\x02\x01\x00", $this->asn1->encodeInteger(0));
        $this->assertSame("\x02\x01\x7F", $this->asn1->encodeInteger(127));
        $this->assertSame("\x02\x02\x00\xFF", $this->asn1->encodeInteger(255));
        $this->assertSame("\x02\x02\x01\x00", $this->asn1->encodeInteger(256));
    }

    public function testEncodeIntegerBytesTrimsAndPads(): void
    {
        $this->assertSame("\x02\x01\x7F", $this->asn1->encodeIntegerBytes("\x00\x7F"));
        $this->assertSame("\x02\x02\x00\x80", $this->asn1->encodeIntegerBytes("\x80"));
    }

    public function testEncodeBoolean(): void
    {
        $this->assertSame("\x01\x01\xFF", $this->asn1->encodeBoolean(true));
        $this->assertSame("\x01\x01\x00", $this->asn1->encodeBoolean(false));
    }

    public function testEncodeNull(): void
    {
        $this->assertSame("\x05\x00", $this->asn1->encodeNull());
    }

    public function testEncodeOctetStringSequenceSet(): void
    {
        $this->assertSame("\x04\x02AB", $this->asn1->encodeOctetString('AB'));
        $this->assertSame("\x30\x02AB", $this->asn1->encodeSequence('AB'));
        $this->assertSame("\x31\x02AB", $this->asn1->encodeSet('AB'));
    }

    public function testEncodeContext(): void
    {
        $this->assertSame("\xA0\x02AB", $this->asn1->encodeContext(0, 'AB'));
        $this->assertSame("\xA3\x02AB", $this->asn1->encodeContext(3, 'AB'));
    }

    public function testEncodeObjectIdentifier(): void
    {
        // sha256WithRSAEncryption: 1.2.840.113549.1.1.11
        $this->assertSame(
            '06092a864886f70d01010b',
            \bin2hex($this->asn1->encodeObjectIdentifier('1.2.840.113549.1.1.11')),
        );
    }

    public function testReadTlvRoundTrip(): void
    {
        $der = $this->asn1->encodeSequence($this->asn1->encodeInteger(256));
        $offset = 0;
        $tlv = $this->asn1->readTlv($der, $offset);
        $this->assertSame(0x30, $tlv['tag']);
        $this->assertSame(\strlen($der), $offset);
        $this->assertSame($der, $tlv['raw']);

        $inner = 0;
        $intTlv = $this->asn1->readTlv($tlv['value'], $inner);
        $this->assertSame(0x02, $intTlv['tag']);
        $this->assertSame(256, $this->asn1->decodeInteger($intTlv['value']));
    }

    public function testReadTlvRejectsTruncatedData(): void
    {
        $this->expectException(Exception::class);
        $offset = 0;
        $this->asn1->readTlv("\x30\x05\x00", $offset);
    }

    public function testDecodeIntegerRejectsEmpty(): void
    {
        $this->expectException(Exception::class);
        $this->asn1->decodeInteger('');
    }

    public function testEncodeIntegerBytesEmptyInputYieldsZero(): void
    {
        $this->assertSame("\x02\x01\x00", $this->asn1->encodeIntegerBytes(''));
    }

    public function testEncodeObjectIdentifierRejectsSingleArc(): void
    {
        $this->expectException(Exception::class);
        $this->asn1->encodeObjectIdentifier('1');
    }

    public function testEncodeObjectIdentifierClampsNegativeArc(): void
    {
        $this->assertSame('06022a00', \bin2hex($this->asn1->encodeObjectIdentifier('1.2.-1')));
    }

    public function testReadTlvRejectsEmptyData(): void
    {
        $this->expectException(Exception::class);
        $offset = 0;
        $this->asn1->readTlv('', $offset);
    }

    public function testReadTlvRejectsMissingLength(): void
    {
        $this->expectException(Exception::class);
        $offset = 0;
        $this->asn1->readTlv("\x30", $offset);
    }

    public function testReadTlvRejectsUnsupportedLongFormLength(): void
    {
        $this->expectException(Exception::class);
        $offset = 0;
        // 0x85 announces a 5-octet length, which exceeds the supported 4 octets.
        $this->asn1->readTlv("\x04\x85\x00\x00\x00\x00\x00", $offset);
    }

    public function testReadTlvHandlesLongFormLength(): void
    {
        // A 200-byte payload forces a multi-octet (long-form) DER length.
        $payload = \str_repeat("\x41", 200);
        $der = $this->asn1->encodeOctetString($payload);
        $offset = 0;
        $tlv = $this->asn1->readTlv($der, $offset);
        $this->assertSame(0x04, $tlv['tag']);
        $this->assertSame($payload, $tlv['value']);
        $this->assertSame(\strlen($der), $offset);
    }

    // Coverage note: Asn1::encodeLength() throws on a length needing more than
    // 127 octets to represent. That requires content larger than 2^1016 bytes,
    // which is unrepresentable by a PHP int and unallocatable, so the guard is
    // defensive and cannot be exercised in a unit test.
}
