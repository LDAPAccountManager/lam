<?php

/**
 * TestUtil.php
 *
 * @since     2020-12-19
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
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
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 *
 * @SuppressWarnings("PHPMD.NumberOfChildren")
 */
class TestUtil extends TestCase
{
    public function bcAssertEqualsWithDelta(
        mixed $expected,
        mixed $actual,
        float $delta = 0.01,
        string $message = '',
    ): void {
        parent::assertEqualsWithDelta($expected, $actual, $delta, $message);
    }

    /**
     * @param class-string<\Throwable> $exception
     */
    public function bcExpectException(string $exception): void
    {
        if (!\is_a($exception, \Throwable::class, true)) {
            self::fail('Expected a throwable class name.');
        }

        parent::expectException($exception);
    }

    /**
     * @return array<int, string>
     */
    protected function getResponseHeaders(): array
    {
        if (\function_exists('xdebug_get_headers')) {
            /** @var list<string> $rawHeaders */
            $rawHeaders = xdebug_get_headers();
            $headers = [];
            foreach ($rawHeaders as $header) {
                $headers[] = $header;
            }

            return $headers;
        }

        return \headers_list();
    }
}
