<?php

/**
 * OutputTestOutFont.php
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

/**
 * @phpstan-import-type TFontData from \Com\Tecnick\Pdf\Font\Load
 */
class OutputTestOutFont extends \Com\Tecnick\Pdf\Font\OutFont
{
    /**
     * @param TFontData $font
     */
    public function runUniToCid(array &$font, int $cidoffset): void
    {
        $this->uniToCid($font, $cidoffset);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Font\Exception
     */
    public function runGetFontFullPath(string $fontdir, string $file): string
    {
        return $this->getFontFullPath($fontdir, $file);
    }

    public function runGetKeyValOut(string $key, mixed $val): string
    {
        return $this->getKeyValOut($key, $val);
    }
}
