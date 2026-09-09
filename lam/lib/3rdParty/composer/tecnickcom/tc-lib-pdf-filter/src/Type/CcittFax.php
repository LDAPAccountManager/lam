<?php

declare(strict_types=1);

/**
 * CcittFax.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter\Type;

use Com\Tecnick\Pdf\Filter\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Filter\Type\CcittFax
 *
 * CCITTFaxDecode filter (PDF 32000-1:2008 §7.4.6).
 * Decompresses bi-level (1 bit per pixel) image data encoded with the CCITT
 * facsimile standards (ITU-T T.4 Group 3 or T.6 Group 4), by wrapping the raw
 * bitstream in a TIFF container and decoding it with the Imagick extension
 * (ext-imagick). The result is a PNG image.
 *
 * DecodeParms defaults (PDF 32000-1:2008 Table 11):
 * - K: 0 (= Group 3 1-D; negative = Group 4 / T.6; positive = Group 3 2-D)
 * - Columns: 1728 (standard fax width)
 * - Rows: 0 (decode until end-of-data)
 * - BlackIs1: false (0 bits are black)
 * - EncodedByteAlign: false (lines are not padded to a byte boundary)
 *
 * EncodedByteAlign maps to TIFF T4Options bit 2 and so applies to Group 3 only;
 * TIFF T6Options has no equivalent bit and it is ignored for Group 4.
 *
 * When Rows is absent the image height is estimated from the encoded length as
 * though the data were uncompressed, which under-estimates it and truncates the
 * image. Pass Rows (the image dictionary /Height) to decode the whole image.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * @phpstan-type TiffTag array{tag: int, type: int, count: int, value: int}
 * @phpstan-type TiffTagList array<int, TiffTag>
 */
class CcittFax implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * @var int CCITT compression group (3 or 4); affects TIFF header.
     */
    private int $group;

    /**
     * @var int Image width in pixels.
     */
    private int $columns;

    /**
     * @var int Image height in pixels; 0 = unknown / auto-detect.
     */
    private int $rows;

    /**
     * @var bool Whether 1 bits represent black (true) or white (false).
     */
    private bool $blackIs1;

    /**
     * @var bool Whether the encoding uses two-dimensional Group 3 (K > 0).
     */
    private bool $twoDimensional;

    /**
     * @var bool Whether each encoded line starts on a byte boundary.
     */
    private bool $encodedByteAlign;

    /**
     * Constructor.
     *
     * @param array<mixed> $params Optional DecodeParms dictionary (PDF 32000-1:2008 Table 11).
     *   - 'K' (int): compression mode; negative = pure 2-D (T.6 / Group 4),
     *     0 = pure 1-D (T.4 / Group 3), positive = mixed 1-D/2-D (T.4 / Group 3 2-D).
     *   - 'Columns' (int): image width (default 1728).
     *   - 'Rows' (int): image height; 0 = unknown (default 0).
     *   - 'BlackIs1' (bool): whether 1 bits are black (default false).
     *   - 'EncodedByteAlign' (bool): whether each line starts on a byte boundary (default false).
     */
    public function __construct(array $params = [])
    {
        $this->applyParams($params);
    }

    /**
     * Set the decoding parameters from a DecodeParms dictionary.
     *
     * @param array<array-key, mixed> $params DecodeParms dictionary.
     */
    private function applyParams(array $params): void
    {
        // PDF K to ITU-T group: K < 0 is Group 4 (T.6), K >= 0 is Group 3 (T.4)
        $kParam = (int) ($params['K'] ?? 0);
        $this->group = $kParam < 0 ? 4 : 3;
        $this->twoDimensional = $kParam > 0;
        $this->columns = \max(1, (int) ($params['Columns'] ?? 1728));
        $this->rows = \max(0, (int) ($params['Rows'] ?? 0));
        $this->blackIs1 = $this->toBool($params['BlackIs1'] ?? null);
        $this->encodedByteAlign = $this->toBool($params['EncodedByteAlign'] ?? null);
    }

    /**
     * Normalize a DecodeParms flag that may arrive as bool, int, float or string.
     *
     * @param mixed $value Raw parameter value.
     */
    private function toBool(mixed $value): bool
    {
        return match (true) {
            \is_bool($value) => $value,
            \is_int($value), \is_float($value) => $value !== 0,
            \is_string($value) => \in_array(\strtolower($value), ['1', 'true', 'yes', 'on'], true),
            default => false,
        };
    }

    /**
     * Decode the data.
     *
     * @param string               $data   Raw CCITT-compressed image data.
     * @param array<string, mixed> $params Optional DecodeParms dictionary; when not
     *   empty it replaces the one given to the constructor.
     *
     * @return string Decoded PNG image bytes.
     *
     * @throws PPException if Imagick is not available or decoding fails.
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            return '';
        }

        if ($params !== []) {
            $this->applyParams($params);
        }

        // unqualified so the namespace fallback resolves it, allowing substitution in tests
        if (!extension_loaded('imagick')) {
            throw new PPException('CCITTFaxDecode requires the Imagick PHP extension (ext-imagick)');
        }

        try {
            $tiff = $this->buildTiffHeader($data);

            $imagick = $this->newImagick();
            $imagick->readImageBlob($tiff);
            $imagick->setImageFormat('png');

            return $imagick->getImageBlob();
        } catch (\ImagickException $e) {
            throw new PPException('CCITTFaxDecode: Imagick failed to decode the stream: ' . $e->getMessage(), 0, $e);
        }
    }

    /**
     * Build a minimal TIFF container around the CCITT data.
     *
     * @param string $ccittData Raw CCITT-compressed data.
     *
     * @return string Binary TIFF data.
     */
    private function buildTiffHeader(string $ccittData): string
    {
        $ifdOffset = 8;
        // little-endian byte order, version, offset of the first IFD
        $tiff = 'II' . \pack('v', 42) . \pack('V', $ifdOffset);

        $ccittLength = \strlen($ccittData);
        // TIFF 6.0 requires an even value offset, so an odd-length payload is padded
        // before the RATIONAL values that follow it; StripByteCounts keeps the true length
        $padding = ($ccittLength % 2) === 0 ? '' : "\x00";
        $tagDefs = $this->buildTiffTagDefinitions($ccittLength);
        $tagDefs = $this->populateTiffOffsets($tagDefs, $ifdOffset, $ccittLength + \strlen($padding));
        $tags = $this->packTiffTags($tagDefs);

        // IFD: tag count, tags, offset of the next IFD (0 = end)
        $tiff .= \pack('v', \count($tags));
        $tiff .= \implode('', $tags);
        $tiff .= \pack('V', 0);

        $tiff .= $ccittData . $padding;

        // XResolution and YResolution, 72/1 dpi
        $tiff .= \pack('VV', 72, 1);
        $tiff .= \pack('VV', 72, 1);

        return $tiff;
    }

    /**
     * Build the TIFF tag definitions.
     *
     * @param int $ccittLength Raw CCITT payload length.
     *
     * @return TiffTagList
     */
    private function buildTiffTagDefinitions(int $ccittLength): array
    {
        $height = $this->rows > 0 ? $this->rows : (int) \ceil(($ccittLength * 8) / $this->columns);
        $compression = $this->group === 3 ? 3 : 4;
        // PDF BlackIs1 = false (default) means a 0 bit is black, that is TIFF
        // PhotometricInterpretation 1 (BlackIsZero); true means 0 is white (WhiteIsZero)
        $photometric = $this->blackIs1 ? 0 : 1;

        $tags = [
            // ImageWidth
            [
                'tag' => 256,
                'type' => $this->dimensionFieldType($this->columns),
                'count' => 1,
                'value' => $this->columns,
            ],
            // ImageLength (height)
            ['tag' => 257, 'type' => $this->dimensionFieldType($height), 'count' => 1, 'value' => $height],
            ['tag' => 258, 'type' => 3, 'count' => 1, 'value' => 1], // BitsPerSample
            ['tag' => 259, 'type' => 3, 'count' => 1, 'value' => $compression], // Compression: 3 = Group 3, 4 = Group 4
            ['tag' => 262, 'type' => 3, 'count' => 1, 'value' => $photometric], // PhotometricInterpretation
            ['tag' => 273, 'type' => 4, 'count' => 1, 'value' => 0], // StripOffsets, filled in later
            ['tag' => 279, 'type' => 4, 'count' => 1, 'value' => $ccittLength], // StripByteCounts
            ['tag' => 282, 'type' => 5, 'count' => 1, 'value' => 0], // XResolution, filled in later
            ['tag' => 283, 'type' => 5, 'count' => 1, 'value' => 0], // YResolution, filled in later
        ];

        if ($this->group === 3) {
            // T4Options (tag 292): bit 0 selects two-dimensional Group 3 coding (K > 0),
            // bit 2 marks lines padded with fill bits to a byte boundary (EncodedByteAlign);
            // tags must stay sorted by id, so 292 comes after 283
            $t4options = ($this->twoDimensional ? 0x1 : 0) | ($this->encodedByteAlign ? 0x4 : 0);
            $tags[] = ['tag' => 292, 'type' => 4, 'count' => 1, 'value' => $t4options];
        }

        return $tags;
    }

    /**
     * TIFF field type for an image dimension: SHORT (3) when the value fits,
     * LONG (4) otherwise.
     *
     * @param int $value Dimension in pixels.
     */
    private function dimensionFieldType(int $value): int
    {
        return $value > 0xFFFF ? 4 : 3;
    }

    /**
     * Fill in the tag values that are file offsets, known once the IFD size is.
     *
     * @param TiffTagList $tagDefs        Tag definitions.
     * @param int         $ifdOffset      Byte offset of the IFD.
     * @param int         $strippedLength CCITT payload length including its alignment padding.
     *
     * @return TiffTagList
     */
    private function populateTiffOffsets(array $tagDefs, int $ifdOffset, int $strippedLength): array
    {
        $ifdSize = 2 + (\count($tagDefs) * 12) + 4;
        $stripOffset = $ifdOffset + $ifdSize;
        $xResolutionOffset = $stripOffset + $strippedLength;
        $yResolutionOffset = $xResolutionOffset + 8;

        foreach ($tagDefs as $index => $tagDef) {
            if ($tagDef['tag'] === 273) {
                $tagDefs[$index]['value'] = $stripOffset;
                continue;
            }

            if ($tagDef['tag'] === 282) {
                $tagDefs[$index]['value'] = $xResolutionOffset;
                continue;
            }

            if ($tagDef['tag'] === 283) {
                $tagDefs[$index]['value'] = $yResolutionOffset;
            }
        }

        return $tagDefs;
    }

    /**
     * Convert the TIFF tag definitions to binary IFD entries.
     *
     * @param TiffTagList $tagDefs Tag definitions.
     *
     * @return array<int, string>
     */
    private function packTiffTags(array $tagDefs): array
    {
        $tags = [];
        foreach ($tagDefs as $tagDef) {
            $tags[] = \pack('vvVV', $tagDef['tag'], $tagDef['type'], $tagDef['count'], $tagDef['value']);
        }

        return $tags;
    }

    /**
     * Instantiate Imagick (overridable in tests).
     */
    protected function newImagick(): \Imagick
    {
        return new \Imagick();
    }
}
