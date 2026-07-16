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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter\Type;

use Com\Tecnick\Pdf\Filter\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Filter\Type\AsciiHex
 *
 * ASCIIHex
 * Decodes data encoded in an ASCII hexadecimal representation, reproducing the original binary data.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class AsciiHex implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Decode the data
     *
     * @param string $data   Data to decode.
     * @param array<string, mixed> $params Optional filter parameters.
     *
     * @return string Decoded data string.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            return '';
        }

        // all white-space characters shall be ignored
        $data = \preg_replace('/[\s]+/', '', $data);
        if ($data === null) {
            throw new PPException('invalid code');
        }

        // check for EOD character: GREATER-THAN SIGN (3Eh)
        $eod = \strpos($data, '>');
        if ($eod !== false) {
            // remove EOD and extra data (if any)
            $data = \substr($data, 0, $eod);
            $eod = true;
        }

        // get data length
        $data_length = \strlen($data);
        if (($data_length % 2) !== 0) {
            if (!$eod) {
                throw new PPException('invalid code');
            }

            // odd number of hexadecimal digits
            // EOD shall behave as if a 0 (zero) followed the last digit (PDF 32000-1:2008 §7.4.2)
            $data .= '0';
        }

        // check for invalid characters
        $invalid = \preg_match('/[^a-fA-F\d]/', $data);
        if ($invalid === false || $invalid > 0) {
            throw new PPException('invalid code');
        }

        // get one byte of binary data for each pair of ASCII hexadecimal digits
        return \pack('H*', $data);
    }
}
