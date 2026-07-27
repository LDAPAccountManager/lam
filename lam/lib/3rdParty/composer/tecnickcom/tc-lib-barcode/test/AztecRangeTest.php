<?php

/**
 * AztecRangeTest.php
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

use Com\Tecnick\Barcode\Type\Square\Aztec\AztecRange;

/**
 * AztecRange enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class AztecRangeTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('A', AztecRange::Automatic->value);
        $this->assertSame('F', AztecRange::FullRange->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(AztecRange::Automatic, AztecRange::fromLoose('A'));
        $this->assertSame(AztecRange::FullRange, AztecRange::fromLoose('F'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(AztecRange::FullRange, AztecRange::fromLoose(AztecRange::FullRange));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (AztecRange::cases() as $case) {
            $this->assertSame($case, AztecRange::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownFallsBackToAutomatic(): void
    {
        $this->assertSame(AztecRange::Automatic, AztecRange::fromLoose('X'));
        $this->assertSame(AztecRange::Automatic, AztecRange::fromLoose(''));
    }
}
