<?php

declare(strict_types=1);

/**
 * BlendMode.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * This file is part of tc-lib-pdf-graph software library.
 */

namespace Com\Tecnick\Pdf\Graph;

/**
 * Com\Tecnick\Pdf\Graph\BlendMode
 *
 * Backed enum for the PDF blend modes (PDF 32000-1:2008 - 11.3.5). The backing
 * value of each case is the canonical /BM name validated by Gradient::getAlpha().
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
enum BlendMode: string
{
    case Normal = 'Normal';

    case Multiply = 'Multiply';

    case Screen = 'Screen';

    case Overlay = 'Overlay';

    case Darken = 'Darken';

    case Lighten = 'Lighten';

    case ColorDodge = 'ColorDodge';

    case ColorBurn = 'ColorBurn';

    case HardLight = 'HardLight';

    case SoftLight = 'SoftLight';

    case Difference = 'Difference';

    case Exclusion = 'Exclusion';

    case Hue = 'Hue';

    case Saturation = 'Saturation';

    case Color = 'Color';

    case Luminosity = 'Luminosity';

    /**
     * Resolve a loose blend mode value to the matching enum case.
     *
     * Accepts an enum instance (returned unchanged) or a string (with an
     * optional leading '/' stripped, as PDF /BM names carry). Unknown values
     * fall back to Normal, matching Gradient::getAlpha().
     *
     * @param string|self $value Blend mode name or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        if ($value !== '' && $value[0] === '/') {
            $value = \substr($value, 1);
        }

        return self::tryFrom($value) ?? self::Normal;
    }
}
