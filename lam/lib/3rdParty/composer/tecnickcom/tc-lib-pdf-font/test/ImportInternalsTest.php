<?php

/**
 * ImportInternalsTest.php
 *
 * @since     2026-05-05
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Font\Import;

/**
 * Tests for protected methods of Import that cannot be reached through the
 * normal public constructor path in isolation.
 *
 * @since     2026-05-05
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
class ImportInternalsTest extends TestUtil
{
    private function buildImport(): Import
    {
        $class = new \ReflectionClass(Import::class);
        return $class->newInstanceWithoutConstructor();
    }

    /**
     * @param array<int, mixed> $args
     */
    private function callStringMethod(object $obj, string $method, array $args = []): string
    {
        $ref = new \ReflectionMethod($obj, $method);

        if (!\is_string($ref->invokeArgs($obj, $args))) {
            $this->fail('Expected string return value.');
        }

        return (string) $ref->invokeArgs($obj, $args);
    }

    private function setFdt(object $obj, mixed $fdt): void
    {
        $prop = new \ReflectionProperty($obj, 'fdt');
        $prop->setValue($obj, $fdt);
    }

    // -------------------------------------------------------------------------
    // updateCIDtoGIDmap
    // -------------------------------------------------------------------------

    public function testUpdateCIDtoGIDmapSetsGlyphPairBytes(): void
    {
        $instance = $this->buildImport();
        // 65536 CID slots × 2 bytes = 131072 bytes
        $map = str_repeat("\x00", 131072);
        $result = $this->callStringMethod($instance, 'updateCIDtoGIDmap', [$map, 65, 42]);
        // gid 42 = 0x002A → high byte = 0x00, low byte = 0x2A
        $this->assertSame(0, ord($result[65 * 2]));
        $this->assertSame(42, ord($result[(65 * 2) + 1]));
    }

    public function testUpdateCIDtoGIDmapIgnoresCidOutOfRange(): void
    {
        $instance = $this->buildImport();
        $map = str_repeat("\x00", 131072);
        $result = $this->callStringMethod($instance, 'updateCIDtoGIDmap', [$map, 0x10000, 5]);
        // CID 0x10000 is out of the 0..0xFFFF range → map unchanged
        $this->assertSame($map, $result);
    }

    public function testUpdateCIDtoGIDmapIgnoresGidAbove0xffff(): void
    {
        $instance = $this->buildImport();
        $map = str_repeat("\x00", 131072);
        // gid = 0x1002A is outside the representable 16-bit range → entry left as 0 (notdef)
        $result = $this->callStringMethod($instance, 'updateCIDtoGIDmap', [$map, 0, 0x1002A]);
        $this->assertSame(0, ord($result[0]));
        $this->assertSame(0, ord($result[1]));
    }

    public function testUpdateCIDtoGIDmapIgnoresNegativeGid(): void
    {
        $instance = $this->buildImport();
        $map = str_repeat("\x00", 131072);
        // gid < 0 → condition ($gid >= 0) is false → map unchanged
        $result = $this->callStringMethod($instance, 'updateCIDtoGIDmap', [$map, 10, -1]);
        $this->assertSame($map, $result);
    }

    // -------------------------------------------------------------------------
    // getEncodingTable
    // -------------------------------------------------------------------------

    public function testGetEncodingTableReturnsCp1252ForType1NonSymbolic(): void
    {
        $instance = $this->buildImport();
        $this->setFdt($instance, ['type' => 'Type1', 'Flags' => 32]);
        // Flags & 4 == 0 → non-symbolic Type1 → cp1252
        $result = $this->callStringMethod($instance, 'getEncodingTable', ['']);
        $this->assertSame('cp1252', $result);
    }

    public function testGetEncodingTableReturnsEmptyForType1Symbolic(): void
    {
        $instance = $this->buildImport();
        $this->setFdt($instance, ['type' => 'Type1', 'Flags' => 4]);
        // Flags & 4 != 0 → symbolic → empty string
        $result = $this->callStringMethod($instance, 'getEncodingTable', ['']);
        $this->assertSame('', $result);
    }

    public function testGetEncodingTableReturnsEmptyForTrueTypeUnicode(): void
    {
        $instance = $this->buildImport();
        $this->setFdt($instance, ['type' => 'TrueTypeUnicode', 'Flags' => 0]);
        $result = $this->callStringMethod($instance, 'getEncodingTable', ['']);
        $this->assertSame('', $result);
    }

    public function testGetEncodingTablePassesThroughExplicitEncoding(): void
    {
        $instance = $this->buildImport();
        $this->setFdt($instance, ['type' => 'TrueType', 'Flags' => 0]);
        $result = $this->callStringMethod($instance, 'getEncodingTable', ['iso-8859-1']);
        $this->assertSame('iso-8859-1', $result);
    }

    // -------------------------------------------------------------------------
    // findOutputPath
    // -------------------------------------------------------------------------

    public function testFindOutputPathReturnsKPathFontsWhenDefined(): void
    {
        $this->setupTest();
        $instance = $this->buildImport();
        $result = $this->callStringMethod($instance, 'findOutputPath', ['']);
        $this->assertSame(constant('K_PATH_FONTS'), $result);
    }

    public function testFindOutputPathReturnsProvidedWritablePath(): void
    {
        $outdir = dirname(__DIR__) . '/target/tmptest/internals/';
        system('mkdir -p ' . $outdir);
        $instance = $this->buildImport();
        $result = $this->callStringMethod($instance, 'findOutputPath', [$outdir]);
        $this->assertSame($outdir, $result);
    }
}
