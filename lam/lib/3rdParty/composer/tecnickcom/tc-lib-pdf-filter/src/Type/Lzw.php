<?php

declare(strict_types=1);

/**
 * Lzw.php
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
 * Com\Tecnick\Pdf\Filter\Type\Lzw
 *
 * LZWDecode
 * Decompresses data encoded using the LZW (Lempel-Ziv-Welch) adaptive compression method,
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
class Lzw implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * @var int Code-length increase offset: 1 = early change (PDF default), 0 = standard LZW.
     */
    private int $earlyChange = 1;

    /**
     * Decode the data
     *
     * @param string $data   Data to decode.
     * @param array<string, mixed> $params Optional filter parameters.
     *   - 'EarlyChange' (int): 1 (default) increases the code length one code early; 0 disables it.
     *
     * @return string Decoded data string.
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            return '';
        }

        $this->earlyChange = (int) ($params['EarlyChange'] ?? 1) === 0 ? 0 : 1;

        // data length
        $data_length = \strlen($data);
        // convert string to binary string
        $bitstring = '';
        for ($i = 0; $i < $data_length; ++$i) {
            $bitstring .= \sprintf('%08b', \ord($data[$i]));
        }

        // get the number of bits
        $data_length = \strlen($bitstring);
        // initialize code length in bits
        $bitlen = 9;
        // initialize dictionary index
        $dix = 258;
        // initialize the dictionary (with the first 256 entries).
        $dictionary = $this->getInitialDictionary();

        // previous val
        $prev_index = 0;
        $decoded = '';
        /** @var array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string} $state */
        $state = [
            'bitlen' => $bitlen,
            'dix' => $dix,
            'prev_index' => $prev_index,
            'dictionary' => $dictionary,
            'decoded' => $decoded,
        ];

        // while we encounter EOD marker (257), read code_length bits
        while ($data_length > 0 && ($index = (int) \bindec(\substr($bitstring, 0, $state['bitlen']))) !== 257) {
            // remove read bits from string
            $bitstring = \substr($bitstring, $state['bitlen']);
            // update number of bits
            $data_length -= $state['bitlen'];
            $state = $this->processIndex($index, $state);
        }

        return $state['decoded'];
    }

    /**
     * @return array<int, string>
     */
    private function getInitialDictionary(): array
    {
        $dictionary = [];
        for ($i = 0; $i < 256; ++$i) {
            $dictionary[$i] = \chr($i);
        }

        return $dictionary;
    }

    /**
     * @param array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string} $state
     *
     * @return array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string}
     */
    private function processIndex(int $index, array $state): array
    {
        if ($index === 256) {
            $state['bitlen'] = 9;
            $state['dix'] = 258;
            $state['prev_index'] = 256;
            $state['dictionary'] = $this->getInitialDictionary();
            return $state;
        }

        if ($state['prev_index'] === 256) {
            $state['decoded'] .= $state['dictionary'][$index] ?? '';
            $state['prev_index'] = $index;
            return $state;
        }

        $dic_val = '';

        if ($index < $state['dix']) {
            $current = $state['dictionary'][$index] ?? '';
            $previous = $state['dictionary'][$state['prev_index']] ?? '';
            $state['decoded'] .= $current;
            $dic_val = $previous . ($current[0] ?? '');
            $state['prev_index'] = $index;
        }

        if ($index >= $state['dix']) {
            $previous = $state['dictionary'][$state['prev_index']] ?? '';
            $dic_val = $previous . ($previous[0] ?? '');
            $state['decoded'] .= $dic_val;
        }

        // Stop growing once the 12-bit code space (4096 entries) is exhausted: further
        // codes are unaddressable and a well-formed stream emits a clear code (256) first.
        if ($state['dix'] < 4096) {
            $state['dictionary'][$state['dix']] = $dic_val;
            ++$state['dix'];
            // With early change the width grows one code early (PDF default); the
            // EarlyChange=0 offset shifts each threshold up by one.
            $state['bitlen'] = match ($state['dix'] + $this->earlyChange) {
                2048 => 12,
                1024 => 11,
                512 => 10,
                default => $state['bitlen'],
            };
        }

        return $state;
    }
}
