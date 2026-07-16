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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Com\Tecnick\Pdf\Font;

/**
 * Shared font paths rooted at the library base directory.
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
