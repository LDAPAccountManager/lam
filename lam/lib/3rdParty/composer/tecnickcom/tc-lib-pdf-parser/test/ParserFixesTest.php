<?php

/**
 * ParserFixesTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Pdfparser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Parser\Parser;
use PHPUnit\Framework\TestCase;

/**
 * Regression tests for parser hardening and stream-decoding fixes.
 *
 * @phpstan-import-type RawObjectArray from \Com\Tecnick\Pdf\Parser\Process\RawObject
 */
class ParserFixesTest extends TestCase
{
    /**
     * F1: the same parser instance must be reusable for multiple documents.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testParserInstanceIsReusableAcrossDocuments(): void
    {
        $pdf = (string) \file_get_contents('resources/test/example_005.pdf');
        $parser = new Parser(['ignore_filter_errors' => true]);

        $first = $parser->parse($pdf);
        $second = $parser->parse($pdf);

        $this->assertSame(\md5(\serialize($first)), \md5(\serialize($second)));
    }

    /**
     * F2: a truncated object body running to EOF must not emit PHP warnings.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testTruncatedObjectBodyDoesNotEmitWarnings(): void
    {
        $header = "%PDF-1.4\n";
        $objBody = "1 0 obj\n(unterminated literal string with no closing parenthesis\n";
        $objOffset = \strlen($header);
        $document = $header . $objBody;
        $xrefOffset = \strlen($document);
        $document .=
            "xref\n0 2\n0000000000 65535 f \n"
            . $this->xrefInUseEntry($objOffset)
            . "trailer\n<< /Size 2 /Root 1 0 R >>\nstartxref\n"
            . $xrefOffset
            . "\n%%EOF";

        $warnings = [];
        \set_error_handler(static function (int $_errno, string $errstr) use (&$warnings): bool {
            $warnings[] = $errstr;
            return true;
        });

        try {
            $parser = new Parser(['ignore_filter_errors' => true]);
            $parser->parse($document);
        } finally {
            \restore_error_handler();
        }

        $this->assertSame([], $warnings);
    }

    /**
     * F3: a false "endstream" marker inside the payload must not truncate the stream
     * when a direct /Length declares the real length.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testStreamIsNotTruncatedAtFalseEndstreamWithDirectLength(): void
    {
        $payload = "ABC endstream FAKE\nXYZ-real-tail";
        $this->assertSame(
            $payload,
            $this->extractStreamPayload($this->buildFalseEndstreamPdf((string) \strlen($payload), $payload)),
        );
    }

    /**
     * F3: the same protection must work when /Length is an indirect reference.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testStreamIsNotTruncatedAtFalseEndstreamWithIndirectLength(): void
    {
        $payload = "ABC endstream FAKE\nXYZ-real-tail";
        $this->assertSame($payload, $this->extractStreamPayload($this->buildFalseEndstreamPdf('5 0 R', $payload)));
    }

    /**
     * F4: a PNG predictor declared in DecodeParms of a regular FlateDecode stream
     * must be reversed (Colors/BitsPerComponent honoured).
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testPngPredictorIsReversedForRegularFlateStream(): void
    {
        $colors = 3;
        $columns = 2;
        $rawRows = [
            [10,  20,  30, 40, 50, 60],
            [11,  22,  33, 44, 55, 66],
            [200, 100, 50, 7,  8,  9],
        ];

        $rawFlat = '';
        foreach ($rawRows as $row) {
            $rawFlat .= \pack('C*', ...$row);
        }

        $predicted = '';
        foreach ($rawRows as $row) {
            // PNG Sub filter (type 1): encoded = raw - left
            $predicted .= \chr(1);
            $count = \count($row);
            for ($i = 0; $i < $count; ++$i) {
                $left = $i >= $colors ? (int) ($row[$i - $colors] ?? 0) : 0;
                $predicted .= \chr(((int) ($row[$i] ?? 0) - $left) & 0xff);
            }
        }

        $compressed = (string) \gzcompress($predicted);
        $dict =
            '<< /Length '
            . \strlen($compressed)
            . ' /Filter /FlateDecode'
            . ' /DecodeParms << /Predictor 12 /Colors '
            . $colors
            . ' /BitsPerComponent 8 /Columns '
            . $columns
            . ' >> >>';

        $header = "%PDF-1.4\n";
        $obj1 = "1 0 obj\n" . $dict . "\nstream\n" . $compressed . "\nendstream\nendobj\n";
        $obj1Offset = \strlen($header);
        $document = $header . $obj1;
        $obj2Offset = \strlen($document);
        $document .= "2 0 obj\n<< /Type /Catalog >>\nendobj\n";
        $xrefOffset = \strlen($document);
        $document .=
            "xref\n0 3\n0000000000 65535 f \n"
            . $this->xrefInUseEntry($obj1Offset)
            . $this->xrefInUseEntry($obj2Offset)
            . "trailer\n<< /Size 3 /Root 2 0 R >>\nstartxref\n"
            . $xrefOffset
            . "\n%%EOF";

        $parser = new Parser();
        [, $objects] = $parser->parse($document);

        $decoded = null;
        foreach ($objects['1_0'] ?? [] as $element) {
            if ($element[0] !== 'stream' || !\array_key_exists(3, $element)) {
                continue;
            }

            $decoded = $element[3][0];
        }

        $this->assertSame($rawFlat, $decoded);
    }

    /**
     * F4: a TIFF Predictor 2 declared in DecodeParms of a regular FlateDecode stream
     * must be reversed.
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testTiffPredictorIsReversedForRegularFlateStream(): void
    {
        $colors = 1;
        $columns = 4;
        $rawRows = [
            [5, 10, 3, 250],
            [1, 2, 3, 4],
        ];

        $rawFlat = '';
        foreach ($rawRows as $row) {
            $rawFlat .= \pack('C*', ...$row);
        }

        $predicted = '';
        foreach ($rawRows as $row) {
            // TIFF horizontal differencing: encoded = sample - sample-to-the-left
            for ($i = 0; $i < $columns; ++$i) {
                $left = $i >= $colors ? (int) ($row[$i - $colors] ?? 0) : 0;
                $predicted .= \chr(((int) ($row[$i] ?? 0) - $left) & 0xff);
            }
        }

        $compressed = (string) \gzcompress($predicted);
        $dict =
            '<< /Length '
            . \strlen($compressed)
            . ' /Filter /FlateDecode'
            . ' /DecodeParms << /Predictor 2 /Colors '
            . $colors
            . ' /BitsPerComponent 8 /Columns '
            . $columns
            . ' >> >>';

        $header = "%PDF-1.4\n";
        $obj1 = "1 0 obj\n" . $dict . "\nstream\n" . $compressed . "\nendstream\nendobj\n";
        $obj1Offset = \strlen($header);
        $document = $header . $obj1;
        $obj2Offset = \strlen($document);
        $document .= "2 0 obj\n<< /Type /Catalog >>\nendobj\n";
        $xrefOffset = \strlen($document);
        $document .=
            "xref\n0 3\n0000000000 65535 f \n"
            . $this->xrefInUseEntry($obj1Offset)
            . $this->xrefInUseEntry($obj2Offset)
            . "trailer\n<< /Size 3 /Root 2 0 R >>\nstartxref\n"
            . $xrefOffset
            . "\n%%EOF";

        $parser = new Parser();
        [, $objects] = $parser->parse($document);

        $decoded = null;
        foreach ($objects['1_0'] ?? [] as $element) {
            if ($element[0] !== 'stream' || !\array_key_exists(3, $element)) {
                continue;
            }

            $decoded = $element[3][0];
        }

        $this->assertSame($rawFlat, $decoded);
    }

    /**
     * F4: a cross-reference stream that uses a predictor must still be decoded
     * correctly (the predictor must not be applied twice).
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testCrossReferenceStreamWithPredictorIsNotDoubleDecoded(): void
    {
        $width = [1, 2, 1];
        $rowlen = \max(0, (int) \array_sum($width));

        $header = "%PDF-1.4\n";
        $obj1Offset = \strlen($header);
        $document = $header . "1 0 obj\n<< /Type /Catalog >>\nendobj\n";
        $xrefObjOffset = \strlen($document);

        $entries = [
            [0, 0,              0],
            [1, $obj1Offset,    0],
            [1, $xrefObjOffset, 0],
        ];

        $predicted = '';
        $prev = \array_fill(0, $rowlen, 0);
        foreach ($entries as $entry) {
            $row = $this->encodeXrefEntry($entry, $width);
            $predicted .= \chr(2); // PNG Up filter
            for ($i = 0; $i < $rowlen; ++$i) {
                $predicted .= \chr(((int) ($row[$i] ?? 0) - (int) ($prev[$i] ?? 0)) & 0xff);
            }

            $prev = $row;
        }

        $compressed = (string) \gzcompress($predicted);
        $dict =
            '<< /Type /XRef /Size 3 /Root 1 0 R /W [1 2 1] /Filter /FlateDecode'
            . ' /DecodeParms << /Predictor 12 /Columns '
            . $rowlen
            . ' >> /Length '
            . \strlen($compressed)
            . ' >>';
        $document .= "2 0 obj\n" . $dict . "\nstream\n" . $compressed . "\nendstream\nendobj\n";
        $document .= "startxref\n" . $xrefObjOffset . "\n%%EOF";

        $parser = new Parser();
        [$xref, $objects] = $parser->parse($document);

        $this->assertSame($obj1Offset, $xref['xref']['1_0'] ?? null);
        $this->assertSame($xrefObjOffset, $xref['xref']['2_0'] ?? null);
        $this->assertSame('1_0', $xref['trailer']['root']);
        $this->assertSame('<<', $objects['1_0'][0][0] ?? null);
    }

    /**
     * F7: a trailer dictionary that contains a nested dictionary must be parsed
     * without being truncated at the first ">>".
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    public function testTrailerWithNestedDictionaryIsParsed(): void
    {
        $header = "%PDF-1.4\n";
        $objOffset = \strlen($header);
        $document = $header . "1 0 obj\n<< /Type /Catalog >>\nendobj\n";
        $xrefOffset = \strlen($document);
        $document .=
            "xref\n0 2\n0000000000 65535 f \n"
            . $this->xrefInUseEntry($objOffset)
            . "trailer\n<< /Custom << /Nested 1 >> /Size 2 /Root 1 0 R >>\nstartxref\n"
            . $xrefOffset
            . "\n%%EOF";

        $parser = new Parser();
        [$xref] = $parser->parse($document);

        $this->assertSame('1_0', $xref['trailer']['root']);
        $this->assertSame(2, $xref['trailer']['size']);
    }

    /**
     * Build a PDF whose object 1 stream contains a false "endstream" marker.
     *
     * @param string $lengthEntry The /Length entry value (a number or an "N 0 R" reference).
     * @param string $payload     The real stream payload.
     */
    private function buildFalseEndstreamPdf(string $lengthEntry, string $payload): string
    {
        $length = \strlen($payload);
        $header = "%PDF-1.4\n";

        $obj1 = "1 0 obj\n<< /Length " . $lengthEntry . " >>\nstream\n" . $payload . "\nendstream\nendobj\n";
        $obj1Offset = \strlen($header);
        $document = $header . $obj1;

        $obj2Offset = \strlen($document);
        $document .= "2 0 obj\n<< /Type /Catalog >>\nendobj\n";

        $obj5Offset = \strlen($document);
        $document .= "5 0 obj\n" . $length . "\nendobj\n";

        $xrefOffset = \strlen($document);
        $document .=
            "xref\n0 6\n0000000000 65535 f \n"
            . $this->xrefInUseEntry($obj1Offset)
            . $this->xrefInUseEntry($obj2Offset)
            . "0000000000 00000 f \n"
            . "0000000000 00000 f \n"
            . $this->xrefInUseEntry($obj5Offset)
            . "trailer\n<< /Size 6 /Root 2 0 R >>\nstartxref\n"
            . $xrefOffset
            . "\n%%EOF";

        return $document;
    }

    /**
     * Parse a PDF and return the raw payload of the stream in object "1_0".
     *
     * @throws \Com\Tecnick\Pdf\Parser\Exception
     */
    private function extractStreamPayload(string $document): string
    {
        $parser = new Parser(['decode_streams' => false]);
        [, $objects] = $parser->parse($document);

        foreach ($objects['1_0'] ?? [] as $element) {
            if ($element[0] === 'stream' && \is_string($element[1])) {
                return $element[1];
            }
        }

        return '';
    }

    /**
     * Encode a single xref-stream entry into its byte values using the given field widths.
     *
     * @param array{0: int, 1: int, 2: int} $entry Entry values (type, field2, field3).
     * @param array{0: int, 1: int, 2: int} $width Field widths in bytes.
     *
     * @return array<int, int> Byte values for the encoded row.
     */
    private function encodeXrefEntry(array $entry, array $width): array
    {
        $bytes = [];
        foreach ([0, 1, 2] as $field) {
            $value = $entry[$field] ?? 0;
            $fieldWidth = $width[$field] ?? 0;
            for ($byte = $fieldWidth - 1; $byte >= 0; --$byte) {
                $bytes[] = ($value >> ($byte * 8)) & 0xff;
            }
        }

        return $bytes;
    }

    /**
     * Build a classic in-use xref entry line for the given object offset.
     */
    private function xrefInUseEntry(int $offset): string
    {
        return \sprintf("%010d 00000 n \n", $offset);
    }
}
