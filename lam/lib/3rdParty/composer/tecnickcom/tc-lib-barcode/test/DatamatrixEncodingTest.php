<?php

/**
 * DatamatrixEncodingTest.php
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

use Com\Tecnick\Barcode\Type\Square\Datamatrix\DatamatrixEncoding;

/**
 * DatamatrixEncoding enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class DatamatrixEncodingTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('ASCII', DatamatrixEncoding::ASCII->value);
        $this->assertSame('C40', DatamatrixEncoding::C40->value);
        $this->assertSame('TXT', DatamatrixEncoding::TXT->value);
        $this->assertSame('X12', DatamatrixEncoding::X12->value);
        $this->assertSame('EDF', DatamatrixEncoding::EDF->value);
        $this->assertSame('BASE256', DatamatrixEncoding::BASE256->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(DatamatrixEncoding::C40, DatamatrixEncoding::fromLoose('C40'));
        $this->assertSame(DatamatrixEncoding::BASE256, DatamatrixEncoding::fromLoose('BASE256'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(DatamatrixEncoding::X12, DatamatrixEncoding::fromLoose(DatamatrixEncoding::X12));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (DatamatrixEncoding::cases() as $case) {
            $this->assertSame($case, DatamatrixEncoding::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownFallsBackToAscii(): void
    {
        $this->assertSame(DatamatrixEncoding::ASCII, DatamatrixEncoding::fromLoose('EDIFACT'));
        $this->assertSame(DatamatrixEncoding::ASCII, DatamatrixEncoding::fromLoose(''));
    }
}
