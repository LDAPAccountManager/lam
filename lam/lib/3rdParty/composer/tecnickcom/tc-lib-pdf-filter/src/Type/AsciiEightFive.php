<?php

declare(strict_types=1);

/**
 * AsciiEightFive.php
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
 * Com\Tecnick\Pdf\Filter\Type\AsciiEightFive
 *
 * ASCII85Decode filter (PDF 32000-1:2008 §7.4.3).
 * Decodes data written in the ASCII base-85 representation.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class AsciiEightFive implements \Com\Tecnick\Pdf\Filter\Type\Template
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
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    public function decode(string $data, array $params = []): string
    {
        if ($data === '') {
            return '';
        }

        $data = $this->normalizeInput($data);

        [$decoded, $group_pos, $tuple] = $this->decodeTuples($data);
        $tuple = $this->applyPadding($group_pos, $tuple);

        return $decoded . $this->getLastTuple($group_pos, $tuple);
    }

    /**
     * Strip the white space and the EOD marker, and reject characters outside the alphabet.
     *
     * @param string $data Data to decode.
     *
     * @throws PPException
     */
    private function normalizeInput(string $data): string
    {
        // all white-space characters shall be ignored (PDF 32000-1:2008 Table 1 also lists NUL)
        $data = \preg_replace('/[\x00\s]+/', '', $data);
        if ($data === null) {
            throw new PPException('invalid code: white-space removal failed');
        }

        // EOD marker: the 2-character sequence ~> (7Eh)(3Eh); it and any data after it are dropped
        $eod = \strpos($data, '~>');
        if ($eod !== false) {
            $data = \substr($data, 0, $eod);
        }

        // valid bytes are '!'..'u' (0x21-0x75) and 'z' (0x7A)
        $invalid = \preg_match('/[^\x21-\x75\x7A]/', $data);
        if ($invalid === false || $invalid > 0) {
            throw new PPException('invalid code: character outside the ASCII85 alphabet');
        }

        return $data;
    }

    /**
     * Decode the complete five-character groups.
     *
     * @param string $data Normalized data.
     *
     * @return array{string, int, int} Decoded bytes, position within the trailing
     *   partial group, and the value accumulated for it.
     *
     * @throws PPException
     */
    private function decodeTuples(string $data): array
    {
        $zseq = \chr(0) . \chr(0) . \chr(0) . \chr(0);
        $group_pos = 0;
        $tuple = 0;
        $decoded = '';
        $data_length = \strlen($data);
        for ($i = 0; $i < $data_length; ++$i) {
            $char = \ord($data[$i]);
            if ($char === 122) {
                if ($group_pos !== 0) {
                    throw new PPException('invalid code: z inside a group');
                }

                $decoded .= $zseq;
                continue;
            }

            $tuple += ($char - 33) * $this->getPow85($group_pos);
            if ($group_pos === 4) {
                // a group encodes a 32-bit integer: anything above 0xFFFF_FFFF is malformed
                if ($tuple > 0xFFFF_FFFF) {
                    throw new PPException('invalid code: group value above 32 bits');
                }

                $decoded .=
                    \chr(($tuple >> 24) & 0xFF)
                    . \chr(($tuple >> 16) & 0xFF)
                    . \chr(($tuple >> 8) & 0xFF)
                    . \chr($tuple & 0xFF);
                $tuple = 0;
                $group_pos = 0;
                continue;
            }

            $group_pos = match ($group_pos) {
                0 => 1,
                1 => 2,
                2 => 3,
                default => 4,
            };
        }

        return [$decoded, $group_pos, $tuple];
    }

    /**
     * Weight of the character at $group_pos within its five-character group.
     *
     * @param int $group_pos Position within the group (0 to 4).
     */
    private function getPow85(int $group_pos): int
    {
        return match ($group_pos) {
            0 => 85 * 85 * 85 * 85,
            1 => 85 * 85 * 85,
            2 => 85 * 85,
            3 => 85,
            default => 1,
        };
    }

    /**
     * Pad a final partial group to five characters.
     *
     * PDF 32000-1:2008 §7.4.3 pads with the character 'u' (value 84), which is
     * worth 85^d - 1 for the d missing characters.
     *
     * @param int $group_pos Number of characters present in the final group.
     * @param int $tuple     Value accumulated for the final group.
     */
    private function applyPadding(int $group_pos, int $tuple): int
    {
        return $tuple
        + match ($group_pos) {
            2 => (85 * 85 * 85) - 1,
            3 => (85 * 85) - 1,
            4 => 84,
            default => 0,
        };
    }

    /**
     * Decode the padded final group, one byte less than the characters it holds.
     *
     * @param int $group_pos Number of characters present in the final group.
     * @param int $tuple     Padded value of the final group.
     *
     * @return string Decoded bytes of the final group.
     *
     * @throws PPException
     */
    protected function getLastTuple(int $group_pos, int $tuple): string
    {
        // a final group of one character encodes nothing
        if ($group_pos === 1) {
            throw new PPException('invalid code: final group has a single character');
        }

        // the padded final group encodes a 32-bit integer too
        if ($tuple > 0xFFFF_FFFF) {
            throw new PPException('invalid code: final group value above 32 bits');
        }

        return match ($group_pos) {
            4 => \chr(($tuple >> 24) & 0xFF) . \chr(($tuple >> 16) & 0xFF) . \chr(($tuple >> 8) & 0xFF),
            3 => \chr(($tuple >> 24) & 0xFF) . \chr(($tuple >> 16) & 0xFF),
            2 => \chr(($tuple >> 24) & 0xFF),
            default => '',
        };
    }
}
