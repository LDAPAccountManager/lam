<?php

declare(strict_types=1);

/**
 * AsciiHex.php
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
 * Com\Tecnick\Pdf\Filter\Type\AsciiHex
 *
 * ASCIIHexDecode filter (PDF 32000-1:2008 §7.4.2).
 * Decodes data written as pairs of ASCII hexadecimal digits.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class AsciiHex implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Decode the data.
     *
     * @param string               $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary.
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

        // all white-space characters shall be ignored (PDF 32000-1:2008 Table 1 also lists NUL)
        $data = \preg_replace('/[\x00\s]+/', '', $data);
        if ($data === null) {
            throw new PPException('invalid code: white-space removal failed');
        }

        // EOD marker: GREATER-THAN SIGN (3Eh); it and any data after it are dropped
        $eod = \strpos($data, '>');
        $hasEod = $eod !== false;
        if ($eod !== false) {
            $data = \substr($data, 0, $eod);
        }

        $data_length = \strlen($data);
        if (($data_length % 2) !== 0) {
            if (!$hasEod) {
                throw new PPException('invalid code: odd number of hexadecimal digits without EOD');
            }

            // EOD shall behave as if a 0 (zero) followed the last digit (PDF 32000-1:2008 §7.4.2)
            $data .= '0';
        }

        $invalid = \preg_match('/[^a-fA-F\d]/', $data);
        if ($invalid === false || $invalid > 0) {
            throw new PPException('invalid code: character outside the hexadecimal alphabet');
        }

        // one byte per pair of hexadecimal digits
        return \pack('H*', $data);
    }
}
