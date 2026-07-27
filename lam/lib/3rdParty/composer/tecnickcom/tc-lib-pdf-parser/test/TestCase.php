<?php

/**
 * TestCase.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Pdfparser
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-parser
 *
 * This file is part of tc-lib-pdf-parser software library.
 */

namespace Test;

use PHPUnit\Framework\TestCase as FrameworkTestCase;

/**
 * Base test case with cross-version helpers.
 */
abstract class TestCase extends FrameworkTestCase
{
    /**
     * Assert that the expected exception message contains the given substring.
     *
     * Uses expectExceptionMessageMatches(), the only message assertion that is
     * available and not deprecated across PHPUnit 11.5, 12.5 and 13.2 (PHP 8.2+).
     * The deprecated expectExceptionMessage() and the 13.2-only
     * expectExceptionMessageIsOrContains() are intentionally avoided.
     *
     * @param string $message Substring expected within the exception message.
     */
    protected function expectExceptionMessageContains(string $message): void
    {
        $this->expectExceptionMessageMatches('/' . preg_quote($message, '/') . '/');
    }
}
