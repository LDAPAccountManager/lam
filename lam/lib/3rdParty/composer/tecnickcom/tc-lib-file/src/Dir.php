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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 *
 * This file is part of tc-lib-file software library.
 */

namespace Com\Tecnick\File;

/**
 * Com\Tecnick\File\Dir
 *
 * Function to read byte-level data
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
class Dir
{
    /**
     * Returns the full path of a parent directory
     *
     * @param string $name Name of the parent folder to search
     * @param string $dir  Starting directory
     *
     * @return string Directory name
     */
    public function findParentDir(string $name, string $dir = __DIR__): string
    {
        $allowedBases = $this->getOpenBasedirPaths();

        while ($dir !== '') {
            if ($dir === \dirname($dir)) {
                $dir = '';
            }

            $candidate = $dir . DIRECTORY_SEPARATOR . $name;
            if ($this->isPathAllowed($candidate, $allowedBases) && \is_writable($candidate)) {
                $dir = $candidate;
                break;
            }

            $dir = \dirname($dir);
        }

        if (\substr($dir, -1) !== DIRECTORY_SEPARATOR) {
            $dir .= DIRECTORY_SEPARATOR;
        }

        return $dir;
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
     * Tells whether the given path can be safely probed under the active open_basedir restriction.
     *
     * Probing a path outside the allowed list raises an open_basedir E_WARNING (which can
     * corrupt the output stream or be promoted to an exception by the application error handler),
     * so such paths must be skipped. When no restriction is in effect every path is allowed.
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
