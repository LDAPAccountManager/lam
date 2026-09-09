<?php

declare(strict_types=1);

/**
 * Widget.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Output;

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Output\Widget
 *
 * Emits a signature field's widget annotation (/Subtype /Widget, /FT /Sig). The
 * same shape serves the signed field (with a /V reference to the /Sig value
 * object) and the reserved empty approval fields (no /V). The rectangle, the
 * page object number, and any appearance fragment are computed by the host,
 * which knows the page geometry and appearance resources.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Widget
{
    /**
     * Emit a signature widget annotation object.
     *
     * @param int           $objectId       Annotation object number.
     * @param string        $rect           Rectangle coordinates "x0 y0 x1 y1".
     * @param int           $pageObjectId   Object number of the page the widget is on (/P).
     * @param string        $fieldName      Partial field name (/T).
     * @param int|null      $valueObjectId  /V value object number; null for an unsigned field.
     * @param string        $appearance     Optional pre-built appearance fragment (e.g. " /AS /N /AP << ... >>"),
     *                                      written into the dictionary as given, as the
     *                                      /Reference and /M fragments Signature takes are.
     * @param callable|null $stringEncoder  fn(string $text, int $objectId): string.
     *
     * @throws Exception If the rectangle, an object number, or the field name is
     *                   malformed, or the string encoder returns a non-string value.
     */
    public function annotation(
        int $objectId,
        string $rect,
        int $pageObjectId,
        string $fieldName,
        ?int $valueObjectId = null,
        string $appearance = '',
        ?callable $stringEncoder = null,
    ): string {
        // Object numbers are interpolated as indirect references, and ISO 32000-1
        // section 7.3.10 admits only positive ones.
        foreach (['object' => $objectId, 'page object' => $pageObjectId] as $label => $number) {
            if ($number < 1) {
                throw new Exception('Invalid widget ' . $label . ' number: ' . $number);
            }
        }

        if ($valueObjectId !== null && $valueObjectId < 1) {
            throw new Exception('Invalid widget value object number: ' . $valueObjectId);
        }

        // ISO 32000-1 section 12.7.3.2: a period separates the components of a fully
        // qualified field name, so a partial name cannot contain one.
        if (\str_contains($fieldName, '.')) {
            throw new Exception('Invalid widget field name: ' . $fieldName);
        }

        // An invisible field carries no appearance rectangle; /Rect [] is not a valid
        // annotation, so the degenerate rectangle stands in for it.
        if ($rect === '') {
            $rect = '0 0 0 0';
        }

        // The rectangle is interpolated into the dictionary as written, so it has to
        // be the four PDF numbers it claims to be and nothing else. ISO 32000-1
        // section 7.3.3 admits a leading sign and a decimal point on either side of
        // the digits, but no exponent.
        // \z rather than $, which PCRE matches before a final newline.
        $number = '[+-]?(?:\d+\.?\d*|\.\d+)';
        if (\preg_match('/^' . $number . '(?: ' . $number . '){3}\z/', $rect) !== 1) {
            throw new Exception('Invalid widget rectangle: ' . $rect);
        }

        $out = $objectId . " 0 obj\n";
        $out .= '<< /Type /Annot /Subtype /Widget';
        $out .= ' /Rect [' . $rect . ']';
        $out .= ' /P ' . $pageObjectId . ' 0 R';
        $out .= ' /F 4 /FT /Sig';
        $out .= ' /T ' . PdfString::encode($fieldName, $objectId, $stringEncoder);
        $out .= ' /Ff 0';
        $out .= $appearance;

        if ($valueObjectId !== null) {
            $out .= ' /V ' . $valueObjectId . ' 0 R';
        }

        return $out . " >>\nendobj\n";
    }
}
