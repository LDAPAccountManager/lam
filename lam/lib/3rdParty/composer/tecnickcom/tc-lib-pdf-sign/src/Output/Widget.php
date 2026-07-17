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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
     * @param string        $appearance     Optional pre-built appearance fragment (e.g. " /AS /N /AP << ... >>").
     * @param callable|null $stringEncoder  fn(string $text, int $objectId): string.
     *
     * @throws Exception If the string encoder returns a non-string value.
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
