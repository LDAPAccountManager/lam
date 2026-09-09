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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter\Type;

use Com\Tecnick\Pdf\Filter\Exception as PPException;
use Com\Tecnick\Pdf\Filter\Predictor;

/**
 * Com\Tecnick\Pdf\Filter\Type\Lzw
 *
 * LZWDecode filter (PDF 32000-1:2008 §7.4.4).
 * Decompresses LZW (Lempel-Ziv-Welch) data and reverses the optional predictor.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class Lzw implements \Com\Tecnick\Pdf\Filter\Type\Template
{
    /**
     * @var int Code-length increase offset: 1 = early change (PDF default), 0 = standard LZW.
     */
    private int $earlyChange = 1;

    /**
     * @var int Maximum number of decoded bytes to produce; 0 = unlimited.
     */
    private int $maxOutputSize = 0;

    /**
     * Decode the data.
     *
     * @param string               $data   Data to decode.
     * @param array<string, mixed> $params Optional DecodeParms dictionary.
     *   - 'EarlyChange' (int): 1 (default) increases the code length one code early; 0 disables it.
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

        $this->earlyChange = (int) ($params['EarlyChange'] ?? 1) === 0 ? 0 : 1;
        $this->maxOutputSize = \max(0, (int) ($params['MaxOutputSize'] ?? 0));

        // total number of readable bits
        $data_length = \strlen($data) * 8;
        // bit offset of the next code
        $offset = 0;

        /** @var array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string} $state */
        $state = [
            // code length in bits
            'bitlen' => 9,
            // next dictionary index
            'dix' => 258,
            // previous code; 256 (clear) marks "no previous entry"
            'prev_index' => 256,
            'dictionary' => $this->getInitialDictionary(),
            'decoded' => '',
        ];

        // read codes until the EOD marker (257); a whole code must remain, so
        // the trailing bits padding the last byte are not read as one
        while (
            ($offset + $state['bitlen']) <= $data_length
            && ($index = $this->readBits($data, $offset, $state['bitlen'])) !== 257
        ) {
            $offset += $state['bitlen'];
            $this->processIndex($index, $state);
        }

        return (new Predictor())->apply($state['decoded'], $params);
    }

    /**
     * Read the big-endian unsigned integer held by $len bits starting at bit $offset.
     *
     * @param string $data   Raw byte string.
     * @param int    $offset Bit offset of the first bit to read.
     * @param int    $len    Number of bits to read.
     */
    private function readBits(string $data, int $offset, int $len): int
    {
        $value = 0;
        $end = $offset + $len;
        while ($offset < $end) {
            $available = 8 - ($offset % 8);
            $take = \min($available, $end - $offset);
            $byte = \ord($data[\intdiv($offset, 8)]);
            $value = ($value << $take) | (($byte >> ($available - $take)) & ((1 << $take) - 1));
            $offset += $take;
        }

        return $value;
    }

    /**
     * Dictionary holding the 256 single-byte entries.
     *
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
     * Apply one code to the decoder state, which is updated in place.
     *
     * @param int $index Code to apply.
     * @param array{bitlen: int, dix: int, prev_index: int, dictionary: array<int, string>, decoded: string} $state
     *
     * @throws PPException
     */
    private function processIndex(int $index, array &$state): void
    {
        if ($index === 256) {
            $state['bitlen'] = 9;
            $state['dix'] = 258;
            $state['prev_index'] = 256;
            $state['dictionary'] = $this->getInitialDictionary();
            return;
        }

        // a code may only name an existing entry, or the entry being built (KwKwK)
        if (!isset($state['dictionary'][$index]) && ($index !== $state['dix'] || $state['prev_index'] === 256)) {
            throw new PPException('invalid code');
        }

        if ($state['prev_index'] === 256) {
            $state['decoded'] .= $state['dictionary'][$index];
            $state['prev_index'] = $index;
            $this->guardOutputSize($state['decoded']);
            return;
        }

        if ($index < $state['dix']) {
            $current = $state['dictionary'][$index] ?? '';
            $previous = $state['dictionary'][$state['prev_index']] ?? '';
            $state['decoded'] .= $current;
            $dic_val = $previous . ($current[0] ?? '');
            $state['prev_index'] = $index;
        } else {
            // self-referential code (KwKwK): the entry being built expands to
            // the previous entry plus its own first byte
            $previous = $state['dictionary'][$state['prev_index']] ?? '';
            $dic_val = $previous . ($previous[0] ?? '');
            $state['decoded'] .= $dic_val;
            $state['prev_index'] = $index;
        }

        // the dictionary stops growing once the 12-bit code space (4096 entries) is full
        if ($state['dix'] < 4096) {
            $state['dictionary'][$state['dix']] = $dic_val;
            ++$state['dix'];
            // with early change the code width grows one code early (PDF default)
            $state['bitlen'] = match ($state['dix'] + $this->earlyChange) {
                2048 => 12,
                1024 => 11,
                512 => 10,
                default => $state['bitlen'],
            };
        }

        $this->guardOutputSize($state['decoded']);
    }

    /**
     * Enforce the optional decoded-size cap.
     *
     * @param string $decoded Data decoded so far.
     *
     * @throws PPException
     */
    private function guardOutputSize(string $decoded): void
    {
        if ($this->maxOutputSize > 0 && \strlen($decoded) > $this->maxOutputSize) {
            throw new PPException('decoded data exceeds MaxOutputSize of ' . $this->maxOutputSize . ' bytes');
        }
    }
}
