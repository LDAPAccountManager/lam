<?php

/**
 * ArabicTest.php
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
 * Arabic Test
 *
 * @since       2011-05-23
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 */
class ArabicTest extends TestCase
{
    public function testDiacritic(): void
    {
        $this->assertEquals(5, \count(\Com\Tecnick\Unicode\Data\Arabic::DIACRITIC));
    }

    public function testlaa(): void
    {
        $this->assertEquals(4, \count(\Com\Tecnick\Unicode\Data\Arabic::LAA));
    }

    public function testSubstitute(): void
    {
        $this->assertEquals(76, \count(\Com\Tecnick\Unicode\Data\Arabic::SUBSTITUTE));
    }
}
