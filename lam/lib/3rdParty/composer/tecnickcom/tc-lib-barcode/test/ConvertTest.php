<?php

/**
 * ConvertTest.php
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

use Test\Fixture\InternalConvert;

class ConvertTest extends TestUtil
{
    /**
     * @throws \Com\Tecnick\Barcode\Exception
     */
    public function testProcessBinarySequenceThrowsOnEmptyRows(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $helper = new InternalConvert();
        $helper->exposeProcessBinarySequence([]);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     */
    public function testProcessBinarySequenceParsesStringRows(): void
    {
        $helper = new InternalConvert();
        $helper->exposeProcessBinarySequence(['1010', '0101']);

        $this->assertSame(
            [
                [0, 0, 1, 1],
                [2, 0, 1, 1],
                [1, 1, 1, 1],
                [3, 1, 1, 1],
            ],
            $helper->getBars(),
        );
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     */
    public function testRawRowsAndNumberConversionBranches(): void
    {
        $helper = new InternalConvert();

        $this->assertSame(['1010', '0101'], $helper->exposeGetRawCodeRows(' ,[1010,0101], '));

        $this->assertSame('00', $helper->exposeConvertDecToHex('abc'));
        $this->assertSame('00', $helper->exposeConvertDecToHex('0'));
        $this->assertSame('FF', $helper->exposeConvertDecToHex('255'));
        $this->assertSame('255', $helper->exposeConvertHexToDec('FF'));
    }

    public function testGridArraySkipsInvalidBarsAndRotatedEarlyReturn(): void
    {
        $helper = new InternalConvert();
        $helper->setColsRows(5, 3);
        $helper->setBars([
            [0, 0, 0, 1],
            [1, 0, 2, 0],
            [1, 1, 2, 1],
        ]);

        $grid = $helper->getGridArray('.', '#');
        $this->assertSame(
            [
                ['.', '.', '.', '.', '.'],
                ['.', '#', '#', '.', '.'],
                ['.', '.', '.', '.', '.'],
            ],
            $grid,
        );

        $helper->setColsRows(0, 0);
        $helper->setBars([]);
        $this->assertSame([], $helper->exposeGetRotatedBarArray());
    }

    public function testRectHelpersAndRotatedBarsExtraction(): void
    {
        $helper = new InternalConvert();
        $helper->setColsRows(3, 2);
        $helper->setBars([
            [0, 0, 1, 2],
            [2, 0, 1, 2],
        ]);
        $helper->setRatiosAndPadding(2.0, 3.0, ['T' => 4, 'R' => 0, 'B' => 0, 'L' => 5]);

        $this->assertSame(
            [
                [0, 0, 1, 2],
                [2, 0, 1, 2],
            ],
            $helper->exposeGetRotatedBarArray(),
        );

        $this->assertSame([7.0, 7.0, 8.0, 9.0], $helper->exposeGetBarRectXYXY([1, 1, 1, 1]));
        $this->assertSame([7.0, 7.0, 2.0, 3.0], $helper->exposeGetBarRectXYWH([1, 1, 1, 1]));
    }
}
