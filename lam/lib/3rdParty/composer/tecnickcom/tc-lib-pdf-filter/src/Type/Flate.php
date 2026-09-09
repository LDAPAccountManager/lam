<?php

declare(strict_types=1);

/**
 * Flate.php
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
use Com\Tecnick\Pdf\Filter\Predictor;

/**
 * Com\Tecnick\Pdf\Filter\Type\Flate
 *
 * FlateDecode filter (PDF 32000-1:2008 §7.4.4).
 * Decompresses zlib/deflate data and reverses the optional predictor.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class Flate implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Decode the data.
     *
     * @param string               $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary.
     *   - 'MaxOutputSize' (int): decoded-size cap in bytes; 0 (default) = unlimited.
     *   - 'Predictor', 'Colors', 'BitsPerComponent', 'Columns': see Predictor.
     *
     * @return string Decoded data.
     *
     * @throws PPException
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            // through the predictor, which validates the DecodeParms
            return (new Predictor())->apply('', $params);
        }

        $maxOutputSize = \max(0, (int) ($params['MaxOutputSize'] ?? 0));

        $handler = static fn(): bool => true;

        \set_error_handler($handler);
        try {
            $decoded = \gzuncompress($data, $maxOutputSize);

            if ($decoded === false) {
                // raw deflate payload, without the zlib wrapper
                $decoded = \gzinflate($data, $maxOutputSize);
            }

            if ($decoded === false && \strlen($data) > 2) {
                // zlib header with an invalid checksum
                $decoded = \gzinflate(\substr($data, 2), $maxOutputSize);
            }

            if ($decoded === false) {
                // gzip-wrapped payload
                $decoded = \gzdecode($data, $maxOutputSize);
            }
        } finally {
            \restore_error_handler();
        }

        if ($decoded === false) {
            // zlib reports a capped decode and a corrupt stream the same way
            throw new PPException(
                $maxOutputSize > 0
                    ? 'invalid code, or decoded data exceeds MaxOutputSize of ' . $maxOutputSize . ' bytes'
                    : 'invalid code',
            );
        }

        // zlib bounds its allocation rounds rather than the exact byte count
        if ($maxOutputSize > 0 && \strlen($decoded) > $maxOutputSize) {
            throw new PPException('decoded data exceeds MaxOutputSize of ' . $maxOutputSize . ' bytes');
        }

        return (new Predictor())->apply($decoded, $params);
    }
}
