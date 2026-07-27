<?php

/**
 * OutputTestOutput.php
 *
 * @since     2026-06-16
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
 * Test helper exposing protected Output members.
 *
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 */
class OutputTestOutput extends \Com\Tecnick\Pdf\Font\Output
{
    /**
     * @param TFontData        $font     Extracted font metrics.
     * @param array<int, bool> $subchars Subset characters (charcode => enabled).
     */
    public function runSubsetCacheKey(string $font_data, array $font, array $subchars): string
    {
        return $this->subsetCacheKey($font_data, $font, $subchars);
    }
}
