<?php

/**
 * ParserTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Pdfparser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Parser\Parser;
use PHPUnit\Framework\Attributes\DataProvider;
use PHPUnit\Framework\TestCase;

/**
 * Filter Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfParser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 */
class ParserTest extends TestCase
{
    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    #[DataProvider('getParseProvider')]
    public function testParse(string $filename, string $hash): void
    {
        $cfg = [
            'ignore_filter_errors' => true,
        ];
        $rawdata = \file_get_contents($filename);
        $this->assertNotFalse($rawdata);
        $parser = new Parser($cfg);
        $data = $parser->parse($rawdata);
        $this->assertEquals($hash, \md5(\serialize($data)));
    }

    /**
     * @return array<int, array{0:string, 1:string}>
     */
    public static function getParseProvider(): array
    {
        return [
            ['resources/test/example_005.pdf', '510a5ea860470dae0781f4bd8d5eb250'],
            ['resources/test/example_036.pdf', '2869501cf41a4c4a0402c00832329e25'],
            ['resources/test/example_046.pdf', 'cfaee514b9c09aa282b4e2a8f0061a3d'],
        ];
    }

    /**
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParseHandlesMultiRangeXrefIndexRegression(): void
    {
        $parser = new Parser(['ignore_filter_errors' => true]);
        $data = $parser->parse($this->buildMultiRangeXrefIndexPdf());

        $xref = $data[0];
        $objects = $data[1];

        $this->assertSame('41_0', $xref['trailer']['root']);
        $this->assertArrayNotHasKey('encrypt', $xref['trailer']);

        $this->assertNotNull($objects['41_0'][0] ?? null);
        $this->assertSame('<<', $objects['41_0'][0][0]);
        $this->assertNotNull($objects['42_0'][0] ?? null);
        $this->assertSame('<<', $objects['42_0'][0][0]);
        $this->assertNotNull($objects['43_0'][0] ?? null);
        $this->assertSame('<<', $objects['43_0'][0][0]);

        $this->assertSame('7aa83a972ca79bafe26be3620b7512e2', \md5(\serialize($data)));
    }

    private function buildMultiRangeXrefIndexPdf(): string
    {
        $pdf = "%PDF-1.7\n";
        $offsets = [];
        $addObject = static function (string &$pdfData, array &$objOffsets, int $objNum, string $body): void {
            $objOffsets[$objNum] = \strlen($pdfData);
            $pdfData .= $objNum . " 0 obj\n" . $body . "\nendobj\n";
        };

        $addObject($pdf, $offsets, 3, '<< /Dummy 3 >>');
        $addObject($pdf, $offsets, 15, '<< /Dummy 15 >>');
        $addObject($pdf, $offsets, 17, '<< /Dummy 17 >>');
        $addObject($pdf, $offsets, 18, '<< /Dummy 18 >>');
        $addObject($pdf, $offsets, 41, '<< /Type /Catalog /Pages 42 0 R >>');
        $addObject($pdf, $offsets, 42, '<< /Type /Pages /Count 1 /Kids [43 0 R] >>');
        $addObject($pdf, $offsets, 43, '<< /Type /Page /Parent 42 0 R /MediaBox [0 0 10 10] >>');

        $indexObjects = [3, 15, 17, 18, 41, 42, 43];
        $bytes = [];
        foreach ($indexObjects as $objNum) {
            $offset = (int) ($offsets[$objNum] ?? 0);
            $bytes[] = 0;
            $bytes[] = 1;
            $bytes[] = ($offset >> 16) & 0xff;
            $bytes[] = ($offset >> 8) & 0xff;
            $bytes[] = $offset & 0xff;
        }

        $stream = \pack('C*', ...$bytes);
        $streamLen = \strlen($stream);
        $xrefBody =
            '<< /Type /XRef'
            . ' /Size 60'
            . ' /Root 41 0 R'
            . ' /Index [3 1 15 1 17 2 41 3]'
            . ' /W [1 3 0]'
            . ' /Length '
            . $streamLen
            . ' /DecodeParms << /Columns 4 /Predictor 12 >>'
            . " >>\nstream\n"
            . $stream
            . "\nendstream";

        $addObject($pdf, $offsets, 50, $xrefBody);

        $startxref = (int) ($offsets[50] ?? 0);
        $pdf .= "startxref\n" . $startxref . "\n%%EOF";

        return $pdf;
    }
}
