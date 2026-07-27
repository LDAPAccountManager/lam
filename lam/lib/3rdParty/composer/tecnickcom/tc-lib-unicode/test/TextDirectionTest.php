<?php

/**
 * TextDirectionTest.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 *
 * This file is part of tc-lib-unicode software library.
 */

namespace Test;

use Com\Tecnick\Unicode\Bidi;
use Com\Tecnick\Unicode\TextDirection;

/**
 * TextDirection enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class TextDirectionTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('', TextDirection::Auto->value);
        $this->assertSame('R', TextDirection::Rtl->value);
        $this->assertSame('L', TextDirection::Ltr->value);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(TextDirection::Auto, TextDirection::fromLoose(''));
        $this->assertSame(TextDirection::Rtl, TextDirection::fromLoose('R'));
        $this->assertSame(TextDirection::Ltr, TextDirection::fromLoose('L'));
    }

    public function testFromLooseIsLenientOnFirstCharacter(): void
    {
        $this->assertSame(TextDirection::Rtl, TextDirection::fromLoose('r'));
        $this->assertSame(TextDirection::Ltr, TextDirection::fromLoose('l'));
        $this->assertSame(TextDirection::Rtl, TextDirection::fromLoose('RTL'));
        $this->assertSame(TextDirection::Ltr, TextDirection::fromLoose('ltr'));
        $this->assertSame(TextDirection::Rtl, TextDirection::fromLoose('right'));
        $this->assertSame(TextDirection::Ltr, TextDirection::fromLoose('left'));
    }

    public function testFromLooseFallsBackToAuto(): void
    {
        $this->assertSame(TextDirection::Auto, TextDirection::fromLoose('X'));
        $this->assertSame(TextDirection::Auto, TextDirection::fromLoose('auto'));
        $this->assertSame(TextDirection::Auto, TextDirection::fromLoose('1'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(TextDirection::Rtl, TextDirection::fromLoose(TextDirection::Rtl));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (TextDirection::cases() as $case) {
            $this->assertSame($case, TextDirection::fromLoose($case->value));
        }
    }

    /**
     * The widened Bidi constructor accepts a TextDirection and behaves exactly
     * like the equivalent legacy string.
     *
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testBidiAcceptsEnum(): void
    {
        $fromEnum = new Bidi('left to right', null, null, TextDirection::Rtl, true);
        $fromString = new Bidi('left to right', null, null, 'R', true);
        $this->assertSame('right to left', $fromEnum->getString());
        $this->assertSame($fromString->getString(), $fromEnum->getString());
    }
}
