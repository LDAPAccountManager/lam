<?php

declare(strict_types=1);

/**
 * WidgetTest.php
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

namespace Test\Output;

use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Output\Widget;
use PHPUnit\Framework\TestCase;

/**
 * Widget Output Test
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
class WidgetTest extends TestCase
{
    private Widget $widget;

    protected function setUp(): void
    {
        $this->widget = new Widget();
    }

    public function testSignedFieldWidget(): void
    {
        $out = $this->widget->annotation(8, '10.0 20.0 110.0 60.0', 5, 'Signature', 9, ' /AS /N /AP << /N 20 0 R >>');

        $this->assertStringStartsWith("8 0 obj\n", $out);
        $this->assertStringEndsWith(" >>\nendobj\n", $out);
        $this->assertStringContainsString('/Type /Annot /Subtype /Widget', $out);
        $this->assertStringContainsString('/Rect [10.0 20.0 110.0 60.0]', $out);
        $this->assertStringContainsString('/P 5 0 R', $out);
        $this->assertStringContainsString('/F 4 /FT /Sig', $out);
        $this->assertStringContainsString('/T (Signature)', $out);
        $this->assertStringContainsString('/Ff 0', $out);
        $this->assertStringContainsString('/AS /N /AP << /N 20 0 R >>', $out);
        $this->assertStringContainsString('/V 9 0 R', $out);
    }

    public function testEmptyFieldWidgetHasNoValueOrAppearance(): void
    {
        $out = $this->widget->annotation(4, '0 0 100 40', 5, 'Reviewer [002]');
        $this->assertStringContainsString('/T (Reviewer [002])', $out);
        $this->assertStringNotContainsString('/V ', $out);
        $this->assertStringNotContainsString('/AP', $out);
    }

    public function testUsesInjectedStringEncoder(): void
    {
        $encoder = static fn(string $text, int $_objectId): string => '<' . \bin2hex($text) . '>';
        $out = $this->widget->annotation(4, '0 0 1 1', 5, 'Sig', null, '', $encoder);
        $this->assertStringContainsString('/T <' . \bin2hex('Sig') . '>', $out);
    }

    public function testRejectsNonStringEncoderResult(): void
    {
        $encoder = static fn(string $_text, int $objectId): int => $objectId;
        $this->expectException(Exception::class);
        $this->widget->annotation(4, '0 0 1 1', 5, 'Sig', null, '', $encoder);
    }
}
