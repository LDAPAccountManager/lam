<?php

/**
 * TestUtil.php
 *
 * @since     2020-12-19
 * @category  Library
 * @package   file
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 *
 * This file is part of tc-lib-file software library.
 */

namespace Test;

use PHPUnit\Framework\TestCase;

/**
 * Test Util
 *
 * @since     2020-12-19
 * @category  Library
 * @package   file
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-file
 */
class TestUtil extends TestCase
{
    /**
     * @param class-string<\Throwable> $exception
     */
    public function bcExpectException(string $exception): void
    {
        parent::expectException($exception);
    }

    /**
     * Execute a callback and assert that it triggers a matching user deprecation.
     *
     * @param callable():void $callback
     */
    public function bcAssertUserDeprecationMessageMatches(string $pattern, callable $callback): void
    {
        $messages = [];

        \set_error_handler(static function (int $errno, string $errstr) use (&$messages): bool {
            if ($errno !== E_USER_DEPRECATED) {
                return false;
            }

            $messages[] = $errstr;
            return true;
        });

        try {
            $callback();
        } finally {
            \restore_error_handler();
        }

        $this->assertNotEmpty($messages, 'Expected a user deprecation but none was triggered.');

        foreach ($messages as $message) {
            if (\preg_match($pattern, $message) === 1) {
                return;
            }
        }

        $this->fail(
            'User deprecation message did not match pattern ' . $pattern . '. Got: ' . \implode(' | ', $messages),
        );
    }

    /**
     * Execute a callback while swallowing user deprecations.
     *
     * @template T
     * @param callable():T $callback
     * @return T
     */
    public function bcRunIgnoringUserDeprecations(callable $callback): mixed
    {
        \set_error_handler(static fn(int $errno): bool => $errno === E_USER_DEPRECATED);

        try {
            return $callback();
        } finally {
            \restore_error_handler();
        }
    }
}
