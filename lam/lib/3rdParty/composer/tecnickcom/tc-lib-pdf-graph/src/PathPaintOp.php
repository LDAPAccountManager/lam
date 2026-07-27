<?php

declare(strict_types=1);

/**
 * PathPaintOp.php
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
 * Com\Tecnick\Pdf\Graph\PathPaintOp
 *
 * Backed enum for the PDF path-painting operators (PDF 32000-1:2008 - 8.5.3).
 * The backing value of each case is the canonical operator produced by
 * Style::getPathPaintOp(); its many input aliases collapse onto these cases via
 * fromLoose().
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
enum PathPaintOp: string
{
    case Stroke = 'S';

    case CloseStroke = 's';

    case Fill = 'f';

    case CloseFill = 'h f';

    case FillEvenOdd = 'f*';

    case CloseFillEvenOdd = 'h f*';

    case FillStroke = 'B';

    case FillStrokeEvenOdd = 'B*';

    case CloseFillStroke = 'b';

    case CloseFillStrokeEvenOdd = 'b*';

    case Clip = 'W n';

    case ClipEvenOdd = 'W* n';

    case Close = 'h';

    case NoOp = 'n';

    /**
     * Resolve a loose path paint mode to the matching canonical operator case.
     *
     * Accepts an enum instance (returned unchanged) or any of the input aliases
     * accepted by Style::getPathPaintOp() (case sensitive). Unknown values fall
     * back to Stroke, matching the default of Style::getPathPaintOp().
     *
     * @param string|self $value Path paint mode or enum case.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return match ($value) {
            'S', 'D' => self::Stroke,
            's', 'h S', 'd' => self::CloseStroke,
            'f', 'F' => self::Fill,
            'h f' => self::CloseFill,
            'f*', 'F*' => self::FillEvenOdd,
            'h f*' => self::CloseFillEvenOdd,
            'B', 'FD', 'DF' => self::FillStroke,
            'B*', 'F*D', 'DF*' => self::FillStrokeEvenOdd,
            'b', 'h B', 'fd', 'df' => self::CloseFillStroke,
            'b*', 'h B*', 'f*d', 'df*' => self::CloseFillStrokeEvenOdd,
            'W n', 'CNZ' => self::Clip,
            'W* n', 'CEO' => self::ClipEvenOdd,
            'h' => self::Close,
            'n' => self::NoOp,
            default => self::Stroke,
        };
    }
}
