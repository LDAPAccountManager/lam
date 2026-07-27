<?php

/**
 * BidiTest.php
 *
 * @since     2011-05-23
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
use PHPUnit\Framework\Attributes\DataProvider;

/**
 * Bidi Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   Unicode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-unicode
 */
class BidiTest extends TestUtil
{
    private static function decodeJsonString(string $json): string
    {
        /** @var string */
        return \json_decode($json);
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Unicode\Exception::class);
        new \Com\Tecnick\Unicode\Bidi();
    }

    /**
     * @param ?string $str      String to convert (if null it will be generated from $chrarr or $ordarr)
     * @param ?array<string>  $chrarr   Array of UTF-8 chars (if empty it will be generated from $str or $ordarr)
     * @param ?array<int>  $ordarr   Array of UTF-8 codepoints (if empty it will be generated from $str or $chrarr)
     * @param string $forcedir If 'R' forces RTL, if 'L' forces LTR
     * @param bool   $shaping  If true enable the shaping algorithm
     *
     * @throws \Com\Tecnick\Unicode\Exception
     */
    #[DataProvider('inputDataProvider')]
    public function testStr(
        ?string $str = null,
        ?array $chrarr = null,
        ?array $ordarr = null,
        string $forcedir = '',
        bool $shaping = true,
    ): void {
        $bidi = new Bidi($str, $chrarr, $ordarr, $forcedir, $shaping);
        $this->assertEquals('test', $bidi->getString());
        $this->assertEquals(['t', 'e', 's', 't'], $bidi->getChrArray());
        $this->assertEquals([116, 101, 115, 116], $bidi->getOrdArray());
        $this->assertEquals(
            [
                116 => true,
                101 => true,
                115 => true,
            ],
            $bidi->getCharKeys(),
        );
        $this->assertEquals(4, $bidi->getNumChars());
    }

    /**
     * @return array<int, array{?string, ?array<string>, ?array<int>, string, bool}>
     */
    public static function inputDataProvider(): array
    {
        return [
            ['test', null, null, '', true],
            [null, ['t', 'e', 's', 't'], null, '', true],
            [null, null, [116, 101, 115, 116], '', true],
            ['test', ['t', 'e', 's', 't'], null, '', true],
            ['test', null, [116, 101, 115, 116], '', true],
            [null, ['t', 'e', 's', 't'], [116, 101, 115, 116], '', true],
            ['test', ['t', 'e', 's', 't'], [116, 101, 115, 116], '', true],
            ['test', null, null, 'L', true],
            ['test', null, null, 'R', true],
        ];
    }

    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    #[DataProvider('bidiStrDataProvider')]
    public function testBidiStr(string $str, mixed $expected, string $forcedir = ''): void
    {
        $bidi = new Bidi($str, null, null, $forcedir, true);
        $this->assertEquals($expected, $bidi->getString());
    }

    /**
     * @return array<int, array{string, string, string}>
     */
    public static function bidiStrDataProvider(): array
    {
        return [
            [
                "\n\nABC\nEFG\n\nHIJ\n\n",
                "\n\nABC\nEFG\n\nHIJ\n\n",
                'L',
            ],
            [
                self::decodeJsonString('"\u202EABC\u202C"'),
                'CBA',
                '',
            ],
            [
                'left to right',
                'right to left',
                'R',
            ],
            [
                'left to right ',
                ' right to left',
                'R',
            ],
            [
                self::decodeJsonString('"smith (fabrikam \u0600\u0601\u0602) \u05de\u05d6\u05dc"'),
                self::decodeJsonString('"\u05dc\u05d6\u05de (\u0602\u0601\u0600 fabrikam) smith"'),
                'R',
            ],
            [
                self::decodeJsonString('"\u0600\u0601\u0602 book(s)"'),
                self::decodeJsonString('"book(s) \u0602\u0601\u0600"'),
                'R',
            ],
            [
                self::decodeJsonString('"\u0600\u0601(\u0602\u0603[&ef]!)gh"'),
                self::decodeJsonString('"gh(![ef&]\u0603\u0602)\u0601\u0600"'),
                'R',
            ],
            [
                'تشكيل اختبار',
                'ﺭﺎﺒﺘﺧﺍ ﻞﻴﻜﺸﺗ',
                '',
            ],
            [
                self::decodeJsonString('"\u05de\u05d6\u05dc \u05d8\u05d5\u05d1"'),
                self::decodeJsonString('"\u05d1\u05d5\u05d8 \u05dc\u05d6\u05de"'),
                '',
            ],
            [
                self::decodeJsonString(
                    '"\u0644\u0644\u0647 \u0600\u0601\u0602 \uFB50'
                    . ' \u0651\u064c\u0651\u064d\u0651\u064e\u0651\u064f\u0651\u0650'
                    . ' \u0644\u0622"',
                ),
                self::decodeJsonString(
                    '"\ufef5\ufedf \ufc62\ufc61\ufc60\ufc5f\ufc5e \ufb50 \u0602\u0601\u0600 \ufdf2"',
                ),
                '',
            ],
            [
                self::decodeJsonString('"A\u2067\u05d8\u2069B"'),
                self::decodeJsonString('"A\u2067\u05d8\u2069B"'),
                '',
            ],
            [
                // Unterminated isolate: RLI with no matching PDI. Exercises StepXten's
                // findMatchingPdiStart() returning -1 and the eos-from-paragraph-level fallback.
                self::decodeJsonString('"\u05d0\u2067\u05d1"'),
                self::decodeJsonString('"\u05d1\u2067\u05d0"'),
                '',
            ],
            [
                // Unterminated FSI wrapping LTR text: covers FSI auto-direction with no matching PDI.
                self::decodeJsonString('"\u05d0\u2068ab\u05d1"'),
                self::decodeJsonString('"ab\u05d1\u2068\u05d0"'),
                '',
            ],
            [
                // RLI + PDI
                self::decodeJsonString('"The words \"\u2067\u05de\u05d6\u05dc [mazel] \u05d8\u05d5\u05d1 [tov]\u2069\"'
                . ' mean \"Congratulations!\""'),
                'The words "⁧[tov] בוט [mazel] לזמ⁩" mean "Congratulations!"',
                '',
            ],
            [
                // RLE + PDF
                self::decodeJsonString('"it is called \"\u202bAN INTRODUCTION TO java\u202c\" - $19.95 in hardcover."'),
                'it is called "java TO INTRODUCTION AN" - $19.95 in hardcover.',
                '',
            ],
            [
                // RLI + PDI
                self::decodeJsonString('"it is called \"\u2067AN INTRODUCTION TO java\u2069\" - $19.95 in hardcover."'),
                'it is called "⁧java TO INTRODUCTION AN⁩" - $19.95 in hardcover.',
                '',
            ],
            [
                // Hebrew with embedded paragraph separator (covers getParagraphs() splitting and re-insertion)
                self::decodeJsonString('"\u05de\u05d6\u05dc \u05d8\u05d5\u05d1"')
                    . "\n"
                    . self::decodeJsonString('"\u05de\u05d6\u05dc \u05d8\u05d5\u05d1"'),
                self::decodeJsonString('"\u05d1\u05d5\u05d8 \u05dc\u05d6\u05de"')
                    . "\n"
                    . self::decodeJsonString('"\u05d1\u05d5\u05d8 \u05dc\u05d6\u05de"'),
                '',
            ],
            [
                // Hebrew ending with paragraph separator (covers empty last paragraph handling)
                self::decodeJsonString('"\u05de\u05d6\u05dc \u05d8\u05d5\u05d1"') . "\n",
                self::decodeJsonString('"\u05d1\u05d5\u05d8 \u05dc\u05d6\u05de"') . "\n",
                '',
            ],
            [
                // Arabic with forced LTR direction (covers getPel() returning 0 for forcedir='L')
                'تشكيل اختبار',
                self::decodeJsonString('"\ufede\ufef4\ufedc\ufeb8\ufe97\u0020\ufead\ufe8e\ufe92\ufe98\ufea7\ufe8d"'),
                'L',
            ],
        ];
    }

    /**
     * Test Bidi with edge-case ordarr inputs (negative codepoints and unknown-type codepoints).
     * These cover the defensive continue-branches in process() when the last char of a paragraph
     * is negative (line 322) or not present in the bidi type table (line 326).
     */
    /**
     * @throws \Com\Tecnick\Unicode\Exception
     */
    public function testBidiWithSpecialOrdarr(): void
    {
        // Negative codepoint as last char: covers the $lastchar < 0 branch
        $bidi1 = new \Com\Tecnick\Unicode\Bidi(null, null, [0x05D0, -1], 'R', false);
        $this->assertEquals([-1, 1488], $bidi1->getOrdArray());

        // Codepoint 0xE001 (Private Use Area, not in the bidi type table):
        // covers the !isset(UniType::UNI[$lastchar]) branch
        $bidi2 = new \Com\Tecnick\Unicode\Bidi(null, null, [0x05D0, 0xE001], 'R', false);
        $this->assertEquals([57345, 1488], $bidi2->getOrdArray());
    }
}
