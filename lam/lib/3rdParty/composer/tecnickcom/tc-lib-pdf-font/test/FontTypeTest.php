<?php

/**
 * FontTypeTest.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * This file is part of tc-lib-pdf-font software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\FontType;
use Com\Tecnick\Pdf\Font\Import;

/**
 * FontType enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
class FontTypeTest extends TestUtil
{
    public function testCaseBackingValues(): void
    {
        $this->assertSame('', FontType::Auto->value);
        $this->assertSame('Core', FontType::Core->value);
        $this->assertSame('TrueType', FontType::TrueType->value);
        $this->assertSame('TrueTypeUnicode', FontType::TrueTypeUnicode->value);
        $this->assertSame('Type1', FontType::Type1->value);
        $this->assertSame('CID0JP', FontType::Cid0Jp->value);
        $this->assertSame('CID0KR', FontType::Cid0Kr->value);
        $this->assertSame('CID0CS', FontType::Cid0Cs->value);
        $this->assertSame('CID0CT', FontType::Cid0Ct->value);
    }

    /**
     * @throws FontException
     */
    public function testFromLooseCanonical(): void
    {
        $this->assertSame(FontType::Auto, FontType::fromLoose(''));
        $this->assertSame(FontType::TrueTypeUnicode, FontType::fromLoose('TrueTypeUnicode'));
        $this->assertSame(FontType::Cid0Jp, FontType::fromLoose('CID0JP'));
    }

    /**
     * @throws FontException
     */
    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(FontType::Type1, FontType::fromLoose(FontType::Type1));
    }

    /**
     * @throws FontException
     */
    public function testFromLooseRoundTrip(): void
    {
        foreach (FontType::cases() as $case) {
            $this->assertSame($case, FontType::fromLoose($case->value));
        }
    }

    /**
     * Font type names are case sensitive, so a wrong-case value is unknown.
     *
     * @throws FontException
     */
    public function testFromLooseIsCaseSensitive(): void
    {
        $this->bcExpectException(FontException::class);
        FontType::fromLoose('type1');
    }

    /**
     * @throws FontException
     */
    public function testFromLooseUnknownThrows(): void
    {
        $this->bcExpectException(FontException::class);
        FontType::fromLoose('OpenType');
    }

    /**
     * The widened Import constructor accepts a FontType enum and resolves it the
     * same way as the equivalent legacy string.
     *
     * @throws FontException
     * @throws \Com\Tecnick\File\Exception
     * @throws \RangeException
     */
    public function testImportAcceptsEnum(): void
    {
        $fin = \dirname(__DIR__) . '/util/vendor/tecnickcom/tc-font-mirror/core/Helvetica.afm';
        $outdir = \dirname(__DIR__) . '/target/tmptest/fonttype/';
        \system('rm -rf ' . $outdir . ' && mkdir -p ' . $outdir);

        $import = new Import($fin, $outdir, FontType::Core);
        $this->assertNotSame('', $import->getFontName());
    }
}
