<?php

/**
 * AztecHintTest.php
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

use Com\Tecnick\Barcode\Type\Square\Aztec\AztecHint;

/**
 * AztecHint enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class AztecHintTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('A', AztecHint::Automatic->value);
        $this->assertSame('B', AztecHint::Binary->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(AztecHint::Automatic, AztecHint::fromLoose('A'));
        $this->assertSame(AztecHint::Binary, AztecHint::fromLoose('B'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(AztecHint::Binary, AztecHint::fromLoose(AztecHint::Binary));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (AztecHint::cases() as $case) {
            $this->assertSame($case, AztecHint::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownFallsBackToAutomatic(): void
    {
        $this->assertSame(AztecHint::Automatic, AztecHint::fromLoose('C'));
        $this->assertSame(AztecHint::Automatic, AztecHint::fromLoose(''));
    }
}
