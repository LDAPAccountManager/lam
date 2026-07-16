<?php

/**
 * MirrorTest.php
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
 * Mirror Test
 *
 * @since       2011-05-23
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 */
class MirrorTest extends TestCase
{
    public function testMap(): void
    {
        $this->assertEquals(352, \count(\Com\Tecnick\Unicode\Data\Mirror::UNI));
    }

    public function testSymmetry(): void
    {
        $map = \Com\Tecnick\Unicode\Data\Mirror::UNI;
        foreach ($map as $from => $to) {
            $this->assertArrayHasKey($to, $map);
            $this->assertSame($from, $map[$to] ?? null);
        }
    }
}
