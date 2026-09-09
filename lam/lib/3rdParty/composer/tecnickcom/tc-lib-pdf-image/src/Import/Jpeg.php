<?php

declare(strict_types=1);

/**
 * Jpeg.php
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

/**
 * Com\Tecnick\Pdf\Image\Import\Jpeg
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
class Jpeg implements ImageImportInterface
{
    /**
     * Identifier that opens the payload of an ICC profile APP2 segment.
     */
    private const ICC_TAG = "ICC_PROFILE\x00";

    /**
     * Number of bytes of the ICC_TAG identifier.
     */
    private const ICC_TAG_LENGTH = 12;

    /**
     * Minimum size of an ICC profile: the 128-byte header plus the tag count.
     */
    private const MIN_ICC_LENGTH = 132;

    /**
     * ICC data colour space signature expected for each image colour space.
     *
     * @var array<string, string>
     */
    private const ICC_COLSPACE = [
        'DeviceGray' => 'GRAY',
        'DeviceRGB' => 'RGB ',
        'DeviceCMYK' => 'CMYK',
    ];

    /**
     * Extract data from a JPEG image.
     *
     * @param ImageBaseData $data Image raw data.
     *
     * @return ImageBaseData Image raw data array.
     */
    public function getData(array $data): array
    {
        $data['filter'] = 'DCTDecode';
        $data['data'] = $data['raw'];

        $icc = $this->getIccProfile($data['raw'], $data['colspace']);
        if ($icc !== '') {
            $data['icc'] = $icc;
        }

        return $data;
    }

    /**
     * Extract the ICC profile carried by the APP2 segments of a JPEG image.
     *
     * The segments are located by walking the marker chain from SOI, so that
     * bytes resembling an ICC_PROFILE header elsewhere in the file are not
     * picked up. The profile is only returned when every announced chunk is
     * present and it describes the image colour space.
     *
     * @param string $raw      Raw image data.
     * @param string $colspace Image color space.
     *
     * @return string ICC profile, or an empty string if there is no usable one.
     */
    private function getIccProfile(string $raw, string $colspace): string
    {
        $chunks = [];
        $total = 0;

        foreach ($this->getApp2Segments($raw) as $payload) {
            if (\strlen($payload) < (self::ICC_TAG_LENGTH + 2) || !\str_starts_with($payload, self::ICC_TAG)) {
                continue;
            }

            $total = \ord($payload[self::ICC_TAG_LENGTH + 1]);
            $chunks[\ord($payload[self::ICC_TAG_LENGTH])] = \substr($payload, self::ICC_TAG_LENGTH + 2);
        }

        // the chunks must be exactly the announced sequence 1..$total
        \ksort($chunks);
        if ($total === 0 || \array_keys($chunks) !== \range(1, $total)) {
            return '';
        }

        $icc = \implode('', $chunks);
        if (\strlen($icc) < self::MIN_ICC_LENGTH || \substr($icc, 36, 4) !== 'acsp') {
            return '';
        }

        if (\substr($icc, 16, 4) !== (self::ICC_COLSPACE[$colspace] ?? '')) {
            return '';
        }

        return $icc;
    }

    /**
     * Walk the JPEG marker chain and return the payload of every APP2 segment.
     *
     * The walk stops at the start of scan (SOS) or at the end of image (EOI).
     *
     * @param string $raw Raw image data.
     *
     * @return array<int, string> APP2 segment payloads.
     */
    private function getApp2Segments(string $raw): array
    {
        $length = \strlen($raw);
        if (\substr($raw, 0, 2) !== "\xff\xd8") {
            return [];
        }

        $segments = [];
        $offset = 2;

        while (($offset + 4) <= $length) {
            if ($raw[$offset] !== "\xff") {
                break; // not on a marker boundary
            }

            $marker = \ord($raw[$offset + 1]);

            if ($marker === 0xff) {
                ++$offset; // fill byte
                continue;
            }

            if ($marker === 0x01 || $marker >= 0xd0 && $marker <= 0xd8) {
                $offset += 2; // standalone marker, no payload
                continue;
            }

            if ($marker === 0xd9 || $marker === 0xda) {
                break; // end of image, or start of the entropy-coded scan
            }

            $seglen = (\ord($raw[$offset + 2]) << 8) | \ord($raw[$offset + 3]);
            if ($seglen < 2) {
                break; // malformed length: the walk cannot advance safely
            }

            if ($marker === 0xe2) {
                $segments[] = \substr($raw, $offset + 4, $seglen - 2);
            }

            $offset += 2 + $seglen;
        }

        return $segments;
    }
}
