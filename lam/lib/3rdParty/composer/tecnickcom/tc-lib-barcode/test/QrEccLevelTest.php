<?php

/**
 * QrEccLevelTest.php
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

use Com\Tecnick\Barcode\Type\Square\QrCode\QrEccLevel;

/**
 * QrEccLevel enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class QrEccLevelTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('L', QrEccLevel::L->value);
        $this->assertSame('M', QrEccLevel::M->value);
        $this->assertSame('Q', QrEccLevel::Q->value);
        $this->assertSame('H', QrEccLevel::H->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(QrEccLevel::H, QrEccLevel::fromLoose('H'));
        $this->assertSame(QrEccLevel::Q, QrEccLevel::fromLoose('Q'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(QrEccLevel::M, QrEccLevel::fromLoose(QrEccLevel::M));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (QrEccLevel::cases() as $case) {
            $this->assertSame($case, QrEccLevel::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownFallsBackToL(): void
    {
        $this->assertSame(QrEccLevel::L, QrEccLevel::fromLoose('X'));
        $this->assertSame(QrEccLevel::L, QrEccLevel::fromLoose(''));
        $this->assertSame(QrEccLevel::L, QrEccLevel::fromLoose('h'));
    }
}
