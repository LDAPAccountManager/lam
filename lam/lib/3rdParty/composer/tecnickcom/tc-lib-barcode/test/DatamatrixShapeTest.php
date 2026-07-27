<?php

/**
 * DatamatrixShapeTest.php
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

use Com\Tecnick\Barcode\Type\Square\Datamatrix\DatamatrixShape;

/**
 * DatamatrixShape enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class DatamatrixShapeTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('S', DatamatrixShape::Square->value);
        $this->assertSame('R', DatamatrixShape::Rectangular->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(DatamatrixShape::Square, DatamatrixShape::fromLoose('S'));
        $this->assertSame(DatamatrixShape::Rectangular, DatamatrixShape::fromLoose('R'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(DatamatrixShape::Rectangular, DatamatrixShape::fromLoose(DatamatrixShape::Rectangular));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (DatamatrixShape::cases() as $case) {
            $this->assertSame($case, DatamatrixShape::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownFallsBackToSquare(): void
    {
        $this->assertSame(DatamatrixShape::Square, DatamatrixShape::fromLoose('r'));
        $this->assertSame(DatamatrixShape::Square, DatamatrixShape::fromLoose(''));
        $this->assertSame(DatamatrixShape::Square, DatamatrixShape::fromLoose('X'));
    }
}
