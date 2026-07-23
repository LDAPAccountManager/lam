<?php

/**
 * TrueTypeTest.php
 *
 * @since     2011-05-23
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

use Com\Tecnick\File\Byte;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\Import\TrueType;

/**
 * TrueType Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 */
class TrueTypeTest extends TestUtil
{
    public function testGetCIDToGIDMapFormat13SetsNotDefGlyph(): void
    {
        // Format 13 subtable with numGroups=0: maps nothing, only .notdef fallback is added.
        $font =
            "\x00\x0d" // format = 13
            . "\x00\x00" // reserved
            . "\x00\x00\x00\x10" // length = 16
            . "\x00\x00\x00\x00" // language
            . "\x00\x00\x00\x00"; // numGroups = 0

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame([0 => 0], $this->getCtgData($fontData));
        $this->assertSame('TrueTypeUnicode', $this->getFontDataString($fontData, 'type'));
    }

    public function testProcessFormat13MapsCharsToSingleGlyph(): void
    {
        // Binary blob for a Format 13 subtable read after the format field (offset=2):
        //   reserved    : \x00\x00           (2 bytes)
        //   length      : \x00\x00\x00\x1c   (4 bytes, unused)
        //   language    : \x00\x00\x00\x00   (4 bytes, unused)
        //   numGroups   : \x00\x00\x00\x01   (4 bytes → 1 group)
        //   startChar   : \x00\x00\x00\x41   (4 bytes → 65 = 'A')
        //   endChar     : \x00\x00\x00\x43   (4 bytes → 67 = 'C')
        //   glyphID     : \x00\x00\x00\x05   (4 bytes → glyph 5)
        $font =
            "\x00\x0d" // format = 13
            . "\x00\x00" // reserved
            . "\x00\x00\x00\x1c" // length
            . "\x00\x00\x00\x00" // language
            . "\x00\x00\x00\x01" // numGroups = 1
            . "\x00\x00\x00\x41" // startCharCode = 65
            . "\x00\x00\x00\x43" // endCharCode   = 67
            . "\x00\x00\x00\x05"; // glyphID       = 5

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        // Characters 65, 66 and 67 must all map to glyph 5 (many-to-one)
        $this->assertSame(5, $this->getCtgGlyph($fontData, 65));
        $this->assertSame(5, $this->getCtgGlyph($fontData, 66));
        $this->assertSame(5, $this->getCtgGlyph($fontData, 67));
        // .notdef fallback must be present
        $this->assertSame(0, $this->getCtgGlyph($fontData, 0));
    }

    public function testProcessFormat13AddsGlyphsToSubset(): void
    {
        $font =
            "\x00\x0d"
            . "\x00\x00"
            . "\x00\x00\x00\x1c"
            . "\x00\x00\x00\x00"
            . "\x00\x00\x00\x01"
            . "\x00\x00\x00\x41" // startCharCode = 65
            . "\x00\x00\x00\x41" // endCharCode   = 65
            . "\x00\x00\x00\x07"; // glyphID       = 7

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        // Mark char 65 as a subset char
        $this->setProperty($instance, 'subchars', [65 => true]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);
        $subGlyphs = $this->getSubglyphs($instance);

        $this->assertSame(7, $this->getCtgGlyph($fontData, 65));
        $this->assertArrayHasKey(7, $subGlyphs);
        $this->assertTrue($subGlyphs[7] ?? false);
    }

    public function testProcessFormat14NonDefaultUVSMapsGlyphs(): void
    {
        // Format 14 subtable with 1 VariationSelector record that has a Non-Default UVS table.
        //
        // Byte layout (subtable starts at offset 0):
        //   0- 1: format              = 14  (0x00,0x0E) — consumed by getCIDToGIDMap before this call
        //   2- 5: length              = 30  (0x00,0x00,0x00,0x1E)
        //   6- 9: numVarSelectorRecs  =  1  (0x00,0x00,0x00,0x01)
        //  10-12: varSelector         = 0x0E0100 (3 bytes)
        //  13-16: defaultUVSOffset    =  0  (absent)
        //  17-20: nonDefaultUVSOffset = 21  (0x00,0x00,0x00,0x15) — relative to subtable start
        //  21-24: numUVSMappings      =  1  (0x00,0x00,0x00,0x01)
        //  25-27: unicodeValue        = U+0082A6 (0x00,0x82,0xA6)
        //  28-29: glyphID             = 1142 (0x04,0x76)
        $font =
            "\x00\x0e" // format = 14
            . "\x00\x00\x00\x1e" // length = 30
            . "\x00\x00\x00\x01" // numVarSelectorRecords = 1
            . "\x0e\x01\x00" // varSelector = U+E0100 (uint24)
            . "\x00\x00\x00\x00" // defaultUVSOffset = 0 (absent)
            . "\x00\x00\x00\x15" // nonDefaultUVSOffset = 21
            . "\x00\x00\x00\x01" // numUVSMappings = 1
            . "\x00\x82\xa6" // unicodeValue = U+0082A6 (uint24)
            . "\x04\x76"; // glyphID = 1142

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        // Non-default UVS mapping: U+0082A6 → glyph 1142
        $this->assertSame(1142, $this->getCtgGlyph($fontData, 0x0082A6));
        // .notdef fallback must still be set
        $this->assertSame(0, $this->getCtgGlyph($fontData, 0));
    }

    public function testProcessFormat14NonDefaultUVSTracksSubglyphs(): void
    {
        // Same layout as above but with a second mapping and subchars tracking.
        //
        // Byte layout (subtable starts at offset 0):
        //   0- 1: format              = 14  (0x00,0x0E)
        //   2- 5: length              = 35
        //   6- 9: numVarSelectorRecs  =  1
        //  10-12: varSelector         = 0x0E0101 (3 bytes)
        //  13-16: defaultUVSOffset    =  0
        //  17-20: nonDefaultUVSOffset = 21
        //  21-24: numUVSMappings      =  2
        //  25-27: unicodeValue[0]     = U+0082A6 → glyphID 7961  (0x00,0x82,0xA6 + 0x1F,0x19)
        //  30-32: unicodeValue[1]     = U+004E4D → glyphID 42    (0x00,0x4E,0x4D + 0x00,0x2A)
        $font =
            "\x00\x0e" // format = 14
            . "\x00\x00\x00\x25" // length = 37
            . "\x00\x00\x00\x01" // numVarSelectorRecords = 1
            . "\x0e\x01\x01" // varSelector = U+E0101 (uint24)
            . "\x00\x00\x00\x00" // defaultUVSOffset = 0
            . "\x00\x00\x00\x15" // nonDefaultUVSOffset = 21
            . "\x00\x00\x00\x02" // numUVSMappings = 2
            . "\x00\x82\xa6" // unicodeValue[0] = U+0082A6
            . "\x1f\x19" // glyphID[0] = 7961
            . "\x00\x4e\x4d" // unicodeValue[1] = U+004E4D
            . "\x00\x2a"; // glyphID[1] = 42

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        // Mark U+0082A6 as a subset char to verify subglyphs tracking
        $this->setProperty($instance, 'subchars', [0x0082A6 => true]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);
        $subGlyphs = $this->getSubglyphs($instance);

        $this->assertSame(7961, $this->getCtgGlyph($fontData, 0x0082A6));
        $this->assertSame(42, $this->getCtgGlyph($fontData, 0x004E4D));
        // Glyph 7961 must be in the subset (0x0082A6 was a subchar)
        $this->assertArrayHasKey(7961, $subGlyphs);
        $this->assertTrue($subGlyphs[7961] ?? false);
        // Glyph 42 must NOT be in the subset (U+004E4D was not a subchar)
        $this->assertArrayNotHasKey(42, $subGlyphs);
    }

    public function testProcessFormat14DefaultUVSOnlyAddsNoCtgEntries(): void
    {
        // Format 14 subtable with 1 VariationSelector record that has only a Default UVS table.
        // Default UVS sequences use the standard cmap glyph — no ctgdata entries should be added.
        //
        // Byte layout (subtable starts at offset 0):
        //   0- 1: format              = 14
        //   2- 5: length              = 26
        //   6- 9: numVarSelectorRecs  =  1
        //  10-12: varSelector         = 0x0E0100
        //  13-16: defaultUVSOffset    = 21  (has a Default UVS table)
        //  17-20: nonDefaultUVSOffset = 0   (absent)
        //  21-24: numUnicodeValueRanges = 1
        //  25-27: startUnicodeValue   = U+004E4D (uint24)
        //  28:    additionalCount     = 2
        $font =
            "\x00\x0e" // format = 14
            . "\x00\x00\x00\x1d" // length = 29
            . "\x00\x00\x00\x01" // numVarSelectorRecords = 1
            . "\x0e\x01\x00" // varSelector = U+E0100 (uint24)
            . "\x00\x00\x00\x15" // defaultUVSOffset = 21
            . "\x00\x00\x00\x00" // nonDefaultUVSOffset = 0 (absent)
            . "\x00\x00\x00\x01" // numUnicodeValueRanges = 1
            . "\x00\x4e\x4d" // startUnicodeValue = U+004E4D (uint24)
            . "\x02"; // additionalCount = 2

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        // Default UVS adds no explicit ctgdata entries beyond .notdef
        $this->assertSame([0 => 0], $this->getCtgData($fontData));
    }

    public function testGetCIDToGIDMapFormat14SetsNotDefGlyph(): void
    {
        // Format 14 subtable with numVarSelectorRecords=0: no mappings → only .notdef fallback added.
        $font =
            "\x00\x0e" // format = 14
            . "\x00\x00\x00\x0a" // length = 10
            . "\x00\x00\x00\x00"; // numVarSelectorRecords = 0

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame([0 => 0], $this->getCtgData($fontData));
    }

    public function testGetCIDToGIDMapThrowsOnUnsupportedFormat(): void
    {
        $instance = $this->buildTrueType("\x00\x0f", [
            'encodingTables' => [
                [
                    'platformID' => 3,
                    'encodingID' => 1,
                    'offset' => 0,
                ],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => [
                    'offset' => 0,
                ],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->bcExpectException(FontException::class);
        $this->invokeMethod($instance, 'getCIDToGIDMap');
    }

    public function testAddCtgItemAddsGlyphToSubset(): void
    {
        $instance = $this->buildTrueType('', [
            'ctgdata' => [],
        ]);

        $this->setProperty($instance, 'subchars', [65 => true]);
        $this->invokeMethod($instance, 'addCtgItem', [65, 7]);

        $fontData = $this->getFontData($instance);
        $subGlyphs = $this->getSubglyphs($instance);

        $this->assertSame(7, $this->getCtgGlyph($fontData, 65));
        $this->assertArrayHasKey(7, $subGlyphs);
        $this->assertTrue($subGlyphs[7] ?? false);
    }

    // -------------------------------------------------------------------------
    // Issue 1: cmap fallback selection
    // -------------------------------------------------------------------------

    public function testSelectEncodingTableReturnsExactMatch(): void
    {
        $tables = [
            ['platformID' => 3, 'encodingID' => 1, 'offset' => 42],
            ['platformID' => 3, 'encodingID' => 10, 'offset' => 99],
        ];
        $instance = $this->buildTrueType('', [
            'encodingTables' => $tables,
            'platform_id' => 3,
            'encoding_id' => 1,
        ]);

        $result = $this->selectEncodingTable($instance);
        $this->assertNotNull($result);

        $this->assertSame(42, $result['offset']);
        $this->assertSame(3, $result['platformID']);
        $this->assertSame(1, $result['encodingID']);
    }

    public function testSelectEncodingTableFallsBackToWindowsUCS4(): void
    {
        // Requested 3/1 is absent; only 3/10 (UCS-4) is available.
        $instance = $this->buildTrueType('', [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 10, 'offset' => 7],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
        ]);

        $result = $this->selectEncodingTable($instance);
        $this->assertNotNull($result);

        $this->assertSame(3, $result['platformID']);
        $this->assertSame(10, $result['encodingID']);
        $this->assertSame(7, $result['offset']);
    }

    public function testSelectEncodingTableFallsBackToWindowsBMP(): void
    {
        // Neither 3/0 (requested) nor 3/10 present; only 3/1 available.
        $instance = $this->buildTrueType('', [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 5],
            ],
            'platform_id' => 3,
            'encoding_id' => 0,
        ]);

        $result = $this->selectEncodingTable($instance);
        $this->assertNotNull($result);

        $this->assertSame(3, $result['platformID']);
        $this->assertSame(1, $result['encodingID']);
    }

    public function testSelectEncodingTableFallsBackToPlatform0(): void
    {
        // No Windows (platform 3) subtables; should fall back to platform 0.
        $instance = $this->buildTrueType('', [
            'encodingTables' => [
                ['platformID' => 0, 'encodingID' => 3, 'offset' => 11],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
        ]);

        $result = $this->selectEncodingTable($instance);
        $this->assertNotNull($result);

        $this->assertSame(0, $result['platformID']);
        $this->assertSame(3, $result['encodingID']);
    }

    public function testSelectEncodingTableReturnsNullWhenNoTableAvailable(): void
    {
        $instance = $this->buildTrueType('', [
            'encodingTables' => [
                ['platformID' => 9, 'encodingID' => 9, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
        ]);

        $result = $this->selectEncodingTable($instance);

        $this->assertNull($result);
    }

    public function testGetCIDToGIDMapThrowsWhenNoTableFound(): void
    {
        // encodingTables contains only an unrecognised platform/encoding pair.
        $instance = $this->buildTrueType('', [
            'encodingTables' => [
                ['platformID' => 9, 'encodingID' => 9, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => [
                'cmap' => ['offset' => 0],
            ],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->bcExpectException(FontException::class);
        $this->invokeMethod($instance, 'getCIDToGIDMap');
    }

    public function testGetCIDToGIDMapThrowsWhenEncodingTablesEmpty(): void
    {
        $instance = $this->buildTrueType('', [
            'encodingTables' => [],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->bcExpectException(FontException::class);
        $this->invokeMethod($instance, 'getCIDToGIDMap');
    }

    public function testGetCIDToGIDMapUsesFallbackTable(): void
    {
        // Requested 3/1 is absent; 3/10 is present with format 13, 0 groups.
        $font =
            "\x00\x0d" // format = 13
            . "\x00\x00" // reserved
            . "\x00\x00\x00\x10" // length = 16
            . "\x00\x00\x00\x00" // language
            . "\x00\x00\x00\x00"; // numGroups = 0

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 10, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);
        // Only .notdef should be present (0 groups → nothing mapped)
        $this->assertSame([0 => 0], $this->getCtgData($fontData));
    }

    // -------------------------------------------------------------------------
    // Issue 2: fsType embedding-policy enforcement
    // -------------------------------------------------------------------------

    public function testApplyEmbeddingPolicyThrowsOnRestrictedLicense(): void
    {
        $instance = $this->buildTrueType('', []);
        $this->bcExpectException(FontException::class);
        // 0x0002 = Restricted License only
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0002]);
    }

    public function testApplyEmbeddingPolicyAllowsPreviewPrint(): void
    {
        $instance = $this->buildTrueType('', ['subset' => true]);
        // 0x0004 = Preview & Print — allowed
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0004]);
        // No exception means the policy passed; subset should be unchanged
        $fontData = $this->getFontData($instance);
        $this->assertTrue($this->getFontDataBool($fontData, 'subset'));
    }

    public function testApplyEmbeddingPolicyPermissiveBitOverridesRestricted(): void
    {
        $instance = $this->buildTrueType('', ['subset' => true]);
        // 0x0006 = 0x0002 | 0x0004: permissive override, should not throw
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0006]);
        $fontData = $this->getFontData($instance);
        $this->assertTrue($this->getFontDataBool($fontData, 'subset'));
    }

    public function testApplyEmbeddingPolicyThrowsOnBitmapOnly(): void
    {
        $instance = $this->buildTrueType('', []);
        $this->bcExpectException(FontException::class);
        // 0x0200 = Bitmap Embedding Only
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0200]);
    }

    public function testApplyEmbeddingPolicyThrowsOnBitmapOnlyWithEditable(): void
    {
        $instance = $this->buildTrueType('', []);
        $this->bcExpectException(FontException::class);
        // 0x0208 = Bitmap Only | Editable — bitmap restriction still applies
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0208]);
    }

    public function testApplyEmbeddingPolicyDisablesSubsetOnNoSubsettingFlag(): void
    {
        $instance = $this->buildTrueType('', ['subset' => true]);
        // 0x0100 = No Subsetting
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0100]);
        $fontData = $this->getFontData($instance);
        $this->assertFalse($this->getFontDataBool($fontData, 'subset'));
    }

    public function testApplyEmbeddingPolicyNoSubsettingWithEditableAllowed(): void
    {
        $instance = $this->buildTrueType('', ['subset' => true]);
        // 0x0108 = Editable | No Subsetting: embed allowed but no subset
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0108]);
        $fontData = $this->getFontData($instance);
        $this->assertFalse($this->getFontDataBool($fontData, 'subset'));
    }

    public function testApplyEmbeddingPolicyInstallableAllowsSubset(): void
    {
        $instance = $this->buildTrueType('', ['subset' => true]);
        // 0x0000 = Installable — no restrictions
        $this->invokeMethod($instance, 'applyEmbeddingPolicy', [0x0000]);
        $fontData = $this->getFontData($instance);
        $this->assertTrue($this->getFontDataBool($fontData, 'subset'));
    }

    // -------------------------------------------------------------------------
    // Issue 3: OS/2 table resilience
    // -------------------------------------------------------------------------

    public function testGetOS2MetricsUsesDefaultsWhenTableAbsent(): void
    {
        // 'table' has no 'OS/2' entry at all.
        $instance = $this->buildTrueType('', [
            'table' => [],
            'urk' => 1.0,
        ]);

        $this->invokeMethod($instance, 'getOS2Metrics');
        $fontData = $this->getFontData($instance);

        $this->assertSame(0, $this->getFontDataInt($fontData, 'AvgWidth'));
        $this->assertSame(70, $this->getFontDataInt($fontData, 'StemV'));
        $this->assertSame(30, $this->getFontDataInt($fontData, 'StemH'));
    }

    public function testGetOS2MetricsThrowsWhenTableTooShort(): void
    {
        $instance = $this->buildTrueType('', [
            'table' => [
                'OS/2' => ['offset' => 0, 'length' => 5],
            ],
            'urk' => 1.0,
        ]);

        $this->bcExpectException(FontException::class);
        $this->invokeMethod($instance, 'getOS2Metrics');
    }

    public function testGetOS2MetricsParsesValidTable(): void
    {
        // Minimal valid OS/2 blob: 10 bytes.
        // version(2) + xAvgCharWidth(2) + usWeightClass(2) + usWidthClass(2) + fsType(2)
        $font =
            "\x00\x04" // version = 4
            . "\x04\x00" // xAvgCharWidth = 1024 raw units
            . "\x01\x90" // usWeightClass = 400
            . "\x00\x05" // usWidthClass  = 5 (unused)
            . "\x00\x08"; // fsType = 0x0008 (Editable — allowed)

        $instance = $this->buildTrueType($font, [
            'table' => [
                'OS/2' => ['offset' => 0, 'length' => 10],
            ],
            'urk' => 1.0,
            'subset' => true,
        ]);

        $this->invokeMethod($instance, 'getOS2Metrics');
        $fontData = $this->getFontData($instance);

        // xAvgCharWidth 0x0400 = 1024 raw, * urk 1.0 = 1024
        $this->assertSame(1024, $this->getFontDataInt($fontData, 'AvgWidth'));
        // usWeightClass 0x0190 = 400; StemV = round(70*400/400) = 70
        $this->assertSame(70, $this->getFontDataInt($fontData, 'StemV'));
        // StemH = round(30*400/400) = 30
        $this->assertSame(30, $this->getFontDataInt($fontData, 'StemH'));
        // Editable flag: subset must remain true (no restriction)
        $this->assertTrue($this->getFontDataBool($fontData, 'subset'));
    }

    public function testGetOS2MetricsNoSubsettingFlagDisablesSubset(): void
    {
        // fsType = 0x0100 (No Subsetting)
        $font =
            "\x00\x04" // version
            . "\x02\x00" // xAvgCharWidth
            . "\x01\x90" // usWeightClass = 400
            . "\x00\x05" // usWidthClass
            . "\x01\x00"; // fsType = 0x0100

        $instance = $this->buildTrueType($font, [
            'table' => ['OS/2' => ['offset' => 0, 'length' => 10]],
            'urk' => 1.0,
            'subset' => true,
        ]);

        $this->invokeMethod($instance, 'getOS2Metrics');
        $fontData = $this->getFontData($instance);

        $this->assertFalse($this->getFontDataBool($fontData, 'subset'));
    }

    /**
     * @param array<string, mixed> $fdt
     *
     * @return array<string, mixed>
     */
    protected function getFontDefaults(array $fdt = []): array
    {
        $defaults = [
            'Ascender' => 0,
            'Ascent' => 0,
            'AvgWidth' => 0.0,
            'CapHeight' => 0,
            'CharacterSet' => '',
            'Descender' => 0,
            'Descent' => 0,
            'EncodingScheme' => '',
            'FamilyName' => '',
            'Flags' => 0,
            'FontBBox' => [],
            'FontName' => '',
            'FullName' => '',
            'IsFixedPitch' => false,
            'ItalicAngle' => 0,
            'Leading' => 0,
            'MaxWidth' => 0,
            'MissingWidth' => 0,
            'StdHW' => 0,
            'StdVW' => 0,
            'StemH' => 0,
            'StemV' => 0,
            'UnderlinePosition' => 0,
            'UnderlineThickness' => 0,
            'Version' => '',
            'Weight' => '',
            'XHeight' => 0,
            'bbox' => '',
            'cbbox' => [],
            'cidinfo' => ['Ordering' => '', 'Registry' => '', 'Supplement' => 0, 'uni2cid' => []],
            'compress' => false,
            'ctg' => '',
            'ctgdata' => [],
            'cw' => [],
            'cwu' => [],
            'datafile' => '',
            'desc' => [
                'Ascent' => 0,
                'AvgWidth' => 0,
                'CapHeight' => 0,
                'Descent' => 0,
                'Flags' => 0,
                'FontBBox' => '',
                'ItalicAngle' => 0,
                'Leading' => 0,
                'MaxWidth' => 0,
                'MissingWidth' => 0,
                'StemH' => 0,
                'StemV' => 0,
                'XHeight' => 0,
            ],
            'diff' => '',
            'diff_n' => 0,
            'dir' => '',
            'dw' => 0,
            'enc' => '',
            'enc_map' => [],
            'encodingTables' => [],
            'encoding_id' => 0,
            'encrypted' => '',
            'fakestyle' => false,
            'family' => '',
            'file' => '',
            'file_n' => 0,
            'file_name' => '',
            'i' => 0,
            'ifile' => '',
            'indexToLoc' => [],
            'input_file' => '',
            'isUnicode' => false,
            'italicAngle' => 0,
            'key' => '',
            'lenIV' => 0,
            'length1' => 0,
            'length2' => 0,
            'linked' => false,
            'mode' => [
                'bold' => false,
                'italic' => false,
                'linethrough' => false,
                'overline' => false,
                'underline' => false,
            ],
            'n' => 0,
            'name' => '',
            'numGlyphs' => 0,
            'numHMetrics' => 0,
            'originalsize' => 0,
            'pdfa' => false,
            'platform_id' => 0,
            'settype' => '',
            'short_offset' => false,
            'size1' => 0,
            'size2' => 0,
            'style' => '',
            'subset' => false,
            'subsetchars' => [],
            'table' => [],
            'tot_num_glyphs' => 0,
            'type' => '',
            'underlinePosition' => 0,
            'underlineThickness' => 0,
            'unicode' => false,
            'unitsPerEm' => 0,
            'up' => 0,
            'urk' => 0.0,
            'ut' => 0,
            'weight' => '',
        ];

        return \array_replace_recursive($defaults, $fdt);
    }

    /**
     * @param array<string, mixed> $fdt
     */
    protected function buildTrueType(string $font, array $fdt): TrueType
    {
        $class = new \ReflectionClass(TrueType::class);
        $instance = $class->newInstanceWithoutConstructor();
        try {
            $byte = new Byte($font);
        } catch (\RangeException $exception) {
            $this->fail($exception->getMessage());
        }

        $this->setProperty($instance, 'font', $font);
        $this->setProperty($instance, 'fdt', $this->getFontDefaults($fdt));
        $this->setProperty($instance, 'fbyte', $byte);
        $this->setProperty($instance, 'offset', 0);

        return $instance;
    }

    /**
     * @param array<int, mixed> $args
     */
    protected function invokeMethod(TrueType $instance, string $method, array $args = []): mixed
    {
        $reflection = new \ReflectionMethod(TrueType::class, $method);

        return $reflection->invokeArgs($instance, $args);
    }

    /** @return array<string, mixed> */
    protected function getFontData(TrueType $instance): array
    {
        return $this->expectFontData($this->getProperty($instance, 'fdt'));
    }

    /** @param array<string, mixed> $fontData */
    protected function getCtgGlyph(array $fontData, int $char): ?int
    {
        $ctgData = $this->getCtgData($fontData);
        $glyph = $ctgData[$char] ?? null;

        if ($glyph !== null) {
            $this->assertIsInt($glyph);
        }

        return $glyph;
    }

    /**
     * @param array<string, mixed> $fontData
     *
     * @return array<int, int>
     */
    protected function getCtgData(array $fontData): array
    {
        if (!isset($fontData['ctgdata']) || !\is_array($fontData['ctgdata'])) {
            $this->fail('Expected ctgdata map.');
        }

        /** @var array<int, int> $ctgData */
        $ctgData = $fontData['ctgdata'];
        return $ctgData;
    }

    /** @param array<string, mixed> $fontData */
    protected function getFontDataString(array $fontData, string $key): string
    {
        if (!isset($fontData[$key]) || !\is_string($fontData[$key])) {
            $this->fail('Expected string font field: ' . $key);
        }

        return $fontData[$key];
    }

    /** @param array<string, mixed> $fontData */
    protected function getFontDataBool(array $fontData, string $key): bool
    {
        if (!isset($fontData[$key]) || !\is_bool($fontData[$key])) {
            $this->fail('Expected bool font field: ' . $key);
        }

        return $fontData[$key];
    }

    /** @param array<string, mixed> $fontData */
    protected function getFontDataInt(array $fontData, string $key): int
    {
        if (!isset($fontData[$key]) || !\is_int($fontData[$key])) {
            $this->fail('Expected int font field: ' . $key);
        }

        return $fontData[$key];
    }

    /**
     * @return array<int, bool>
     */
    protected function getSubglyphs(TrueType $instance): array
    {
        return $this->expectSubglyphs($this->getProperty($instance, 'subglyphs'));
    }

    /**
     * @return array{platformID: int, encodingID: int, offset: int}|null
     */
    protected function selectEncodingTable(TrueType $instance): ?array
    {
        return $this->expectEncodingTable($this->invokeMethod($instance, 'selectEncodingTable'));
    }

    protected function convertStringEncoding(TrueType $instance, string $str, int $platformId, int $encodingId): string
    {
        return $this->expectString(
            $this->invokeMethod($instance, 'convertStringEncoding', [$str, $platformId, $encodingId]),
            'Expected converted string.',
        );
    }

    protected function getProperty(TrueType $instance, string $name): mixed
    {
        $property = new \ReflectionProperty(TrueType::class, $name);

        return $property->getValue($instance);
    }

    protected function setProperty(TrueType $instance, string $name, mixed $value): void
    {
        $property = new \ReflectionProperty(TrueType::class, $name);
        $property->setValue($instance, $value);
    }

    /** @return array<string, mixed> */
    protected function expectFontData(mixed $value): array
    {
        if (!\is_array($value)) {
            $this->fail('Expected font data array.');
        }

        /** @var array<string, mixed> $value */
        return $value;
    }

    /** @return array<int, bool> */
    protected function expectSubglyphs(mixed $value): array
    {
        if (!\is_array($value)) {
            $this->fail('Expected subglyph map.');
        }

        /** @var array<int, bool> $value */
        return $value;
    }

    /** @return array{platformID: int, encodingID: int, offset: int}|null */
    protected function expectEncodingTable(mixed $value): ?array
    {
        if ($value === null) {
            return null;
        }

        if (!\is_array($value)) {
            $this->fail('Expected encoding table array.');
        }

        if (!isset($value['platformID']) || !\is_int($value['platformID'])) {
            $this->fail('Expected platformID.');
        }

        if (!isset($value['encodingID']) || !\is_int($value['encodingID'])) {
            $this->fail('Expected encodingID.');
        }

        if (!isset($value['offset']) || !\is_int($value['offset'])) {
            $this->fail('Expected offset.');
        }

        return [
            'platformID' => $value['platformID'],
            'encodingID' => $value['encodingID'],
            'offset' => $value['offset'],
        ];
    }

    protected function expectString(mixed $value, string $message): string
    {
        if (!\is_string($value)) {
            $this->fail($message);
        }

        return $value;
    }

    // -------------------------------------------------------------------------
    // cmap format 0 – byte encoding table
    // -------------------------------------------------------------------------

    public function testProcessFormat0MapsAllGlyphs(): void
    {
        // Format 0: 256-byte direct lookup. After getCIDToGIDMap reads the 2-byte
        // format field, processFormat0 skips 4 bytes (length + language) then reads
        // 256 single-byte glyph IDs.
        $glyphs = str_repeat("\x00", 256);
        $glyphs[65] = "\x63"; // chr 65 → glyph 99
        $glyphs[90] = "\x0A"; // chr 90 → glyph 10

        $font =
            "\x00\x00" // format = 0
            . "\x01\x06" // length (unused)
            . "\x00\x00" // language (unused)
            . $glyphs;

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame(99, $this->getCtgGlyph($fontData, 65));
        $this->assertSame(10, $this->getCtgGlyph($fontData, 90));
        // All 256 slots populated and type was TrueTypeUnicode → converted to TrueType
        $this->assertSame('TrueType', $this->getFontDataString($fontData, 'type'));
    }

    // -------------------------------------------------------------------------
    // cmap format 2 – high-byte mapping through table
    // -------------------------------------------------------------------------

    public function testProcessFormat2MapsCharsViaSingleByteSubheaders(): void
    {
        // All 256 subHeaderKeys = 0  → single-byte codes, one subHeader at index 0.
        // subHeaders[0]: firstCode=0, entryCount=1, idDelta=0, idRangeOffset=2
        //   Adjusted idRangeOffset = 2 − (2 + (1−0−1)×8) = 0  →  /2 = 0
        // glyphIdArray[0] = 99  →  every single-byte char maps to glyph 99.
        $subHeaderKeys = str_repeat("\x00\x00", 256); // 512 bytes
        $subHeader = "\x00\x00\x00\x01\x00\x00\x00\x02";
        $glyphIdArray = "\x00\x63"; // glyph 99

        $font =
            "\x00\x02" // format = 2
            . "\x02\x18" // length (unused)
            . "\x00\x00" // language (unused)
            . $subHeaderKeys
            . $subHeader
            . $glyphIdArray;

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame(99, $this->getCtgGlyph($fontData, 65));
        $this->assertSame(99, $this->getCtgGlyph($fontData, 0));
        // 256 entries with TrueTypeUnicode → converted to TrueType
        $this->assertSame('TrueType', $this->getFontDataString($fontData, 'type'));
    }

    // -------------------------------------------------------------------------
    // cmap format 6 – trimmed table mapping
    // -------------------------------------------------------------------------

    public function testProcessFormat6MapsCharRange(): void
    {
        // firstCode=65, entryCount=3, glyphs=[10,11,12]
        $font =
            "\x00\x06" // format = 6
            . "\x00\x0F" // length (unused)
            . "\x00\x00" // language (unused)
            . "\x00\x41" // firstCode = 65
            . "\x00\x03" // entryCount = 3
            . "\x00\x0A" // glyph for chr 65 = 10
            . "\x00\x0B" // glyph for chr 66 = 11
            . "\x00\x0C"; // glyph for chr 67 = 12

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame(10, $this->getCtgGlyph($fontData, 65));
        $this->assertSame(11, $this->getCtgGlyph($fontData, 66));
        $this->assertSame(12, $this->getCtgGlyph($fontData, 67));
        // Only 4 ctgdata entries → not 256 → type stays TrueTypeUnicode
        $this->assertSame('TrueTypeUnicode', $this->getFontDataString($fontData, 'type'));
    }

    // -------------------------------------------------------------------------
    // cmap format 8 – mixed 16-bit and 32-bit coverage
    // -------------------------------------------------------------------------

    public function testProcessFormat8WithNoGroupsAddsOnlyNotdef(): void
    {
        // numGroups = 0 → no character mappings; only the .notdef fallback is present.
        $is32 = str_repeat("\x00", 8192);
        $font =
            "\x00\x08" // format = 8
            . "\x00\x00" // reserved (uint16)
            . "\x00\x00\x20\x14" // length (uint32, unused)
            . "\x00\x00\x00\x00" // language (uint32, unused)
            . $is32 // is32[8192]
            . "\x00\x00\x00\x00"; // numGroups = 0

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame([0 => 0], $this->getCtgData($fontData));
    }

    public function testProcessFormat8MapsSingleByteChar(): void
    {
        // numGroups=1: chars 65..65 → glyph 5. is32[8]=0 → single-byte char.
        // Per the cmap format 8 spec the character maps to its real glyph id
        // (startGlyphID = 5), and subglyphs contains glyph 5 when subchars[65] is set.
        $is32 = str_repeat("\x00", 8192);
        $font =
            "\x00\x08" // format = 8
            . "\x00\x00" // reserved
            . "\x00\x00\x20\x26" // length (unused)
            . "\x00\x00\x00\x00" // language (unused)
            . $is32 // is32[8192] all zeros
            . "\x00\x00\x00\x01" // numGroups = 1
            . "\x00\x00\x00\x41" // startCharCode = 65
            . "\x00\x00\x00\x41" // endCharCode   = 65
            . "\x00\x00\x00\x05"; // startGlyphID  = 5

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);
        $this->setProperty($instance, 'subchars', [65 => true]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);
        $subGlyphs = $this->getSubglyphs($instance);

        // ctgdata[65] maps to the real glyph id 5 (startGlyphID)
        $this->assertSame(5, $this->getCtgGlyph($fontData, 65));
        // addCtgItem also recorded glyph 5 in subglyphs
        $this->assertArrayHasKey(5, $subGlyphs);
    }

    // -------------------------------------------------------------------------
    // cmap format 10 – trimmed array
    // -------------------------------------------------------------------------

    public function testProcessFormat10MapsCharRange(): void
    {
        // startCharCode=65, numChars=3, glyphs=[10,11,12]
        $font =
            "\x00\x0A" // format = 10
            . "\x00\x00" // reserved (uint16)
            . "\x00\x00\x00\x1C" // length (uint32, unused)
            . "\x00\x00\x00\x00" // language (uint32, unused)
            . "\x00\x00\x00\x41" // startCharCode = 65 (uint32)
            . "\x00\x00\x00\x03" // numChars = 3 (uint32)
            . "\x00\x0A" // glyph for chr 65 = 10
            . "\x00\x0B" // glyph for chr 66 = 11
            . "\x00\x0C"; // glyph for chr 67 = 12

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame(10, $this->getCtgGlyph($fontData, 65));
        $this->assertSame(11, $this->getCtgGlyph($fontData, 66));
        $this->assertSame(12, $this->getCtgGlyph($fontData, 67));
        $this->assertSame(0, $this->getCtgGlyph($fontData, 0));
    }

    // -------------------------------------------------------------------------
    // cmap format 12 – segmented coverage
    // -------------------------------------------------------------------------

    public function testProcessFormat12MapsSequentialGlyphs(): void
    {
        // 1 group: startCharCode=65, endCharCode=67, startGlyphID=100
        // → ctgdata[65]=100, ctgdata[66]=101, ctgdata[67]=102
        $font =
            "\x00\x0C" // format = 12
            . "\x00\x00" // reserved (uint16)
            . "\x00\x00\x00\x22" // length (uint32, unused)
            . "\x00\x00\x00\x00" // language (uint32, unused)
            . "\x00\x00\x00\x01" // nGroups = 1
            . "\x00\x00\x00\x41" // startCharCode = 65
            . "\x00\x00\x00\x43" // endCharCode   = 67
            . "\x00\x00\x00\x64"; // startGlyphID  = 100

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame(100, $this->getCtgGlyph($fontData, 65));
        $this->assertSame(101, $this->getCtgGlyph($fontData, 66));
        $this->assertSame(102, $this->getCtgGlyph($fontData, 67));
        $this->assertSame(0, $this->getCtgGlyph($fontData, 0));
    }

    public function testProcessFormat12WithZeroGroupsAddsOnlyNotdef(): void
    {
        $font =
            "\x00\x0C" // format = 12
            . "\x00\x00" // reserved
            . "\x00\x00\x00\x16" // length (unused)
            . "\x00\x00\x00\x00" // language (unused)
            . "\x00\x00\x00\x00"; // nGroups = 0

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame([0 => 0], $this->getCtgData($fontData));
    }

    public function testProcessFormat12ClampsRangeToUnicodeMax(): void
    {
        // A group ending at 0xFFFFFFFF must be clamped to U+10FFFF instead of iterating
        // ~4 billion times (DoS guard). Only the two in-range code points are mapped.
        $font =
            "\x00\x0C" // format = 12
            . "\x00\x00" // reserved
            . "\x00\x00\x00\x1C" // length (unused)
            . "\x00\x00\x00\x00" // language (unused)
            . "\x00\x00\x00\x01" // nGroups = 1
            . "\x00\x10\xFF\xFE" // startCharCode = 0x10FFFE
            . "\xFF\xFF\xFF\xFF" // endCharCode   = 0xFFFFFFFF (clamped to 0x10FFFF)
            . "\x00\x00\x00\x07"; // startGlyphID  = 7

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame(7, $this->getCtgGlyph($fontData, 0x10FFFE));
        $this->assertSame(8, $this->getCtgGlyph($fontData, 0x10FFFF));
        // nothing beyond the Unicode maximum was mapped
        $this->assertNull($this->getCtgGlyph($fontData, 0x110000));
    }

    public function testProcessFormat12SkipsGroupAboveUnicodeMax(): void
    {
        // A group that starts beyond U+10FFFF is skipped entirely.
        $font =
            "\x00\x0C" // format = 12
            . "\x00\x00" // reserved
            . "\x00\x00\x00\x1C" // length (unused)
            . "\x00\x00\x00\x00" // language (unused)
            . "\x00\x00\x00\x01" // nGroups = 1
            . "\x00\x11\x00\x00" // startCharCode = 0x110000 (> U+10FFFF)
            . "\x00\x11\x00\x05" // endCharCode   = 0x110005
            . "\x00\x00\x00\x07"; // startGlyphID  = 7

        $instance = $this->buildTrueType($font, [
            'encodingTables' => [
                ['platformID' => 3, 'encodingID' => 1, 'offset' => 0],
            ],
            'platform_id' => 3,
            'encoding_id' => 1,
            'table' => ['cmap' => ['offset' => 0]],
            'type' => 'TrueTypeUnicode',
        ]);

        $this->invokeMethod($instance, 'getCIDToGIDMap');
        $fontData = $this->getFontData($instance);

        $this->assertSame([0 => 0], $this->getCtgData($fontData));
    }

    // -------------------------------------------------------------------------
    // checkRequiredTables
    // -------------------------------------------------------------------------

    public function testCheckRequiredTablesThrowsWhenTableMissing(): void
    {
        // every mandatory table is present except 'glyf'
        $tables = [];
        foreach (['head', 'loca', 'cmap', 'name', 'post', 'hhea', 'maxp', 'hmtx'] as $tag) {
            $tables[$tag] = ['checkSum' => 0, 'data' => '', 'length' => 0, 'offset' => 0];
        }

        $instance = $this->buildTrueType('', ['table' => $tables]);

        $this->expectException(FontException::class);
        $this->invokeMethod($instance, 'checkRequiredTables');
    }

    // -------------------------------------------------------------------------
    // convertStringEncoding
    // -------------------------------------------------------------------------

    public function testConvertStringEncodingForUnicodePlatformUtf16be(): void
    {
        $instance = $this->buildTrueType('', []);
        // platformId=0 (Unicode) → UTF-16BE. "\x00\x41" = 'A'
        $result = $this->convertStringEncoding($instance, "\x00\x41", 0, 0);
        $this->assertSame('A', $result);
    }

    public function testConvertStringEncodingForWindowsPlatformDefaultUtf16be(): void
    {
        $instance = $this->buildTrueType('', []);
        // platformId=3, encodingId=0 → default UTF-16BE. "\x00\x42" = 'B'
        $result = $this->convertStringEncoding($instance, "\x00\x42", 3, 0);
        $this->assertSame('B', $result);
    }

    public function testConvertStringEncodingForWindowsPlatformEncodingId1Utf16be(): void
    {
        $instance = $this->buildTrueType('', []);
        // platformId=3, encodingId=1 → default UTF-16BE. "\x00\x43" = 'C'
        $result = $this->convertStringEncoding($instance, "\x00\x43", 3, 1);
        $this->assertSame('C', $result);
    }

    public function testConvertStringEncodingForMacintoshPlatformAsciiChar(): void
    {
        $instance = $this->buildTrueType('', []);
        // platformId=1 (Macintosh/MacRoman). ASCII 0x41 = 'A' in both MacRoman and UTF-8.
        $result = $this->convertStringEncoding($instance, "\x41", 1, 0);
        $this->assertSame('A', $result);
    }

    public function testConvertStringEncodingForWindowsPlatformCp936(): void
    {
        $instance = $this->buildTrueType('', []);
        // platformId=3, encodingId=3 → CP936.
        // CP936/GBK 0x41 (single-byte ASCII-compatible) = 'A'
        $result = $this->convertStringEncoding($instance, "\x41", 3, 3);
        $this->assertSame('A', $result);
    }
}
