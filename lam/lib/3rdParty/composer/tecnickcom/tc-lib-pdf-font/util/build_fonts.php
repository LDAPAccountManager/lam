#!/usr/bin/env php
<?php
/**
 * build_fonts.php
 *
 * @since       2026-10-03
 * @category    Library
 * @package     PdfFont
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 *
 * Command-line tool to download the font sources and convert them into target/fonts.
 * Installs the library dependencies when no Composer autoloader is found.
 * Clears target/fonts before the conversion.
 * It does not require make or a POSIX shell.
 *
 * Usage:
 *     php util/build_fonts.php
 *
 * Exit codes:
 *     1     not run from the command line
 *     7     unable to remove target/fonts
 *     8     composer install failed
 *     other exit code of bulk_convert.php
 */

if (\php_sapi_name() != 'cli') {
    \fwrite(STDERR, 'You need to run this command from console.'."\n");
    exit(1);
}

/**
 * Run a command, streaming its output, and return the exit code.
 * An array command is executed without a shell.
 *
 * @param array<int, string>|string $cmd Command to execute.
 * @param string                    $cwd Working directory.
 */
function runCommand(array|string $cmd, string $cwd): int
{
    $proc = \proc_open($cmd, array(0 => STDIN, 1 => STDOUT, 2 => STDERR), $pipes, $cwd);
    if ($proc === false) {
        return 127;
    }
    $code = \proc_close($proc);
    return ($code < 0) ? 1 : $code;
}

/**
 * Run "composer install --no-dev" in the specified directory.
 * Uses the Composer binary set in the COMPOSER_BINARY environment variable when available.
 *
 * @param string $cwd Working directory.
 */
function composerInstall(string $cwd): int
{
    $args = array('install', '--no-dev', '--no-interaction');
    $bin = \getenv('COMPOSER_BINARY');
    if (\is_string($bin) && ($bin !== '') && \is_file($bin)) {
        return runCommand(\array_merge(array(PHP_BINARY, '-d', 'apc.enable_cli=0', $bin), $args), $cwd);
    }
    return runCommand('composer '.\implode(' ', $args), $cwd);
}

/**
 * Remove a symbolic link.
 *
 * @param string $path Link to remove.
 */
function removeLink(string $path): bool
{
    // Windows removes directory links with rmdir()
    if ((PHP_OS_FAMILY === 'Windows') && \is_dir($path)) {
        return \rmdir($path);
    }
    return \unlink($path);
}

/**
 * Recursively delete a directory.
 * Symbolic links are removed without deleting their targets.
 *
 * @param string $dir Directory to delete.
 */
function removeDir(string $dir): bool
{
    if (\is_link($dir)) {
        return removeLink($dir);
    }
    if (!\is_dir($dir)) {
        return true;
    }
    $items = new RecursiveIteratorIterator(
        new RecursiveDirectoryIterator($dir, FilesystemIterator::SKIP_DOTS),
        RecursiveIteratorIterator::CHILD_FIRST
    );
    foreach ($items as $item) {
        $path = $item->getPathname();
        if ($item->isLink()) {
            $removed = removeLink($path);
        } elseif ($item->isDir()) {
            $removed = \rmdir($path);
        } else {
            $removed = \unlink($path);
        }
        if (!$removed) {
            return false;
        }
    }
    // release the directory handle held by the iterator
    unset($items);
    return \rmdir($dir);
}

/**
 * Check whether a Composer autoloader is available for bulk_convert.php.
 */
function hasAutoloader(): bool
{
    foreach (
        array(
            \dirname(__DIR__).'/vendor',  // standalone repository checkout
            \dirname(__DIR__, 3),         // installed under <vendor-dir>/tecnickcom/tc-lib-pdf-font
        ) as $vendorDir
    ) {
        if (\is_file($vendorDir.'/autoload.php') && \is_file($vendorDir.'/composer/installed.json')) {
            return true;
        }
    }
    return false;
}

$pkgdir = \dirname(__DIR__);
$fontdir = $pkgdir.'/target/fonts';

if (!hasAutoloader()) {
    \fwrite(STDOUT, '>>> Installing the library dependencies'."\n");
    if (composerInstall($pkgdir) !== 0) {
        \fwrite(STDERR, 'ERROR: composer install failed in '.$pkgdir."\n\n");
        exit(8);
    }
}

\fwrite(STDOUT, '>>> Downloading the font sources'."\n");
if (composerInstall(__DIR__) !== 0) {
    \fwrite(STDERR, 'ERROR: composer install failed in '.__DIR__."\n\n");
    exit(8);
}

\fwrite(STDOUT, '>>> Removing '.$fontdir."\n");
if (!removeDir($fontdir)) {
    \fwrite(STDERR, 'ERROR: Unable to remove '.$fontdir."\n\n");
    exit(7);
}

exit(runCommand(array(PHP_BINARY, 'bulk_convert.php'), __DIR__));
