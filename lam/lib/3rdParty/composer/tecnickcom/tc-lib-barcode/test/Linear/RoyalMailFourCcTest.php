<?php

/**
 * RoyalMailFourCcTest.php
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
class RoyalMailFourCcTest extends TestUtil
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
        $barcode = $this->getTestObject();
        $type = $barcode->getBarcodeObj('RMS4CC', '0123456789');
        $grid = $type->getGrid();
        $expected =
            "1000001010000010100000101000001010000010100000101000100010001000100010001000100010001000101\n"
            . "1010101010101010101010101010101010101010101010101010101010101010101010101010101010101010101\n"
            . "0000001010001000100010100010000010100010001010000000001010001000100010100010000010000010101\n";
        $this->assertEquals($expected, $grid);
    }

    /**
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testInvalidInput(): void
    {
        $this->bcExpectException(\Com\Tecnick\Barcode\Exception::class);
        $barcode = $this->getTestObject();
        $barcode->getBarcodeObj('RMS4CC', '}{');
    }

    /**
     * Regression: a letter-valued check character used to be mangled to '0'.
     *
     * @throws \Com\Tecnick\Barcode\Exception
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testLetterCheckDigit(): void
    {
        $barcode = $this->getTestObject();
        $this->assertSame('1234J', $barcode->getBarcodeObj('RMS4CC', '1234')->getExtendedCode());
        $this->assertSame('CU9NH', $barcode->getBarcodeObj('RMS4CC', 'CU9N')->getExtendedCode());
    }
}
