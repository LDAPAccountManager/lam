<?php

declare(strict_types=1);

/**
 * Draw.php
 *
 * @since     2011-05-23
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

use Com\Tecnick\Pdf\Graph\Exception as GraphException;

/**
 * Com\Tecnick\Pdf\Graph\Draw
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * @phpstan-import-type StyleDataOpt from \Com\Tecnick\Pdf\Graph\Base
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
class Draw extends \Com\Tecnick\Pdf\Graph\Gradient
{
    /**
     * Draws a line between two points.
     *
     * @param float        $posx1 Abscissa of first point.
     * @param float        $posy1 Ordinate of first point.
     * @param float        $posx2 Abscissa of second point.
     * @param float        $posy2 Ordinate of second point.
     * @param StyleDataOpt $style Line style to apply.
     *
     * @return string PDF command
     */
    public function getLine(float $posx1, float $posy1, float $posx2, float $posy2, array $style = []): string
    {
        return (
            $this->getStyleCmd($style)
            . $this->getRawPoint($posx1, $posy1)
            . $this->getRawLine($posx2, $posy2)
            . $this->getPathPaintOp('S')
        );
    }

    /**
     * Draws a Bezier curve.
     * The Bezier curve is a tangent to the line between the control points at either end of the curve.
     *
     * @param float        $posx0 Abscissa of start point.
     * @param float        $posy0 Ordinate of start point.
     * @param float        $posx1 Abscissa of control point 1.
     * @param float        $posy1 Ordinate of control point 1.
     * @param float        $posx2 Abscissa of control point 2.
     * @param float        $posy2 Ordinate of control point 2.
     * @param float        $posx3 Abscissa of end point.
     * @param float        $posy3 Ordinate of end point.
     * @param string|PathPaintOp       $mode  Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt $style Style.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     */
    public function getCurve(
        float $posx0,
        float $posy0,
        float $posx1,
        float $posy1,
        float $posx2,
        float $posy2,
        float $posx3,
        float $posy3,
        string|PathPaintOp $mode = 'S',
        array $style = [],
    ): string {
        return (
            $this->getStyleCmd($style)
            . $this->getRawPoint($posx0, $posy0)
            . $this->getRawCurve($posx1, $posy1, $posx2, $posy2, $posx3, $posy3)
            . $this->getPathPaintOp($mode)
        );
    }

    /**
     * Draws a poly-Bezier curve.
     * Each Bezier curve segment is a tangent to the line between the control points at either end of the curve.
     *
     * @param float               $posx0    Abscissa of start point.
     * @param float               $posy0    Ordinate of start point.
     * @param array<array<float>> $segments An array of bezier descriptions. Format: array(x1, y1, x2, y2, x3, y3).
     * @param string|PathPaintOp              $mode     Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt        $style    Style.
     *
     * @return string PDF command
     */
    public function getPolycurve(
        float $posx0,
        float $posy0,
        array $segments,
        string|PathPaintOp $mode = 'S',
        array $style = [],
    ): string {
        $out = $this->getStyleCmd($style) . $this->getRawPoint($posx0, $posy0);
        foreach ($segments as $segment) {
            if (!isset($segment[0], $segment[1], $segment[2], $segment[3], $segment[4], $segment[5])) {
                continue;
            }

            $posx1 = $segment[0];
            $posy1 = $segment[1];
            $posx2 = $segment[2];
            $posy2 = $segment[3];
            $posx3 = $segment[4];
            $posy3 = $segment[5];
            $out .= $this->getRawCurve($posx1, $posy1, $posx2, $posy2, $posx3, $posy3);
        }

        return $out . $this->getPathPaintOp($mode);
    }

    /**
     * Draws an ellipse.
     * An ellipse is formed from n Bezier curves.
     *
     * @param float        $posx  Abscissa of center point.
     * @param float        $posy  Ordinate of center point.
     * @param float        $hrad  Horizontal radius.
     * @param float        $vrad  Vertical radius.
     * @param float        $angle Angle oriented (anti-clockwise). Default value: 0.
     * @param float        $angs  Angle in degrees at which starting drawing.
     * @param float        $angf  Angle in degrees at which stop drawing.
     * @param string|PathPaintOp       $mode  Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt $style Style.
     * @param int          $ncv   Number of curves used to draw a 90 degrees portion of ellipse.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     */
    public function getEllipse(
        float $posx,
        float $posy,
        float $hrad,
        float $vrad = 0,
        float $angle = 0,
        float $angs = 0,
        float $angf = 360,
        string|PathPaintOp $mode = 'S',
        array $style = [],
        int $ncv = 2,
    ): string {
        if ($vrad === 0.0) {
            $vrad = $hrad;
        }

        return (
            $this->getStyleCmd($style)
            . $this->getRawEllipticalArc(
                $posx,
                $posy,
                $hrad,
                $vrad,
                $angle,
                $angs,
                $angf,
                false,
                $ncv,
                true,
                true,
                false,
            )
            . $this->getPathPaintOp($mode)
        );
    }

    /**
     * Draws a circle.
     * A circle is formed from n Bezier curves.
     *
     * @param float        $posx  Abscissa of center point.
     * @param float        $posy  Ordinate of center point.
     * @param float        $rad   Radius.
     * @param float        $angs  Angle in degrees at which starting drawing.
     * @param float        $angf  Angle in degrees at which stop drawing.
     * @param string|PathPaintOp       $mode  Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt $style Style.
     * @param int          $ncv   Number of curves used to draw a 90 degrees portion of ellipse.
     *
     * @return string PDF command
     */
    public function getCircle(
        float $posx,
        float $posy,
        float $rad,
        float $angs = 0,
        float $angf = 360,
        string|PathPaintOp $mode = 'S',
        array $style = [],
        int $ncv = 2,
    ): string {
        return $this->getEllipse($posx, $posy, $rad, $rad, 0, $angs, $angf, $mode, $style, $ncv);
    }

    /**
     * Draws a circle pie sector.
     *
     * @param float        $posx  Abscissa of center point.
     * @param float        $posy  Ordinate of center point.
     * @param float        $rad   Radius.
     * @param float        $angs  Angle in degrees at which starting drawing.
     * @param float        $angf  Angle in degrees at which stop drawing.
     * @param string|PathPaintOp       $mode  Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt $style Style.
     * @param int          $ncv   Number of curves used to draw a 90 degrees portion of ellipse.
     *
     * @return string PDF command
     */
    public function getPieSector(
        float $posx,
        float $posy,
        float $rad,
        float $angs = 0,
        float $angf = 360,
        string|PathPaintOp $mode = 'FD',
        array $style = [],
        int $ncv = 2,
    ): string {
        return (
            $this->getStyleCmd($style)
            . $this->getRawEllipticalArc($posx, $posy, $rad, $rad, 0, $angs, $angf, true, $ncv, true, true, false)
            . $this->getPathPaintOp($mode)
        );
    }

    /**
     * Draws a basic polygon.
     *
     * @param array<float> $points Points - array containing 4 points for each segment: (x0, y0, x1, y1, x2, y2, ...)
     * @param string|PathPaintOp       $mode   Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt $style  Style.
     *
     * @return string PDF command
     */
    public function getBasicPolygon(array $points, string|PathPaintOp $mode = 'S', array $style = []): string
    {
        $nco = \count($points); // number of coordinates
        if ($nco < 4 || ($points[0] ?? null) === null || ($points[1] ?? null) === null) {
            return ''; // need at least 2 points (4 coordinates)
        }

        $out = $this->getStyleCmd($style) . $this->getRawPoint($points[0], $points[1]);
        for ($idx = 2; $idx < $nco; $idx += 2) {
            if (($points[$idx] ?? null) === null || ($points[$idx + 1] ?? null) === null) {
                continue;
            }

            $posx = $points[$idx] ?? 0.0;
            $posy = $points[$idx + 1] ?? 0.0;
            $out .= $this->getRawLine($posx, $posy);
        }

        return $out . $this->getPathPaintOp($mode);
    }

    /**
     * Returns the polygon default style command and initialize the first segment style if missing.
     *
     * @param array<StyleDataOpt> $styles Array of styles -
     *        one style entry for each polygon segment and/or one global "all" entry.
     *
     * @return string PDF command
     */
    protected function getDefaultSegStyle(array $styles = []): string
    {
        $out = '';
        $styleAll = $styles['all'] ?? [];
        if ($styleAll !== []) {
            $out .= $this->getStyleCmd($styleAll);
        }

        return $out;
    }

    /**
     * Draws a polygon with a different style for each segment.
     *
     * @param array<float>        $points Points - array with values (x0, y0, x1, y1,..., x(n-1), y(n-1))
     * @param string|PathPaintOp              $mode   Mode of rendering. @see getPathPaintOp()
     * @param array<StyleDataOpt> $styles Array of styles -
     *        one style entry for each polygon segment and/or one global "all" entry.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    public function getPolygon(array $points, string|PathPaintOp $mode = 'S', array $styles = []): string
    {
        if ($mode instanceof PathPaintOp) {
            $mode = $mode->value;
        }

        $nco = \count($points); // number of points
        if ($nco < 6 || ($points[0] ?? null) === null || ($points[1] ?? null) === null) {
            return ''; // we need at least 3 points
        }

        $nseg = (int) ($nco / 2); // number of segments (including the closing one)

        $out = $this->getDefaultSegStyle($styles);

        $lastx = $points[$nco - 2] ?? null;
        $lasty = $points[$nco - 1] ?? null;
        if (
            $this->isClosingMode($mode)
            && $lastx !== null
            && $lasty !== null
            && ($lastx !== $points[0] || $lasty !== $points[1])
        ) {
            // close polygon by adding the first point (x, y) at the end
            $points[$nco++] = $points[0];
            $points[$nco++] = $points[1];
            $style0 = $styles[0] ?? [];
            if ($style0 !== []) {
                // copy style for the last segment
                $styles[$nseg - 1] = $style0;
            }
        }

        // paint the filling
        if ($this->isFillingMode($mode)) {
            $out .= $this->getBasicPolygon($points, $this->getModeWithoutStroke($mode));
            if ($this->isClippingMode($mode)) {
                return $out;
            }
        }

        if (!$this->isStrokingMode($mode)) {
            return $out;
        }

        // When the outline is uniformly styled (no per-segment style overrides),
        // stroke it as a single continuous path so that adjacent edges are joined
        // at the vertices (miter/round/bevel) instead of being drawn as
        // independent, butt-capped segments that would leave the corners open.
        $hasSegmentStyles = false;
        foreach ($styles as $skey => $sval) {
            if (!\is_int($skey) || $sval === []) {
                continue;
            }

            $hasSegmentStyles = true;
            break;
        }

        if (!$hasSegmentStyles) {
            $closed = $points[0] === ($points[$nco - 2] ?? null) && $points[1] === ($points[$nco - 1] ?? null);
            return $out . $this->getBasicPolygon($points, $closed ? 's' : 'S');
        }

        $nco -= 3;

        // paint the outline
        for ($idx = 0; $idx < $nco; $idx += 2) {
            $segid = (int) ($idx / 2);
            $styles[$segid] ??= [];

            if (
                ($points[$idx] ?? null) === null
                || ($points[$idx + 1] ?? null) === null
                || ($points[$idx + 2] ?? null) === null
                || ($points[$idx + 3] ?? null) === null
            ) {
                continue;
            }

            $posx1 = $points[$idx] ?? 0.0;
            $posy1 = $points[$idx + 1] ?? 0.0;
            $posx2 = $points[$idx + 2] ?? 0.0;
            $posy2 = $points[$idx + 3] ?? 0.0;
            $segstyle = $styles[$segid] ?? [];

            $out .= $this->getLine($posx1, $posy1, $posx2, $posy2, $segstyle);
        }

        return $out;
    }

    /**
     * Draws a regular polygon.
     *
     * @param float               $posx     Abscissa of center point.
     * @param float               $posy     Ordinate of center point.
     * @param float               $radius   Radius of inscribed circle.
     * @param int                 $sides    Number of sides.
     * @param float               $angle    Angle of the orientation (anti-clockwise).
     * @param string|PathPaintOp              $mode     Mode of rendering. @see getPathPaintOp()
     * @param array<StyleDataOpt> $styles   Array of styles -
     *        one style entry for each polygon segment and/or one global "all" entry.
     * @param string|PathPaintOp              $cirmode  Mode of rendering of the inscribed circle (if any). @see getPathPaintOp()
     * @param StyleDataOpt        $cirstyle Style of inscribed circle.
     *
     * @return string PDF command
     */
    public function getRegularPolygon(
        float $posx,
        float $posy,
        float $radius,
        int $sides,
        float $angle = 0,
        string|PathPaintOp $mode = 'S',
        array $styles = [],
        string|PathPaintOp $cirmode = '',
        array $cirstyle = [],
    ): string {
        if ($sides < 3) { // triangle is the minimum polygon
            return '';
        }

        $out = '';
        if ($cirmode !== '') {
            $out .= $this->getCircle($posx, $posy, $radius, 0, 360, $cirmode, $cirstyle);
        }

        $points = [];
        for ($idx = 0; $idx < $sides; ++$idx) {
            $angrad = $this->degToRad($angle + (($idx * 360) / $sides));
            $points[] = $posx + ($radius * \sin($angrad));
            $points[] = $posy + ($radius * \cos($angrad));
        }

        return $out . $this->getPolygon($points, $mode, $styles);
    }

    /**
     * Draws a star polygon.
     *
     * @param float               $posx     Abscissa of center point.
     * @param float               $posy     Ordinate of center point.
     * @param float               $radius   Radius of inscribed circle.
     * @param int                 $nvert    Number of vertices.
     * @param int                 $ngaps    Number of gaps (if ($ngaps % $nvert = 1) then is a regular polygon).
     * @param float               $angle    Angle oriented (anti-clockwise).
     * @param string|PathPaintOp              $mode     Mode of rendering. @see getPathPaintOp()
     * @param array<StyleDataOpt> $styles   Array of styles -
     *        one style entry for each polygon segment and/or one global "all" entry.
     * @param string|PathPaintOp              $cirmode  Mode of rendering of the inscribed circle (if any). @see getPathPaintOp()
     * @param StyleDataOpt        $cirstyle Style of inscribed circle.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     */
    public function getStarPolygon(
        float $posx,
        float $posy,
        float $radius,
        int $nvert,
        int $ngaps,
        float $angle = 0,
        string|PathPaintOp $mode = 'S',
        array $styles = [],
        string|PathPaintOp $cirmode = '',
        array $cirstyle = [],
    ): string {
        if ($nvert < 2) {
            return '';
        }

        $out = '';
        if ($cirmode !== '') {
            $out .= $this->getCircle($posx, $posy, $radius, 0, 360, $cirmode, $cirstyle);
        }

        $points = [];
        $visited = [];
        $idx = 0;
        while ($idx >= 0 && !isset($visited[$idx])) {
            $visited[$idx] = true;
            $angrad = $this->degToRad($angle + (($idx * 360) / $nvert));
            $points[] = $posx + ($radius * \sin($angrad));
            $points[] = $posy + ($radius * \cos($angrad));
            $idx = ($idx + $ngaps) % $nvert;
        }

        if (\count($points) < 6) {
            return $out;
        }

        return $out . $this->getPolygon($points, $mode, $styles);
    }

    /**
     * Draws a rectangle with a different style for each segment.
     *
     * @param float               $posx   Abscissa of upper-left corner.
     * @param float               $posy   Ordinate of upper-left corner.
     * @param float               $width  Width.
     * @param float               $height Height.
     * @param string|PathPaintOp              $mode   Mode of rendering. @see getPathPaintOp()
     * @param array<StyleDataOpt> $styles Array of styles -
     *        one style entry for each side (T,R,B,L) and/or one global "all" entry.
     *
     * @return string PDF command
     */
    public function getRect(
        float $posx,
        float $posy,
        float $width,
        float $height,
        string|PathPaintOp $mode = 'S',
        array $styles = [],
    ): string {
        $points = [
            $posx,
            $posy,
            $posx + $width,
            $posy,
            $posx + $width,
            $posy + $height,
            $posx,
            $posy + $height,
            $posx,
            $posy,
        ];
        return $this->getPolygon($points, $mode, $styles);
    }

    /**
     * Draws a rounded rectangle.
     *
     * @param float        $posx   Abscissa of upper-left corner.
     * @param float        $posy   Ordinate of upper-left corner.
     * @param float        $width  Width.
     * @param float        $height Height.
     * @param float        $hrad   X-axis radius of the ellipse used to round off the corners of the rectangle.
     * @param float        $vrad   Y-axis radius of the ellipse used to round off the corners of the rectangle.
     * @param string       $corner Round corners to draw: 0 (square i-corner) or 1 (rounded i-corner) in i-position.
     *                             Positions are in the following order: top right, bottom right, bottom left and
     *                             top left.
     * @param string|PathPaintOp       $mode   Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt $style  Style.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    public function getRoundedRect(
        float $posx,
        float $posy,
        float $width,
        float $height,
        float $hrad,
        float $vrad,
        string $corner = '1111',
        string|PathPaintOp $mode = 'S',
        array $style = [],
    ): string {
        if ($corner === '0000' || $hrad === 0.0 && $vrad === 0.0) {
            // basic rectangle with straight corners
            return $this->getBasicRect($posx, $posy, $width, $height, $mode, $style);
        }

        $out = $this->getStyleCmd($style);
        $out .= $corner[3] !== '0' ? $this->getRawPoint($posx + $hrad, $posy) : $this->getRawPoint($posx, $posy);

        $posxc = $posx + $width - $hrad;
        $posyc = $posy + $vrad;
        $out .= $this->getRawLine($posxc, $posy);
        $arc = (4 / 3) * (\sqrt(2) - 1);
        $harc = $hrad * $arc;
        $varc = $vrad * $arc;

        $out .= $corner[0] !== '0'
            ? $this->getRawCurve($posxc + $harc, $posyc - $vrad, $posxc + $hrad, $posyc - $varc, $posxc + $hrad, $posyc)
            : $this->getRawLine($posx + $width, $posy);

        $posxc = $posx + $width - $hrad;
        $posyc = $posy + $height - $vrad;
        $out .= $this->getRawLine($posx + $width, $posyc);

        $out .= $corner[1] !== '0'
            ? $this->getRawCurve($posxc + $hrad, $posyc + $varc, $posxc + $harc, $posyc + $vrad, $posxc, $posyc + $vrad)
            : $this->getRawLine($posx + $width, $posy + $height);

        $posxc = $posx + $hrad;
        $posyc = $posy + $height - $vrad;
        $out .= $this->getRawLine($posxc, $posy + $height);

        $out .= $corner[2] !== '0'
            ? $this->getRawCurve($posxc - $harc, $posyc + $vrad, $posxc - $hrad, $posyc + $varc, $posxc - $hrad, $posyc)
            : $this->getRawLine($posx, $posy + $height);

        $posxc = $posx + $hrad;
        $posyc = $posy + $vrad;
        $out .= $this->getRawLine($posx, $posyc);

        $out .= $corner[3] !== '0'
            ? $this->getRawCurve($posxc - $hrad, $posyc - $varc, $posxc - $harc, $posyc - $vrad, $posxc, $posyc - $vrad)
            : $this->getRawLine($posx, $posy);

        return $out . $this->getPathPaintOp($mode);
    }

    /**
     * Draws an arrow.
     *
     * @param float        $posx0    Abscissa of first point.
     * @param float        $posy0    Ordinate of first point.
     * @param float        $posx1    Abscissa of second point (head side).
     * @param float        $posy1    Ordinate of second point (head side)
     * @param int          $headmode Arrow head mode:
     *                               0 = draw only
     *                               head arms; 1 =
     *                               draw closed head
     *                               without filling;
     *                               2 = closed and
     *                               filled head; 3 =
     *                               filled head.
     * @param float        $armsize  Length of head arms.
     * @param int          $armangle Angle between an head arm and the arrow shaft.
     * @param StyleDataOpt $style    Line style to apply.
     *
     * @return string PDF command
     */
    public function getArrow(
        float $posx0,
        float $posy0,
        float $posx1,
        float $posy1,
        int $headmode = 0,
        float $armsize = 5,
        int $armangle = 15,
        array $style = [],
    ): string {
        // getting arrow direction angle; 0 deg angle is when both arms go along X axis; angle grows clockwise.
        $dir_angle = \atan2($posy0 - $posy1, $posx0 - $posx1);
        if ($dir_angle < 0) {
            $dir_angle += 2 * self::MPI;
        }

        $armangle = $this->degToRad($armangle);
        $sx1 = $posx1;
        $sy1 = $posy1;
        if ($headmode > 0) {
            // calculate the stopping point for the arrow shaft
            $linewidth = $style['lineWidth'] ?? (float) $this->getLastStyleProperty('lineWidth', 0);

            $sx1 = $posx1 + (($armsize - $linewidth) * \cos($dir_angle));
            $sy1 = $posy1 + (($armsize - $linewidth) * \sin($dir_angle));
        }

        $out = $this->getStyleCmd($style);
        // main arrow line / shaft
        $out .= $this->getLine($posx0, $posy0, $sx1, $sy1);
        // left arrowhead arm tip
        $hxl = $posx1 + ($armsize * \cos($dir_angle + $armangle));
        $hyl = $posy1 + ($armsize * \sin($dir_angle + $armangle));
        // right arrowhead arm tip
        $hxr = $posx1 + ($armsize * \cos($dir_angle - $armangle));
        $hyr = $posy1 + ($armsize * \sin($dir_angle - $armangle));
        $modemap = [
            0 => 'S',
            1 => 's',
            2 => 'b',
            3 => 'f',
        ];
        $points = [$hxl, $hyl, $posx1, $posy1, $hxr, $hyr];
        // The style was already emitted above and persists in the graphics
        // state, so the arrowhead does not need to repeat it.
        return $out . $this->getBasicPolygon($points, $modemap[$headmode] ?? 'S');
    }

    /**
     * Get a registration mark.
     *
     * @param float  $posx   Abscissa of center point.
     * @param float  $posy   Ordinate of center point.
     * @param float  $rad    Radius.
     * @param bool   $double If true prints two concentric crop marks.
     * @param string $color  Color.
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getRegistrationMark(
        float $posx,
        float $posy,
        float $rad,
        bool $double = false,
        string $color = 'all',
    ): string {
        $style = [
            'lineWidth' => \max(0.5 / $this->kunit, $rad / 30),
            'lineCap' => 'butt',
            'lineJoin' => 'miter',
            'miterLimit' => 10.0 / $this->kunit,
            'dashArray' => [],
            'dashPhase' => 0,
            'lineColor' => $color,
            'fillColor' => $color,
        ];

        $colobj = $this->pdfColor->getColorObject($color);
        if (!$colobj instanceof \Com\Tecnick\Color\Model) {
            throw new GraphException('Unknown color: ' . $color);
        }

        $out =
            $colobj->getPdfColor()
            . $this->getPieSector($posx, $posy, $rad, 90, 180, 'F')
            . $this->getPieSector($posx, $posy, $rad, 270, 360, 'F')
            . $this->getCircle($posx, $posy, $rad, 0, 360, 'S', [], 8);
        if ($double) {
            $radi = $rad * 0.5;
            $out .=
                $colobj->invertColor()->getPdfColor()
                . $this->getPieSector($posx, $posy, $radi, 90, 180, 'F')
                . $this->getPieSector($posx, $posy, $radi, 270, 360, 'F')
                . $this->getCircle($posx, $posy, $radi, 0, 360, 'S', [], 8)
                . $colobj->getPdfColor()
                . $this->getPieSector($posx, $posy, $radi, 0, 90, 'F')
                . $this->getPieSector($posx, $posy, $radi, 180, 270, 'F')
                . $this->getCircle($posx, $posy, $radi, 0, 360, 'S', [], 8);
        }

        return $this->getStartTransform() . $this->getStyleCmd($style) . $out . $this->getStopTransform();
    }

    /**
     * Get a CMYK registration mark.
     *
     * @param float $posx Abscissa of center point.
     * @param float $posy Ordinate of center point.
     * @param float $rad  Radius.
     *
     * @return string PDF command
     */
    public function getCmykRegistrationMark(float $posx, float $posy, float $rad): string
    {
        // internal radius
        $radi = $rad * 0.6;
        // external radius
        $rade = $rad * 1.3;
        // line style for external circle
        $style = [
            'lineWidth' => \max(0.5 / $this->kunit, $rad / 30),
            'lineCap' => 'butt',
            'lineJoin' => 'miter',
            'miterLimit' => 10.0 / $this->kunit,
            'dashArray' => [],
            'dashPhase' => 0,
            'lineColor' => 'All',
            'fillColor' => '',
        ];

        $cyan = $this->pdfColor->getColorObject('Cyan');
        $magenta = $this->pdfColor->getColorObject('Magenta');
        $yellow = $this->pdfColor->getColorObject('Yellow');
        $key = $this->pdfColor->getColorObject('Key');

        return (
            $this->getStartTransform()
            . ($cyan instanceof \Com\Tecnick\Color\Model ? $cyan->getPdfColor() : '')
            . $this->getPieSector($posx, $posy, $radi, 270, 360, 'F')
            . ($magenta instanceof \Com\Tecnick\Color\Model ? $magenta->getPdfColor() : '')
            . $this->getPieSector($posx, $posy, $radi, 0, 90, 'F')
            . ($yellow instanceof \Com\Tecnick\Color\Model ? $yellow->getPdfColor() : '')
            . $this->getPieSector($posx, $posy, $radi, 90, 180, 'F')
            . ($key instanceof \Com\Tecnick\Color\Model ? $key->getPdfColor() : '')
            . $this->getPieSector($posx, $posy, $radi, 180, 270, 'F')
            . $this->getStyleCmd($style)
            . $this->getCircle($posx, $posy, $rad, 0, 360, 'S', [], 8)
            . $this->getLine($posx, $posy - $rade, $posx, $posy - $radi)
            . $this->getLine($posx, $posy + $radi, $posx, $posy + $rade)
            . $this->getLine($posx - $rade, $posy, $posx - $radi, $posy)
            . $this->getLine($posx + $radi, $posy, $posx + $rade, $posy)
            . $this->getStopTransform()
        );
    }
}
