<?php

declare(strict_types=1);

/**
 * FileWriter.php
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

use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\FileWriter
 *
 * Writes the artifacts produced by the font import.
 *
 * @since     2026-08-14
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
final class FileWriter
{
    /**
     * Write a font artifact, raising an exception when it cannot be stored in full.
     *
     * The number of bytes written is compared with the size of the data, and the diagnostic
     * fwrite() emits on failure is captured and carried by the exception message.
     *
     * @param ObjFile $fileHelper File helper used to open the file.
     * @param string  $file       Full path of the file to write.
     * @param string  $data       Data to store.
     *
     * @throws \Com\Tecnick\File\Exception if the file cannot be opened.
     * @throws FontException if the data cannot be stored in full.
     */
    public static function write(ObjFile $fileHelper, string $file, string $data): void
    {
        $fpt = $fileHelper->fopenLocal($file, 'wb');

        $reason = '';
        \set_error_handler(static function (int $level, string $message) use (&$reason): bool {
            unset($level);
            $reason = ' (' . $message . ')';
            return true;
        }, E_WARNING | E_NOTICE);

        try {
            $written = \fwrite($fpt, $data);
        } finally {
            \restore_error_handler();
            \fclose($fpt);
        }

        if ($written !== \strlen($data)) {
            throw new FontException('Unable to write the file: ' . $file . $reason);
        }
    }
}
