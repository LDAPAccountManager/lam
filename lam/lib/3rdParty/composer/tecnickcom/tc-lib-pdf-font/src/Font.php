<?php

declare(strict_types=1);

/**
 * Font.php
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

namespace Com\Tecnick\Pdf\Font;

use Com\Tecnick\File\File as ObjFile;
use Com\Tecnick\Pdf\Font\Exception as FontException;

/**
 * Com\Tecnick\Pdf\Font\Font
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfFont
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-font
 *
 * @phpstan-import-type TFontData from Load
 */
class Font extends \Com\Tecnick\Pdf\Font\Load
{
    /**
     * Load an imported font
     *
     * The definition file (and the font file itself when embedding) must be present either in the current directory
     * or in the one indicated by K_PATH_FONTS if the constant is defined.
     *
     * @param string $font     Font family.
     *                         If it is a standard family name, it will override the corresponding font.
     * @param string $style    Font style. Possible values are (case-insensitive):
     *                         regular (default)
     *                         B: bold
     *                         I: italic
     *                         U: underline
     *                         D: strikeout (linethrough)
     *                         O: overline
     * @param string $ifile    The font definition file (or empty for autodetect).
     *                         By default, the name is built from the family and
     *                         style, in lower case with no spaces.
     * @param bool   $subset   If true embed only a subset of the font
     *                         (stores only the information related to
     *                         the used characters); If false embed
     *                         full font; This option is valid only for
     *                         TrueTypeUnicode fonts and is disabled
     *                         for PDF/A. If you want to enable users
     *                         to modify the document, set this
     *                         parameter to false. If you subset the
     *                         font, the person who receives your PDF
     *                         would need to have your same font in
     *                         order to make changes to your PDF. The
     *                         file size of the PDF would also be
     *                         smaller because you are embedding only a subset.
     * @param bool   $unicode  True in Unicode mode, False otherwise.
     * @param bool   $pdfa     True in PDF/A mode, False otherwise.
     * @param bool   $compress Set to false to disable stream compression.
     * @param ObjFile|null $fileHelper Optional file helper for font loading.
     *
     * @throws FontException in case of error
     */
    public function __construct(
        string $font,
        string $style = '',
        string $ifile = '',
        bool $subset = false,
        bool $unicode = true,
        bool $pdfa = false,
        bool $compress = true,
        ?ObjFile $fileHelper = null,
    ) {
        parent::__construct($fileHelper);

        if ($font === '') {
            throw new FontException('empty font family name');
        }

        if ($ifile !== '') {
            $validatedIfile = $ifile;
            if (!$this->fileHelper->isValidFile($validatedIfile)) {
                throw new FontException('Invalid font ifile: ' . $ifile);
            }
        }

        $this->data['ifile'] = $ifile;
        $this->data['family'] = $font;
        $this->data['unicode'] = $unicode;
        $this->data['pdfa'] = $pdfa;
        $this->data['compress'] = $compress;
        $this->data['subset'] = $subset;
        $this->data['subsetchars'] = \array_fill(0, 256, true);

        // generate the font key and set styles
        $this->setStyle($style);
    }

    /**
     * Get the font key
     */
    public function getFontkey(): string
    {
        return $this->data['key'];
    }

    /**
     * Get the font data
     *
     * @return TFontData
     */
    public function getFontData(): array
    {
        return $this->data;
    }

    /**
     * Set style and normalize the font name
     *
     * @param string $style Style
     */
    protected function setStyle(string $style): void
    {
        $style = \strtoupper($style);
        if (\str_ends_with($this->data['family'], 'I')) {
            $style .= 'I';
            $this->data['family'] = \substr($this->data['family'], 0, -1);
        }

        if (\str_ends_with($this->data['family'], 'B')) {
            $style .= 'B';
            $this->data['family'] = \substr($this->data['family'], 0, -1);
        }

        // normalize family name
        $this->data['family'] = \strtolower($this->data['family']);
        if (!$this->data['unicode'] && $this->data['family'] === 'arial') {
            $this->data['family'] = 'helvetica';
        }

        if ($this->data['family'] === 'symbol' || $this->data['family'] === 'zapfdingbats') {
            $style = '';
        }

        if ($this->data['pdfa'] && isset(Core::FONT[$this->data['family']])) {
            // core fonts must be embedded in PDF/A
            $this->data['family'] = 'pdfa' . $this->data['family'];
        }

        $this->setStyleMode($style);
    }

    /**
     * Set style mode properties
     *
     * @param string $style Style
     */
    protected function setStyleMode(string $style): void
    {
        $suffix = '';
        if (\str_contains($style, 'B')) {
            $this->data['mode']['bold'] = true;
            $suffix .= 'B';
        }

        if (\str_contains($style, 'I')) {
            $this->data['mode']['italic'] = true;
            $suffix .= 'I';
        }

        $this->data['style'] = $suffix;
        if (\str_contains($style, 'U')) {
            $this->data['style'] .= 'U';
            $this->data['mode']['underline'] = true;
        }

        if (\str_contains($style, 'D')) {
            $this->data['style'] .= 'D';
            $this->data['mode']['linethrough'] = true;
        }

        if (\str_contains($style, 'O')) {
            $this->data['style'] .= 'O';
            $this->data['mode']['overline'] = true;
        }

        $this->data['key'] = $this->data['family'] . $suffix;
    }
}
