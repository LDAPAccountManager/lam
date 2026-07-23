<?php

declare(strict_types=1);

/**
 * Cache.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filecache
 *
 * This file is part of tc-lib-pdf-filecache software library.
 */

namespace Com\Tecnick\File;

/**
 * Com\Tecnick\Pdf\File\Cache
 *
 * File caching system with per-instance path and prefix.
 * Each Cache instance maintains its own cache directory path and file prefix.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filecache
 */
class Cache
{
    /**
     * Pattern matching characters that are not allowed in cache filename tokens
     * (prefix, type, key). Stripping them keeps generated names valid on every
     * platform (notably Windows) and prevents glob metacharacters from reaching
     * delete()/deleteOlderThan(). getNewFileName() and delete() must use the
     * same pattern so the prefix-based glob matches the files actually created.
     */
    private const SAFE_NAME_PATTERN = '/[^a-zA-Z0-9_\-]/';

    /**
     * Cache path (per-instance)
     *
     * @var string
     */
    protected string $path = '';

    /**
     * File prefix (per-instance)
     */
    protected string $prefix;

    /**
     * Set the file prefix (common name)
     *
     * @param ?string $prefix Common prefix to be used for all cache files
     */
    public function __construct(?string $prefix = null)
    {
        $this->defineSystemCachePath();
        $this->setCachePath();
        $prefix ??= \rtrim(
            \base64_encode(\pack('H*', \md5(\uniqid((string) \mt_rand(0, \mt_getrandmax()), true)))),
            '=',
        );

        $safePrefix = \preg_replace(self::SAFE_NAME_PATTERN, '', \strtr($prefix, '+/', '-_')) ?? '';
        $this->prefix = '_' . $safePrefix . '_';
    }

    /**
     * Get the cache directory path
     */
    public function getCachePath(): string
    {
        return $this->path;
    }

    /**
     * Set the default cache directory path
     *
     * @param ?string $path Cache directory path; if null use the K_PATH_CACHE value
     */
    public function setCachePath(?string $path = null): void
    {
        if ($path === null || \str_contains($path, '://') || !\is_writable($path)) {
            // Normalize the fallback too: K_PATH_CACHE may be supplied by the host
            // application without a trailing separator, and getNewFileName()/delete()
            // concatenate $this->path directly. Without normalization the generated
            // file would escape the cache directory (e.g. ".../cachedir" + "_pfx_..."
            // lands next to the directory instead of inside it).
            $this->path = $this->normalizePath((string) \constant('K_PATH_CACHE'));
            return;
        }

        $this->path = $this->normalizePath($path);
    }

    /**
     * Get the file prefix
     */
    public function getFilePrefix(): string
    {
        return $this->prefix;
    }

    /**
     * Returns a temporary filename for caching files.
     * Throws an exception when tempnam() fails, consistent with the rest of the library.
     *
     * @param string $type Type of file
     * @param string $key  File key (used to retrieve file from cache)
     *
     * @return string Temporary filename
     *
     * @throws \Com\Tecnick\File\Exception when a temporary file cannot be created.
     */
    public function getNewFileName(string $type = 'tmp', string $key = '0'): string
    {
        // Sanitize so the generated name matches the patterns used by delete()
        // and contains no characters that are invalid in Windows filenames.
        $safeType = \preg_replace(self::SAFE_NAME_PATTERN, '', $type) ?? '';
        $safeKey = \preg_replace(self::SAFE_NAME_PATTERN, '', $key) ?? '';

        // tempnam() atomically creates a unique, private file, but on Windows it
        // keeps only the first three characters of the prefix. Create the file
        // with tempnam(), then rename it to a name carrying the full prefix so
        // the prefix-based glob in delete()/deleteOlderThan() works on every
        // platform. The rename preserves tempnam()'s restrictive permissions.
        $file = \tempnam($this->path, $this->prefix);
        if ($file === false) {
            throw new Exception('unable to create a temporary file in: ' . $this->path);
        }

        $unique = \str_replace('.', '', \uniqid('', true));
        $target = $this->path . $this->prefix . $safeType . '_' . $safeKey . '_' . $unique;

        $renamed = $this->withoutFsWarnings(static fn(): bool => \rename($file, $target));

        return $renamed ? $target : $file;
    }

    /**
     * Delete cached files
     *
     * @param ?string $type Type of files to delete
     * @param ?string $key  Specific file key to delete
     */
    public function delete(?string $type = null, ?string $key = null): void
    {
        $safeType = $type !== null ? \preg_replace(self::SAFE_NAME_PATTERN, '', $type) : null;
        $safeKey = $key !== null ? \preg_replace(self::SAFE_NAME_PATTERN, '', $key) : null;

        $path = $this->path . $this->prefix;
        if ($safeType !== null) {
            $path .= $safeType . '_';
            if ($safeKey !== null) {
                $path .= $safeKey . '_';
            }
        }

        $path .= '*';
        $files = \glob($path);
        if ($files === [] || $files === false) {
            return;
        }

        // Suppress warnings: a file may vanish (concurrent cleanup, external
        // process) between glob() and unlink(); such races are non-fatal.
        $this->withoutFsWarnings(static function () use ($files): void {
            foreach ($files as $file) {
                \unlink($file);
            }
        });
    }

    /**
     * Delete cache files older than the given number of seconds.
     *
     * @param int $seconds Maximum age in seconds; files whose mtime is older are removed.
     */
    public function deleteOlderThan(int $seconds): void
    {
        $pattern = $this->path . $this->prefix . '*';
        $files = \glob($pattern);
        if ($files === [] || $files === false) {
            return;
        }

        $cutoff = \time() - $seconds;

        // Suppress warnings: filemtime()/unlink() may fail if a file disappears
        // between glob() and the call (concurrent cleanup); such races are non-fatal.
        $this->withoutFsWarnings(static function () use ($files, $cutoff): void {
            foreach ($files as $file) {
                $mtime = \filemtime($file);
                if ($mtime !== false && $mtime < $cutoff) {
                    \unlink($file);
                }
            }
        });
    }

    /**
     * Execute a callable while suppressing expected filesystem warnings.
     *
     * The cache helpers race against other processes and against files that may
     * vanish between glob() and the operation; those calls already signal failure
     * via their return values, so the raw PHP warnings only add noise.
     *
     * @template T
     *
     * @param callable():T $callback
     *
     * @return T
     */
    private function withoutFsWarnings(callable $callback): mixed
    {
        \set_error_handler(static fn(): bool => true, E_WARNING);

        try {
            return $callback();
        } finally {
            \restore_error_handler();
        }
    }

    /**
     * Set the K_PATH_CACHE constant (if not set) to the default system directory for temporary files
     */
    protected function defineSystemCachePath(): void
    {
        if (\defined('K_PATH_CACHE')) {
            return;
        }

        $kPathCache = \ini_get('upload_tmp_dir');
        if ($kPathCache === false || $kPathCache === '') {
            $kPathCache = \sys_get_temp_dir();
        }
        \define('K_PATH_CACHE', $this->normalizePath($kPathCache));
    }

    /**
     * Normalize cache path
     *
     * @param string $path Path to normalize
     */
    protected function normalizePath(string $path): string
    {
        $rpath = \realpath($path);
        if ($rpath === false) {
            return '';
        }

        if (!\str_ends_with($rpath, \DIRECTORY_SEPARATOR)) {
            $rpath .= \DIRECTORY_SEPARATOR;
        }

        return $rpath;
    }
}
