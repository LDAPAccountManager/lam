<?php

declare(strict_types=1);

/**
 * PdfColorWriterInterface.php
 *
 * @since     2026-08-26
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Com\Tecnick\Color;

/**
 * Com\Tecnick\Color\PdfColorWriterInterface
 *
 * Writes the PDF and Acrobat JavaScript color forms.
 *
 * Implemented by Pdf.
 *
 * @since     2026-08-26
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
interface PdfColorWriterInterface
{
    /**
     * Get the color components format used in PDF documents.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $stroke    True for stroking, false for non-stroking.
     * @param float  $tint      Intensity of the color (from 0 to 1). Spot colors only.
     * @param bool   $allowSpot True to resolve (and register) spot colors, false to force a device color.
     */
    public function getPdfColor(string $color, bool $stroke = false, float $tint = 1, bool $allowSpot = true): string;

    /**
     * Get the stroked color components format used in PDF documents.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param float  $tint      Intensity of the color (from 0 to 1). Spot colors only.
     * @param bool   $allowSpot True to resolve (and register) spot colors, false to force a device color.
     */
    public function getPdfStrokeColor(string $color, float $tint = 1, bool $allowSpot = true): string;

    /**
     * Get the fill color components format used in PDF documents.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param float  $tint      Intensity of the color (from 0 to 1). Spot colors only.
     * @param bool   $allowSpot True to resolve (and register) spot colors, false to force a device color.
     */
    public function getPdfFillColor(string $color, float $tint = 1, bool $allowSpot = true): string;

    /**
     * Get the RGB color components format used in PDF documents.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $allowSpot True to resolve spot colors, false to force a device color.
     */
    public function getPdfRgbComponents(string $color, bool $allowSpot = true): string;

    /**
     * Get the CMYK color components format used in PDF documents.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $allowSpot True to resolve spot colors, false to force a device color.
     */
    public function getPdfCmykComponents(string $color, bool $allowSpot = true): string;

    /**
     * Convert a color to an Acrobat JavaScript color string.
     *
     * @param string $color color name or color object
     */
    public function getJsColorString(string $color): string;

    /**
     * Returns a color object from an HTML, CSS or Spot color representation.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $allowSpot True to resolve spot colors, false to force a device color.
     */
    public function getColorObject(string $color, bool $allowSpot = true): ?Model;
}
