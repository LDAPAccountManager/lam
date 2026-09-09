<?php

declare(strict_types=1);

/**
 * File.php
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

use Com\Tecnick\File\Exception as FileException;

/**
 * Com\Tecnick\File\File
 *
 * Local and remote file reads behind host and path allowlists
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
class File
{
    /**
     * Array of default cURL options for curl_setopt_array.
     *
     * @var array<int, bool|int|string> cURL options.
     */
    protected const CURLOPT_DEFAULT = [
        CURLOPT_CONNECTTIMEOUT => 5,
        CURLOPT_MAXREDIRS => 0,
        CURLOPT_PROTOCOLS => CURLPROTO_HTTPS | CURLPROTO_HTTP,
        CURLOPT_REDIR_PROTOCOLS => CURLPROTO_HTTPS | CURLPROTO_HTTP,
        CURLOPT_TIMEOUT => 30,
        CURLOPT_USERAGENT => 'tc-lib-file',
    ];

    /**
     * Array of fixed cURL options for curl_setopt_array.
     *
     * @var array<int, bool|int|string> cURL options.
     */
    protected const CURLOPT_FIXED = [
        CURLOPT_FAILONERROR => true,
        CURLOPT_RETURNTRANSFER => true,
        CURLOPT_SSL_VERIFYHOST => 2,
        CURLOPT_SSL_VERIFYPEER => true,
    ];

    /**
     * Custom cURL options for curl_setopt_array.
     *
     * @var array<int, bool|int|string|callable|resource> cURL options.
     */
    protected array $curlopts = [];

    /**
     * Default cURL options, initialized from the CURLOPT_DEFAULT constant.
     *
     * @var array<int, bool|int|string|callable|resource> cURL options.
     */
    protected array $defaultCurlOpts;

    /**
     * Fixed cURL options, initialized from the CURLOPT_FIXED constant.
     * Applied last, so they cannot be overridden by the other option layers.
     *
     * @var array<int, bool|int|string|callable|resource> cURL options.
     */
    protected array $fixedCurlOpts;

    /**
     * Allowlist of trusted hosts, used to validate URLs and the HTTP_HOST and
     * SCRIPT_URI request metadata. An empty array (the default) trusts no host;
     * '*' trusts every host and disables host validation.
     *
     * Entries are normalized: lowercased, with the trailing root dot removed.
     * An entry takes either of two forms:
     * - 'example.com' constrains the host only and matches it on any port.
     * - 'example.com:8080' names a single origin and matches only that port.
     *   A URL without an explicit port is matched against the scheme default,
     *   so 'example.com:443' matches 'https://example.com/'.
     *
     * An IPv6 host must be written in bracketed form ('[::1]', '[::1]:8080'),
     * and an internationalized domain in its A-label (punycode) form.
     *
     * @var array<string>
     */
    protected array $allowedHosts = [];

    /**
     * Allowlist of trusted local path roots. An empty array (the default)
     * trusts no path; '*' trusts every path and disables path validation.
     *
     * @var array<string>
     */
    protected array $allowedPaths = [];

    /**
     * Maximum size in bytes for remote file reads.
     * A read exceeding this limit throws an exception.
     *
     * @var int
     */
    protected int $maxRemoteSize = 52_428_800;

    /**
     * Override for filesystem path case-sensitivity.
     *
     * null  = auto-detect (Windows: case-insensitive; macOS: per-volume probe
     *         with case-insensitive fallback; Linux: case-sensitive).
     * true  = treat paths as case-sensitive.
     * false = treat paths as case-insensitive.
     *
     * @var bool|null
     */
    protected ?bool $caseSensitiveOverride = null;

    /**
     * Memoized probeCaseInsensitive() results, keyed by containing directory.
     * Only decided probes are stored.
     *
     * @var array<string, bool>
     */
    private array $caseInsensitiveCache = [];

    /**
     * Initialize the File object.
     *
     * @param array<string> $allowedHosts Allowlist of trusted hostnames.
     * @param int $maxRemoteSize Maximum size in bytes for remote file reads. Must be positive.
     * @param array<int, bool|int|string|callable|resource> $curlopts Custom cURL options to merge over defaults.
     * @param array<int, bool|int|string|callable|resource>|null $defaultCurlOpts Override for the default cURL
     *                                                                            options; null uses CURLOPT_DEFAULT.
     * @param array<int, bool|int|string|callable|resource>|null $fixedCurlOpts Override for the fixed cURL
     *                                                                          options; null uses CURLOPT_FIXED.
     * @param array<string> $allowedPaths Allowlist of trusted file paths.
     * @param bool|null $caseSensitivePaths Override for path case-sensitivity:
     *                                      null = auto-detect, true = case-sensitive,
     *                                      false = case-insensitive.
     *
     * @throws FileException when $maxRemoteSize is not positive.
     */
    public function __construct(
        array $allowedHosts = [],
        int $maxRemoteSize = 52_428_800,
        array $curlopts = [],
        ?array $defaultCurlOpts = null,
        ?array $fixedCurlOpts = null,
        array $allowedPaths = [],
        ?bool $caseSensitivePaths = null,
    ) {
        $this->allowedHosts = $this->normalizeAllowedHosts($allowedHosts);
        $this->setMaxRemoteSize($maxRemoteSize);
        $this->curlopts = $curlopts;
        $this->defaultCurlOpts = $defaultCurlOpts ?? self::defaultCurlOptions();
        $this->fixedCurlOpts = $fixedCurlOpts ?? self::CURLOPT_FIXED;
        $this->caseSensitiveOverride = $caseSensitivePaths;
        $this->allowedPaths = $this->normalizeAllowedPaths($allowedPaths);
    }

    /**
     * Return the default cURL options for this libcurl build.
     *
     * CURLOPT_PROTOCOLS and CURLOPT_REDIR_PROTOCOLS are replaced by their
     * string variants when this build defines them.
     *
     * @return array<int, bool|int|string|callable|resource> cURL options.
     */
    protected static function defaultCurlOptions(): array
    {
        $opts = self::CURLOPT_DEFAULT;

        if (\defined('CURLOPT_PROTOCOLS_STR') && \defined('CURLOPT_REDIR_PROTOCOLS_STR')) {
            unset($opts[CURLOPT_PROTOCOLS], $opts[CURLOPT_REDIR_PROTOCOLS]);
            $opts[CURLOPT_PROTOCOLS_STR] = 'http,https';
            $opts[CURLOPT_REDIR_PROTOCOLS_STR] = 'http,https';
        }

        return $opts;
    }

    /**
     * Set custom cURL options.
     *
     * @param array<int, bool|int|string|callable|resource> $curlopts Custom cURL options to merge over defaults.
     */
    public function setCurlOpts(array $curlopts): static
    {
        $this->curlopts = $curlopts;
        return $this;
    }

    /**
     * Set the allowlist of trusted hostnames.
     *
     * @param array<string> $allowedHosts Trusted hostname strings.
     */
    public function setAllowedHosts(array $allowedHosts): static
    {
        $this->allowedHosts = $this->normalizeAllowedHosts($allowedHosts);
        return $this;
    }

    /**
     * Set the allowlist of trusted file paths.
     *
     * @param array<string> $allowedPaths Trusted file path strings.
     */
    public function setAllowedPaths(array $allowedPaths): static
    {
        $this->allowedPaths = $this->normalizeAllowedPaths($allowedPaths);
        return $this;
    }

    /**
     * Override how path case-sensitivity is determined for allowlist matching.
     *
     * @param bool|null $caseSensitivePaths null = auto-detect per platform/volume,
     *                                      true = case-sensitive, false = case-insensitive.
     */
    public function setCaseSensitivePaths(?bool $caseSensitivePaths): static
    {
        $this->caseSensitiveOverride = $caseSensitivePaths;
        return $this;
    }

    /**
     * Set the maximum size (in bytes) for remote file reads.
     *
     * @param int $maxRemoteSize Maximum allowed bytes; must be positive.
     *
     * @throws FileException when $maxRemoteSize is not positive.
     */
    public function setMaxRemoteSize(int $maxRemoteSize): static
    {
        if ($maxRemoteSize < 1) {
            throw new FileException('the maximum remote size must be positive, got: ' . $maxRemoteSize);
        }

        $this->maxRemoteSize = $maxRemoteSize;
        return $this;
    }

    /**
     * Get the maximum size (in bytes) for remote file reads.
     */
    public function getMaxRemoteSize(): int
    {
        return $this->maxRemoteSize;
    }

    /**
     * Wrapper to use fopen only with local files.
     *
     * @param string $file Name of the file to open.
     * @param string $mode Type of access required to the stream.
     *                     The binary flag ('b') is added when absent.
     *
     * @return resource Returns a file pointer resource on success.
     *
     * @throws FileException in case of error.
     */
    public function fopenLocal(string $file, string $mode): mixed
    {
        if (!$this->isValidFile($file)) {
            throw new FileException('invalid file');
        }

        if (!\str_contains($mode, 'b')) {
            $mode .= 'b';
        }

        $path = $this->resolveValidatedPath($file);
        $handler = $this->withoutPhpWarnings(static fn() => \fopen($path, $mode));
        if ($handler === false) {
            throw new FileException('unable to open the file: ' . $path);
        }

        return $handler;
    }

    /**
     * Read a 4-byte (32 bit) integer from file.
     *
     * @param resource $resource A file system pointer resource that is typically created using \fopen().
     *
     * @return int 4-byte integer.
     *
     * @throws FileException in case of error.
     */
    public function fReadInt(mixed $resource): int
    {
        // rfRead() drains a stream that delivers fewer than 4 bytes per fread().
        $data = $this->rfRead($resource, 4);
        if (\strlen($data) < 4) {
            throw new FileException('unable to read the file');
        }

        $val = \unpack('Ni', $data);
        return $val !== false ? (int) ($val['i'] ?? 0) : 0;
    }

    /**
     * Binary-safe file read.
     * Reads up to length bytes from the file pointer referenced by handle.
     * Reading stops as soon as one of the following conditions is met:
     * length bytes have been read; EOF (end of file) is reached.
     *
     * @param ?resource $resource A file system pointer resource that is typically created using \fopen().
     * @param int       $length   Number of bytes to read; must be positive.
     *
     * @throws FileException when $length is not positive, or in case of a read error.
     */
    public function rfRead(mixed $resource, int $length): string
    {
        if ($length < 1) {
            throw new FileException('the number of bytes to read must be positive, got: ' . $length);
        }

        if (!\is_resource($resource)) {
            throw new FileException('unable to read the file');
        }

        $data = '';
        while (\strlen($data) < $length && !\feof($resource)) {
            $remaining = $length - \strlen($data);
            // fread() warns on an unreadable handle; the false return is handled below.
            $chunk = $this->withoutPhpWarnings(static fn() => \fread($resource, $remaining));
            if ($chunk === false || $chunk === '') {
                break;
            }

            $data .= $chunk;
        }

        if ($data === '') {
            throw new FileException('unable to read the file');
        }

        return $data;
    }

    /**
     * Reads entire file into a string.
     * The file can be also an URL.
     *
     * @param string $file Name of the file or URL to read.
     *
     * @throws FileException in case of error.
     */
    public function fileGetContents(string $file): string
    {
        $alt = $this->getAltFilePaths($file);
        foreach ($alt as $path) {
            $ret = $this->getFileData($path);
            if ($ret !== false) {
                return $ret;
            }
        }

        throw new FileException('unable to read the file: ' . $file);
    }

    /**
     * Reads entire file into a string.
     * The file can be also an URL.
     *
     * @param string $file Name of the file or URL to read.
     *
     * @return string|false File content or FALSE in case the file is unreadable
     *
     * @throws FileException in case the remote transfer is aborted due to max size.
     */
    public function getFileData(string $file): string|false
    {
        $data = $this->getLocalFileData($file);

        if ($data === false) {
            return $this->getUrlData($file);
        }

        return $data;
    }

    /**
     * Reads entire local file into a string.
     *
     * @param string $file Name of the file to read.
     *
     * @return string|false File content, or FALSE when the path is not
     *                      allowlisted, not a valid local path, or unreadable.
     */
    public function getLocalFileData(string $file): string|false
    {
        if (!$this->isValidFile($file)) {
            return false;
        }

        $path = $this->resolveValidatedPath($file);
        return $this->withoutPhpWarnings(static fn() => \file_get_contents($path));
    }

    /**
     * Return the canonical path to open for an already-validated local file.
     *
     * Falls back to the plain path when it does not resolve, which is the case
     * isValidFile() validates through the nearest existing ancestor.
     *
     * @param string $file Local file reference, optionally 'file://'-prefixed.
     */
    protected function resolveValidatedPath(string $file): string
    {
        $path = $this->stripFileScheme($file);
        $realPath = \realpath($path);

        return $realPath !== false ? $realPath : $path;
    }

    /**
     * Return the plain filesystem path for a validated local file reference.
     *
     * The 'file://' scheme added by isValidFile() is removed and the result is
     * trimmed, so the path matches exactly the value isValidFile() validated.
     *
     * @param string $file Local file reference, optionally 'file://'-prefixed.
     */
    protected function stripFileScheme(string $file): string
    {
        return \trim(\str_starts_with($file, 'file://') ? \substr($file, 7) : $file);
    }

    /**
     * Execute a callable while suppressing E_WARNING and E_NOTICE.
     *
     * The wrapped filesystem calls signal failure via their return values,
     * which the public methods convert into a FileException.
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
    private function withoutPhpWarnings(callable $callback): mixed
    {
        $previous = \set_error_handler(static function (int $errno, string $errstr, string $errfile, int $errline) use (
            &$previous,
        ): bool {
            if (($errno & (E_WARNING | E_NOTICE)) !== 0) {
                return true;
            }

            // Hand any other level back to the previously installed handler,
            // or to PHP's own by returning false.
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
     * Progress callback factory rejecting a response that declares a size over
     * the limit, before any of its body is read.
     *
     * The limit itself is enforced by createWriteCallback().
     *
     * @param bool $declaredOversize Flag set when the declared size is over the limit.
     *
     * @return callable Progress callback for CURLOPT_PROGRESSFUNCTION.
     */
    private function createDeclaredSizeCallback(bool &$declaredOversize): callable
    {
        $maxSize = $this->maxRemoteSize;
        return static function ($_curlResource, $downloadSize, $_downloaded, $_uploadSize, $_uploaded) use (
            &$declaredOversize,
            $maxSize,
        ): int {
            // @phpstan-ignore-next-line
            if ((int) $downloadSize > $maxSize) {
                $declaredOversize = true;
                // Returning non-zero aborts the transfer.
                return 1;
            }

            return 0;
        };
    }

    /**
     * Write callback factory enforcing the maximum remote size.
     *
     * The callback receives the bytes that are about to be buffered, after any
     * content decoding and whatever the transfer encoding, which is the
     * quantity $maxRemoteSize bounds.
     *
     * @param string $body      Accumulated response body.
     * @param int    $bytesRead Reference to track the bytes buffered.
     * @param bool   $oversize  Flag set when the limit would be exceeded.
     *
     * @return callable Write callback for CURLOPT_WRITEFUNCTION.
     */
    private function createWriteCallback(string &$body, int &$bytesRead, bool &$oversize): callable
    {
        $maxSize = $this->maxRemoteSize;
        return static function ($_curlResource, string $chunk) use (&$body, &$bytesRead, &$oversize, $maxSize): int {
            $length = \strlen($chunk);
            // For an empty chunk, reporting 0 written is success rather than
            // the abort signal below.
            if ($length === 0) {
                return 0;
            }

            if (($bytesRead + $length) > $maxSize) {
                $oversize = true;
                // Reporting fewer bytes written than received aborts the
                // transfer before this chunk reaches the buffer.
                return 0;
            }

            $bytesRead += $length;
            $body .= $chunk;

            return $length;
        };
    }

    /**
     * Build an absolute URL from a redirect Location header value.
     *
     * Supports absolute, scheme-relative, root-relative and relative
     * redirect targets.
     *
     * @param string $location Redirect target from Location header.
     * @param string $baseUrl  Effective URL of the current response.
     *
     * @return string|false Absolute HTTP(S) URL or false when invalid.
     */
    private function buildRedirectUrl(string $location, string $baseUrl): string|false
    {
        $location = \trim($location);
        if ($location === '') {
            return false;
        }

        if (\preg_match('%^https?://%i', $location) === 1) {
            return $location;
        }

        $base = \parse_url($baseUrl);
        if (!\is_array($base)) {
            return false;
        }

        $scheme = $base['scheme'] ?? null;
        $host = $base['host'] ?? null;
        if (!\is_string($scheme) || !\is_string($host) || $scheme !== 'http' && $scheme !== 'https') {
            return false;
        }

        $authority = $scheme . '://' . $host;
        $port = $base['port'] ?? null;
        if (\is_int($port)) {
            $authority .= ':' . $port;
        }

        if (\str_starts_with($location, '//')) {
            return $scheme . ':' . $location;
        }

        if ($location[0] === '/') {
            return $authority . $location;
        }

        $basePath = $base['path'] ?? '/';

        $dir = (string) \preg_replace('%/[^/]*$%', '/', $basePath);
        if ($dir === '') {
            $dir = '/';
        }

        return $authority . $dir . $location;
    }

    /**
     * Build a cURL header callback that validates each redirect target URL.
     *
     * The callback aborts the transfer when a Location header resolves to an
     * invalid or non-allowlisted URL.
     *
     * @param bool   $invalidRedirect Flag set to true when a redirect is invalid.
     * @param string $initialUrl      Initial request URL.
     *
     * @return callable Header callback for CURLOPT_HEADERFUNCTION.
     */
    private function createRedirectValidationCallback(bool &$invalidRedirect, string $initialUrl): callable
    {
        return function ($curlResource, string $headerLine) use (&$invalidRedirect, $initialUrl): int {
            if (\stripos($headerLine, 'Location:') !== 0) {
                return \strlen($headerLine);
            }

            if (!$curlResource instanceof \CurlHandle) {
                $invalidRedirect = true;
                return 0;
            }

            // libcurl only follows Location on a 3xx response; on any other
            // status the header names no hop that will be taken.
            if (!$this->isRedirectStatus($curlResource)) {
                return \strlen($headerLine);
            }

            $location = \trim(\substr($headerLine, 9));
            if ($location === '') {
                $invalidRedirect = true;
                return 0;
            }

            $effectiveUrl = (string) \curl_getinfo($curlResource, CURLINFO_EFFECTIVE_URL);
            $baseUrl = $effectiveUrl !== '' ? $effectiveUrl : $initialUrl;

            $redirectUrl = $this->buildRedirectUrl($location, $baseUrl);
            if ($redirectUrl === false || !$this->isValidURL($redirectUrl)) {
                $invalidRedirect = true;
                return 0;
            }

            return \strlen($headerLine);
        };
    }

    /**
     * Reads entire remote file into a string using CURL.
     *
     * The cURL path is always used, independently of the allow_url_fopen ini setting.
     *
     * The response is buffered in memory, so a transfer costs up to
     * $maxRemoteSize bytes of PHP memory. The limit is enforced by a write
     * callback that counts the bytes as they are buffered.
     *
     * @param string $url URL to read.
     *
     * @return string|false Remote content, or FALSE when the URL is not
     *                      allowlisted, the response is an unfollowed redirect,
     *                      or the transfer fails.
     *
     * @throws FileException if the remote transfer is aborted due to max size,
     *                       or a configured cURL option is not valid or cannot
     *                       be applied.
     */
    public function getUrlData(string $url): string|false
    {
        // isValidURL() restricts the scheme to http/https.
        if (!$this->isValidURL($url)) {
            return false;
        }

        // try to get remote file data using cURL
        // The false arm narrows $curlHandle to \CurlHandle; ext-curl is required.
        $curlHandle = \curl_init();
        if ($curlHandle === false) {
            return false;
        }

        $curlopts = [];

        $openBasedir = \ini_get('open_basedir');
        if ($openBasedir === false || $openBasedir === '') {
            $curlopts[CURLOPT_FOLLOWLOCATION] = true;
        }

        $curlopts = $this->mergeCurlOptions($curlopts);
        $curlopts[CURLOPT_URL] = $url;

        // The post-merge value is the one libcurl acts on.
        $followLocation = (bool) ($curlopts[CURLOPT_FOLLOWLOCATION] ?? false);

        // Reject a response that declares a size over the limit before any of
        // its body is buffered.
        $declaredOversize = false;
        $curlopts[CURLOPT_NOPROGRESS] = false;
        $declaredSizeCallback = $this->createDeclaredSizeCallback($declaredOversize);
        $curlopts[CURLOPT_PROGRESSFUNCTION] = $declaredSizeCallback;

        // libcurl prefers CURLOPT_XFERINFOFUNCTION when both are set, so it is
        // pinned to the same callback.
        if (\defined('CURLOPT_XFERINFOFUNCTION')) {
            $curlopts[CURLOPT_XFERINFOFUNCTION] = $declaredSizeCallback;
        }

        // Buffer the body here rather than through CURLOPT_RETURNTRANSFER, so
        // that the limit bounds the bytes reaching PHP memory. Assigned after
        // the merge, so it supersedes a caller-supplied write callback.
        $body = '';
        $bytesRead = 0;
        $oversize = false;
        $curlopts[CURLOPT_WRITEFUNCTION] = $this->createWriteCallback($body, $bytesRead, $oversize);

        $invalidRedirect = false;
        $maxRedirects = (int) ($curlopts[CURLOPT_MAXREDIRS] ?? 0);
        if ($maxRedirects !== 0) {
            $curlopts[CURLOPT_HEADERFUNCTION] = $this->createRedirectValidationCallback($invalidRedirect, $url);
        }

        // curl_setopt_array() raises a ValueError for an unrecognized option
        // name and returns false for a rejected value, applying none of the
        // remaining options. Both become the library exception.
        try {
            $optionsApplied = \curl_setopt_array($curlHandle, $curlopts);

            // @mago-expect analysis:avoid-catching-error -- converted to the library exception type.
        } catch (\ValueError $valueError) {
            throw new FileException('invalid cURL option: ' . $valueError->getMessage(), 0, $valueError);
        }

        if (!$optionsApplied) {
            throw new FileException('unable to apply the cURL options: one of them was rejected by this libcurl build');
        }

        $ret = \curl_exec($curlHandle);

        // Checked ahead of the size guards so that a rejected redirect is
        // always reported as an unreadable URL rather than as an oversize
        // transfer.
        if ($invalidRedirect) {
            return false;
        }

        // The transfer was aborted by one of the two size guards; each reports
        // which one acted.
        if ($declaredOversize) {
            throw new FileException(
                'remote file exceeds maximum allowed size of '
                . $this->maxRemoteSize
                . ' bytes (rejected before reading the body: the response declared a larger size)',
            );
        }

        if ($oversize) {
            throw new FileException(
                'remote file exceeds maximum allowed size of '
                . $this->maxRemoteSize
                . ' bytes (aborted after '
                . $bytesRead
                . ' bytes)',
            );
        }

        if ($ret === false) {
            return false;
        }

        // A redirect that libcurl was not asked to follow completes with
        // CURLE_OK, so its 3xx body is rejected here instead of being returned
        // as the file content.
        if (!$followLocation && $this->isRedirectStatus($curlHandle)) {
            return false;
        }

        // The body was accumulated by the write callback, so curl_exec()'s own
        // return value carries no content. The CurlHandle is released
        // automatically when it goes out of scope.
        return $body;
    }

    /**
     * Tell whether the completed transfer ended on a 3xx redirect response.
     *
     * @param \CurlHandle $curlHandle Handle of the completed transfer.
     */
    protected function isRedirectStatus(\CurlHandle $curlHandle): bool
    {
        $code = (int) \curl_getinfo($curlHandle, CURLINFO_HTTP_CODE);

        return $code >= 300 && $code < 400;
    }

    /**
     * Merge the cURL option layers in precedence order.
     *
     * Request options are overlaid with the instance defaults, then the caller's
     * custom options, then the fixed options. Fixed comes last so that
     * security-critical settings (TLS verification, RETURNTRANSFER, FAILONERROR)
     * cannot be disabled through setCurlOpts().
     *
     * @param array<int, bool|int|string|callable|resource> $curlopts Options computed for this request.
     *
     * @return array<int, bool|int|string|callable|resource> Merged cURL options.
     */
    protected function mergeCurlOptions(array $curlopts): array
    {
        $curlopts = \array_replace($curlopts, $this->defaultCurlOpts);
        $curlopts = \array_replace($curlopts, $this->curlopts);

        return \array_replace($curlopts, $this->fixedCurlOpts);
    }

    /**
     * Returns an array of possible alternative file paths or URLs
     *
     * @param string $file Name of the file or URL to read.
     *
     * @return list<string> List of possible alternative file paths or URLs.
     */
    public function getAltFilePaths(string $file): array
    {
        $alt = [$file];
        $alt[] = $this->getAltLocalUrlPath($file);
        $url = $this->getAltMissingUrlProtocol($file);
        $alt[] = $url;
        $alt[] = $this->getAltPathFromUrl($url);
        $alt[] = $this->getAltUrlFromPath($file);
        // Empty candidates are dropped and the result is re-indexed to the
        // 0-based list promised by the return type.
        return \array_values(\array_unique(\array_filter($alt, static fn(string $path): bool => $path !== '')));
    }

    /**
     * Resolve a local file path against explicit base directories.
     *
     * Turns an existing local relative path into an absolute canonical path
     * when one of the provided base directories matches. No trust boundary is
     * checked and no file is read, so the result must be passed through
     * isValidFile() or isAllowedFile() before it is handed to any reader.
     *
     * @param string        $file     Local file path to resolve.
     * @param array<string> $baseDirs Candidate base directories checked in order.
     */
    public function resolveLocalPath(string $file, array $baseDirs = []): string
    {
        $file = \trim($file);
        // A NUL byte makes every path function throw a ValueError, so such a
        // path is returned unresolved.
        if ($file === '' || \str_contains($file, "\0") || $this->hasDoubleDots($file) || \str_contains($file, '://')) {
            return $file;
        }

        $resolved = \realpath($file);
        if (\is_string($resolved) && $resolved !== '') {
            return $resolved;
        }

        foreach ($baseDirs as $baseDir) {
            if ($baseDir === '') {
                continue;
            }

            $resolvedBase = \realpath($baseDir);
            if (!\is_string($resolvedBase) || $resolvedBase === '') {
                continue;
            }

            $resolved = \realpath($resolvedBase . \DIRECTORY_SEPARATOR . $file);
            if (\is_string($resolved) && $resolved !== '') {
                return $resolved;
            }
        }

        return $file;
    }

    /**
     * Replace URL relative path with full real server path
     *
     * @param string $file Relative URL path
     */
    protected function getAltLocalUrlPath(string $file): string
    {
        $documentRoot = $_SERVER['DOCUMENT_ROOT'] ?? null;
        if (
            \strlen($file) > 1
            && $file[0] === '/'
            && $file[1] !== '/'
            && \is_string($documentRoot)
            && $documentRoot !== '/'
        ) {
            $findroot = \strpos($file, $documentRoot);
            if ($findroot === false || $findroot > 1) {
                $file = $this->normalizeLocalSeparators(\htmlspecialchars_decode(\urldecode($documentRoot . $file)));
            }
        }

        return $file;
    }

    /**
     * Collapse mixed directory separators to '/' in a constructed local path.
     *
     * Joining a Windows DOCUMENT_ROOT with a URL-style path yields mixed
     * 'C:\inetpub\wwwroot/path' forms, which PHP accepts written with forward
     * slashes only. No-op on POSIX systems.
     *
     * @param string $path Constructed local path.
     */
    protected function normalizeLocalSeparators(string $path): string
    {
        return \str_replace('\\', '/', $path);
    }

    /**
     * Add missing local URL protocol.
     *
     * @param string $file Relative URL path
     *
     * @return string local path or original $file
     */
    protected function getAltMissingUrlProtocol(string $file): string
    {
        $httpHost = $_SERVER['HTTP_HOST'] ?? null;
        if (\preg_match('%^//%', $file) && \is_string($httpHost) && $this->isValidHost($httpHost)) {
            $file = $this->getDefaultUrlProtocol() . ':' . \str_replace(' ', '%20', $file);
        }

        return \htmlspecialchars_decode($file);
    }

    /**
     * Get the default URL protocol (http or https).
     */
    protected function getDefaultUrlProtocol(): string
    {
        $protocol = 'http';
        $https = $_SERVER['HTTPS'] ?? null;
        if (\is_string($https) && $https !== '' && \strtolower($https) !== 'off') {
            $protocol .= 's';
        }

        return $protocol;
    }

    /**
     * Get the local server path corresponding to a URL.
     *
     * @param string $url Absolute URL to convert to a local path
     *
     * @return string local path or original $url
     */
    protected function getAltPathFromUrl(string $url): string
    {
        $httpHost = $_SERVER['HTTP_HOST'] ?? null;
        $documentRoot = $_SERVER['DOCUMENT_ROOT'] ?? null;

        if (
            \preg_match('%^(https?)://%', $url) !== 1
            || !\is_string($httpHost)
            || !$this->isValidHost($httpHost)
            || !\is_string($documentRoot)
        ) {
            return $url;
        }

        $urldata = \parse_url($url);
        if (\is_array($urldata) && \array_key_exists('query', $urldata)) {
            return $url;
        }

        $host = $this->getDefaultUrlProtocol() . '://' . $httpHost;
        if (\str_starts_with($url, $host)) {
            // Convert URL to full server path, replacing the leading origin only.
            $tmp = $documentRoot . \substr($url, \strlen($host));
            return $this->normalizeLocalSeparators(\htmlspecialchars_decode(\urldecode($tmp)));
        }

        return $url;
    }

    /**
     * Get an alternate URL from a file path, built on the SCRIPT_URI origin.
     *
     * An empty $file is returned unchanged, since the resulting URL would name
     * the site root rather than a file.
     *
     * @param string $file File name and path
     */
    protected function getAltUrlFromPath(string $file): string
    {
        $scriptUri = $_SERVER['SCRIPT_URI'] ?? null;
        if (
            $file !== ''
            && \is_string($scriptUri)
            && $scriptUri !== ''
            && \preg_match('%^(https?)://%', $file) !== 1
            && \preg_match('%^//%', $file) !== 1
        ) {
            $urldata = \parse_url($scriptUri);
            if (
                !\is_array($urldata)
                || !\array_key_exists('scheme', $urldata)
                || !\array_key_exists('host', $urldata)
            ) {
                return $file;
            }

            // An untrusted SCRIPT_URI host leaves the file path unchanged.
            if (!$this->isValidHost($urldata['host'])) {
                return $file;
            }

            // The port is carried over, since it is part of the origin.
            $port = $urldata['port'] ?? null;
            $authority = $urldata['host'] . (\is_int($port) ? ':' . $port : '');

            return $urldata['scheme'] . '://' . $authority . ($file[0] === '/' ? '' : '/') . $file;
        }

        return $file;
    }

    /**
     * Validate an HTTP(S) URL against the configured host allowlist.
     *
     * Returns true only when the URL parses correctly, uses the http or https
     * scheme, and has a non-empty host trusted by isValidUrlHost().
     *
     * $url is passed by reference and is replaced with its trimmed form, so it
     * must be a variable. Use isAllowedUrl() to validate a literal or any other
     * expression.
     *
     * @param string $url URL to validate.
     */
    public function isValidURL(string &$url): bool
    {
        $url = \trim($url);
        // parse_url() tolerates C0 controls and DEL inside a URL; they enable
        // response splitting when a validated URL is emitted into a header.
        if ($url === '' || \preg_match('/[\x00-\x1F\x7F]/', $url) === 1) {
            return false;
        }

        $parts = \parse_url($url);
        if (!\is_array($parts)) {
            return false;
        }

        $scheme = $parts['scheme'] ?? '';
        if ($scheme !== 'http' && $scheme !== 'https') {
            return false;
        }

        $host = $parts['host'] ?? null;
        if (!\is_string($host)) {
            return false;
        }

        // parse_url() reports no port for a URL that uses the scheme default,
        // so the default is filled in and matched against the allowlist.
        $port = $parts['port'] ?? null;
        $defaultPort = match ($scheme) {
            'https' => 443,
            default => 80,
        };

        return $this->isValidUrlHost($host, \is_int($port) ? $port : $defaultPort);
    }

    /**
     * Validate the host and port of a URL against the $allowedHosts allowlist.
     *
     * An entry without a port ('example.com') constrains the host only and
     * accepts any port. An entry that carries one ('example.com:8080') names a
     * single origin and matches only that port.
     *
     * @param string $host Host component of the URL.
     * @param int    $port Port of the URL, or the scheme default.
     */
    protected function isValidUrlHost(string $host, int $port): bool
    {
        $host = $this->normalizeHost($host);
        if ($host === '') {
            return false;
        }

        if (\in_array('*', $this->allowedHosts, true)) {
            return true;
        }

        return \in_array($host, $this->allowedHosts, true) || \in_array($host . ':' . $port, $this->allowedHosts, true);
    }

    /**
     * Validate an HTTP(S) URL against the configured host allowlist.
     *
     * By-value counterpart of isValidURL(), callable with a literal or any
     * other expression and leaving the argument untouched.
     *
     * @param string $url URL to validate.
     */
    public function isAllowedUrl(string $url): bool
    {
        return $this->isValidURL($url);
    }

    /**
     * Validate a local file path against the configured $allowedPaths allowlist.
     *
     * By-value counterpart of isValidFile(), callable with a literal or any
     * other expression and leaving the argument untouched.
     *
     * @param string $file File path to validate.
     */
    public function isAllowedFile(string $file): bool
    {
        return $this->isValidFile($file);
    }

    /**
     * Validate that the given hostname appears in the $allowedHosts allowlist.
     * Returns true when the hostname is trusted, false otherwise.
     * When the allowlist is empty (the default) every host is rejected.
     *
     * @param string $host Hostname to validate (e.g. value of $_SERVER['HTTP_HOST']).
     */
    protected function isValidHost(string $host): bool
    {
        $host = $this->normalizeHost($host);

        return (
            $host !== ''
            && (\in_array('*', $this->allowedHosts, true) || \in_array($host, $this->allowedHosts, true))
        );
    }

    /**
     * Normalize trusted hostnames once at assignment time.
     *
     * @param array<string> $allowedHosts
     *
     * @return array<string>
     */
    protected function normalizeAllowedHosts(array $allowedHosts): array
    {
        $normalized = [];
        foreach ($allowedHosts as $allowedHost) {
            $host = $this->normalizeHost($allowedHost);
            // An empty entry never matches, so it is not stored.
            if ($host !== '') {
                $normalized[] = $host;
            }
        }

        return \array_values(\array_unique($normalized));
    }

    /**
     * Normalize a hostname for allowlist comparison.
     *
     * The name is lowercased (hostnames are case-insensitive per RFC 4343) and
     * the trailing root dot is removed.
     *
     * @param string $host Hostname to normalize.
     */
    protected function normalizeHost(string $host): string
    {
        $host = \strtolower(\trim($host));

        return $host === '*' ? '*' : \rtrim($host, '.');
    }

    /**
     * Check whether a path is inside at least one allowed root.
     *
     * A match requires the exact root or a root followed by a directory
     * separator, so '/var/www_evil' does not match the root '/var/www'.
     *
     * @param string        $path  Path to validate.
     * @param array<string> $roots Allowed path prefixes.
     */
    protected function isPathWithinAllowedRoots(string $path, array $roots): bool
    {
        $path = $this->normalizePathForComparison($path);
        $caseInsensitive = $this->caseInsensitiveFs($path);
        $cmpPath = $caseInsensitive ? $this->foldCase($path) : $path;

        foreach ($roots as $allowedPath) {
            if ($allowedPath === '') {
                continue;
            }

            $root = \rtrim($allowedPath, '/');
            if ($root === '') {
                continue;
            }

            $cmpRoot = $caseInsensitive ? $this->foldCase($root) : $root;
            if ($cmpPath === $cmpRoot || \str_starts_with($cmpPath, $cmpRoot . '/')) {
                return true;
            }
        }

        return false;
    }

    /**
     * Fold a path to lower case for case-insensitive comparison.
     *
     * ext-mbstring folds the whole of Unicode; without it the folding is
     * ASCII-only, since strtolower() is ASCII-only since PHP 8.2.
     *
     * @param string $str Path to fold.
     */
    protected function foldCase(string $str): string
    {
        if (\function_exists('mb_strtolower')) {
            return \mb_strtolower($str, 'UTF-8');
        }

        return \strtolower($str);
    }

    /**
     * Decide whether filesystem path comparison should be case-insensitive.
     *
     * Honors the explicit override when set; otherwise auto-detects from the
     * platform and (on macOS) the specific volume of $hint.
     *
     * @param string $hint A path on the volume being compared, used to probe.
     */
    protected function caseInsensitiveFs(string $hint): bool
    {
        if ($this->caseSensitiveOverride !== null) {
            return !$this->caseSensitiveOverride;
        }

        return $this->caseInsensitiveDefault(\PHP_OS_FAMILY, $hint);
    }

    /**
     * Map an OS family to a default case-sensitivity decision.
     *
     * @param string $osFamily Value of PHP_OS_FAMILY.
     * @param string $hint     A path on the volume being compared, used to probe.
     */
    protected function caseInsensitiveDefault(string $osFamily, string $hint): bool
    {
        return match ($osFamily) {
            'Windows' => true,
            'Darwin' => $this->probeCaseInsensitive($hint) ?? true,
            default => $this->probeCaseInsensitive($hint) ?? false,
        };
    }

    /**
     * Probe whether the volume holding $hint is case-insensitive.
     *
     * Toggles the case of the last ASCII letter of the resolved path and checks
     * whether the variant resolves to the same canonical path. Returns null
     * when the path cannot be resolved or has no letter to toggle, so that the
     * caller applies its platform default.
     *
     * Decided results are memoized per containing directory.
     *
     * @param string $hint Path to probe.
     */
    protected function probeCaseInsensitive(string $hint): ?bool
    {
        $cacheKey = \dirname($hint);
        if (\array_key_exists($cacheKey, $this->caseInsensitiveCache)) {
            return $this->caseInsensitiveCache[$cacheKey];
        }

        $result = $this->detectCaseInsensitive($hint);

        // An undecidable probe says nothing about the volume, so it is not
        // cached and a later path in the same directory is probed again.
        if ($result !== null) {
            $this->caseInsensitiveCache[$cacheKey] = $result;
        }

        return $result;
    }

    /**
     * Run the uncached case-sensitivity probe for $hint.
     *
     * @param string $hint Path to probe.
     */
    private function detectCaseInsensitive(string $hint): ?bool
    {
        $ref = \realpath($hint);
        if ($ref === false) {
            return null;
        }

        $flipped = $this->flipLastAlphaCase($ref);
        if ($flipped === $ref) {
            return null;
        }

        $flippedReal = \realpath($flipped);
        return $flippedReal !== false && $flippedReal === $ref;
    }

    /**
     * Return $str with the case of its last ASCII letter toggled.
     * Returns the string unchanged when it contains no ASCII letter.
     *
     * @param string $str Input string.
     */
    protected function flipLastAlphaCase(string $str): string
    {
        for ($i = \strlen($str) - 1; $i >= 0; $i--) {
            $chr = $str[$i];
            if ($chr >= 'a' && $chr <= 'z') {
                return \substr($str, 0, $i) . \strtoupper($chr) . \substr($str, $i + 1);
            }

            if ($chr >= 'A' && $chr <= 'Z') {
                return \substr($str, 0, $i) . \strtolower($chr) . \substr($str, $i + 1);
            }
        }

        return $str;
    }

    /**
     * Normalize a string to Unicode NFC form when ext-intl is available.
     *
     * Default macOS volumes are normalization-insensitive, so an NFC path and
     * its NFD form name the same file. Without the Normalizer class this is a
     * no-op.
     *
     * @param string $str Input string.
     */
    protected function normalizeUnicode(string $str): string
    {
        if (\class_exists(\Normalizer::class)) {
            $normalized = \Normalizer::normalize($str, \Normalizer::FORM_C);
            if (\is_string($normalized)) {
                return $normalized;
            }
        }

        return $str;
    }

    /**
     * Normalize trusted path roots once at assignment time.
     *
     * Each root is kept in both its literal and its canonical form, so that a
     * root traversing a symlink still matches the resolved candidate path that
     * isValidFile() compares against it.
     *
     * @param array<string> $allowedPaths
     *
     * @return array<string>
     */
    protected function normalizeAllowedPaths(array $allowedPaths): array
    {
        $normalized = [];
        foreach ($allowedPaths as $allowedPath) {
            // '*' passes through unchanged and reaches the list intact.
            $path = \rtrim($this->normalizePathForComparison($allowedPath), '/');
            if ($path === '') {
                continue;
            }

            $normalized[] = $path;

            $realPath = \realpath($allowedPath);
            if ($realPath === false) {
                continue;
            }

            $canonical = \rtrim($this->normalizePathForComparison($realPath), '/');
            if ($canonical !== '') {
                $normalized[] = $canonical;
            }
        }

        return \array_values(\array_unique($normalized));
    }

    /**
     * Validate a local file path against the configured $allowedPaths allowlist.
     *
     * Returns true when:
     * - wildcard trust ('*') is enabled, or
     * - the normalized local path starts with one trusted allowlist prefix.
     *
     * Returns false for parent-directory traversal patterns ('..'),
     * non-file schemes, and when no allowlist entry matches.
     * When the allowlist is empty (default), every path is rejected.
     *
     * The 'file://' schema is added to the input $file parameter if missing.
     *
     * @param string $file File path to validate.
     */
    public function isValidFile(string &$file): bool
    {
        $file = \trim($file);
        // A NUL byte makes every path function throw a ValueError, so such a
        // path is reported as invalid.
        if ($file === '' || \str_contains($file, "\0") || $this->hasDoubleDots($file)) {
            return false;
        }

        if (!\str_contains($file, '://')) {
            $file = 'file://' . $file;
        }

        if (!\str_starts_with($file, 'file://')) {
            return false;
        }

        // remove 'file://' schema
        $filepath = \trim(\substr($file, 7));

        if ($filepath === '') {
            return false;
        }

        // A validated local path must not act as a PHP stream wrapper. A
        // leading Windows drive designator ('C:\dir', 'C:/dir') is exempted;
        // any other leading 'name:' is rejected as a wrapper prefix.
        if (
            \str_contains($filepath, '://')
            || \preg_match('#^[A-Za-z]:[\\\\/]#', $filepath) !== 1
            && \preg_match('#^[a-zA-Z][a-zA-Z0-9+.\-]*:#', $filepath) === 1
        ) {
            return false;
        }

        if (\in_array('*', $this->allowedPaths, true)) {
            return true;
        }

        if (!$this->isPathWithinAllowedRoots($filepath, $this->allowedPaths)) {
            return false;
        }

        // Canonical-path check blocks symlink escapes from trusted roots.
        // For non-existing targets, walk up to the nearest existing ancestor
        // and validate its canonical path.
        $realPathToCheck = $filepath;
        while (\realpath($realPathToCheck) === false) {
            $parentPath = \dirname($realPathToCheck);
            if ($parentPath === $realPathToCheck || $parentPath === '.') {
                return false;
            }

            $realPathToCheck = $parentPath;
        }

        $realPath = \realpath($realPathToCheck);
        if ($realPath === false || !$this->isPathWithinAllowedRoots($realPath, $this->allowedPaths)) {
            return false;
        }

        return true;
    }

    /**
     * Check whether the path contains a parent-directory segment ('..').
     *
     * The test is per segment, so a filename that merely contains two
     * consecutive dots ('report..2024.txt') is valid. Percent-encoded dots and
     * separators and their HTML-entity equivalents are decoded first, and a
     * leading Windows drive designator is stripped, so that 'C:..\file' splits
     * like '..\file'.
     *
     * @param string $path path to check
     *
     * @return bool true if the path contains a parent-directory segment
     */
    protected function hasDoubleDots(string $path): bool
    {
        // ENT_HTML5 covers '&period;' and '&sol;', which ENT_HTML401 does not.
        $decoded = \str_ireplace(
            ['%2E', '%2F', '%5C'],
            ['.', '/', '/'],
            \html_entity_decode($path, ENT_QUOTES | ENT_HTML5, 'UTF-8'),
        );

        $decoded = (string) \preg_replace('/^[A-Za-z]:/', '', $decoded);

        return \in_array('..', \explode('/', \str_replace('\\', '/', $decoded)), true);
    }

    /**
     * Collapse repeated '/' separators and '.' segments in a slash-separated path.
     *
     * A leading '//' is preserved: on POSIX it introduces an implementation
     * defined path, and it is also the UNC form once separators are normalized.
     *
     * @param string $path Path using '/' separators.
     */
    private function collapseRedundantSegments(string $path): string
    {
        $prefix = '';
        if (\str_starts_with($path, '//') && !\str_starts_with($path, '///')) {
            $prefix = '//';
        } elseif (\str_starts_with($path, '/')) {
            $prefix = '/';
        }

        $segments = [];
        foreach (\explode('/', $path) as $segment) {
            if ($segment === '' || $segment === '.') {
                continue;
            }

            $segments[] = $segment;
        }

        // A relative input made only of '.' segments names the current directory.
        if ($prefix === '' && $segments === []) {
            return '.';
        }

        return $prefix . \implode('/', $segments);
    }

    /**
     * Normalize a filesystem path for prefix comparison across platforms.
     *
     * Separators are collapsed to '/', redundant separators and '.' segments
     * are removed, and a Windows drive letter is lowercased. Parent-directory
     * segments are not resolved here: hasDoubleDots() rejects them earlier.
     */
    protected function normalizePathForComparison(string $path): string
    {
        $path = \trim(\str_replace('\\', '/', $path));
        if ($path === '') {
            return '';
        }

        $path = $this->collapseRedundantSegments($path);
        $path = $this->normalizeUnicode($path);

        if (\preg_match('/^[A-Za-z]:$/', $path) === 1) {
            return \strtolower($path) . '/';
        }

        if (\preg_match('/^[A-Za-z]:\//', $path) === 1) {
            $path = \strtolower($path[0]) . \substr($path, 1);
        }

        return $path;
    }
}
