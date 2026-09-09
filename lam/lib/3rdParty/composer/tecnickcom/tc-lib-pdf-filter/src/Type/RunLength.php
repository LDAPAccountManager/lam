<?php

declare(strict_types=1);

/**
 * RunLength.php
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
 * Com\Tecnick\Pdf\Filter\Type\RunLength
 *
 * RunLengthDecode filter (PDF 32000-1:2008 §7.4.5).
 * Decompresses byte-oriented run-length encoded data.
 *
 * A truncated stream is not an error: a missing EOD marker (128), a run marker
 * with no byte after it, and a literal run reaching past the end of the data
 * all end the decode and return the bytes recovered so far.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class RunLength implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Decode the data.
     *
     * @param string               $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary.
     *   - 'MaxOutputSize' (int): decoded-size cap in bytes; 0 (default) = unlimited.
     *
     * @return string Decoded data.
     *
     * @throws PPException
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            return '';
        }

        $maxOutputSize = \max(0, (int) ($params['MaxOutputSize'] ?? 0));

        $decoded = '';
        $data_length = \strlen($data);
        $idx = 0;
        while ($idx < $data_length) {
            $byte = \ord($data[$idx]);
            if ($byte === 128) {
                // a length of 128 denotes EOD
                break;
            }

            if ($byte < 128) {
                // a length of 0 to 127 copies the following length + 1 (1 to 128) bytes literally
                $decoded .= \substr($data, $idx + 1, $byte + 1);
                $this->guardOutputSize($decoded, $maxOutputSize);
                $idx += $byte + 2;
                continue;
            }

            // a length of 129 to 255 repeats the following byte 257 - length (2 to 128) times
            if (($idx + 1) >= $data_length) {
                // truncated run: no byte follows the length marker
                break;
            }

            $decoded .= \str_repeat($data[$idx + 1], 257 - $byte);
            $this->guardOutputSize($decoded, $maxOutputSize);
            $idx += 2;
        }

        return $decoded;
    }

    /**
     * Enforce the optional decoded-size cap.
     *
     * @param string $decoded       Data decoded so far.
     * @param int    $maxOutputSize Cap in bytes; 0 = unlimited.
     *
     * @throws PPException
     */
    private function guardOutputSize(string $decoded, int $maxOutputSize): void
    {
        if ($maxOutputSize > 0 && \strlen($decoded) > $maxOutputSize) {
            throw new PPException('decoded data exceeds MaxOutputSize of ' . $maxOutputSize . ' bytes');
        }
    }
}
