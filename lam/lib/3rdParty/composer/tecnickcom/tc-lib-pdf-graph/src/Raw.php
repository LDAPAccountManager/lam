<?php

declare(strict_types=1);

/**
 * Raw.php
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

/**
 * Com\Tecnick\Pdf\Graph\Raw
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
abstract class Raw extends \Com\Tecnick\Pdf\Graph\Transform
{
    /**
     * Begin a new subpath by moving the current point to the specified coordinates,
     * omitting any connecting line segment.
     *
     * @param float $posx Abscissa of point.
     * @param float $posy Ordinate of point.
     *
     * @return string PDF command
     */
    public function getRawPoint(float $posx, float $posy): string
    {
        return \sprintf('%F %F m' . "\n", $posx * $this->kunit, ($this->pageh - $posy) * $this->kunit);
    }

    /**
     * Append a straight line segment from the current point to the specified one.
     * The new current point shall be the one specified.
     *
     * @param float $posx Abscissa of end point.
     * @param float $posy Ordinate of end point.
     *
     * @return string PDF command
     */
    public function getRawLine(float $posx, float $posy): string
    {
        return \sprintf('%F %F l' . "\n", $posx * $this->kunit, ($this->pageh - $posy) * $this->kunit);
    }

    /**
     * Append a rectangle to the current path as a complete subpath,
     * with lower-left corner in the specified point and dimensions width and height in user units.
     *
     * @param float  $posx   Abscissa of upper-left corner.
     * @param float  $posy   Ordinate of upper-left corner.
     * @param float  $width  Width.
     * @param float  $height Height.
     * @param string|PathPaintOp $mode Mode of rendering (or PathPaintOp enum case). @see getPathPaintOp()
     *
     * @return string PDF command
     */
    public function getRawRect(
        float $posx,
        float $posy,
        float $width,
        float $height,
        string|PathPaintOp $mode = '',
    ): string {
        return \sprintf(
            '%F %F %F %F re' . "\n" . $this->getPathPaintOp($mode, ''),
            $posx * $this->kunit,
            ($this->pageh - $posy) * $this->kunit,
            $width * $this->kunit,
            -$height * $this->kunit,
        );
    }

    /**
     * Append a cubic Bezier curve to the current path.
     * The curve shall extend from the current point to the point (posx3, posy3),
     * using (posx1, posy1) and (posx2, posy2) as the Bezier control points.
     * The new current point shall be (posx3, posy3).
     *
     * @param float $posx1 Abscissa of control point 1.
     * @param float $posy1 Ordinate of control point 1.
     * @param float $posx2 Abscissa of control point 2.
     * @param float $posy2 Ordinate of control point 2.
     * @param float $posx3 Abscissa of end point.
     * @param float $posy3 Ordinate of end point.
     *
     * @return string PDF command
     */
    public function getRawCurve(
        float $posx1,
        float $posy1,
        float $posx2,
        float $posy2,
        float $posx3,
        float $posy3,
    ): string {
        return \sprintf(
            '%F %F %F %F %F %F c' . "\n",
            $posx1 * $this->kunit,
            ($this->pageh - $posy1) * $this->kunit,
            $posx2 * $this->kunit,
            ($this->pageh - $posy2) * $this->kunit,
            $posx3 * $this->kunit,
            ($this->pageh - $posy3) * $this->kunit,
        );
    }

    /**
     * Append a cubic Bezier curve to the current path.
     * The curve shall extend from the current point to the point (posx3, posy3),
     * using the current point and (posx2, posy2) as the Bezier control points.
     * The new current point shall be (posx3, posy3).
     *
     * @param float $posx2 Abscissa of control point 2.
     * @param float $posy2 Ordinate of control point 2.
     * @param float $posx3 Abscissa of end point.
     * @param float $posy3 Ordinate of end point.
     *
     * @return string PDF command
     */
    public function getRawCurveV(float $posx2, float $posy2, float $posx3, float $posy3): string
    {
        return \sprintf(
            '%F %F %F %F v' . "\n",
            $posx2 * $this->kunit,
            ($this->pageh - $posy2) * $this->kunit,
            $posx3 * $this->kunit,
            ($this->pageh - $posy3) * $this->kunit,
        );
    }

    /**
     * Append a cubic Bezier curve to the current path.
     * The curve shall extend from the current point to the point (posx3, posy3),
     * using (posx1, posy1) and (posx3, posy3) as the Bezier control points.
     * The new current point shall be (posx3, posy3).
     *
     * @param float $posx1 Abscissa of control point 1.
     * @param float $posy1 Ordinate of control point 1.
     * @param float $posx3 Abscissa of end point.
     * @param float $posy3 Ordinate of end point.
     *
     * @return string PDF command
     */
    public function getRawCurveY(float $posx1, float $posy1, float $posx3, float $posy3): string
    {
        return \sprintf(
            '%F %F %F %F y' . "\n",
            $posx1 * $this->kunit,
            ($this->pageh - $posy1) * $this->kunit,
            $posx3 * $this->kunit,
            ($this->pageh - $posy3) * $this->kunit,
        );
    }

    /**
     * Initialize angles for the elliptical arc.
     *
     * @param float $ags Angle in degrees at which starting drawing.
     * @param float $agf Angle in degrees at which stop drawing.
     * @param float $rdv Vertical radius (if = 0 then it is a circle).
     * @param float $rdh Horizontal radius.
     * @param bool  $ccw If true draws in counter-clockwise direction.
     * @param bool  $svg If true the angles are in svg mode (already calculated).
     */
    protected function setRawEllipticalArcAngles(
        float &$ags,
        float &$agf,
        float $rdv,
        float $rdh,
        bool $ccw,
        bool $svg,
    ): void {
        $ags = $this->degToRad($ags);
        $agf = $this->degToRad($agf);
        if (!$svg) {
            $ags = \atan2(\sin($ags) / $rdv, \cos($ags) / $rdh);
            $agf = \atan2(\sin($agf) / $rdv, \cos($agf) / $rdh);
        }

        if ($ags < 0) {
            $ags += 2 * self::MPI;
        }

        if ($agf < 0) {
            $agf += 2 * self::MPI;
        }

        if ($ccw && $ags > $agf) {
            // reverse rotation
            $ags -= 2 * self::MPI;
        } elseif (!$ccw && $ags < $agf) {
            // reverse rotation
            $agf -= 2 * self::MPI;
        }
    }

    /**
     * Append an elliptical arc to the current path.
     * An ellipse is formed from n Bezier curves.
     *
     * @param float      $posxc      Abscissa of center point.
     * @param float      $posyc      Ordinate of center point.
     * @param float      $rdh        Horizontal radius.
     * @param float      $rdv        Vertical radius (if = 0 then it is a circle).
     * @param float      $posxang    Angle between the X-axis and the major axis of the ellipse.
     * @param float      $angs       Angle in degrees at which starting drawing.
     * @param float      $angf       Angle in degrees at which stop drawing.
     * @param bool       $pie        If true do not mark the border point (used to draw pie sectors).
     * @param float      $ncv        Number of curves used to draw a 90 degrees portion of ellipse.
     * @param bool       $startpoint If true output a starting point.
     * @param bool       $ccw        If true draws in counter-clockwise direction.
     * @param bool       $svg        If true the angles are in svg mode (already calculated).
     * @param array<float> $bbox     If provided, it will be filled with the bounding box coordinates
     *                               (x min, y min, x max, y max). The box is derived from the Bezier
     *                               control points, so (by the convex-hull property) it is guaranteed
     *                               to contain the arc but may be slightly larger than its tight bounds.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     */
    public function getRawEllipticalArc(
        float $posxc,
        float $posyc,
        float $rdh,
        float $rdv,
        float $posxang = 0.0,
        float $angs = 0.0,
        float $angf = 360.0,
        bool $pie = false,
        float $ncv = 2,
        bool $startpoint = true,
        bool $ccw = true,
        bool $svg = false,
        array &$bbox = [],
    ): string {
        $out = '';
        if ($rdh <= 0 || $rdv < 0) {
            return '';
        }

        if ($rdv === 0.0) {
            // a zero vertical radius means a circle: use the horizontal radius
            // (also avoids a division by zero when computing the arc angles)
            $rdv = $rdh;
        }

        $bbox = [PHP_INT_MAX, PHP_INT_MAX, PHP_INT_MIN, PHP_INT_MIN];
        if ($pie) {
            $out .= $this->getRawPoint($posxc, $posyc); // center of the arc
        }

        $posxang = $this->degToRad($posxang);
        $ags = $angs;
        $agf = $angf;
        $this->setRawEllipticalArcAngles($ags, $agf, $rdv, $rdh, $ccw, $svg);
        $total_angle = $agf - $ags;
        $ncv = \max(2, $ncv);
        $ncv *= (2 * \abs($total_angle)) / self::MPI; // total arcs to draw
        $ncv = (int) (\round($ncv) + 1);
        $arcang = $total_angle / $ncv; // angle of each arc
        $pageh = $this->pageh;
        $posx0 = $posxc; // X center point in PDF coordinates
        $posy0 = $pageh - $posyc; // Y center point in PDF coordinates
        $ang = $ags; // starting angle
        $alpha = \sin($arcang) * ((\sqrt(4 + (3 * (\tan($arcang / 2) ** 2))) - 1) / 3);
        $cos_xang = \cos($posxang);
        $sin_xang = \sin($posxang);
        // first arc point and its Bezier control point
        [$px1, $py1, $qx1, $qy1] = $this->getRawEllipticalArcPoint(
            $posx0,
            $posy0,
            $rdh,
            $rdv,
            $cos_xang,
            $sin_xang,
            $alpha,
            $ang,
        );
        if ($pie) {
            $out .= $this->getRawLine($px1, $pageh - $py1); // line from center to arc starting point
        } elseif ($startpoint) {
            $out .= $this->getRawPoint($px1, $pageh - $py1); // arc starting point
        }

        // draw arcs
        for ($idx = 1; $idx <= $ncv; ++$idx) {
            $ang = $ags + ($idx * $arcang); // starting angle
            if ($idx === $ncv) {
                $ang = $agf;
            }

            // second arc point and its Bezier control point
            [$px2, $py2, $qx2, $qy2] = $this->getRawEllipticalArcPoint(
                $posx0,
                $posy0,
                $rdh,
                $rdv,
                $cos_xang,
                $sin_xang,
                $alpha,
                $ang,
            );
            // draw arc
            $cx1 = $px1 + $qx1;
            $cy1 = $pageh - ($py1 + $qy1);
            $cx2 = $px2 - $qx2;
            $cy2 = $pageh - ($py2 - $qy2);
            $cx3 = $px2;
            $cy3 = $pageh - $py2;
            $out .= $this->getRawCurve($cx1, $cy1, $cx2, $cy2, $cx3, $cy3);
            // get bounding box coordinates
            $bbox = [
                \min($bbox[0], $cx1, $cx2, $cx3),
                \min($bbox[1], $cy1, $cy2, $cy3),
                \max($bbox[2], $cx1, $cx2, $cx3),
                \max($bbox[3], $cy1, $cy2, $cy3),
            ];
            // move to next point
            $px1 = $px2;
            $py1 = $py2;
            $qx1 = $qx2;
            $qy1 = $qy2;
        }

        if ($pie) {
            $out .= $this->getRawLine($posxc, $posyc);
            // get bounding box coordinates
            $bbox = [
                \min($bbox[0], $posxc),
                \min($bbox[1], $posyc),
                \max($bbox[2], $posxc),
                \max($bbox[3], $posyc),
            ];
        }

        return $out;
    }

    /**
     * Computes an elliptical arc point and its Bezier control-point offset.
     *
     * @param float $posx0   X center point in PDF coordinates.
     * @param float $posy0   Y center point in PDF coordinates.
     * @param float $rdh     Horizontal radius.
     * @param float $rdv     Vertical radius.
     * @param float $cosXang Cosine of the ellipse orientation angle.
     * @param float $sinXang Sine of the ellipse orientation angle.
     * @param float $alpha   Bezier control-point scaling factor.
     * @param float $ang     Arc angle in radians.
     *
     * @return array{float, float, float, float} Point and control-point offset: [px, py, qx, qy].
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     */
    private function getRawEllipticalArcPoint(
        float $posx0,
        float $posy0,
        float $rdh,
        float $rdv,
        float $cosXang,
        float $sinXang,
        float $alpha,
        float $ang,
    ): array {
        $cosAng = \cos($ang);
        $sinAng = \sin($ang);
        return [
            $posx0 + ($rdh * $cosXang * $cosAng) - ($rdv * $sinXang * $sinAng),
            $posy0 + ($rdh * $sinXang * $cosAng) + ($rdv * $cosXang * $sinAng),
            $alpha * ((-$rdh * $cosXang * $sinAng) - ($rdv * $sinXang * $cosAng)),
            $alpha * ((-$rdh * $sinXang * $sinAng) + ($rdv * $cosXang * $cosAng)),
        ];
    }

    /**
     * Returns the angle in radians between two vectors with the same origin point.
     * Angles are counted counter-clock wise.
     *
     * @param float $posx1 X coordinate of first vector point.
     * @param float $posy1 Y coordinate of first vector point.
     * @param float $posx2 X coordinate of second vector point.
     * @param float $posy2 Y coordinate of second vector point.
     *
     * @return float Angle in radians
     */
    public function getVectorsAngle(float $posx1, float $posy1, float $posx2, float $posy2): float
    {
        $dprod = ($posx1 * $posx2) + ($posy1 * $posy2);
        $dist1 = \sqrt(($posx1 * $posx1) + ($posy1 * $posy1));
        $dist2 = \sqrt(($posx2 * $posx2) + ($posy2 * $posy2));
        $distprod = $dist1 * $dist2;
        if ($distprod === 0.0) {
            return 0;
        }

        $angle = \acos(\min(1, \max(-1, $dprod / $distprod)));
        if ((($posx1 * $posy2) - ($posx2 * $posy1)) < 0) {
            $angle *= -1;
        }

        return $angle;
    }
}
