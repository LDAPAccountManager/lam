<?php

/**
 * BarcodeTypeTest.php
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Test;

use Com\Tecnick\Barcode\Barcode;
use Com\Tecnick\Barcode\BarcodeType;
use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * BarcodeType enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class BarcodeTypeTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('C128', BarcodeType::C128->value);
        $this->assertSame('C39+', BarcodeType::C39Plus->value);
        $this->assertSame('QRCODE', BarcodeType::QRCODE->value);
        $this->assertSame('DATAMATRIX', BarcodeType::DATAMATRIX->value);
        $this->assertCount(37, BarcodeType::cases());
    }

    /**
     * Every enum backing value must be a valid Barcode::BARCODETYPES key.
     */
    public function testCasesMatchBarcodeTypesKeys(): void
    {
        $keys = \array_keys(Barcode::BARCODETYPES);
        foreach (BarcodeType::cases() as $case) {
            $this->assertContains($case->value, $keys);
        }
        $this->assertCount(\count($keys), BarcodeType::cases());
    }

    /**
     * @throws BarcodeException
     */
    public function testFromLooseCanonical(): void
    {
        $this->assertSame(BarcodeType::QRCODE, BarcodeType::fromLoose('QRCODE'));
        $this->assertSame(BarcodeType::S25Plus, BarcodeType::fromLoose('S25+'));
    }

    /**
     * @throws BarcodeException
     */
    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(BarcodeType::AZTEC, BarcodeType::fromLoose(BarcodeType::AZTEC));
    }

    /**
     * @throws BarcodeException
     */
    public function testFromLooseRoundTrip(): void
    {
        foreach (BarcodeType::cases() as $case) {
            $this->assertSame($case, BarcodeType::fromLoose($case->value));
        }
    }

    /**
     * @throws BarcodeException
     */
    public function testFromLooseUnknownThrows(): void
    {
        $this->bcExpectException(BarcodeException::class);
        BarcodeType::fromLoose('NOPE');
    }

    /**
     * The widened getBarcodeObj() accepts a BarcodeType enum and produces the
     * same barcode as the equivalent leading-token string.
     *
     * @throws BarcodeException
     * @throws \Com\Tecnick\Color\Exception
     */
    public function testGetBarcodeObjAcceptsEnum(): void
    {
        $barcode = new Barcode();
        $fromEnum = $barcode->getBarcodeObj(BarcodeType::C39, 'TEST123');
        $fromString = $barcode->getBarcodeObj('C39', 'TEST123');
        $this->assertSame($fromString->getSvgCode(), $fromEnum->getSvgCode());
    }
}
