<?php

/**
 * TestUtil.php
 *
 * @since     2020-12-19
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 *
 * This file is part of tc-lib-color software library.
 */

namespace Test;

use PHPUnit\Framework\TestCase;

/**
 * Web Color class test
 *
 * @since     2020-12-19
 * @category  Library
 * @package   PdfImage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-image
 */
class TestUtil extends TestCase
{
    protected function getTestFileHelper(): \Com\Tecnick\File\File
    {
        return new \Com\Tecnick\File\File(allowedHosts: ['*'], allowedPaths: ['*']);
    }

    protected function getTestEncrypt(): \Com\Tecnick\Pdf\Encrypt\Encrypt
    {
        return new class extends \Com\Tecnick\Pdf\Encrypt\Encrypt {
            public function __construct() {}

            public function encryptString(string $str, int $objnum = 0): string
            {
                return $str;
            }

            public function escapeDataString(string $str, int $objnum = 0): string
            {
                return '(' . $str . ')';
            }
        };
    }

    /**
     * @param class-string<\Throwable> $exception
     */
    public function bcExpectException(string $exception): void
    {
        parent::expectException($exception);
    }
}
