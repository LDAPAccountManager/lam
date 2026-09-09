<?php

declare(strict_types=1);

/**
 * Dct.php
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

/**
 * Com\Tecnick\Pdf\Filter\Type\Dct
 *
 * DCTDecode filter (PDF 32000-1:2008 §7.4.8).
 * A DCT stream is a self-contained JFIF/JPEG byte sequence and is passed
 * through unchanged; JPEG decompression is left to the image-rendering layer.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class Dct implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * Return the data unchanged.
     *
     * @param string               $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary.
     *
     * @return string Decoded data.
     */
    public function decode(string $data, array $params = []): string
    {
        return $data;
    }
}
