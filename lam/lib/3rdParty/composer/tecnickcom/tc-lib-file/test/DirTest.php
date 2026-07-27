<?php

/**
 * DirTest.php
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

namespace Test;

use PHPUnit\Framework\Attributes\DataProvider;
use PHPUnit\Framework\Attributes\PreserveGlobalState;
use PHPUnit\Framework\Attributes\RunInSeparateProcess;

/**
 * File Color class test
 *
 * @since     2015-07-28
 * @category  Library
 * @package   File
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
class DirTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\File\Dir
    {
        return new \Com\Tecnick\File\Dir();
    }

    #[DataProvider('getAltFilePathsDataProvider')]
    public function testGetAltFilePaths(string $name, string $expected): void
    {
        $testObj = $this->getTestObject();
        $dir = $testObj->findParentDir($name);
        $this->bcAssertMatchesRegularExpression('#' . $expected . '#', $dir);
    }

    /**
     * @return array<array{string, string}>
     */
    public static function getAltFilePathsDataProvider(): array
    {
        return [['', '/src/'], ['missing', '/'], ['src', '/src/']];
    }

    /**
     * The upward directory search must not raise open_basedir warnings when it walks
     * above the allowed paths (see issue #238). Runs in a separate process because
     * open_basedir cannot be relaxed once set.
     */
    #[RunInSeparateProcess]
    #[PreserveGlobalState(false)]
    public function testFindParentDirUnderOpenBasedir(): void
    {
        $libRoot = \dirname(__DIR__);

        // Restrict file access to the library tree (the temp dir is required by the
        // test runner). The ancestors of $libRoot fall outside this restriction, so
        // probing them would raise open_basedir warnings without the guard.
        // @mago-expect lint:no-ini-set -- open_basedir can only be set at runtime in a test.
        \ini_set('open_basedir', $libRoot . PATH_SEPARATOR . \sys_get_temp_dir());

        $warnings = [];
        \set_error_handler(static function (int $_errno, string $errstr) use (&$warnings): bool {
            $warnings[] = $errstr;
            return true;
        });

        try {
            $dir = $this->getTestObject()->findParentDir('missing', $libRoot . '/src');
        } finally {
            \restore_error_handler();
        }

        $this->assertSame([], $warnings, 'open_basedir restriction must not raise warnings');
        $this->bcAssertMatchesRegularExpression('#/$#', $dir);
    }
}
