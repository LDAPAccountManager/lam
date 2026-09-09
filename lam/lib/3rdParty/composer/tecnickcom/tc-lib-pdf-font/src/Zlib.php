<?php

declare(strict_types=1);

/**
 * Zlib.php
 *
 * @since     2026-08-14
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\Zlib
 *
 * Compression and decompression of the stored font artifacts.
 *
 * @since     2026-08-14
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
final class Zlib
{
    /**
     * Uncompress a zlib stream.
     *
     * The E_WARNING gzuncompress() emits for a corrupt stream is silenced.
     *
     * @param string $data      Compressed data.
     * @param int    $maxLength Maximum size of the uncompressed data, or 0 for no limit.
     *                          A stream expanding past it is reported as invalid.
     *
     * @return string|false The uncompressed data, or false when the stream is not valid.
     */
    public static function uncompress(string $data, int $maxLength = 0): string|false
    {
        \set_error_handler(static fn(): bool => true, E_WARNING);

        try {
            return $maxLength > 0 ? \gzuncompress($data, $maxLength) : \gzuncompress($data);
        } finally {
            \restore_error_handler();
        }
    }

    /**
     * Compress data as a zlib stream.
     *
     * Any warning gzcompress() may emit is silenced, as in uncompress().
     *
     * @param string $data    Data to compress.
     * @param string $message Message of the exception raised when the data cannot be compressed.
     * @param int    $level   Compression level (-1 for the zlib default, 0 to 9).
     *
     * @return string The compressed data.
     *
     * @throws FontException if the data cannot be compressed.
     */
    public static function compress(string $data, string $message, int $level = -1): string
    {
        if ($level < -1 || $level > 9) {
            // gzcompress() would raise a ValueError instead of a FontException
            throw new FontException($message . ': invalid compression level ' . $level);
        }

        \set_error_handler(static fn(): bool => true, E_WARNING);

        try {
            $compressed = \gzcompress($data, $level);
        } finally {
            \restore_error_handler();
        }

        // the compression level refused above is the only input gzcompress() rejects
        /** @var string $compressed */
        return $compressed;
    }
}
