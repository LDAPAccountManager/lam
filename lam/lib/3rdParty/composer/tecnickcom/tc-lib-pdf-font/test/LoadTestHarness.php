<?php

/**
 * LoadTestHarness.php
 *
 * @since     2026-05-21
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

class LoadTestHarness extends \Com\Tecnick\Pdf\Font\Load
{
    public function __construct(string $key, string $name)
    {
        parent::__construct();
        $this->data['type'] = 'TrueType';
        $this->data['key'] = $key;
        $this->data['name'] = $name;
        $this->data['fakestyle'] = true;
        $this->data['cw'][32] = 500;
    }

    public function setModeAndMetrics(bool $bold, bool $italic, int $stemv, int $italicAngle, int $flags): void
    {
        $this->data['mode']['bold'] = $bold;
        $this->data['mode']['italic'] = $italic;
        $this->data['mode']['linethrough'] = false;
        $this->data['mode']['overline'] = false;
        $this->data['mode']['underline'] = false;
        $this->data['desc']['StemV'] = $stemv;
        $this->data['desc']['ItalicAngle'] = $italicAngle;
        $this->data['desc']['Flags'] = $flags;
        $this->data['desc']['MissingWidth'] = 0;
    }

    public function getNameValue(): string
    {
        return $this->data['name'];
    }

    /**
     * Expose the resolved font search directories for testing.
     *
     * @return array<string>
     */
    public function exposeFontDirectories(): array
    {
        return $this->findFontDirectories();
    }

    public function getStemVValue(): int
    {
        return $this->data['desc']['StemV'];
    }

    public function getItalicAngleValue(): int
    {
        return $this->data['desc']['ItalicAngle'];
    }

    public function getFlagsValue(): int
    {
        return $this->data['desc']['Flags'];
    }

    protected function getFontInfo(): void
    {
        // Keep the preloaded test data and skip filesystem reads.
    }

    public function setFileData(): void
    {
        // Avoid dependency on actual font files for internal branch checks.
    }
}
