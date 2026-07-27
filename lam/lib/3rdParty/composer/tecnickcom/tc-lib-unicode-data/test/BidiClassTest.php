<?php

/**
 * BidiClassTest.php
 *
 * @since       2026-07-17
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 *
 * This file is part of tc-lib-unicode-data software library.
 */

namespace Test;

use Com\Tecnick\Unicode\Data\BidiClass;
use Com\Tecnick\Unicode\Data\Type;
use PHPUnit\Framework\TestCase;

/**
 * BidiClass enum test
 *
 * @since       2026-07-17
 * @category    Library
 * @package     UnicodeData
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-unicode-data
 */
class BidiClassTest extends TestCase
{
    public function testCasesMatchTypeGroups(): void
    {
        $values = \array_map(static fn(BidiClass $case): string => $case->value, BidiClass::cases());
        $expected = \array_merge(\array_keys(Type::STRONG), \array_keys(Type::WEAK), \array_keys(Type::NEUTRAL));
        $this->assertSame($expected, $values);
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(BidiClass::L, BidiClass::fromLoose('L'));
        $this->assertSame(BidiClass::NSM, BidiClass::fromLoose('NSM'));
        $this->assertSame(BidiClass::ON, BidiClass::fromLoose('ON'));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(BidiClass::AL, BidiClass::fromLoose(BidiClass::AL));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (BidiClass::cases() as $case) {
            $this->assertSame($case, BidiClass::fromLoose($case->value));
        }
    }

    public function testFromLooseUnknownThrows(): void
    {
        $this->expectException(\ValueError::class);
        BidiClass::fromLoose('ZZ');
    }

    public function testFromLooseRejectsExplicitFormattingCode(): void
    {
        // LRE is a valid UNI value but an explicit formatting code, not a BidiClass.
        $this->expectException(\ValueError::class);
        BidiClass::fromLoose('LRE');
    }

    public function testCategoryHelpers(): void
    {
        $this->assertTrue(BidiClass::R->isStrong());
        $this->assertFalse(BidiClass::R->isWeak());
        $this->assertFalse(BidiClass::R->isNeutral());

        $this->assertTrue(BidiClass::EN->isWeak());
        $this->assertFalse(BidiClass::EN->isStrong());

        $this->assertTrue(BidiClass::WS->isNeutral());
        $this->assertFalse(BidiClass::WS->isStrong());
    }

    public function testEveryCaseHasExactlyOneCategory(): void
    {
        foreach (BidiClass::cases() as $case) {
            $count = (int) $case->isStrong() + (int) $case->isWeak() + (int) $case->isNeutral();
            $this->assertSame(1, $count, 'BidiClass ' . $case->value . ' must belong to exactly one category');
        }
    }

    public function testGetBidiClassForMappedCodePoint(): void
    {
        $this->assertSame(BidiClass::L, Type::getBidiClass(65)); // 'A'
        $this->assertSame(BidiClass::WS, Type::getBidiClass(32)); // space
        $this->assertSame(BidiClass::B, Type::getBidiClass(10)); // line feed
    }

    public function testGetBidiClassIsNullForExplicitOrUnmapped(): void
    {
        $this->assertNull(Type::getBidiClass(8234)); // LRE explicit formatting code
        $this->assertNull(Type::getBidiClass(0x10FFFF)); // unmapped code point
    }
}
