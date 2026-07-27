<?php

declare(strict_types=1);

/**
 * Filter.php
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

namespace Com\Tecnick\Pdf\Filter;

/**
 * Com\Tecnick\Pdf\Filter\Filter
 *
 * PHP class for decoding common PDF filters (PDF 32000-2008 - 7.4 Filters)
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class Filter
{
    /**
     * Decode data using the specified filter type.
     *
     * @param string|FilterType    $filter Filter name or FilterType enum case.
     * @param string              $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary for the filter.
     *
     * @return string  Decoded data string.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function decode(string|FilterType $filter, string $data, array $params = []): string
    {
        if (\is_string($filter)) {
            if ($filter === '') {
                return $data;
            }

            $filter = FilterType::fromLoose($filter);
        }

        $obj = match ($filter) {
            FilterType::AsciiHexDecode => new Type\AsciiHex(),
            FilterType::Ascii85Decode => new Type\AsciiEightFive(),
            FilterType::LzwDecode => new Type\Lzw(),
            FilterType::FlateDecode => new Type\Flate(),
            FilterType::RunLengthDecode => new Type\RunLength(),
            FilterType::CcittFaxDecode => new Type\CcittFax($params),
            FilterType::Jbig2Decode => new Type\JbigTwo(),
            FilterType::DctDecode => new Type\Dct(),
            FilterType::JpxDecode => new Type\Jpx(),
            FilterType::Crypt => new Type\Crypt(),
        };

        return $obj->decode($data, $params);
    }

    /**
     * Decode the input data using multiple filters
     *
     * @param array<string|FilterType> $filters Array of decoding filters to apply in order
     * @param string              $data    Data to decode.
     * @param array<string, mixed> $params  Optional DecodeParms dictionary.
     *
     * @return string Decoded data
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function decodeAll(array $filters, string $data, array $params = []): string
    {
        foreach ($filters as $filter) {
            $data = $this->decode($filter, $data, $params);
        }

        return $data;
    }
}
