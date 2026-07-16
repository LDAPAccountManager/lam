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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Com\Tecnick\Pdf\Filter\Type;

use Com\Tecnick\Pdf\Filter\Exception as PPException;

/**
 * Com\Tecnick\Pdf\Filter\Type\AsciiEightFive
 *
 * ASCII85
 * Decodes data encoded in an ASCII base-85 representation, reproducing the original binary data.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class AsciiEightFive implements \Com\Tecnick\Pdf\Filter\Type\Template
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
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    private function normalizeInput(string $data): string
    {
        // all white-space characters shall be ignored
        $data = \preg_replace('/[\s]+/', '', $data);
        if ($data === null) {
            throw new PPException('invalid code');
        }

        // check for EOD: 2-character sequence ~> (7Eh)(3Eh)
        $eod = \strpos($data, '~>');
        if ($eod !== false) {
            // remove EOD and following characters (if any)
            $data = \substr($data, 0, $eod);
        }

        // check for invalid characters: valid bytes are '!'..'u' (0x21-0x75) and 'z' (0x7A)
        $invalid = \preg_match('/[^\x21-\x75\x7A]/', $data);
        if ($invalid === false || $invalid > 0) {
            throw new PPException('invalid code');
        }

        return $data;
    }

    /**
     * @return array{string, int, int}
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
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
                    throw new PPException('invalid code');
                }

                $decoded .= $zseq;
                continue;
            }

            $tuple += ($char - 33) * $this->getPow85($group_pos);
            if ($group_pos === 4) {
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
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    private function getPow85(int $group_pos): int
    {
        return match ($group_pos) {
            0 => 85 * 85 * 85 * 85,
            1 => 85 * 85 * 85,
            2 => 85 * 85,
            3 => 85,
            4 => 1,
            default => throw new PPException('invalid code'),
        };
    }

    private function applyPadding(int $group_pos, int $tuple): int
    {
        return $tuple
        + match ($group_pos) {
            2 => 85 * 85 * 85,
            3 => 85 * 85,
            4 => 85,
            default => 0,
        };
    }

    /**
     * Get last tuple
     *
     * @return string Decoded data string.
     */
    protected function getLastTuple(int $group_pos, int $tuple): string
    {
        // last tuple (if any)
        return match ($group_pos) {
            4 => \chr(($tuple >> 24) & 0xFF) . \chr(($tuple >> 16) & 0xFF) . \chr(($tuple >> 8) & 0xFF),
            3 => \chr(($tuple >> 24) & 0xFF) . \chr(($tuple >> 16) & 0xFF),
            2 => \chr(($tuple >> 24) & 0xFF),
            1 => throw new PPException('invalid code'),
            default => '',
        };
    }
}
