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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter\Type;

/**
 * Com\Tecnick\Pdf\Filter\Type\RunLength
 *
 * RunLengthDecode
 * Decompresses data encoded using a byte-oriented run-length encoding algorithm,
 * reproducing the original text or binary data.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class RunLength implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Decode the data
     *
     * @param string $data   Data to decode.
     * @param array<string, mixed> $params Optional filter parameters.
     *
     * @return string Decoded data string.
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            return '';
        }

        // initialize string to return
        $decoded = '';
        // data length
        $data_length = \strlen($data);
        $idx = 0;
        while ($idx < $data_length) {
            // get current byte value
            $byte = \ord($data[$idx]);
            if ($byte === 128) {
                // a length value of 128 denote EOD
                break;
            }

            if ($byte < 128) {
                // if the length byte is in the range 0 to 127
                // the following length + 1 (1 to 128) bytes shall be copied literally during decompression
                $decoded .= \substr($data, $idx + 1, $byte + 1);
                // move to next block
                $idx += $byte + 2;
                continue;
            }

            // if length is in the range 129 to 255,
            // the following single byte shall be copied 257 - length (2 to 128) times during decompression
            if (($idx + 1) >= $data_length) {
                // truncated run: no byte follows the length marker
                break;
            }

            $decoded .= \str_repeat($data[$idx + 1], 257 - $byte);
            // move to next block
            $idx += 2;
        }

        return $decoded;
    }
}
