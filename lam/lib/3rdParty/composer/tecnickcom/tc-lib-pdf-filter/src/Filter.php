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
 * Decoder for the PDF stream filters (PDF 32000-1:2008 §7.4).
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
     * Decode the data with a single filter.
     *
     * @param string|FilterType    $filter Filter name or FilterType case; an empty name is a pass-through.
     * @param string               $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary for the filter.
     *
     * @return string Decoded data.
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
     * Decode the data with a chain of filters, applied in order.
     *
     * @param array<string|FilterType> $filters Filters to apply in order.
     * @param string                   $data    Data to decode.
     * @param array<array-key, mixed>  $params  Optional DecodeParms: a single dictionary
     *   applied to every filter, or a list holding one dictionary (or null) per filter,
     *   consumed positionally, as /DecodeParms parallel to /Filter. An entry that is not
     *   a dictionary leaves the corresponding filter without parameters.
     *
     * @return string Decoded data.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function decodeAll(array $filters, string $data, array $params = []): string
    {
        $positional = $this->isPositionalParams($params);

        $position = 0;
        foreach ($filters as $filter) {
            $data = $this->decode($filter, $data, $this->paramsAt($params, $positional, $position));
            ++$position;
        }

        return $data;
    }

    /**
     * DecodeParms for one filter of the chain.
     *
     * @param array<array-key, mixed> $params     DecodeParms value.
     * @param bool                    $positional Whether $params is the per-filter list form.
     * @param int                     $position   Index of the filter in the chain.
     *
     * @return array<string, mixed>
     */
    private function paramsAt(array $params, bool $positional, int $position): array
    {
        if (!$positional) {
            /** @var array<string, mixed> $params */
            return $params;
        }

        // a missing, null or non-dictionary entry means no parameters
        if (!\is_array($params[$position] ?? null)) {
            return [];
        }

        /** @var array<string, mixed> */
        return $params[$position];
    }

    /**
     * Whether the DecodeParms value is a per-filter list rather than a single dictionary.
     *
     * A PDF dictionary is keyed by name objects, so a non-empty list is the array form.
     *
     * @param array<array-key, mixed> $params DecodeParms value.
     */
    private function isPositionalParams(array $params): bool
    {
        return $params !== [] && \array_is_list($params);
    }
}
