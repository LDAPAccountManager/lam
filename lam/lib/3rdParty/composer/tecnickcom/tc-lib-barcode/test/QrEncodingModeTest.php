<?php

/**
 * QrEncodingModeTest.php
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

use Com\Tecnick\Barcode\Type\Square\QrCode\QrEncodingMode;

/**
 * QrEncodingMode enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class QrEncodingModeTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('NL', QrEncodingMode::NL->value);
        $this->assertSame('NM', QrEncodingMode::NM->value);
        $this->assertSame('AN', QrEncodingMode::AN->value);
        $this->assertSame('8B', QrEncodingMode::Byte->value);
        $this->assertSame('KJ', QrEncodingMode::KJ->value);
        $this->assertSame('ST', QrEncodingMode::ST->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(QrEncodingMode::Byte, QrEncodingMode::fromLoose('8B'));
        $this->assertSame(QrEncodingMode::AN, QrEncodingMode::fromLoose('AN'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(QrEncodingMode::KJ, QrEncodingMode::fromLoose(QrEncodingMode::KJ));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (QrEncodingMode::cases() as $case) {
            $this->assertSame($case, QrEncodingMode::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownFallsBackToByte(): void
    {
        $this->assertSame(QrEncodingMode::Byte, QrEncodingMode::fromLoose('ZZ'));
        $this->assertSame(QrEncodingMode::Byte, QrEncodingMode::fromLoose(''));
    }
}
