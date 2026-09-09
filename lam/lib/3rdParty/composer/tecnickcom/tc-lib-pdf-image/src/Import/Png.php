<?php

declare(strict_types=1);

/**
 * Png.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-pdf-image software library.
 */

namespace Com\Tecnick\Pdf\Image\Import;

use Com\Tecnick\File\Byte;
use Com\Tecnick\Pdf\Image\Exception as ImageException;

/**
 * Com\Tecnick\Pdf\Image\Import\Png
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 */
class Png implements ImageImportInterface
{
    /**
     * Minimum number of bytes of a PNG data stream:
     * signature (8) + IHDR length and type (8) + IHDR payload (13) + CRC (4).
     */
    private const MIN_LENGTH = 33;

    /**
     * Number of bytes of a chunk header: length (4) + type (4).
     */
    private const CHUNK_HEADER_LENGTH = 8;

    /**
     * Maximum accepted size of a decompressed ICC profile, in bytes.
     */
    private const MAX_ICC_LENGTH = 16_777_216;

    /**
     * Minimum size of an ICC profile: the 128-byte header plus the tag count.
     */
    private const MIN_ICC_LENGTH = 132;

    /**
     * Number of entries of a full colour palette (256 RGB triplets).
     */
    private const MAX_PALETTE_LENGTH = 768;

    /**
     * Bit depths allowed for each PNG colour type (PNG specification 11.2.2).
     *
     * @var array<int, array<int, int>>
     */
    private const BITDEPTHS = [
        0 => [1, 2, 4, 8, 16], // greyscale
        2 => [8, 16], // truecolour
        3 => [1, 2, 4, 8], // indexed
        4 => [8, 16], // greyscale + alpha
        6 => [8, 16], // truecolour + alpha
    ];

    /**
     * Extract data from a PNG image.
     *
     * @param ImageBaseData $data Image raw data.
     *
     * @return ImageBaseData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image is invalid.
     * @throws \RangeException If the byte offset is out of range.
     */
    public function getData(array $data): array
    {
        $data['filter'] = 'FlateDecode';

        // fields derived from the stream are reset, so that parsing the
        // re-encoded form of an image does not inherit the previous pass;
        // the ICC profile is kept, as the re-encode drops it
        $data['data'] = '';
        $data['pal'] = '';
        $data['parms'] = '';
        $data['recode'] = false;
        $data['splitalpha'] = false;
        $data['trns'] = [];

        $byte = new Byte($data['raw']);

        $offset = 0;
        // check signature
        if (\substr($data['raw'], $offset, 8) !== \chr(137) . 'PNG' . \chr(13) . \chr(10) . \chr(26) . \chr(10)) {
            // @codeCoverageIgnoreStart
            throw new ImageException('Not a PNG image');

            // @codeCoverageIgnoreEnd
        }

        if ($byte->getLength() < self::MIN_LENGTH) {
            throw new ImageException('Invalid PNG image: truncated header');
        }

        $offset += 8;
        $offset += 4;

        $data = $this->getIhdrChunk($byte, $data, $offset);

        $offset += 3;

        // check compression, filter and interlacing settings
        $unsupported =
            $byte->getByte($offset - 3) !== 0 || $byte->getByte($offset - 2) !== 0 || $byte->getByte($offset - 1) !== 0;

        $offset += 4;

        if ($unsupported) {
            if ($data['recoded']) {
                // this image has been already re-encoded
                // @codeCoverageIgnoreStart
                throw new ImageException('Unsupported feature');

                // @codeCoverageIgnoreEnd
            }

            // re-encode PNG, keeping the profile that the re-encode would drop
            $data = $this->getIccpData($byte, $data, $offset);
            $data['recode'] = true;
            return $data;
        }

        if (\str_contains($data['colspace'], '+Alpha')) {
            // alpha channel: split images (plain + alpha)
            $data = $this->getIccpData($byte, $data, $offset);
            $data['splitalpha'] = true;
            $data['colspace'] = \substr($data['colspace'], 0, -6);
            return $data;
        }

        $data['parms'] =
            '/DecodeParms << /Predictor 15 /Colors '
            . $data['channels']
            . ' /BitsPerComponent '
            . $data['bits']
            . ' /Columns '
            . $data['width']
            . ' >>';

        return $this->getChunks($byte, $data, $offset);
    }

    /**
     * Scan the chunks preceding the image data for an ICC profile.
     *
     * Used on the paths that hand the samples over to GD (alpha split,
     * unsupported compression, filter or interlacing) and return before the
     * full chunk walk. No image data is read.
     *
     * @param Byte          $byte   Byte class object wrapping the raw image data.
     * @param ImageBaseData $data   Image raw data.
     * @param int           $offset Offset of the first chunk after IHDR.
     *
     * @return ImageBaseData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the profile cannot be read.
     * @throws \RangeException If the byte offset is out of range.
     */
    private function getIccpData(Byte $byte, array $data, int $offset): array
    {
        $length = $byte->getLength();

        while (($offset + self::CHUNK_HEADER_LENGTH) <= $length) {
            $len = $byte->getULong($offset);
            $offset += 4;
            $type = \substr($data['raw'], $offset, 4);
            $offset += 4;

            if ($type === 'iCCP') {
                return $this->getIccpChunk($byte, $data, $offset, $len);
            }

            if ($type === 'IDAT' || $type === 'IEND') {
                break; // the profile always precedes the image data
            }

            $offset += $len;
            $offset += 4;
        }

        return $data;
    }

    /**
     * Extract the IHDR chunk data.
     *
     * The header chunk (IHDR) contains basic information about the image data and must appear as the first chunk,
     * and there must only be one header chunk in a PNG data stream.
     *
     * @param Byte  $byte   Byte class object wrapping the raw image data.
     * @param ImageBaseData $data   Image raw data.
     * @param int   $offset Current byte offset.
     *
     * @return ImageBaseData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image is invalid.
     * @throws \RangeException If the byte offset is out of range.
     */
    protected function getIhdrChunk(Byte $byte, array $data, int &$offset): array
    {
        if (\substr($data['raw'], $offset, 4) !== 'IHDR') {
            // @codeCoverageIgnoreStart
            throw new ImageException('Invalid PNG image');

            // @codeCoverageIgnoreEnd
        }

        $offset += 4;
        $data['width'] = $byte->getULong($offset);
        $offset += 4;
        $data['height'] = $byte->getULong($offset);
        $offset += 4;
        $data['bits'] = $byte->getByte($offset);
        ++$offset;
        $chc = $byte->getByte($offset); // channels code
        ++$offset;
        $data['channels'] = $chc === 2 ? 3 : 1;
        $chcmap = [
            0 => 'DeviceGray',
            2 => 'DeviceRGB',
            3 => 'Indexed',
            4 => 'DeviceGray+Alpha',
            6 => 'DeviceRGB+Alpha',
        ];
        if (isset($chcmap[$chc], self::BITDEPTHS[$chc])) {
            $data['colspace'] = $chcmap[$chc];

            // the depth is checked here against the colour type, as
            // getimagesizefromstring() does not check the pair
            if (!\in_array($data['bits'], self::BITDEPTHS[$chc], true)) {
                throw new ImageException('Invalid PNG image: unsupported bit depth for the color mode');
            }
        } else {
            throw new ImageException('Unknown color mode');
        }

        return $data;
    }

    /**
     * Extract chunks data from a PNG image.
     *
     * @param Byte  $byte   Byte class object wrapping the raw image data.
     * @param ImageBaseData $data   Image raw data.
     * @param int   $offset Current byte offset.
     *
     * @return ImageBaseData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the image is invalid.
     * @throws \RangeException If the byte offset is out of range.
     */
    protected function getChunks(Byte $byte, array $data, int $offset): array
    {
        $length = $byte->getLength();

        // the walk ends on the IEND chunk; a stream with too few bytes left to
        // hold another chunk header is truncated
        while (($offset + self::CHUNK_HEADER_LENGTH) <= $length) {
            $len = $byte->getULong($offset);
            $offset += 4;
            $type = \substr($data['raw'], $offset, 4);
            $offset += 4;
            if ($type === 'PLTE') {
                $data = $this->getPlteChunk($data, $offset, $len);
            } elseif ($type === 'tRNS') {
                $data = $this->getTrnsChunk($data, $offset, $len);
            } elseif ($type === 'IDAT') {
                $data = $this->getIdatChunk($data, $offset, $len);
            } elseif ($type === 'iCCP') {
                $data = $this->getIccpChunk($byte, $data, $offset, $len);
            } elseif ($type === 'IEND') {
                // The image trailer chunk (IEND) must be the final chunk
                // and marks the end of the PNG file or data stream.
                if ($data['data'] === '') {
                    throw new ImageException('Invalid PNG image: missing image data');
                }

                if ($data['colspace'] === 'Indexed' && $data['pal'] === '') {
                    // @codeCoverageIgnoreStart
                    throw new ImageException('The color palette is missing');

                    // @codeCoverageIgnoreEnd
                }

                return $data;
            } else {
                $offset += $len;
                $offset += 4;
            }
        }

        throw new ImageException('Invalid PNG image: missing IEND chunk');
    }

    /**
     * Extract the PLTE chunk data.
     *
     * The palette chunk (PLTE) stores the colormap data associated with the image data.
     * This chunk is present only if the image data uses a color palette and must appear before the image data chunk.
     *
     * @param ImageBaseData $data   Image raw data.
     * @param int   $offset Current byte offset.
     * @param int   $len    Number of bytes in this chunk.
     *
     * @return ImageBaseData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the palette is malformed.
     */
    protected function getPlteChunk(array $data, int &$offset, int $len): array
    {
        $pal = \substr($data['raw'], $offset, $len);
        $offset += $len;
        $offset += 4;

        // the palette holds at most 256 RGB triplets
        $size = \strlen($pal);
        if ($size === 0 || ($size % 3) !== 0 || $size > self::MAX_PALETTE_LENGTH) {
            throw new ImageException('Invalid PNG image: malformed color palette');
        }

        $data['pal'] = $pal;
        return $data;
    }

    /**
     * Extract the tRNS chunk data.
     *
     * @param ImageBaseData $data   Image raw data.
     * @param int   $offset Current byte offset.
     * @param int   $len    Number of bytes in this chunk.
     *
     * @return ImageBaseData Image raw data array.
     */
    protected function getTrnsChunk(array $data, int &$offset, int $len): array
    {
        // read transparency info
        $trns = \substr($data['raw'], $offset, $len);
        $offset += $len;
        $offset += 4;

        // assigned rather than appended: a further tRNS chunk replaces the
        // values of the previous one
        $data['trns'] = $this->getTrnsValues($data, $trns);

        // a palette entry that is neither opaque nor fully transparent cannot
        // be expressed as a /Mask range, so the image is re-encoded and split
        // into a plain image and a soft mask
        if ($data['colspace'] === 'Indexed' && $this->hasPartialAlpha($data['trns'])) {
            $data['recode'] = true;
        }

        return $data;
    }

    /**
     * Check whether an indexed transparency list holds a partially opaque entry.
     *
     * @param array<int, int> $trns Transparency values.
     */
    private function hasPartialAlpha(array $trns): bool
    {
        foreach ($trns as $val) {
            if ($val > 0 && $val < 255) {
                return true;
            }
        }

        return false;
    }

    /**
     * Decode the tRNS chunk payload for the image colour space.
     *
     * The greyscale and truecolour spaces carry one 2-byte sample per channel;
     * the indexed one carries a single alpha byte per palette entry. A payload
     * too short for the colour space is discarded.
     *
     * @param ImageBaseData $data Image raw data.
     * @param string        $trns tRNS chunk payload.
     *
     * @return array<int, int> Transparency values.
     */
    private function getTrnsValues(array $data, string $trns): array
    {
        if ($data['colspace'] === 'Indexed') {
            // never more entries than the palette holds
            $entries = \intdiv(\strlen($data['pal']), 3);
            return \array_map(static fn(string $chr): int => \ord($chr), \str_split(\substr($trns, 0, $entries)));
        }

        $samples = $data['colspace'] === 'DeviceRGB' ? 3 : 1;
        if (\strlen($trns) < ($samples * 2)) {
            return [];
        }

        $values = [];
        for ($idx = 0; $idx < $samples; ++$idx) {
            $pos = $idx * 2;
            // samples are 2 bytes big-endian, but only span both bytes at depth 16
            $values[] = $data['bits'] === 16 ? (\ord($trns[$pos]) << 8) | \ord($trns[$pos + 1]) : \ord($trns[$pos + 1]);
        }

        return $values;
    }

    /**
     * Extract the IDAT chunk data.
     *
     * The image data chunk (IDAT) stores the actual image data,
     * and multiple image data chunks may occur in a data stream and must be stored in contiguous order.
     *
     * @param ImageBaseData $data   Image raw data.
     * @param int   $offset Current byte offset.
     * @param int   $len    Number of bytes in this chunk.
     *
     * @return ImageBaseData Image raw data array.
     */
    protected function getIdatChunk(array $data, int &$offset, int $len): array
    {
        $data['data'] .= \substr($data['raw'], $offset, $len);
        $offset += $len;
        $offset += 4;
        return $data;
    }

    /**
     * Extract the iCCP chunk data.
     *
     * @param Byte  $byte   Byte class object.
     * @param ImageBaseData $data   Image raw data.
     * @param int   $offset Current byte offset.
     * @param int   $len    Number of bytes in this chunk.
     *
     * @return ImageBaseData Image raw data array.
     *
     * @throws \Com\Tecnick\Pdf\Image\Exception If the chunk is truncated or the filter method is unknown.
     * @throws \RangeException If attempting to read beyond available bytes.
     */
    protected function getIccpChunk(Byte $byte, array $data, int &$offset, int $len): array
    {
        // skip the profile name (up to 79 bytes) and its NUL terminator
        $pos = 0;
        $end = \min($byte->getLength(), $offset + 80);
        while ($offset < $end && $byte->getByte($offset++) !== 0) {
            ++$pos;
        }

        if ($offset >= $byte->getLength()) {
            throw new ImageException('Invalid PNG image: truncated iCCP chunk');
        }

        // get compression method
        if ($byte->getByte($offset++) !== 0) {
            // @codeCoverageIgnoreStart
            throw new ImageException('Unknown compression method');

            // @codeCoverageIgnoreEnd
        }

        // read ICC Color Profile, bounding the expansion of the payload
        $len -= $pos + 2;
        if ($len <= 0) {
            // a chunk shorter than its own name leaves no payload
            throw new ImageException('Invalid PNG image: malformed iCCP chunk');
        }

        $icc = \gzuncompress(\substr($data['raw'], $offset, $len), self::MAX_ICC_LENGTH);
        if ($icc === false) {
            throw new ImageException('Error while decompressing ICC profile');
        }

        // a profile that does not describe this image is dropped
        if ($this->isValidIccProfile($icc, $data['colspace'])) {
            $data['icc'] = $icc;
        }

        $offset += $len;
        $offset += 4;
        return $data;
    }

    /**
     * Check that an ICC profile is well formed and describes the image colour space.
     *
     * The profile header carries the 'acsp' signature at offset 36 and the
     * data colour space at offset 16, which must match the colour type the
     * image samples are written with.
     *
     * @param string $icc      Decompressed ICC profile.
     * @param string $colspace Image color space.
     */
    private function isValidIccProfile(string $icc, string $colspace): bool
    {
        if (\strlen($icc) < self::MIN_ICC_LENGTH || \substr($icc, 36, 4) !== 'acsp') {
            return false;
        }

        // an indexed image profile describes its RGB palette entries
        return \substr($icc, 16, 4) === (\str_starts_with($colspace, 'DeviceGray') ? 'GRAY' : 'RGB ');
    }
}
