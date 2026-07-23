<?php

/**
 * TypeTest.php
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
 * Type Test
 *
 * @since       2011-05-23
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 */
class TypeTest extends TestCase
{
    public function testStrong(): void
    {
        $this->assertEquals(3, \count(\Com\Tecnick\Unicode\Data\Type::STRONG));
    }

    public function testWeak(): void
    {
        $this->assertEquals(7, \count(\Com\Tecnick\Unicode\Data\Type::WEAK));
    }

    public function testNeutral(): void
    {
        $this->assertEquals(4, \count(\Com\Tecnick\Unicode\Data\Type::NEUTRAL));
    }

    public function testExplicitFormatting(): void
    {
        $this->assertEquals(9, \count(\Com\Tecnick\Unicode\Data\Type::EXPLICIT_FORMATTING));
    }

    public function testUni(): void
    {
        $this->assertEquals(17720, \count(\Com\Tecnick\Unicode\Data\Type::UNI));
    }
}
