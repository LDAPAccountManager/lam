<?php

declare(strict_types=1);

/**
 * SignatureTest.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Test\Output;

use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Output\Signature;
use PHPUnit\Framework\TestCase;

/**
 * Signature Output Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class SignatureTest extends TestCase
{
    private Signature $signature;

    protected function setUp(): void
    {
        $this->signature = new Signature();
    }

    private const DOCMDP_REFERENCE =
        ' /Reference [ << /Type /SigRef /TransformMethod /DocMDP'
            . ' /TransformParams << /Type /TransformParams /P 2 /V /1.2 >> >> ]';

    private const DATE_VALUE = "(D:20231114221320+00'00')";

    public function testValueObjectWithReferenceAndInfo(): void
    {
        $out = $this->signature->valueObject(
            12,
            'ETSI.CAdES.detached',
            self::DOCMDP_REFERENCE,
            ['Name' => 'Jane Doe', 'Location' => 'Rome', 'Reason' => 'Approval', 'ContactInfo' => 'jane@example.org'],
            self::DATE_VALUE,
        );

        $this->assertStringStartsWith("12 0 obj\n", $out);
        $this->assertStringEndsWith(" >>\nendobj\n", $out);
        $this->assertStringContainsString('/Type /Sig /Filter /Adobe.PPKLite /SubFilter /ETSI.CAdES.detached', $out);
        $this->assertStringContainsString(Signature::BYTE_RANGE_PLACEHOLDER, $out);
        $this->assertStringContainsString(
            '/Contents<' . \str_repeat('0', Signature::DEFAULT_CONTENTS_LENGTH) . '>',
            $out,
        );
        $this->assertStringContainsString(self::DOCMDP_REFERENCE, $out);
        $this->assertStringContainsString('/Name (Jane Doe)', $out);
        $this->assertStringContainsString('/Location (Rome)', $out);
        $this->assertStringContainsString('/Reason (Approval)', $out);
        $this->assertStringContainsString('/ContactInfo (jane@example.org)', $out);
        // The /M date token is appended verbatim (already encoded by the caller).
        $this->assertStringContainsString(' /M ' . self::DATE_VALUE . ' >>', $out);
    }

    public function testEmptyReferenceIsOmitted(): void
    {
        $out = $this->signature->valueObject(3, 'ETSI.CAdES.detached', '', [], self::DATE_VALUE);
        $this->assertStringNotContainsString('/Reference', $out);
        $this->assertStringNotContainsString('/Name', $out);
        $this->assertStringContainsString('/SubFilter /ETSI.CAdES.detached', $out);
    }

    public function testCustomContentsLength(): void
    {
        $out = $this->signature->valueObject(1, 'adbe.pkcs7.detached', '', [], self::DATE_VALUE, 128);
        $this->assertStringContainsString('/Contents<' . \str_repeat('0', 128) . '>', $out);
        $this->assertStringNotContainsString(\str_repeat('0', 129), $out);
    }

    public function testDefaultEncoderEscapesLiteralStrings(): void
    {
        $out = $this->signature->valueObject(1, 'adbe.pkcs7.detached', '', ['Name' => 'A (B) \\ C'], self::DATE_VALUE);
        $this->assertStringContainsString('/Name (A \\(B\\) \\\\ C)', $out);
    }

    public function testUsesInjectedStringEncoder(): void
    {
        $encoder = static fn(string $text, int $_objectId): string => '<' . \bin2hex($text) . '>';
        $out = $this->signature->valueObject(
            5,
            'ETSI.CAdES.detached',
            '',
            ['Reason' => 'Hi'],
            self::DATE_VALUE,
            Signature::DEFAULT_CONTENTS_LENGTH,
            $encoder,
        );
        $this->assertStringContainsString('/Reason <' . \bin2hex('Hi') . '>', $out);
    }

    public function testRejectsNonStringEncoderResult(): void
    {
        $encoder = static fn(string $_text, int $objectId): int => $objectId;
        $this->expectException(Exception::class);
        $this->signature->valueObject(
            5,
            'ETSI.CAdES.detached',
            '',
            ['Reason' => 'Hi'],
            self::DATE_VALUE,
            Signature::DEFAULT_CONTENTS_LENGTH,
            $encoder,
        );
    }
}
