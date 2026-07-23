<?php

/**
 * RawTest.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test\Linear;

use Test\TestUtil;

/**
 * Barcode class test
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class RawTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Barcode\Barcode
    {
        return new \Com\Tecnick\Barcode\Barcode();
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetGrid(): void
    {
        $testObj = $this->getTestObject();
        $testObj = $this->getTestObject();

        $type = $testObj->getBarcodeObj('LRAW', '01001100011100001111,10110011100011110000');
        $grid = $type->getGrid();
        $expected = "01001100011100001111\n10110011100011110000\n";
        $this->assertEquals($expected, $grid);
    }

    /**
     * Regression: bracket-separated rows used to be merged into a single row.
     *
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testBracketSeparatedRows(): void
    {
        $testObj = $this->getTestObject();
        $type = $testObj->getBarcodeObj('LRAW', '[101][010]');
        $this->assertSame("101\n010\n", $type->getGrid());
    }
}
