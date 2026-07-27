<?php

/**
 * LoadTest.php
 *
 * @since     2026-05-21
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

/**
 * Load Test
 *
 * @since     2026-05-21
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 */
class LoadTest extends TestUtil
{
    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadAppliesFallbackStylesForMissingVariantFiles(): void
    {
        $load = new LoadTestHarness('customfont', '');
        $load->setModeAndMetrics(true, true, 0, 0, 0);

        $load->load();

        $this->assertSame('customfontBoldItalic', $load->getNameValue());
        $this->assertSame(123, $load->getStemVValue());
        $this->assertSame(-11, $load->getItalicAngleValue());
        $this->assertSame(64, $load->getFlagsValue());
    }

    public function testFindFontDirectoriesExcludesEmptyRootEntry(): void
    {
        $load = new LoadTestHarness('customfont', '');

        $dirs = $load->exposeFontDirectories();

        // An empty entry makes findFontFile() probe the filesystem root
        // (is_readable('' . DIRECTORY_SEPARATOR . $file) === is_readable('/<font>.json')),
        // which fails and emits a warning under open_basedir restrictions.
        // See https://github.com/tecnickcom/tc-lib-pdf/issues/238
        $this->assertNotContains('', $dirs);
    }

    /** @throws \Com\Tecnick\Pdf\Font\Exception */
    public function testLoadUpdatesExistingBoldAndItalicMetrics(): void
    {
        $load = new LoadTestHarness('customfont', 'Custom');
        $load->setModeAndMetrics(true, true, 100, -20, 1);

        $load->load();

        $this->assertSame('CustomBoldItalic', $load->getNameValue());
        $this->assertSame(175, $load->getStemVValue());
        $this->assertSame(-31, $load->getItalicAngleValue());
        $this->assertSame(65, $load->getFlagsValue());
    }
}
