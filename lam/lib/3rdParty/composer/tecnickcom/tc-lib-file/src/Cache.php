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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-file
 *
 * This file is part of tc-lib-file software library.
 */

namespace Com\Tecnick\File;

/**
 * Com\Tecnick\File\Cache
 *
 * File caching system with per-instance directory path and file prefix
 *
 * @since     2011-05-23
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
class Cache
{
    /**
     * Pattern matching the characters stripped from the cache filename tokens
     * (prefix, type and key), so that generated names are valid on every
     * platform and carry no glob metacharacter.
     *
     * '_' is excluded because it separates the prefix, type and key fields of a
     * generated name: allowed inside a field, it would let the prefix scan in
     * delete() match across field boundaries.
     */
    private const SAFE_NAME_PATTERN = '/[^a-zA-Z0-9\-]/';

    /**
     * Number of distinct file names getNewFileName() tries before giving up.
     */
    private const MAX_NAME_ATTEMPTS = 3;

    /**
     * Cache directory path
     *
     * @var string
     */
    protected string $path = '';

    /**
     * Prefix common to all the cache files of this instance
     */
    protected string $prefix;

    /**
     * Initialize the cache directory and file prefix.
     *
     * @param ?string $prefix Common prefix for all cache files; null draws a
     *                        random one.
     *
     * @throws \Com\Tecnick\File\Exception when the cache directory cannot be
     *                                     resolved, or no secure randomness is available.
     */
    public function __construct(?string $prefix = null)
    {
        $this->defineSystemCachePath();
        $this->setCachePath();
        // A default prefix is drawn from the CSPRNG, since the cache directory
        // is world-writable on a default install.
        $prefix ??= $this->randomToken(16);

        // '+' and '/' are mapped to '-' rather than stripped, so that a base64
        // prefix keeps its entropy.
        $safePrefix = \preg_replace(self::SAFE_NAME_PATTERN, '', \strtr($prefix, '+/', '--')) ?? '';

        // A prefix made entirely of stripped characters would collapse to '__'
        // and be shared by every such instance.
        if ($safePrefix === '') {
            $safePrefix = $this->randomToken(16);
        }

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
     * Falls back to K_PATH_CACHE when $path is null, names a stream wrapper, or
     * is not a writable directory. The fallback is silent: compare
     * getCachePath() against the argument to detect it.
     *
     * @param ?string $path Cache directory path; if null use the K_PATH_CACHE value
     *
     * @throws \Com\Tecnick\File\Exception when the resulting directory cannot be resolved.
     */
    public function setCachePath(?string $path = null): static
    {
        if ($path === null || \str_contains($path, '://') || !\is_dir($path) || !\is_writable($path)) {
            // The fallback is normalized too, since K_PATH_CACHE may be
            // supplied without a trailing separator and the path is
            // concatenated directly by getNewFileName() and delete().
            $this->path = $this->normalizePath((string) \constant('K_PATH_CACHE'));
            return $this;
        }

        $this->path = $this->normalizePath($path);
        return $this;
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
     *
     * The returned name carries a random suffix, so it cannot be reconstructed
     * from $type and $key: the caller is responsible for storing it.
     *
     * @param string $type Type of file (used to scope delete())
     * @param string $key  File key (used to scope delete())
     *
     * @return string Temporary filename
     *
     * @throws \Com\Tecnick\File\Exception when a cache file cannot be created,
     *                                     or no secure randomness is available.
     */
    public function getNewFileName(string $type = 'tmp', string $key = '0'): string
    {
        // Sanitized with the same pattern delete() applies, so that the scan
        // matches the files actually created.
        $safeType = \preg_replace(self::SAFE_NAME_PATTERN, '', $type) ?? '';
        $safeKey = \preg_replace(self::SAFE_NAME_PATTERN, '', $key) ?? '';

        // tempnam() atomically creates a unique, private file, but on Windows
        // it keeps only the first three characters of the prefix. The file is
        // renamed below to a name carrying the full prefix, so that the scan in
        // delete() and deleteOlderThan() finds it on every platform. The rename
        // preserves tempnam()'s restrictive permissions.
        $file = $this->withoutFsWarnings(fn(): string|false => \tempnam($this->path, $this->prefix));

        if ($file === false) {
            throw new Exception('unable to create a temporary file in: ' . $this->path);
        }

        // On an unwritable directory tempnam() raises a notice and falls back
        // to the system temp directory. A file created there is outside the
        // configured cache, so it is removed and the failure reported.
        if (\realpath(\dirname($file)) !== \realpath(\rtrim($this->path, \DIRECTORY_SEPARATOR))) {
            $this->withoutFsWarnings(static fn(): bool => \unlink($file));

            throw new Exception('unable to create a temporary file in: ' . $this->path);
        }

        $base = $this->path . $this->prefix . $safeType . '_' . $safeKey . '_';

        // rename() overwrites its target, so the suffix is drawn from the
        // CSPRNG and the name is checked for a collision before the move.
        for ($attempt = 0; $attempt < self::MAX_NAME_ATTEMPTS; $attempt++) {
            $target = $base . $this->randomToken(16);
            if (\file_exists($target)) {
                continue;
            }

            if ($this->withoutFsWarnings(static fn(): bool => \rename($file, $target))) {
                return $target;
            }
        }

        // The tempnam() file is removed rather than returned: on Windows its
        // truncated prefix would not match the scan in delete() and
        // deleteOlderThan(), so it could never be reclaimed.
        $this->withoutFsWarnings(static fn(): bool => \unlink($file));

        throw new Exception('unable to create a cache file in: ' . $this->path);
    }

    /**
     * Delete cached files
     *
     * A file name is built as prefix + type + key, so a key can only be matched
     * once a type narrows the name, and a key without a type is rejected.
     *
     * @param ?string $type Type of files to delete; null means every type.
     * @param ?string $key  Specific file key to delete; requires $type.
     *
     * @throws Exception when $key is given without $type.
     */
    public function delete(?string $type = null, ?string $key = null): void
    {
        if ($type === null && $key !== null) {
            throw new Exception('a cache key can only be deleted together with its type');
        }

        $safeType = $type !== null ? \preg_replace(self::SAFE_NAME_PATTERN, '', $type) : null;
        $safeKey = $key !== null ? \preg_replace(self::SAFE_NAME_PATTERN, '', $key) : null;

        $prefix = $this->prefix;
        if ($safeType !== null) {
            $prefix .= $safeType . '_';
            if ($safeKey !== null) {
                $prefix .= $safeKey . '_';
            }
        }

        $files = $this->findFiles($prefix);
        if ($files === []) {
            return;
        }

        // A file may vanish between the scan and unlink(); such races are
        // non-fatal, so the warnings are suppressed.
        $this->withoutFsWarnings(static function () use ($files): void {
            foreach ($files as $file) {
                \unlink($file);
            }
        });
    }

    /**
     * Return the cache files whose name starts with the given prefix.
     *
     * The directory is scanned rather than matched with glob(), because the
     * cache path may contain glob metacharacters ('[', '*', '?').
     *
     * @param string $prefix Filename prefix to match.
     *
     * @return list<string> Paths of the matching files.
     */
    private function findFiles(string $prefix): array
    {
        $entries = $this->withoutFsWarnings(fn(): array|false => \scandir($this->path));
        if ($entries === false) {
            return [];
        }

        $files = [];
        foreach ($entries as $entry) {
            if (!\str_starts_with($entry, $prefix)) {
                continue;
            }

            $path = $this->path . $entry;
            // A directory bearing the prefix would fail in unlink() with the
            // warning suppressed.
            if (!\is_file($path)) {
                continue;
            }

            $files[] = $path;
        }

        return $files;
    }

    /**
     * Delete cache files older than the given number of seconds.
     *
     * @param int $seconds Maximum age in seconds; files whose mtime is older are
     *                     removed. Must not be negative.
     *
     * @throws \Com\Tecnick\File\Exception when $seconds is negative.
     */
    public function deleteOlderThan(int $seconds): void
    {
        if ($seconds < 0) {
            throw new Exception('the maximum age must not be negative, got: ' . $seconds);
        }

        $files = $this->findFiles($this->prefix);
        if ($files === []) {
            return;
        }

        $cutoff = \time() - $seconds;

        // filemtime() and unlink() may fail if a file disappears between the
        // scan and the call; such races are non-fatal.
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
     * Draw a hex-encoded random token from the system CSPRNG.
     *
     * The \Random\RandomException raised by random_bytes() on an unavailable
     * CSPRNG is converted to the library exception type.
     *
     * @param int<1, max> $bytes Number of random bytes to draw.
     *
     * @throws \Com\Tecnick\File\Exception when no secure randomness is available.
     */
    protected function randomToken(int $bytes): string
    {
        try {
            return \bin2hex(\random_bytes($bytes));
        } catch (\Random\RandomException $randomException) {
            throw new Exception('unable to obtain secure randomness', 0, $randomException);
        }
    }

    /**
     * Execute a callable while suppressing E_WARNING and E_NOTICE.
     *
     * The wrapped filesystem calls signal failure via their return values.
     * Notices are covered too, because tempnam() reports its fallback to the
     * system temp directory that way.
     *
     * The handler is registered without a level mask and filters internally,
     * so that every other level still reaches the handler installed by the
     * application.
     *
     * @template T
     *
     * @param callable():T $callback
     *
     * @return T
     */
    private function withoutFsWarnings(callable $callback): mixed
    {
        $previous = \set_error_handler(static function (int $errno, string $errstr, string $errfile, int $errline) use (
            &$previous,
        ): bool {
            if (($errno & (E_WARNING | E_NOTICE)) !== 0) {
                return true;
            }

            // Not ours to swallow: hand it back to the handler that was
            // installed, or to PHP's own by returning false.
            if (!\is_callable($previous)) {
                return false;
            }

            return $previous($errno, $errstr, $errfile, $errline) === true;
        });

        try {
            return $callback();
        } finally {
            \restore_error_handler();
        }
    }

    /**
     * Set the K_PATH_CACHE constant (if not set) to the default system directory for temporary files
     *
     * @throws \Com\Tecnick\File\Exception when the system temporary directory cannot be resolved.
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
     * Resolves the path and appends a trailing separator. An unresolvable path
     * is rejected, since the cache files would then be written to and searched
     * for in two different directories.
     *
     * @param string $path Path to normalize
     *
     * @throws \Com\Tecnick\File\Exception when the path cannot be resolved.
     */
    protected function normalizePath(string $path): string
    {
        $rpath = \realpath($path);
        if ($rpath === false) {
            throw new Exception('unable to resolve the cache directory: ' . $path);
        }

        if (!\str_ends_with($rpath, \DIRECTORY_SEPARATOR)) {
            $rpath .= \DIRECTORY_SEPARATOR;
        }

        return $rpath;
    }
}
