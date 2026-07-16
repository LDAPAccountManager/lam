<?php

/**
 * LatinTest.php
 *
 * @since       2011-05-23
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 *
 * This file is part of tc-lib-unicode-data software library.
 */

namespace Test;

use PHPUnit\Framework\TestCase;

/**
 * Latin Test
 *
 * @since       2011-05-23
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 */
class LatinTest extends TestCase
{
    public function testMap(): void
    {
        $this->assertEquals(27, \count(\Com\Tecnick\Unicode\Data\Latin::SUBSTITUTE));
    }

    public function testKnownSubstitutions(): void
    {
        $map = \Com\Tecnick\Unicode\Data\Latin::SUBSTITUTE;
        $this->assertSame(128, $map[8364] ?? null); // Euro
        $this->assertSame(153, $map[8482] ?? null); // trademark
        $this->assertSame(156, $map[339] ?? null); // oe
    }
}
