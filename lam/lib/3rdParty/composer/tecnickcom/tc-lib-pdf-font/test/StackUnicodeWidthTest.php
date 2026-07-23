<?php

/**
 * StackUnicodeWidthTest.php
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

use Com\Tecnick\File\Exception as FileException;
use Com\Tecnick\Pdf\Font\Exception as FontException;
use Com\Tecnick\Pdf\Font\Import;
use Com\Tecnick\Pdf\Font\Stack;

/**
 * Verifies that Stack::getCharWidth() resolves widths for non-ASCII Unicode
 * codepoints via the cwu table populated during Core font import.
 */
class StackUnicodeWidthTest extends TestUtil
{
    private const HELVETICA_AFM = __DIR__ . '/../util/vendor/tecnickcom/tc-font-mirror/core/Helvetica.afm';

    private function prepareTestEnvironment(): void
    {
        parent::setupTest();
    }

    private function fontPath(): string
    {
        return parent::getFontPath();
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    private function loadHelveticaStack(): Stack
    {
        $this->prepareTestEnvironment();
        new Import((string) \realpath(self::HELVETICA_AFM), $this->fontPath());
        $objnum = 1;
        $stack = new Stack(0.75, true, true, false);
        $stack->insert($objnum, 'helvetica', '', 10);
        return $stack;
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStackResolvesEmdashByUnicode(): void
    {
        $stack = $this->loadHelveticaStack();
        // em-dash U+2014 → AFM width 1000, at 10pt with stretching=1: 1000 * (10/1000) * 1 = 10.0
        $this->bcAssertEqualsWithDelta(10.0, $stack->getCharWidth(0x2014), 0.001);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStackResolvesEndashByUnicode(): void
    {
        $stack = $this->loadHelveticaStack();
        // en-dash U+2013 → AFM width 556
        $this->bcAssertEqualsWithDelta(5.56, $stack->getCharWidth(0x2013), 0.001);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStackResolvesBulletByUnicode(): void
    {
        $stack = $this->loadHelveticaStack();
        // bullet U+2022 → AFM width 350
        $this->bcAssertEqualsWithDelta(3.5, $stack->getCharWidth(0x2022), 0.001);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStackResolvesEuroByUnicode(): void
    {
        $stack = $this->loadHelveticaStack();
        // Euro U+20AC → AFM width 556
        $this->bcAssertEqualsWithDelta(5.56, $stack->getCharWidth(0x20AC), 0.001);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStackResolvesSpaceByCodepoint(): void
    {
        $stack = $this->loadHelveticaStack();
        // space U+0020 → AFM width 278 (sanity check, no regression)
        $this->bcAssertEqualsWithDelta(2.78, $stack->getCharWidth(0x20), 0.001);
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStackReturnZeroForShy(): void
    {
        $stack = $this->loadHelveticaStack();
        $this->assertSame(0.0, $stack->getCharWidth(0xAD));
    }

    /**
     * @throws FileException
     * @throws FontException
     * @throws \RangeException
     */
    public function testStackReturnZeroForZwsp(): void
    {
        $stack = $this->loadHelveticaStack();
        $this->assertSame(0.0, $stack->getCharWidth(0x200B));
    }
}
