<?php

declare(strict_types=1);

/**
 * PdfString.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Output;

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Output\PdfString
 *
 * Encodes a text value as a PDF string token, either through a host-supplied
 * encoder (which may apply UTF-16, escaping, and encryption) or, when none is
 * given, a built-in fallback.
 *
 * The fallback emits a literal string for printable ASCII, escaping the three
 * delimiters. Everything else is emitted as a UTF-16BE hex string with a byte
 * order mark: raw bytes above 0x7E in a literal string are read as
 * PDFDocEncoding and render as mojibake, and a raw CR or CRLF is folded into a
 * single LF (ISO 32000-1 section 7.3.4.2). The hex form has neither problem.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class PdfString
{
    /**
     * Escape sequences for the delimiters a literal string cannot carry raw.
     *
     * @var array<string, string>
     */
    private const ESCAPES = [
        '\\' => '\\\\',
        '(' => '\\(',
        ')' => '\\)',
    ];

    /**
     * Encode a text value as a PDF string token.
     *
     * @param callable|null $encoder fn(string $text, int $objectId): string
     *
     * @throws Exception If the encoder returns a non-string value.
     */
    public static function encode(string $text, int $objectId, ?callable $encoder = null): string
    {
        if ($encoder === null) {
            return self::fallback($text);
        }

        /** @var mixed $result */
        $result = $encoder($text, $objectId);
        if (!\is_string($result)) {
            throw new Exception('Invalid string encoder result');
        }

        return $result;
    }

    /**
     * Encode a text value without a host encoder.
     *
     * Tested against 0 rather than 1, so a PCRE error takes the hex branch along
     * with a match: preg_match() answers false on a resource limit.
     */
    private static function fallback(string $text): string
    {
        if (\preg_match('/[^\x20-\x7E]/', $text) !== 0) {
            return self::utf16HexString($text);
        }

        return '(' . \strtr($text, self::ESCAPES) . ')';
    }

    /**
     * Encode a text value as a UTF-16BE hex string with a byte order mark.
     *
     * The input is treated as UTF-8. Each byte that is not part of a valid UTF-8
     * sequence is emitted as U+FFFD rather than raw, so the token is always
     * well-formed and the characters around the bad byte survive. The conversion
     * is done here rather than through mbstring or iconv to keep the package's
     * extension requirements to hash, openssl, and pcre.
     */
    private static function utf16HexString(string $text): string
    {
        $out = "\xFE\xFF";
        foreach (self::codePoints($text) as $codePoint) {
            if ($codePoint > 0xFFFF) {
                $codePoint -= 0x1_0000;
                $out .= \pack('nn', 0xD800 | ($codePoint >> 10), 0xDC00 | ($codePoint & 0x3FF));
                continue;
            }

            $out .= \pack('n', $codePoint);
        }

        return '<' . \strtoupper(\bin2hex($out)) . '>';
    }

    /**
     * Lead byte range to [sequence length, low bound and high bound of the second byte].
     *
     * The narrowed second-byte ranges are what reject an overlong encoding, a
     * surrogate, and a code point above U+10FFFF (Unicode 15 table 3-7). A lead
     * byte outside the listed ranges (0x80-0xC1, 0xF5-0xFF) starts no sequence.
     *
     * @var array<int, array{int, int, int, int}> [low lead, high lead, low second, high second]
     */
    private const SEQUENCES = [
        [0xC2, 0xDF, 0x80, 0xBF], // 2 bytes
        [0xE0, 0xE0, 0xA0, 0xBF], // 3 bytes, no overlong
        [0xE1, 0xEC, 0x80, 0xBF], // 3 bytes
        [0xED, 0xED, 0x80, 0x9F], // 3 bytes, no surrogate
        [0xEE, 0xEF, 0x80, 0xBF], // 3 bytes
        [0xF0, 0xF0, 0x90, 0xBF], // 4 bytes, no overlong
        [0xF1, 0xF3, 0x80, 0xBF], // 4 bytes
        [0xF4, 0xF4, 0x80, 0x8F], // 4 bytes, no code point above U+10FFFF
    ];

    /**
     * Decode a UTF-8 string to a list of code points, substituting U+FFFD for each
     * byte that is not part of a valid sequence.
     *
     * Decoded here rather than with a PCRE /u pattern, which fails for the whole
     * subject on the first bad byte.
     *
     * @return list<int>
     */
    private static function codePoints(string $text): array
    {
        $points = [];
        $length = \strlen($text);
        $offset = 0;
        while ($offset < $length) {
            $point = self::decodeSequence($text, $offset, $length);
            if ($point === null) {
                $points[] = 0xFFFD;
                ++$offset;
                continue;
            }

            $points[] = $point;
        }

        return $points;
    }

    /**
     * Decode one UTF-8 sequence, advancing the offset past it.
     *
     * @param int $offset Read cursor; advanced only when a whole sequence is read.
     *
     * @return int|null The code point, or null when the byte at $offset starts none.
     */
    private static function decodeSequence(string $text, int &$offset, int $length): ?int
    {
        $lead = \ord($text[$offset]);
        if ($lead < 0x80) {
            ++$offset;
            return $lead;
        }

        foreach (self::SEQUENCES as [$lowLead, $highLead, $lowSecond, $highSecond]) {
            if ($lead < $lowLead || $lead > $highLead) {
                continue;
            }

            $size = match (true) {
                $lead >= 0xF0 => 4,
                $lead >= 0xE0 => 3,
                default => 2,
            };

            if (($offset + $size) > $length) {
                return null;
            }

            $second = \ord($text[$offset + 1]);
            if ($second < $lowSecond || $second > $highSecond) {
                return null;
            }

            $point = $lead & (0x7F >> $size);
            for ($idx = 1; $idx < $size; ++$idx) {
                $byte = \ord($text[$offset + $idx]);
                if ($idx > 1 && ($byte & 0xC0) !== 0x80) {
                    return null;
                }

                $point = ($point << 6) | ($byte & 0x3F);
            }

            $offset += $size;
            return $point;
        }

        return null;
    }
}
