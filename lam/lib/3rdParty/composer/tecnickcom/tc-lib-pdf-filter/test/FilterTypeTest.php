<?php

/**
 * FilterTypeTest.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 *
 * This file is part of tc-lib-pdf-filter software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Filter\Filter;
use Com\Tecnick\Pdf\Filter\FilterType;

/**
 * FilterType enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFilter
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-filter
 */
class FilterTypeTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertEquals('ASCIIHexDecode', FilterType::AsciiHexDecode->value);
        $this->assertEquals('ASCII85Decode', FilterType::Ascii85Decode->value);
        $this->assertEquals('LZWDecode', FilterType::LzwDecode->value);
        $this->assertEquals('FlateDecode', FilterType::FlateDecode->value);
        $this->assertEquals('RunLengthDecode', FilterType::RunLengthDecode->value);
        $this->assertEquals('CCITTFaxDecode', FilterType::CcittFaxDecode->value);
        $this->assertEquals('JBIG2Decode', FilterType::Jbig2Decode->value);
        $this->assertEquals('DCTDecode', FilterType::DctDecode->value);
        $this->assertEquals('JPXDecode', FilterType::JpxDecode->value);
        $this->assertEquals('Crypt', FilterType::Crypt->value);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testFromLooseCanonical(): void
    {
        $this->assertSame(FilterType::FlateDecode, FilterType::fromLoose('FlateDecode'));
        $this->assertSame(FilterType::Ascii85Decode, FilterType::fromLoose('ASCII85Decode'));
        $this->assertSame(FilterType::Crypt, FilterType::fromLoose('Crypt'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(FilterType::DctDecode, FilterType::fromLoose(FilterType::DctDecode));
    }

    /**
     * Round-trip invariant: every case resolves back from its own backing value.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testFromLooseRoundTrip(): void
    {
        foreach (FilterType::cases() as $case) {
            $this->assertSame($case, FilterType::fromLoose($case->value));
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testFromLooseUnknownThrows(): void
    {
        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);
        FilterType::fromLoose('Unknownn');
    }

    /**
     * PDF filter names are case sensitive, so a wrong-case value is unknown.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testFromLooseIsCaseSensitive(): void
    {
        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);
        FilterType::fromLoose('flatedecode');
    }

    /**
     * The empty string is "no filter", not a FilterType, so it is unknown here.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testFromLooseEmptyThrows(): void
    {
        $this->bcExpectException('\\' . \Com\Tecnick\Pdf\Filter\Exception::class);
        FilterType::fromLoose('');
    }

    /**
     * The widened decode() accepts a FilterType enum and behaves like the string path.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testDecodeAcceptsEnum(): void
    {
        $filter = new Filter();
        $code = '74 63 2D 6C 69 62 2D 70 64 66 2D 66 69 6C 74 65 72>';
        $this->assertEquals('tc-lib-pdf-filter', $filter->decode(FilterType::AsciiHexDecode, $code));
        $this->assertEquals('tc-lib-pdf-filter', $filter->decode('ASCIIHexDecode', $code));
    }

    /**
     * decodeAll() applies a chain that mixes enum and string filter identifiers.
     *
     * @throws \Com\Tecnick\Pdf\Filter\Exception
     */
    public function testDecodeAllAcceptsEnum(): void
    {
        $filter = new Filter();
        $code = '74 63 2D 6C 69 62 2D 70 64 66 2D 66 69 6C 74 65 72>';
        $this->assertEquals('tc-lib-pdf-filter', $filter->decodeAll([FilterType::AsciiHexDecode], $code));
    }
}
