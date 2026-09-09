<?php

declare(strict_types=1);

/**
 * FontPaths.php
 *
 * @since     2026-06-08
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

use Com\Tecnick\File\Dir;

/**
 * Com\Tecnick\Pdf\Font\FontPaths
 *
 * Shared font paths rooted at the library base directory.
 *
 * @since     2026-06-08
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
class FontPaths
{
    /**
     * Returns the library root directory.
     */
    public static function getLibraryRoot(): string
    {
        return \rtrim(\dirname(__DIR__), '/\\');
    }

    /**
     * Returns the full path of a font file, or an empty string when it is not found.
     *
     * @param string $fontdir Original font directory.
     * @param string $file    Font file name.
     */
    public static function findFontFile(string $fontdir, string $file): string
    {
        if ($file === '') {
            return '';
        }

        $dirobj = new Dir();
        $kpathfonts = \defined('K_PATH_FONTS') ? (string) \constant('K_PATH_FONTS') : '';
        // directories to search, most specific first
        $dirs = \array_unique([
            $fontdir,
            $kpathfonts,
            $dirobj->findParentDir('fonts', __DIR__),
            '.',
        ]);
        foreach ($dirs as $dir) {
            if ($dir === '') {
                // an empty entry would resolve to the filesystem root
                continue;
            }

            $path = \rtrim($dir, '/\\') . DIRECTORY_SEPARATOR . $file;
            if (\is_file($path) && \is_readable($path)) {
                return $path;
            }
        }

        return '';
    }

    /**
     * Returns the default input font directory.
     */
    public static function getInputPath(): string
    {
        return self::getLibraryRoot() . '/fonts';
    }

    /**
     * Returns the default output font directory.
     */
    public static function getOutputPath(): string
    {
        return self::getLibraryRoot() . '/target/fonts';
    }

    /**
     * Build trusted roots for local font file access.
     *
     * @return array<string>
     */
    public static function buildAllowedPaths(): array
    {
        $roots = [
            self::getInputPath(),
            self::getOutputPath(),
        ];

        if (\defined('K_PATH_FONTS')) {
            $kpathfonts = (string) \constant('K_PATH_FONTS');
            if ($kpathfonts !== '') {
                $roots[] = $kpathfonts;
            }
        }

        $allowed = [];
        foreach ($roots as $root) {
            $normalized = \rtrim($root, '/\\');
            if ($normalized === '') {
                continue;
            }

            $allowed[] = $normalized;

            $resolved = \realpath($normalized);
            if ($resolved !== false) {
                $allowed[] = \rtrim($resolved, '/\\');
            }
        }

        return \array_values(\array_unique($allowed));
    }
}
