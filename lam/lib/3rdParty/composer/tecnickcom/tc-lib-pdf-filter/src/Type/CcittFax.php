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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter\Type;

use Com\Tecnick\Pdf\Filter\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Filter\Type\CcittFax
 *
 * CCITTFaxDecode filter (PDF 32000-2008 §7.4.6).
 * Decompresses bi-level (1-bit-per-pixel) image data encoded with CCITT
 * facsimile standards (ITU-T T.4 Group 3 or T.6 Group 4).
 *
 * This implementation builds a TIFF container around the raw CCITT bitstream
 * and uses Imagick to decode it. TIFF natively supports CCITT Group 3 and
 * Group 4 compression, so this avoids having to implement the Huffman tables
 * and bit-stream reader in pure PHP.
 *
 * DecodeParms defaults (PDF 32000-1:2008, Table 11):
 * - K: 0 (= Group 3 1-D; negative = Group 4 / T.6; positive = Group 3 2-D)
 * - Columns: 1728 (standard fax width)
 * - Rows: 0 (use until end-of-data)
 * - BlackIs1: false (white-run first)
 *
 * Suggested PHP extension: ext-imagick
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
     * Constructor.
     *
     * @param array<mixed> $params DecodeParms dictionary (optional).
     *   - 'K' (int): Compression mode (PDF 32000-1:2008, Table 11).
     *     negative = pure 2-D (T.6 / Group 4); 0 = pure 1-D (T.4 / Group 3);
     *     positive = mixed 1-D/2-D (T.4 / Group 3 2-D).
     *   - 'Columns' (int): Image width (default 1728).
     *   - 'Rows' (int): Image height; 0 = unknown (default 0).
     *   - 'BlackIs1' (bool): Whether 1 bits are black (default false).
     */
    public function __construct(array $params = [])
    {
        // PDF K -> ITU-T group: K < 0 => Group 4 (T.6); K >= 0 => Group 3 (T.4).
        $kParam = (int) ($params['K'] ?? 0);
        $this->group = $kParam < 0 ? 4 : 3;
        $this->twoDimensional = $kParam > 0;
        $this->columns = max(1, (int) ($params['Columns'] ?? 1728));
        $this->rows = max(0, (int) ($params['Rows'] ?? 0));
        $this->blackIs1 = $this->toBool($params['BlackIs1'] ?? null);
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
     * Builds a TIFF container around the CCITT bitstream and uses Imagick
     * to decode it.
     *
     * @param string               $data   Raw CCITT-compressed image data.
     * @param array<string, mixed> $params Optional filter parameters (unused; params set via constructor).
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
            throw new PPException('CCITTFaxDecode: Imagick failed to decode the stream: ' . $e->getMessage());
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
        // TIFF little-endian header
        $ifdOffset = 8;
        $tiff = 'II' . pack('v', 42) . pack('V', $ifdOffset); // Byte order + version + offset to first IFD

        $ccittLength = strlen($ccittData);
        $tagDefs = $this->buildTiffTagDefinitions($ccittLength);
        $tagDefs = $this->populateTiffOffsets($tagDefs, $ifdOffset, $ccittLength);
        $tags = $this->packTiffTags($tagDefs);

        // Write IFD
        $tiff .= pack('v', count($tags)); // Number of tags
        $tiff .= implode('', $tags);
        $tiff .= pack('V', 0); // Next IFD offset (0 = end)

        // Append image data
        $tiff .= $ccittData;

        // Append resolution (72 dpi)
        $tiff .= pack('VV', 72, 1); // XResolution: 72/1
        $tiff .= pack('VV', 72, 1); // YResolution: 72/1

        return $tiff;
    }

    /**
     * Build static TIFF tag definitions.
     *
     * @param int $ccittLength Raw CCITT payload length.
     *
     * @return TiffTagList
     */
    private function buildTiffTagDefinitions(int $ccittLength): array
    {
        $height = $this->rows > 0 ? $this->rows : (int) ceil(($ccittLength * 8) / $this->columns);
        $compression = $this->group === 3 ? 3 : 4;
        $photometric = $this->blackIs1 ? 1 : 0;

        $tags = [
            ['tag' => 256, 'type' => 3, 'count' => 1, 'value' => $this->columns], // ImageWidth
            ['tag' => 257, 'type' => 3, 'count' => 1, 'value' => $height], // ImageLength / height
            ['tag' => 258, 'type' => 3, 'count' => 1, 'value' => 1], // BitsPerSample = 1
            ['tag' => 259, 'type' => 3, 'count' => 1, 'value' => $compression], // Compression: 4 = Group 4, 3 = Group 3
            ['tag' => 262, 'type' => 3, 'count' => 1, 'value' => $photometric], // PhotometricInterpretation : 0 = white, 1 = black
            ['tag' => 273, 'type' => 4, 'count' => 1, 'value' => 0], // StripOffsets : points to image data
            ['tag' => 279, 'type' => 4, 'count' => 1, 'value' => $ccittLength], // StripByteCounts
            ['tag' => 282, 'type' => 5, 'count' => 1, 'value' => 0], // XResolution
            ['tag' => 283, 'type' => 5, 'count' => 1, 'value' => 0], // YResolution
        ];

        if ($this->group === 3) {
            // T4Options (tag 292) bit 0 selects two-dimensional Group 3 coding (K > 0).
            // Tags must stay sorted by id, so 292 is appended after 283.
            $tags[] = ['tag' => 292, 'type' => 4, 'count' => 1, 'value' => $this->twoDimensional ? 1 : 0];
        }

        return $tags;
    }

    /**
     * Populate offset-based values after final IFD size is known.
     *
     * @param TiffTagList $tagDefs
     * @param int         $ifdOffset Byte offset of the IFD.
     * @param int         $ccittLength Raw CCITT payload length.
     *
     * @return TiffTagList
     */
    private function populateTiffOffsets(array $tagDefs, int $ifdOffset, int $ccittLength): array
    {
        $ifdSize = 2 + (count($tagDefs) * 12) + 4;
        $stripOffset = $ifdOffset + $ifdSize;
        $xResolutionOffset = $stripOffset + $ccittLength;
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
     * Convert TIFF tag definitions to binary IFD entries.
     *
     * @param TiffTagList $tagDefs
     *
     * @return array<int, string>
     */
    private function packTiffTags(array $tagDefs): array
    {
        $tags = [];
        foreach ($tagDefs as $tagDef) {
            $tags[] = pack('vvVV', $tagDef['tag'], $tagDef['type'], $tagDef['count'], $tagDef['value']);
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
