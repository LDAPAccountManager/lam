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

use Com\Tecnick\File\File as ObjFile;

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
     * Canonical K_PATH_FONTS directory, set once it resolves to an existing directory.
     */
    private static string $configPath = '';

    /**
     * File helper confined to the package font directories, created on first use.
     */
    private static ?ObjFile $packageHelper = null;

    /**
     * Allowlist of $packageHelper.
     *
     * @var array<string>
     */
    private static array $packageHelperRoots = [];

    /**
     * Returns the K_PATH_FONTS value, or an empty string when it is undefined or empty.
     *
     * An existing directory is returned as its canonical realpath, without a trailing separator.
     * Any other value is returned unchanged.
     */
    public static function getConfigPath(): string
    {
        if (self::$configPath !== '') {
            return self::$configPath;
        }

        if (!\defined('K_PATH_FONTS')) {
            return '';
        }

        $kpathfonts = (string) \constant('K_PATH_FONTS');
        if ($kpathfonts === '') {
            return '';
        }

        $resolved = \realpath($kpathfonts);
        if ($resolved === false || !\is_dir($resolved)) {
            return $kpathfonts;
        }

        self::$configPath = $resolved;

        return $resolved;
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

        $kpathfonts = self::getConfigPath();
        // directories to search, most specific first
        $dirs = \array_unique([
            $fontdir,
            $kpathfonts,
            ...self::getPackagePaths(),
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
     * Returns the canonical package font directories: the input directory (getInputPath())
     * and the generated font set (getOutputPath()).
     *
     * Directories that do not exist or resolve to a filesystem root are left out.
     *
     * @return array<string> Directory paths without a trailing separator.
     */
    public static function getPackagePaths(): array
    {
        $paths = [];
        foreach ([static::getInputPath(), static::getOutputPath()] as $path) {
            $real = \realpath($path);
            if ($real === false || !\is_dir($real) || \dirname($real) === $real) {
                continue;
            }

            $paths[] = $real;
        }

        return \array_values(\array_unique($paths));
    }

    /**
     * Returns true when the file is inside a package font directory.
     *
     * The check is made by a file helper whose allowlist is getPackagePaths().
     *
     * @param string $file File path.
     */
    public static function isPackageFile(string $file): bool
    {
        $helper = self::getPackageHelper();

        return $helper !== null && $helper->isAllowedFile($file);
    }

    /**
     * Reads a font file.
     *
     * A file inside a package font directory is read through a file helper whose
     * allowlist is getPackagePaths(); any other file is read through $fileHelper.
     *
     * @internal
     *
     * @param ObjFile|null $fileHelper File helper for files outside the package font directories.
     * @param string       $file       Font file path.
     * @param bool         $remote     Allow $fileHelper to read a URL.
     *
     * @return string|false File content, or false when the file cannot be read.
     *
     * @throws \Com\Tecnick\File\Exception when a remote transfer is aborted.
     */
    public static function readFontFile(?ObjFile $fileHelper, string $file, bool $remote = false): string|false
    {
        $helper = self::getPackageHelper();
        if ($helper !== null && $helper->isAllowedFile($file)) {
            return $helper->getLocalFileData($file);
        }

        if ($fileHelper === null) {
            return false;
        }

        return $remote ? $fileHelper->getFileData($file) : $fileHelper->getLocalFileData($file);
    }

    /**
     * Returns the file helper confined to the package font directories, or null when none
     * of them exists.
     */
    private static function getPackageHelper(): ?ObjFile
    {
        $roots = self::getPackagePaths();
        if ($roots === []) {
            return null;
        }

        if (self::$packageHelper === null || self::$packageHelperRoots !== $roots) {
            self::$packageHelper = new ObjFile(allowedPaths: $roots);
            self::$packageHelperRoots = $roots;
        }

        return self::$packageHelper;
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
            $roots[] = (string) \constant('K_PATH_FONTS');
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
