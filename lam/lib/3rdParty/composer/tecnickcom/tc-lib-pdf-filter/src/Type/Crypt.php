<?php

declare(strict_types=1);

/**
 * Crypt.php
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
 * Com\Tecnick\Pdf\Filter\Type\Crypt
 *
 * Crypt filter (PDF 32000-1:2008 §7.4.10).
 * When DecodeParms/Name is absent, Identity or None, the stream is passed
 * through unchanged (PDF 32000-1:2008 §7.6.5). Any other name identifies a
 * crypt filter of the document /CF dictionary, which this library does not
 * apply: decoding throws instead.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class Crypt implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Decode the data.
     *
     * @param string               $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary.
     *   - 'Name' (string): crypt filter name; only Identity and None are supported.
     *
     * @return string Decoded data.
     *
     * @throws PPException if a crypt filter other than Identity or None is requested.
     */
    public function decode(string $data, array $params = []): string
    {
        if (!\array_key_exists('Name', $params)) {
            return $data;
        }

        if (!\is_string($params['Name'])) {
            throw new PPException('unsupported Crypt filter name: ' . \get_debug_type($params['Name']));
        }

        if (\in_array($this->nameObject($params['Name']), ['Identity', 'None'], true)) {
            return $data;
        }

        throw new PPException('unsupported Crypt filter name: ' . $params['Name']);
    }

    /**
     * Strip the leading solidus of a PDF name object, as FilterType::fromLoose
     * does for filter names.
     *
     * @param string $value Crypt filter name.
     */
    private function nameObject(string $value): string
    {
        return \str_starts_with($value, '/') ? \substr($value, 1) : $value;
    }
}
