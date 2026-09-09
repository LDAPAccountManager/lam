<?php

declare(strict_types=1);

/**
 * Dir.php
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-file
 *
 * This file is part of tc-lib-file software library.
 */

namespace Com\Tecnick\File;

/**
 * Com\Tecnick\File\Dir
 *
 * Writable parent-directory lookup
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
class Dir
{
    /**
     * Returns the full path of a writable parent directory.
     *
     * Walks up from $dir looking for a writable directory named $name. A
     * read-only directory of that name, or a regular file bearing it, is
     * skipped.
     *
     * @param string $name Single directory name to search for. An empty or
     *                     absolute name, or one containing a path separator or
     *                     a parent-directory segment, is reported as not found.
     * @param string $dir  Starting directory. A relative one is anchored to the
     *                     current working directory first.
     *
     * @return string Directory name with a trailing separator, or an empty
     *                string when no writable match exists up to the filesystem
     *                root.
     */
    public function findParentDir(string $name, string $dir = __DIR__): string
    {
        if (!$this->isPlainDirName($name)) {
            return '';
        }

        return $this->walkUpFor($name, $this->toAbsoluteDir($dir));
    }

    /**
     * Walk up from $dir returning the first writable directory named $name.
     *
     * @param string $name Plain directory name to look for.
     * @param string $dir  Anchored starting directory.
     *
     * @return string Match with a trailing separator, or '' when there is none.
     */
    private function walkUpFor(string $name, string $dir): string
    {
        $allowedBases = $this->getOpenBasedirPaths();

        while ($dir !== '') {
            if ($dir === \dirname($dir)) {
                $dir = '';
            }

            $candidate = $dir . DIRECTORY_SEPARATOR . $name;
            // is_writable() is also true for a regular file, so is_dir() is
            // what restricts the match to a directory.
            if ($this->isPathAllowed($candidate, $allowedBases) && \is_dir($candidate) && \is_writable($candidate)) {
                return \str_ends_with($candidate, DIRECTORY_SEPARATOR) ? $candidate : $candidate . DIRECTORY_SEPARATOR;
            }

            $dir = \dirname($dir);
        }

        return '';
    }

    /**
     * Anchor a relative starting directory to the current working directory.
     *
     * The upward walk stops when a path is its own dirname, which for an
     * absolute path is the filesystem root and for a relative one is '.', so a
     * relative path is anchored first to walk the same ancestors as the
     * absolute path naming it.
     *
     * An empty starting directory is returned unchanged.
     *
     * @param string $dir Starting directory, absolute or relative.
     */
    private function toAbsoluteDir(string $dir): string
    {
        if ($dir === '' || $this->isAbsoluteDir($dir)) {
            return $dir;
        }

        $cwd = \getcwd();

        // With no current directory to anchor to, the relative path is left
        // as it is.
        return $cwd === false ? $dir : $cwd . DIRECTORY_SEPARATOR . $dir;
    }

    /**
     * Tells whether the given path is anchored rather than relative.
     *
     * Covers the POSIX form, the Windows drive form ('C:\dir', 'C:/dir') and
     * the UNC form ('\\server\share'), whatever the platform in use.
     *
     * @param string $dir Path to check.
     */
    private function isAbsoluteDir(string $dir): bool
    {
        return (
            \str_starts_with($dir, '/')
            || \str_starts_with($dir, '\\')
            || \preg_match('#^[A-Za-z]:[\\\\/]#', $dir) === 1
        );
    }

    /**
     * Tells whether $name is a single directory name that can be appended to a
     * candidate path.
     *
     * findParentDir() concatenates $name onto each ancestor of the starting
     * directory, so only a plain name yields an ancestor of it.
     *
     * @param string $name Candidate directory name.
     */
    private function isPlainDirName(string $name): bool
    {
        return (
            $name !== ''
            && $name !== '.'
            && $name !== '..'
            && !\str_contains($name, '/')
            && !\str_contains($name, '\\')
            && !\str_contains($name, "\0")
            && \preg_match('#^[A-Za-z]:#', $name) !== 1
        );
    }

    /**
     * Returns the list of directories allowed by the active open_basedir restriction.
     *
     * An empty array means that no restriction is in effect.
     *
     * @return array<string> Allowed base directories without trailing separator.
     */
    private function getOpenBasedirPaths(): array
    {
        $openBasedir = \ini_get('open_basedir');
        if ($openBasedir === false || $openBasedir === '') {
            return [];
        }

        $paths = [];
        foreach (\explode(PATH_SEPARATOR, $openBasedir) as $path) {
            $path = \rtrim($path, '/\\');
            if ($path !== '') {
                $paths[] = $path;
            }
        }

        return $paths;
    }

    /**
     * Tells whether the given path can be probed under the active open_basedir restriction.
     *
     * Probing a path outside the allowed list raises an open_basedir E_WARNING,
     * so such paths are skipped. When no restriction is in effect every path is
     * allowed.
     *
     * @param string        $path         Path to check.
     * @param array<string> $allowedBases Allowed base directories (empty when unrestricted).
     */
    private function isPathAllowed(string $path, array $allowedBases): bool
    {
        if ($allowedBases === []) {
            return true;
        }

        foreach ($allowedBases as $base) {
            if ($path === $base || \str_starts_with($path, $base . DIRECTORY_SEPARATOR)) {
                return true;
            }
        }

        return false;
    }
}
