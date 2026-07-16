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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-pdf-image software library.
 */

namespace Com\Tecnick\Pdf\Image\Import;

use Com\Tecnick\File\Byte;

/**
 * Com\Tecnick\Pdf\Image\Import\Jpeg
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * @phpstan-import-type ImageBaseData from \Com\Tecnick\Pdf\Image\Import
 */
class Jpeg implements ImageImportInterface
{
    /**
     * Extract data from a JPEG image.
     *
     * @param ImageBaseData $data Image raw data.
     *
     * @return ImageBaseData Image raw data array.
     *
     * @throws \RangeException If the byte offset is out of range.
     */
    public function getData(array $data): array
    {
        $data['filter'] = 'DCTDecode';
        $data['data'] = $data['raw'];
        $byte = new Byte($data['raw']);
        // extract embedded ICC profile (if any)
        $icc = [];
        $offset = 0;
        // Length in bytes of the "ICC_PROFILE" marker string, used as the
        // minimum forward step so a malformed segment can never stall the scan.
        $markerlen = 11;
        while (($pos = \strpos($data['raw'], 'ICC_PROFILE', $offset)) !== false) {
            if ($pos < 2) {
                // Not enough bytes before the marker to read the APP2 length.
                $offset = $pos + $markerlen;
                continue;
            }

            // get ICC sequence length
            $length = $byte->getUShort($pos - 2) - 16;
            if ($length <= 0) {
                // Malformed or spurious marker: skip past it so $offset always
                // advances and the loop cannot get stuck re-finding this match.
                $offset = $pos + $markerlen;
                continue;
            }

            // marker sequence number
            $msn = \max(1, \ord($data['raw'][$pos + 12] ?? "\x00"));
            // number of markers (total of APP2 used)
            //$nom = \max(1, \ord($data['raw'][($pos + 13)]));
            // get sequence segment
            $icc[$msn - 1] = \substr($data['raw'], $pos + 14, $length);
            // move forward to next sequence
            $offset = $pos + 14 + $length;
        }

        // order and compact ICC segments
        if ($icc !== []) {
            \ksort($icc);
            $icc = \implode('', $icc);
            if (\substr($icc, 36, 4) === 'acsp') {
                // valid ICC profile
                $data['icc'] = $icc;
            }
        }

        return $data;
    }
}
