<?php

/**
 * FileTest.php
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

namespace Test;

use PHPUnit\Framework\Attributes\DataProvider;

/**
 * File Color class test
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
class FileTest extends TestUtil
{
    /**
     * Port the local PHP built-in HTTP server is listening on (0 = not started).
     */
    private static int $serverPort = 0;

    /**
     * Process handle returned by proc_open() for the built-in server.
     *
     * @var resource|null
     */
    private static mixed $serverProcess = null;

    /**
     * Start a local PHP built-in HTTP server so that cURL tests can reach
     * a real HTTP endpoint without requiring external network access.
     */
    public static function setUpBeforeClass(): void
    {
        // Ensure FORCE_CURL is defined so that getUrlData() proceeds even
        // when allow_url_fopen is enabled in the test environment.
        if (!\defined('FORCE_CURL')) {
            \define('FORCE_CURL', true);
        }

        if (!\function_exists('curl_init')) {
            return;
        }

        // Explicit opt-out for sandboxed/CI environments that block loopback
        // networking or forbid spawning a child process. Set
        // TC_LIB_FILE_SKIP_HTTP_SERVER=1 to skip the local server entirely; the
        // tests that depend on it are then reported as skipped instead of paying
        // the (bounded) readiness probe below.
        $skip = \getenv('TC_LIB_FILE_SKIP_HTTP_SERVER');
        if ($skip !== false && $skip !== '' && $skip !== '0') {
            return;
        }

        // proc_open() may be disabled via disable_functions in hardened setups.
        if (!\function_exists('proc_open')) {
            return;
        }

        // Find a free TCP port by binding to port 0 and reading the assignment.
        // Suppress a possible bind warning (the false return is handled below;
        // the @ operator is disallowed by the linter).
        $errno = 0;
        $errstr = '';
        \set_error_handler(static fn(): bool => true);
        $sock = \stream_socket_server('tcp://127.0.0.1:0', $errno, $errstr);
        \restore_error_handler();
        if ($sock === false) {
            return;
        }

        $name = (string) \stream_socket_get_name($sock, false);
        \fclose($sock);
        $matches = [];
        \preg_match('/(\d+)$/', $name, $matches);
        $port = (int) ($matches[1] ?? 0);

        if ($port === 0) {
            return;
        }

        $docRoot = __DIR__ . '/http';
        $cmd = \sprintf('php -S 127.0.0.1:%d -t %s', $port, \escapeshellarg($docRoot));

        $descriptors = [['pipe', 'r'], ['pipe', 'w'], ['pipe', 'w']];
        $serverPipes = [];
        \set_error_handler(static fn(): bool => true);
        $proc = \proc_open($cmd, $descriptors, $serverPipes);
        \restore_error_handler();
        if (!\is_resource($proc)) {
            return;
        }

        foreach ($serverPipes as $pipe) {
            \fclose($pipe);
        }

        // Wait until the server accepts connections (bounded to ~10 s). Bail out
        // immediately when the child has already exited (e.g. php -S could not
        // bind), so a non-functional environment is detected without looping.
        $ready = false;
        for ($i = 0; $i < 50; $i++) {
            $status = \proc_get_status($proc);
            if (!$status['running']) {
                break;
            }

            \set_error_handler(static fn(): bool => true);
            $conn = \fsockopen('127.0.0.1', $port, $errno, $errstr, 0.1);
            \restore_error_handler();
            if ($conn !== false) {
                \fclose($conn);
                $ready = true;
                break;
            }

            \usleep(100_000);
        }

        if (!$ready) {
            self::terminateServer($proc);
            return;
        }

        self::$serverPort = $port;
        self::$serverProcess = $proc;
    }

    /**
     * Shut down the local HTTP server started in setUpBeforeClass().
     */
    public static function tearDownAfterClass(): void
    {
        if (self::$serverProcess !== null) {
            self::terminateServer(self::$serverProcess);
            self::$serverProcess = null;
        }

        self::$serverPort = 0;
    }

    /**
     * Stop the spawned HTTP server without letting proc_close() block.
     *
     * proc_close() waits for the child to exit, so a child that ignores SIGTERM
     * would hang the run. Escalate to SIGKILL while it is still running so the
     * close can never block.
     *
     * @param resource $proc Process handle from proc_open().
     */
    private static function terminateServer(mixed $proc): void
    {
        if (!\is_resource($proc)) {
            return;
        }

        \proc_terminate($proc); // SIGTERM
        $status = \proc_get_status($proc);
        if ($status['running']) {
            \proc_terminate($proc, 9); // SIGKILL
        }

        \proc_close($proc);
    }

    protected function getTestObject(): \Com\Tecnick\File\File
    {
        return new \Com\Tecnick\File\File();
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFopenLocal(): void
    {
        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);
        $handle = $file->fopenLocal(__FILE__, 'r');
        $this->bcAssertIsResource($handle);
        \fclose($handle);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFopenLocalNonLocal(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->fopenLocal('http://www.example.com/test.txt', 'r');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFopenLocalMissing(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->fopenLocal('/missing_error.txt', 'r');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFopenLocalOpenFailureAfterValidation(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);
        $file->fopenLocal('/definitely-missing-' . \uniqid('', true) . '.txt', 'r');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFopenLocalDoubleDot(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->fopenLocal('/tmp/invalid/../test.txt', 'r');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testfReadInt(): void
    {
        $file = $this->getTestObject();
        $handle = \fopen(__FILE__, 'r');
        $this->assertNotFalse($handle);
        $res = $file->fReadInt($handle);
        // '<?ph' = 60 63 112 104 = 00111100 00111111 01110000 01101000 = 1010790504
        $this->assertEquals(1_010_790_504, $res);
        \fclose($handle);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testfReadIntReadFailureException(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();

        $tmp = \tempnam(\sys_get_temp_dir(), 'tc');
        $this->assertNotFalse($tmp);
        $handle = \fopen($tmp, 'w');
        $this->assertNotFalse($handle);
        $file->fReadInt($handle);
        \fclose($handle);
        \unlink($tmp);
    }

    /**
     * A stream that delivers fewer than 4 bytes per fread() must still be drained
     * up to the 4 bytes the integer needs. A single fread($h, 4) would return a
     * short read and unpack('N', ...) would silently yield 0.
     *
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFReadIntPartialReadStream(): void
    {
        $wrapperName = 'tcsinglebyteint';
        if (!\in_array($wrapperName, \stream_get_wrappers(), true)) {
            \stream_wrapper_register($wrapperName, SingleByteStreamWrapper::class);
        }

        $file = $this->getTestObject();
        $handle = \fopen($wrapperName . '://data', 'rb');
        $this->assertNotFalse($handle);

        try {
            // Wrapper yields 'abcdefgh' one byte per read; first 4 bytes 'abcd'.
            $res = $file->fReadInt($handle);
            $expected = (\ord('a') << 24) | (\ord('b') << 16) | (\ord('c') << 8) | \ord('d');
            $this->assertSame($expected, $res);
        } finally {
            \fclose($handle);
            \stream_wrapper_unregister($wrapperName);
        }
    }

    /**
     * A stream that ends before 4 bytes are available must raise a FileException
     * rather than silently returning 0.
     *
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFReadIntTruncatedStreamThrows(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();

        $tmp = \tempnam(\sys_get_temp_dir(), 'tc');
        $this->assertNotFalse($tmp);
        \file_put_contents($tmp, 'xy'); // only 2 bytes
        $handle = \fopen($tmp, 'rb');
        $this->assertNotFalse($handle);

        try {
            $file->fReadInt($handle);
        } finally {
            \fclose($handle);
            \unlink($tmp);
        }
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfRead(): void
    {
        $file = $this->getTestObject();
        $handle = \fopen(\dirname(__DIR__) . '/src/File.php', 'rb');
        $this->assertNotFalse($handle);
        $res = $file->rfRead($handle, 2);
        $this->assertEquals('<?', $res);
        $res = $file->rfRead($handle, 3);
        $this->assertEquals('php', $res);
        \fclose($handle);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfReadException(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->rfRead(null, 2);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfReadClosedHandleException(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $handle = \fopen(__FILE__, 'rb');
        // ensure static analyzers know fopen succeeded
        $this->assertNotFalse($handle);

        \fclose($handle);
        // handle is still typed resource by analyzers even after close
        $file->rfRead($handle, 1);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfReadZeroLength(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $handle = \fopen(__FILE__, 'rb');
        $this->assertNotFalse($handle);
        // length 0: the while-loop condition (0 < 0) is immediately false,
        // so $data stays empty and FileException is thrown.
        $rfm = new \ReflectionMethod($file, 'rfRead');
        $rfm->invoke($file, $handle, 0);
        \fclose($handle);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfReadEofShorter(): void
    {
        $file = $this->getTestObject();
        $tmp = \tempnam(\sys_get_temp_dir(), 'tc');
        $this->assertNotFalse($tmp);
        \file_put_contents($tmp, 'xy');
        $handle = \fopen($tmp, 'rb');
        $this->assertNotFalse($handle);
        $res = $file->rfRead($handle, 10);
        $this->assertEquals('xy', $res);
        \fclose($handle);
        \unlink($tmp);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfReadRecursiveBufferedStream(): void
    {
        if (!\in_array('tcreadpartial', \stream_get_wrappers(), true)) {
            \stream_wrapper_register('tcreadpartial', RecursiveReadStreamWrapper::class);
        }

        $file = new RecursiveReadFile();
        $handle = \fopen('tcreadpartial://buffered', 'rb');
        $this->assertNotFalse($handle);

        try {
            $res = $file->rfRead($handle, 5);
            $this->assertSame('abcde', $res);
        } finally {
            \fclose($handle);
            \stream_wrapper_unregister('tcreadpartial');
        }
    }

    public function testHasUnreadBytes(): void
    {
        $file = $this->getTestObject();
        $handle = \fopen(\dirname(__DIR__) . '/src/File.php', 'rb');
        $this->assertNotFalse($handle);

        try {
            $this->assertSame('<?', \fread($handle, 2));

            $rfm = new \ReflectionMethod($file, 'hasUnreadBytes');
            $this->assertTrue($rfm->invoke($file, $handle) === true);
        } finally {
            \fclose($handle);
        }
    }

    /**
     * @param string       $file     File path
     * @param list<string> $expected Expected result
     */
    #[DataProvider('getAltFilePathsDataProvider')]
    public function testGetAltFilePaths(string $file, array $expected): void
    {
        $testObj = new \Com\Tecnick\File\File(['localhost']);
        $_SERVER['DOCUMENT_ROOT'] = '/var/www';
        $_SERVER['HTTP_HOST'] = 'localhost';
        $_SERVER['HTTPS'] = 'on';
        $_SERVER['SCRIPT_URI'] = 'https://localhost/path/example.php';
        $alt = $testObj->getAltFilePaths($file);
        $this->assertEquals($expected, $alt);
    }

    /**
     * Data provider for testGetAltFilePaths
     *
     * getAltFilePaths() returns a 0-indexed list: array_unique() drops duplicate
     * candidates and the result is re-indexed, so the expected values are plain
     * sequential lists (the surviving keys carry no meaning).
     *
     * @return array<array{string, list<string>}>
     */
    public static function getAltFilePathsDataProvider(): array
    {
        return [
            [
                'http://www.example.com/test.txt',
                [
                    'http://www.example.com/test.txt',
                ],
            ],
            [
                'https://localhost/path/test.txt',
                [
                    'https://localhost/path/test.txt',
                    '/var/www/path/test.txt',
                ],
            ],
            [
                '//www.example.com/space test.txt',
                [
                    '//www.example.com/space test.txt',
                    'https://www.example.com/space%20test.txt',
                ],
            ],
            [
                '/path/test.txt',
                [
                    '/path/test.txt',
                    '/var/www/path/test.txt',
                    'https://localhost/path/test.txt',
                ],
            ],
            [
                'https://localhost/path/test.php?a=0&b=1&amp;c=2;&amp;d="a+b%20c"',
                [
                    'https://localhost/path/test.php?a=0&b=1&amp;c=2;&amp;d="a+b%20c"',
                    'https://localhost/path/test.php?a=0&b=1&c=2;&d="a+b%20c"',
                ],
            ],
            [
                'path/test.txt',
                [
                    'path/test.txt',
                    'https://localhost/path/test.txt',
                ],
            ],
        ];
    }

    /**
     * Ensure getAltUrlFromPath returns the input when SCRIPT_URI cannot be parsed
     * (covers the guarded return on line 363 of File.php).
     */
    public function testGetAltUrlFromPathUnparseableUri(): void
    {
        $testObj = $this->getTestObject();

        // set SCRIPT_URI to something parse_url will handle but without scheme/host
        $_SERVER['SCRIPT_URI'] = 'not-a-url';

        $rfm = new \ReflectionMethod($testObj, 'getAltUrlFromPath');
        $input = 'some/path.txt';
        $result = (string) $rfm->invoke($testObj, $input);
        $this->assertSame($input, $result, 'Expected original path when SCRIPT_URI lacks scheme/host');
    }

    public function testGetAltUrlFromPathSpoofedScriptUriRejected(): void
    {
        $testObj = $this->getTestObject();
        // No allowedHosts configured → deny SCRIPT_URI host.
        $_SERVER['SCRIPT_URI'] = 'https://attacker.internal/app/script.php';

        $rfm = new \ReflectionMethod($testObj, 'getAltUrlFromPath');
        $input = 'data/file.txt';
        $result = (string) $rfm->invoke($testObj, $input);
        $this->assertSame($input, $result, 'Spoofed SCRIPT_URI host must not be used to build a URL');
    }

    public function testGetAltUrlFromPathAllowlistedScriptUriAccepted(): void
    {
        $testObj = new \Com\Tecnick\File\File(['myapp.example.com']);
        $_SERVER['SCRIPT_URI'] = 'https://myapp.example.com/app/script.php';

        $rfm = new \ReflectionMethod($testObj, 'getAltUrlFromPath');
        $result = (string) $rfm->invoke($testObj, 'data/file.txt');
        $this->assertSame('https://myapp.example.com/data/file.txt', $result);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFileGetContentsMissingException(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->fileGetContents('missing.txt');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFileGetContentsDoubleDotException(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->fileGetContents('/tmp/something/../test.txt');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFileGetContentsForbiddenProtocolException(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->fileGetContents('phar://test.txt');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFileGetContents(): void
    {
        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);
        $res = $file->fileGetContents(__FILE__);
        $this->assertEquals('<?php', \substr($res, 0, 5));
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFileGetContentsCurl(): void
    {
        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file = $this->getTestObject();
        $file->fileGetContents('http://www.example.com/test.txt');
    }

    public function testMaxRemoteSizeDefault(): void
    {
        $file = $this->getTestObject();
        $this->assertSame(52428800, $file->getMaxRemoteSize());
    }

    public function testMaxRemoteSizeConfigurable(): void
    {
        $file = $this->getTestObject();
        $file->setMaxRemoteSize(1048576); // 1MB
        $this->assertSame(1048576, $file->getMaxRemoteSize());
    }

    public function testSetAllowedHostsIsFluentAndUsedByValidator(): void
    {
        $file = new class() extends \Com\Tecnick\File\File {
            public function validateHostProxy(string $host): bool
            {
                return $this->isValidHost($host);
            }
        };

        $ret = $file->setAllowedHosts(['trusted.example']);
        $this->assertSame($file, $ret);

        $this->assertTrue($file->validateHostProxy('trusted.example'));
        $this->assertFalse($file->validateHostProxy('other.example'));
        $this->assertFalse($file->validateHostProxy(''));
    }

    public function testSetAllowedPathsIsFluentAndUsedByValidator(): void
    {
        $file = new \Com\Tecnick\File\File();

        $tmpDir = \sys_get_temp_dir() . '/tc-lib-file-' . \uniqid('', true);
        $this->assertTrue(\mkdir($tmpDir, 0o755, true));

        $allowedPath = $tmpDir . '/file.txt';
        $this->assertSame(\file_put_contents($allowedPath, 'ok'), 2);

        try {
            $ret = $file->setAllowedPaths([$tmpDir]);
            $this->assertSame($file, $ret);

            $this->assertTrue($file->isValidFile($allowedPath));

            $blockedPath = \sys_get_temp_dir() . '/blocked-' . \uniqid('', true) . '.txt';
            $this->assertFalse($file->isValidFile($blockedPath));
        } finally {
            if (\is_file($allowedPath)) {
                \unlink($allowedPath);
            }

            if (\is_dir($tmpDir)) {
                \rmdir($tmpDir);
            }
        }
    }

    public function testResolveLocalPathUsesExplicitBaseDirs(): void
    {
        $file = new \Com\Tecnick\File\File();

        $baseDir = \sys_get_temp_dir() . '/tc-lib-file-' . \uniqid('', true);
        $imagesDir = $baseDir . '/images';
        $imagePath = $imagesDir . '/tcpdf_logo.jpg';

        $this->assertTrue(\mkdir($imagesDir, 0o755, true));
        $this->assertSame(2, \file_put_contents($imagePath, 'ok'));

        try {
            $resolved = $file->resolveLocalPath('images/tcpdf_logo.jpg', [$baseDir]);

            $this->assertSame(\realpath($imagePath), $resolved);
        } finally {
            if (\is_file($imagePath)) {
                \unlink($imagePath);
            }
            if (\is_dir($imagesDir)) {
                \rmdir($imagesDir);
            }
            if (\is_dir($baseDir)) {
                \rmdir($baseDir);
            }
        }
    }

    public function testResolveLocalPathLeavesSchemedInputsUntouched(): void
    {
        $file = new \Com\Tecnick\File\File();
        $url = 'https://example.com/logo.jpg';

        $this->assertSame($url, $file->resolveLocalPath($url, [__DIR__]));
    }

    public function testResolveLocalPathResolvesExistingPathWithoutBaseDirs(): void
    {
        $file = new \Com\Tecnick\File\File();

        // An existing path resolves directly via realpath(), before any base dir
        // is consulted.
        $this->assertSame(\realpath(__FILE__), $file->resolveLocalPath(__FILE__));
    }

    public function testResolveLocalPathSkipsEmptyAndInvalidBaseDirsThenFallsBack(): void
    {
        $file = new \Com\Tecnick\File\File();

        $missing = 'tc-missing-' . \uniqid('', true) . '-file.bin';
        $invalidBase = \sys_get_temp_dir() . '/tc-no-such-dir-' . \uniqid('', true);
        $this->assertFalse(\realpath($invalidBase));

        // '' is skipped, the non-existent base dir fails realpath() and is also
        // skipped, and with nothing left to try the original relative path is
        // returned unchanged.
        $this->assertSame($missing, $file->resolveLocalPath($missing, ['', $invalidBase]));
    }

    public function testHasDoubleDots(): void
    {
        $file = new class() extends \Com\Tecnick\File\File {
            public function hasDoubleDotsProxy(string $path): bool
            {
                return $this->hasDoubleDots($path);
            }
        };

        $res = $file->hasDoubleDotsProxy('/tmp/../test.txt');
        $this->assertTrue($res);
        $res = $file->hasDoubleDotsProxy('/tmp/test.txt');
        $this->assertFalse($res);
    }

    // -------------------------------------------------------------------------
    // Issue 1: CURLOPT_REDIR_PROTOCOLS is present in CURLOPT_DEFAULT
    // -------------------------------------------------------------------------

    public function testCurlOptRedirProtocolsPresent(): void
    {
        $ref = new \ReflectionClassConstant(\Com\Tecnick\File\File::class, 'CURLOPT_DEFAULT');
        /** @var array<int, mixed> $opts */
        $opts = $ref->getValue();
        $this->assertArrayHasKey(CURLOPT_REDIR_PROTOCOLS, $opts);
        // Only HTTP/HTTPS allowed for redirects — no FTP.
        $this->assertSame(CURLPROTO_HTTPS | CURLPROTO_HTTP, $opts[CURLOPT_REDIR_PROTOCOLS] ?? null);
    }

    // -------------------------------------------------------------------------
    // Issue 7: TLS verification flags pinned in fixed options
    // -------------------------------------------------------------------------

    public function testSslVerificationPinnedInFixedOptions(): void
    {
        $ref = new \ReflectionClassConstant(\Com\Tecnick\File\File::class, 'CURLOPT_FIXED');
        /** @var array<int, mixed> $opts */
        $opts = $ref->getValue();
        // SSL verification must be pinned in FIXED to prevent override
        $this->assertArrayHasKey(CURLOPT_SSL_VERIFYHOST, $opts);
        $this->assertArrayHasKey(CURLOPT_SSL_VERIFYPEER, $opts);
        // Verify strict verification is enforced
        $this->assertSame(2, $opts[CURLOPT_SSL_VERIFYHOST] ?? null);
        $this->assertTrue(($opts[CURLOPT_SSL_VERIFYPEER] ?? null) === true);
    }

    public function testSslVerificationCannotBeOverriddenByCustomOptions(): void
    {
        $testObj = $this->getTestObject();
        // Set custom curl options that try to disable SSL verification
        $testObj->setCurlOpts([
            CURLOPT_SSL_VERIFYPEER => false,
            CURLOPT_SSL_VERIFYHOST => 0,
        ]);

        // Get the fixed options to verify they are unaffected
        $refProperty = new \ReflectionProperty($testObj, 'fixedCurlOpts');
        /** @var array<int, mixed> $fixedOpts */
        $fixedOpts = $refProperty->getValue($testObj);

        // Verify fixed options still have strict verification enabled
        // (they should override any custom options due to merge order in getUrlData)
        $this->assertSame(2, $fixedOpts[CURLOPT_SSL_VERIFYHOST] ?? null);
        $this->assertTrue(($fixedOpts[CURLOPT_SSL_VERIFYPEER] ?? null) === true);
    }

    // -------------------------------------------------------------------------
    // Issue 3: validateHost() / HTTP_HOST SSRF protection
    // -------------------------------------------------------------------------

    public function testSpoofedHttpHostIsRejectedByDefault(): void
    {
        $testObj = $this->getTestObject();
        // No allowedHosts configured → every host is denied.
        $_SERVER['HTTP_HOST'] = 'evil.internal';
        $_SERVER['HTTPS'] = 'on';

        $rfm = new \ReflectionMethod($testObj, 'getAltMissingUrlProtocol');
        $input = '//evil.internal/steal';
        $result = (string) $rfm->invoke($testObj, $input);
        // Without a trusted host the path must come back unchanged (decoded only).
        $this->assertSame(
            \htmlspecialchars_decode($input),
            $result,
            'Spoofed HTTP_HOST must not be used to build a URL',
        );
    }

    public function testAllowlistedHttpHostIsAccepted(): void
    {
        $testObj = new \Com\Tecnick\File\File(['myapp.example.com']);
        $_SERVER['HTTP_HOST'] = 'myapp.example.com';
        $_SERVER['HTTPS'] = 'on';

        $rfm = new \ReflectionMethod($testObj, 'getAltMissingUrlProtocol');
        $result = (string) $rfm->invoke($testObj, '//myapp.example.com/path/file.txt');
        $this->assertSame('https://myapp.example.com/path/file.txt', $result);
    }

    public function testGetAltPathFromUrlSpoofedHostRejected(): void
    {
        $testObj = $this->getTestObject();
        // No allowedHosts → deny.
        $_SERVER['HTTP_HOST'] = 'attacker.internal';
        $_SERVER['DOCUMENT_ROOT'] = '/var/www';
        $_SERVER['HTTPS'] = 'off';

        $rfm = new \ReflectionMethod($testObj, 'getAltPathFromUrl');
        $url = 'http://attacker.internal/secret';
        $result = (string) $rfm->invoke($testObj, $url);
        $this->assertSame($url, $result, 'Spoofed host must not be used to build a local path');
    }

    public function testValidatePathAcceptsAllowedPrefix(): void
    {
        $baseDir = \sys_get_temp_dir() . '/tc-lib-file-' . \uniqid('', true);
        $this->assertTrue(\mkdir($baseDir, 0o755, true));

        $testObj = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, [$baseDir]);

        try {
            $path = $baseDir . '/assets/file.txt';
            $this->assertTrue($testObj->isValidFile($path));
        } finally {
            if (\is_dir($baseDir . '/assets')) {
                \rmdir($baseDir . '/assets');
            }
            if (\is_dir($baseDir)) {
                \rmdir($baseDir);
            }
        }
    }

    public function testValidatePathRejectsNonMatchingPrefix(): void
    {
        $baseDir = \sys_get_temp_dir() . '/tc-lib-file-' . \uniqid('', true);
        $this->assertTrue(\mkdir($baseDir, 0o755, true));

        $testObj = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, [$baseDir]);

        try {
            $path = \sys_get_temp_dir() . '/tc-lib-file-' . \uniqid('', true) . '/file.txt';
            $this->assertFalse($testObj->isValidFile($path));
        } finally {
            if (\is_dir($baseDir)) {
                \rmdir($baseDir);
            }
        }
    }

    public function testValidatePathRejectsSiblingPrefixBypass(): void
    {
        $baseDir = \sys_get_temp_dir() . '/tc-lib-file-' . \uniqid('', true);
        $this->assertTrue(\mkdir($baseDir, 0o755, true));

        $testObj = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, [$baseDir]);

        try {
            $path = $baseDir . '_evil/secret.txt';
            $this->assertFalse($testObj->isValidFile($path));
        } finally {
            if (\is_dir($baseDir)) {
                \rmdir($baseDir);
            }
        }
    }

    public function testIsPathWithinAllowedRootsSkipsEmptyRoots(): void
    {
        $testObj = new class() extends \Com\Tecnick\File\File {
            /**
             * @param array<int, string> $roots
             */
            public function isPathWithinAllowedRootsProxy(string $path, array $roots): bool
            {
                return $this->isPathWithinAllowedRoots($path, $roots);
            }
        };

        $this->assertTrue($testObj->isPathWithinAllowedRootsProxy('/var/www/app/file.txt', ['', '/', '/var/www']));
    }

    public function testAllowedPathsAreNormalizedInConstructorForWindowsPaths(): void
    {
        $testObj = new class([], 52_428_800, [], null, null, [' C:\\Trusted\\Base\\ ', 'D:', '']) extends
            \Com\Tecnick\File\File {
            public function isPathAllowedProxy(string $path): bool
            {
                return $this->isPathWithinAllowedRoots($path, $this->allowedPaths);
            }

            /**
             * @return array<int, string>
             */
            public function getAllowedPathsProxy(): array
            {
                return \array_values($this->allowedPaths);
            }
        };

        $this->assertSame(['c:/Trusted/Base', 'd:'], $testObj->getAllowedPathsProxy());
        $this->assertTrue($testObj->isPathAllowedProxy('c:/Trusted/Base/file.txt'));
        $this->assertTrue($testObj->isPathAllowedProxy('d:/folder/file.txt'));
    }

    public function testAllowedPathsAreNormalizedInSetterForWindowsPaths(): void
    {
        $testObj = new class() extends \Com\Tecnick\File\File {
            public function isPathAllowedProxy(string $path): bool
            {
                return $this->isPathWithinAllowedRoots($path, $this->allowedPaths);
            }

            /**
             * @return array<int, string>
             */
            public function getAllowedPathsProxy(): array
            {
                return \array_values($this->allowedPaths);
            }
        };

        $testObj->setAllowedPaths([' E:\\Share\\Root\\ ', 'E:\\Share\\Root\\']);

        $this->assertSame(['e:/Share/Root'], $testObj->getAllowedPathsProxy());
        $this->assertTrue($testObj->isPathAllowedProxy('e:/Share/Root/file.txt'));
        $this->assertFalse($testObj->isPathAllowedProxy('e:/Share/Root_evil/file.txt'));
    }

    public function testValidatePathReturnsFalseWhenNearestParentCannotBeResolved(): void
    {
        $testObj = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['foo']);

        $file = 'foo';
        $this->assertFalse($testObj->isValidFile($file));
    }

    public function testValidatePathRejectsSymlinkEscape(): void
    {
        if (!\function_exists('symlink')) {
            $this->markTestSkipped('symlink is not available in this environment');
        }

        $withoutWarnings = static function (callable $callback): mixed {
            \set_error_handler(static fn(): bool => true, E_WARNING | E_NOTICE | E_USER_WARNING | E_USER_NOTICE);

            try {
                return $callback();
            } finally {
                \restore_error_handler();
            }
        };

        $base = \sys_get_temp_dir() . '/tcfile_' . \uniqid('', true);
        $allowedDir = $base . '/allowed';
        \mkdir($allowedDir, 0o777, true);
        $link = $allowedDir . '/passwd-link';

        if (!$withoutWarnings(static fn(): bool => \symlink('/etc/passwd', $link))) {
            $this->markTestSkipped('unable to create symlink in this environment');
        }

        $testObj = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, [$base]);

        try {
            $this->assertFalse($testObj->isValidFile($link));
        } finally {
            if (\is_link($link) || \file_exists($link)) {
                $withoutWarnings(static fn(): bool => \unlink($link));
            }

            if (\is_dir($allowedDir)) {
                $withoutWarnings(static fn(): bool => \rmdir($allowedDir));
            }

            if (\is_dir($base)) {
                $withoutWarnings(static fn(): bool => \rmdir($base));
            }
        }
    }

    public function testValidatePathRejectsSymlinkDirectoryEscapeForMissingTarget(): void
    {
        if (!\function_exists('symlink')) {
            $this->markTestSkipped('symlink is not available in this environment');
        }

        $withoutWarnings = static function (callable $callback): mixed {
            \set_error_handler(static fn(): bool => true, E_WARNING | E_NOTICE | E_USER_WARNING | E_USER_NOTICE);

            try {
                return $callback();
            } finally {
                \restore_error_handler();
            }
        };

        $base = \sys_get_temp_dir() . '/tcfile_' . \uniqid('', true);
        $allowedDir = $base . '/allowed';
        $outsideDir = \sys_get_temp_dir() . '/tcfile_outside_' . \uniqid('', true);
        \mkdir($allowedDir, 0o777, true);
        \mkdir($outsideDir, 0o777, true);

        $link = $allowedDir . '/escape-link';
        if (!$withoutWarnings(static fn(): bool => \symlink($outsideDir, $link))) {
            $this->markTestSkipped('unable to create symlink in this environment');
        }

        $testObj = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, [$base]);
        $target = $link . '/new-file.txt';

        try {
            $this->assertFalse($testObj->isValidFile($target));
        } finally {
            if (\is_link($link) || \file_exists($link)) {
                $withoutWarnings(static fn(): bool => \unlink($link));
            }

            if (\is_dir($outsideDir)) {
                $withoutWarnings(static fn(): bool => \rmdir($outsideDir));
            }

            if (\is_dir($allowedDir)) {
                $withoutWarnings(static fn(): bool => \rmdir($allowedDir));
            }

            if (\is_dir($base)) {
                $withoutWarnings(static fn(): bool => \rmdir($base));
            }
        }
    }

    public function testIsValidUrlReturnsFalseWhenParseFails(): void
    {
        $testObj = new \Com\Tecnick\File\File(['localhost']);
        $url = 'http://:\\';

        $this->assertFalse($testObj->isValidURL($url));
    }

    public function testIsValidUrlReturnsFalseWhenHostMissing(): void
    {
        $testObj = new \Com\Tecnick\File\File(['localhost']);
        $url = 'http:/path/without/host';

        $this->assertFalse($testObj->isValidURL($url));
    }

    public function testIsValidUrlReturnsFalseWhenTrimmedUrlIsEmpty(): void
    {
        $testObj = new \Com\Tecnick\File\File(['localhost']);
        $url = " \t\n\r ";

        $this->assertFalse($testObj->isValidURL($url));
    }

    public function testValidatePathRejectsNonFileScheme(): void
    {
        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);

        $ftpPath = 'ftp://example.com/file.txt';
        $this->assertFalse($file->isValidFile($ftpPath));

        $pharPath = 'phar://archive/file.txt';
        $this->assertFalse($file->isValidFile($pharPath));

        $httpPath = 'http://www.example.com/file.txt';
        $this->assertFalse($file->isValidFile($httpPath));

        $localPath = './some/file.txt';
        $this->assertTrue($file->isValidFile($localPath));

        $fileUrl = 'file://some/file.txt';
        $this->assertTrue($file->isValidFile($fileUrl));
    }

    public function testValidatePathRejectsEmptyFileUrlPathEvenWithWildcardTrust(): void
    {
        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);

        $emptyFileUrl = 'file://   ';
        $this->assertFalse($file->isValidFile($emptyFileUrl));
    }

    // -------------------------------------------------------------------------
    // Issue 8: iterative rfRead — single-byte chunk delivery
    // -------------------------------------------------------------------------

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfReadSingleByteChunks(): void
    {
        $wrapperName = 'tcsinglebyte';
        if (!\in_array($wrapperName, \stream_get_wrappers(), true)) {
            \stream_wrapper_register($wrapperName, SingleByteStreamWrapper::class);
        }

        $file = $this->getTestObject();
        $handle = \fopen($wrapperName . '://data', 'rb');
        $this->assertNotFalse($handle);

        try {
            $res = $file->rfRead($handle, 4);
            $this->assertSame('abcd', $res);
        } finally {
            \fclose($handle);
            \stream_wrapper_unregister($wrapperName);
        }
    }

    // -------------------------------------------------------------------------
    // rfRead inner break: fread returns '' before feof signals end-of-stream
    // -------------------------------------------------------------------------

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testRfReadBreakOnEmptyChunk(): void
    {
        $wrapperName = 'tcemptyread';
        if (!\in_array($wrapperName, \stream_get_wrappers(), true)) {
            \stream_wrapper_register($wrapperName, EmptyReadStreamWrapper::class);
        }

        $file = $this->getTestObject();
        $handle = \fopen($wrapperName . '://data', 'rb');
        $this->assertNotFalse($handle);

        try {
            // The wrapper returns 'ab' on the first read then '' forever while
            // stream_eof() never returns true, so rfRead() hits the inner break
            // (File.php line 194) after the second fread() call.
            $res = $file->rfRead($handle, 10);
            $this->assertSame('ab', $res);
        } finally {
            \fclose($handle);
            \stream_wrapper_unregister($wrapperName);
        }
    }

    // -------------------------------------------------------------------------
    // Progress callback direct-invocation tests (cURL size-limit logic)
    // -------------------------------------------------------------------------

    public function testProgressCallbackReturnsZeroBelowLimit(): void
    {
        $file = $this->getTestObject();
        $file->setMaxRemoteSize(100);

        $rfm = new \ReflectionMethod($file, 'createProgressCallback');

        $bytesRead = 0;
        $args = [&$bytesRead];
        /** @var callable $callback */
        $callback = $rfm->invokeArgs($file, $args);

        // 50 bytes downloaded — well below the 100-byte limit → return 0
        $result = (int) $callback(null, 50, 50, 0, 0);
        $this->assertSame(0, $result);
    }

    public function testProgressCallbackReturnsAbortSignalAboveLimit(): void
    {
        $file = $this->getTestObject();
        $file->setMaxRemoteSize(100);

        $rfm = new \ReflectionMethod($file, 'createProgressCallback');

        $bytesRead = 0;
        $args = [&$bytesRead];
        /** @var callable $callback */
        $callback = $rfm->invokeArgs($file, $args);

        // 200 bytes downloaded — exceeds the 100-byte limit → return 1 (abort)
        $result = (int) $callback(null, 200, 200, 0, 0);
        $this->assertSame(1, $result);
    }

    public function testBuildRedirectUrlCoversUnsupportedAndRelativeForms(): void
    {
        $file = $this->getTestObject();

        $buildRedirectUrl = \Closure::bind(
            static fn(
                \Com\Tecnick\File\File $obj,
                string $location,
                string $baseUrl,
            ): string|false => $obj->buildRedirectUrl($location, $baseUrl),
            null,
            \Com\Tecnick\File\File::class,
        );

        $this->assertFalse($buildRedirectUrl($file, '   ', 'https://example.test/base'));
        $this->assertFalse($buildRedirectUrl($file, '/next', 'http://:\\'));
        $this->assertFalse($buildRedirectUrl($file, '/next', 'ftp://example.test/base'));
        $this->assertSame('https://cdn.example.test/file.txt', $buildRedirectUrl(
            $file,
            '//cdn.example.test/file.txt',
            'https://example.test/base',
        ));
        $this->assertSame('https://example.test/path/next.txt', $buildRedirectUrl(
            $file,
            'next.txt',
            'https://example.test/path/current.php',
        ));
    }

    public function testRedirectValidationCallbackRejectsEmptyAndNonCurlLocationHeaders(): void
    {
        $file = new \Com\Tecnick\File\File(['allowed.example']);

        $rfm = new \ReflectionMethod($file, 'createRedirectValidationCallback');

        $invalidRedirect = false;
        $args = [&$invalidRedirect, 'https://allowed.example/start'];
        /** @var callable $callback */
        $callback = $rfm->invokeArgs($file, $args);
        $this->assertSame(0, $callback(null, "Location:   \r\n"));

        $invalidRedirect = false;
        $args = [&$invalidRedirect, 'https://allowed.example/start'];
        /** @var callable $callback */
        $callback = $rfm->invokeArgs($file, $args);
        $this->assertSame(0, $callback(null, "Location: /next\r\n"));
    }

    // -------------------------------------------------------------------------
    // Local HTTP server tests — cURL size-limit enforcement and return value
    // -------------------------------------------------------------------------

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testGetUrlDataWithValidRedirectWhenMaxRedirsEnabled(): void
    {
        if (self::$serverPort === 0 || !\function_exists('curl_init')) {
            $this->markTestSkipped('Local HTTP server not available');
        }

        if ((string) \ini_get('open_basedir') !== '') {
            $this->markTestSkipped('Redirect-follow tests require open_basedir to be disabled');
        }

        $file = new \Com\Tecnick\File\File(['127.0.0.1']);
        $file->setCurlOpts([CURLOPT_MAXREDIRS => 3]);

        $result = $file->getUrlData('http://127.0.0.1:' . self::$serverPort . '/redirect.php?to=/empty.php');
        $this->assertSame('', $result);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testGetUrlDataReturnsFalseOnInvalidRedirectWhenMaxRedirsEnabled(): void
    {
        if (self::$serverPort === 0 || !\function_exists('curl_init')) {
            $this->markTestSkipped('Local HTTP server not available');
        }

        if ((string) \ini_get('open_basedir') !== '') {
            $this->markTestSkipped('Redirect-follow tests require open_basedir to be disabled');
        }

        $file = new \Com\Tecnick\File\File(['127.0.0.1']);
        $file->setCurlOpts([CURLOPT_MAXREDIRS => 3]);

        $result = $file->getUrlData('http://127.0.0.1:' . self::$serverPort . '/redirect.php?to=http://example.com/');
        $this->assertFalse($result);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testGetUrlDataSizeExceeded(): void
    {
        if (self::$serverPort === 0 || !\function_exists('curl_init')) {
            $this->markTestSkipped('Local HTTP server not available');
        }

        $file = new \Com\Tecnick\File\File(['127.0.0.1']);
        // Set a very small limit so the 1 000-byte response from large.php
        // triggers CURLE_ABORTED_BY_CALLBACK (errno 42).
        $file->setMaxRemoteSize(10);

        $this->bcExpectException(\Com\Tecnick\File\Exception::class);
        $file->getUrlData('http://127.0.0.1:' . self::$serverPort . '/large.php');
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testGetUrlDataReturnTrue(): void
    {
        if (self::$serverPort === 0 || !\function_exists('curl_init')) {
            $this->markTestSkipped('Local HTTP server not available');
        }

        // Create a File instance with no fixed cURL options so that
        // CURLOPT_RETURNTRANSFER is not set.  curl_exec() then returns true
        // on success, exercising the `$ret === true ? '' : $ret` branch.
        $file = new \Com\Tecnick\File\File(['127.0.0.1'], 52428800, [], [], []);

        \ob_start();
        $result = $file->getUrlData('http://127.0.0.1:' . self::$serverPort . '/empty.php');
        \ob_end_clean();

        $this->assertSame('', $result);
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testGetUrlDataCurlExecFailureReturnsFalse(): void
    {
        if (!\function_exists('curl_init')) {
            $this->markTestSkipped('cURL extension not available');
        }

        $file = new \Com\Tecnick\File\File(['127.0.0.1']);
        $result = $file->getUrlData('http://127.0.0.1:1/unreachable.txt');
        $this->assertFalse($result);
    }

    // -------------------------------------------------------------------------
    // Cross-platform: case-sensitivity, binary mode, Unicode, separators
    // -------------------------------------------------------------------------

    public function testAllowlistIsCaseInsensitiveWhenOverrideOff(): void
    {
        $proxy = new class() extends \Com\Tecnick\File\File {
            public function isPathAllowedProxy(string $path): bool
            {
                return $this->isPathWithinAllowedRoots($path, $this->allowedPaths);
            }
        };
        $proxy->setAllowedPaths(['/srv/App', 'C:\\Trusted\\Base']);
        $proxy->setCaseSensitivePaths(false); // force case-insensitive matching

        $this->assertTrue($proxy->isPathAllowedProxy('/srv/app/file.txt'));
        $this->assertTrue($proxy->isPathAllowedProxy('/SRV/APP/file.txt'));
        $this->assertTrue($proxy->isPathAllowedProxy('c:/trusted/base/file.txt'));
        $this->assertTrue($proxy->isPathAllowedProxy('C:/TRUSTED/BASE/file.txt'));
        // still rejects a genuinely different sibling root
        $this->assertFalse($proxy->isPathAllowedProxy('/srv/app_evil/file.txt'));
    }

    public function testAllowlistIsCaseSensitiveWhenOverrideOn(): void
    {
        $proxy = new class() extends \Com\Tecnick\File\File {
            public function isPathAllowedProxy(string $path): bool
            {
                return $this->isPathWithinAllowedRoots($path, $this->allowedPaths);
            }
        };
        $proxy->setAllowedPaths(['/srv/App']);
        $proxy->setCaseSensitivePaths(true); // force case-sensitive matching

        $this->assertTrue($proxy->isPathAllowedProxy('/srv/App/file.txt'));
        $this->assertFalse($proxy->isPathAllowedProxy('/srv/app/file.txt'));
    }

    public function testAllowlistDefaultIsCaseSensitiveOnLinux(): void
    {
        if (\PHP_OS_FAMILY !== 'Linux') {
            $this->markTestSkipped('Linux-specific default behavior');
        }

        $proxy = new class() extends \Com\Tecnick\File\File {
            public function isPathAllowedProxy(string $path): bool
            {
                return $this->isPathWithinAllowedRoots($path, $this->allowedPaths);
            }
        };
        $proxy->setAllowedPaths(['/srv/App']); // override left null -> auto-detect

        $this->assertTrue($proxy->isPathAllowedProxy('/srv/App/file.txt'));
        $this->assertFalse($proxy->isPathAllowedProxy('/srv/app/file.txt'));
    }

    public function testCaseInsensitiveDefaultPerOsFamily(): void
    {
        $proxy = new class() extends \Com\Tecnick\File\File {
            public ?bool $probeResult = null;

            public function caseInsensitiveDefaultProxy(string $osFamily, string $hint): bool
            {
                return $this->caseInsensitiveDefault($osFamily, $hint);
            }

            protected function probeCaseInsensitive(string $hint): ?bool
            {
                return $this->probeResult;
            }
        };

        // Windows is always case-insensitive, regardless of the probe.
        $proxy->probeResult = null;
        $this->assertTrue($proxy->caseInsensitiveDefaultProxy('Windows', '/whatever'));

        // macOS: probe decides; null falls back to case-insensitive.
        $proxy->probeResult = null;
        $this->assertTrue($proxy->caseInsensitiveDefaultProxy('Darwin', '/whatever'));
        $proxy->probeResult = true;
        $this->assertTrue($proxy->caseInsensitiveDefaultProxy('Darwin', '/whatever'));
        $proxy->probeResult = false;
        $this->assertFalse($proxy->caseInsensitiveDefaultProxy('Darwin', '/whatever'));

        // Linux/other: probe decides; null falls back to case-sensitive.
        $proxy->probeResult = null;
        $this->assertFalse($proxy->caseInsensitiveDefaultProxy('Linux', '/whatever'));
        $proxy->probeResult = true;
        $this->assertTrue($proxy->caseInsensitiveDefaultProxy('Linux', '/whatever'));
    }

    public function testProbeCaseInsensitive(): void
    {
        $proxy = new class() extends \Com\Tecnick\File\File {
            public function probeProxy(string $hint): ?bool
            {
                return $this->probeCaseInsensitive($hint);
            }
        };

        // Unresolvable path -> null (caller applies platform default).
        $missing = \sys_get_temp_dir() . '/tc-nope-' . \uniqid('', true);
        $this->assertNull($proxy->probeProxy($missing));

        // Existing path -> concrete bool reflecting the host filesystem.
        $result = $proxy->probeProxy(__FILE__);
        $this->assertIsBool($result);
        if (\PHP_OS_FAMILY === 'Linux') {
            $this->assertFalse($result);
        }

        // When the resolved path has no letter to toggle, the probe cannot
        // decide and returns null.
        $noFlip = new class() extends \Com\Tecnick\File\File {
            public function probeProxy(string $hint): ?bool
            {
                return $this->probeCaseInsensitive($hint);
            }

            protected function flipLastAlphaCase(string $str): string
            {
                return $str; // simulate a path with no ASCII letter
            }
        };
        $this->assertNull($noFlip->probeProxy(__FILE__));
    }

    public function testFlipLastAlphaCase(): void
    {
        $proxy = new class() extends \Com\Tecnick\File\File {
            public function flipProxy(string $str): string
            {
                return $this->flipLastAlphaCase($str);
            }
        };

        $this->assertSame('baR', $proxy->flipProxy('bar'));
        $this->assertSame('bar', $proxy->flipProxy('baR'));
        $this->assertSame('foo.txT', $proxy->flipProxy('foo.txt'));
        $this->assertSame('12.34', $proxy->flipProxy('12.34')); // no letter to flip
    }

    public function testNormalizeUnicodeFoldsNfdToNfc(): void
    {
        if (!\class_exists(\Normalizer::class)) {
            $this->markTestSkipped('ext-intl is not available');
        }

        $proxy = new class() extends \Com\Tecnick\File\File {
            public function normalizeUnicodeProxy(string $str): string
            {
                return $this->normalizeUnicode($str);
            }
        };

        $nfc = "caf\u{00E9}"; // é as a single precomposed code point
        $nfd = "cafe\u{0301}"; // e + combining acute accent

        $this->assertNotSame($nfc, $nfd, 'sanity: the two forms differ as byte strings');
        $this->assertSame($nfc, $proxy->normalizeUnicodeProxy($nfd));
        $this->assertSame($nfc, $proxy->normalizeUnicodeProxy($nfc));

        // Invalid UTF-8 makes Normalizer::normalize() return false; the method
        // must degrade to returning the input unchanged.
        $invalid = "\xff\xfe";
        $this->assertSame($invalid, $proxy->normalizeUnicodeProxy($invalid));
    }

    public function testNormalizeLocalSeparators(): void
    {
        $proxy = new class() extends \Com\Tecnick\File\File {
            public function sepProxy(string $path): string
            {
                return $this->normalizeLocalSeparators($path);
            }
        };

        $this->assertSame('C:/inetpub/wwwroot/path', $proxy->sepProxy('C:\\inetpub\\wwwroot/path'));
        $this->assertSame('/var/www/x', $proxy->sepProxy('/var/www/x'));
    }

    public function testGetAltLocalUrlPathNormalizesWindowsDocumentRoot(): void
    {
        $proxy = new class(['localhost']) extends \Com\Tecnick\File\File {
            public function altLocalUrlPathProxy(string $file): string
            {
                return $this->getAltLocalUrlPath($file);
            }
        };

        $previous = $_SERVER['DOCUMENT_ROOT'] ?? null;
        $_SERVER['DOCUMENT_ROOT'] = 'C:\\inetpub\\wwwroot';
        try {
            $this->assertSame('C:/inetpub/wwwroot/path/test.txt', $proxy->altLocalUrlPathProxy('/path/test.txt'));
        } finally {
            if ($previous === null) {
                unset($_SERVER['DOCUMENT_ROOT']);
            } else {
                $_SERVER['DOCUMENT_ROOT'] = $previous;
            }
        }
    }

    /**
     * @throws \Com\Tecnick\File\Exception
     */
    public function testFopenLocalForcesBinaryMode(): void
    {
        $tmp = \tempnam(\sys_get_temp_dir(), 'tcb');
        $this->assertIsString($tmp);
        \file_put_contents($tmp, 'data');

        // Wildcard allowlist keeps the focus on the mode handling.
        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);

        try {
            $handle = $file->fopenLocal($tmp, 'r');
            $meta = \stream_get_meta_data($handle);
            $this->assertSame('rb', $meta['mode']);
            \fclose($handle);

            // An explicit binary mode is preserved (not doubled).
            $handle = $file->fopenLocal($tmp, 'rb');
            $meta = \stream_get_meta_data($handle);
            $this->assertSame('rb', $meta['mode']);
            \fclose($handle);
        } finally {
            \unlink($tmp);
        }
    }

    public function testStripFileScheme(): void
    {
        $proxy = new class() extends \Com\Tecnick\File\File {
            public function stripProxy(string $file): string
            {
                return $this->stripFileScheme($file);
            }
        };

        // POSIX absolute: the empty-host 'file:///...' form yields a plain path.
        $this->assertSame('/etc/hosts', $proxy->stripProxy('file:///etc/hosts'));
        // Windows drive path: must become the bare path, NOT 'file://C:\...'
        // (which PHP would parse with host "C:" and fail to open).
        $this->assertSame('C:/Users/me/doc.txt', $proxy->stripProxy('file://C:/Users/me/doc.txt'));
        $this->assertSame('C:\\Users\\me\\doc.txt', $proxy->stripProxy('file://C:\\Users\\me\\doc.txt'));
        // A value without the scheme is returned unchanged.
        $this->assertSame('/plain/path', $proxy->stripProxy('/plain/path'));
    }

    /**
     * A 'file://<scheme>://...' input must be rejected even under wildcard
     * trust, so stripping the 'file://' scheme cannot expose an attacker-chosen
     * stream wrapper (php:// arbitrary read, http:// SSRF, phar://, data://) to
     * fopen()/file_get_contents().
     *
     * @throws \Com\Tecnick\File\Exception
     */
    public function testValidatePathRejectsNestedStreamWrapper(): void
    {
        $tmp = \tempnam(\sys_get_temp_dir(), 'tcw');
        $this->assertIsString($tmp);
        \file_put_contents($tmp, 'SECRET-CONTENT');

        // Wildcard trust: the nested-scheme guard must still reject these.
        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);

        $vectors = [
            'file://php://filter/convert.base64-encode/resource=' . $tmp,
            'file://phar://archive.phar/payload',
            'file://http://example.com/resource',
            'file://data://text/plain;base64,U0VDUkVU',
            // The data wrapper also fires WITHOUT '://' (bare 'data:' form),
            // which a plain '://' check would miss.
            'file://data:text/plain;base64,U0VDUkVU',
            'file://data:text/plain,SECRET',
        ];

        try {
            foreach ($vectors as $vector) {
                $candidate = $vector;
                $this->assertFalse($file->isValidFile($candidate), 'isValidFile: ' . $vector);
                $this->assertFalse($file->getLocalFileData($vector), 'getLocalFileData: ' . $vector);
            }
        } finally {
            \unlink($tmp);
        }
    }

    /**
     * The validated path handed to the filesystem must be the bare path, so a
     * real read succeeds (on Windows the 'file://C:\...' form would not open).
     *
     * @throws \Com\Tecnick\File\Exception
     */
    public function testGetLocalFileDataReadsValidatedPath(): void
    {
        $tmp = \tempnam(\sys_get_temp_dir(), 'tcr');
        $this->assertIsString($tmp);
        \file_put_contents($tmp, 'hello-bytes');

        $file = new \Com\Tecnick\File\File([], 52_428_800, [], null, null, ['*']);

        try {
            $this->assertSame('hello-bytes', $file->getLocalFileData($tmp));
        } finally {
            \unlink($tmp);
        }
    }
}
