<?php

/**
 * TypeCoverageTest.php
 *
 * @since       2026-05-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test;

use Test\Fixture\InternalBarcodeType;

class TypeCoverageTest extends TestUtil
{
    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testBarsArrayFiltersInvalidBarsInBothPasses(): void
    {
        $type = new InternalBarcodeType();
        $type->setRowsColsForTest(3, 3);
        $type->setRatiosForTest(1.0, 1.0);
        $type->setBarsForTest([
            [0, 0, 0, 1],
            [0, 0, 1, 0],
            [1, 1, 1, 1],
        ]);
        $type->setRotatedBarsOverrideForTest([
            [0, 0, 0, 1],
            [0, 0, 1, 0],
            [2, 2, 1, 1],
        ]);

        $this->assertSame(
            [
                [2.0, 7.0, 2.0, 7.0],
                [3.0, 8.0, 3.0, 8.0],
            ],
            $type->getBarsArrayXYXY(),
        );

        $this->assertSame(
            [
                [2.0, 7.0, 1.0, 1.0],
                [3.0, 8.0, 1.0, 1.0],
            ],
            $type->getBarsArrayXYWH(),
        );
    }
}
