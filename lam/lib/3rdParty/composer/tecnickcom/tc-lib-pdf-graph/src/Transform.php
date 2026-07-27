<?php

declare(strict_types=1);

/**
 * Transform.php
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
 * Com\Tecnick\Pdf\Graph\Transform
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * @phpstan-import-type TTMatrix from \Com\Tecnick\Pdf\Graph\Base
 */
abstract class Transform extends \Com\Tecnick\Pdf\Graph\Style
{
    /**
     * Current ID for transformation matrix.
     */
    protected int $ctmid = -1;

    /**
     * Array (stack) of Current Transformation Matrix (CTM),
     * which maps user space coordinates used within a PDF content stream into output device coordinates.
     *
     * @var array<int, array<int, TTMatrix>>
     */
    protected array $ctm = [];

    /**
     * Returns the transformation stack.
     *
     * @return array<int, array<int, TTMatrix>>
     */
    public function getTransformStack(): array
    {
        return $this->ctm;
    }

    /**
     * Returns the transformation stack index.
     */
    public function getTransformIndex(): int
    {
        return $this->ctmid;
    }

    /**
     * Starts a 2D transformation saving current graphic state.
     * This function must be called before calling transformation methods
     */
    public function getStartTransform(): string
    {
        $this->saveStyleStatus();
        $this->ctm[++$this->ctmid] = [];
        return Base::GSAVE;
    }

    /**
     * Stops a 2D transformation restoring previous graphic state.
     * This function must be called after calling transformation methods.
     */
    public function getStopTransform(): string
    {
        if (($this->ctm[$this->ctmid] ?? null) === null) {
            return '';
        }

        unset($this->ctm[$this->ctmid]);
        --$this->ctmid;
        $this->restoreStyleStatus();
        return Base::GRESTORE;
    }

    /**
     * Get the transformation matrix (CTM) PDF string
     *
     * @param TTMatrix $ctm Transformation matrix array.
     */
    public function getTransformation(array $ctm): string
    {
        $this->ctm[$this->ctmid][] = $ctm;
        return \sprintf('%F %F %F %F %F %F cm' . "\n", $ctm[0], $ctm[1], $ctm[2], $ctm[3], $ctm[4], $ctm[5]);
    }

    /**
     * Vertical and horizontal non-proportional Scaling.
     *
     * @param float $skx  Horizontal scaling factor.
     * @param float $sky  vertical scaling factor.
     * @param float $posx Abscissa of the scaling center.
     * @param float $posy Ordinate of the scaling center.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getScaling(float $skx, float $sky, float $posx, float $posy): string
    {
        if ($skx === 0.0 || $sky === 0.0) {
            throw new GraphException('Scaling factors must be different than zero');
        }

        $posy = ($this->pageh - $posy) * $this->kunit;
        $posx *= $this->kunit;
        $ctm = [$skx, 0, 0, $sky, $posx * (1 - $skx), $posy * (1 - $sky)];
        return $this->getTransformation($ctm);
    }

    /**
     * Horizontal Scaling.
     *
     * @param float $skx  Horizontal scaling factor.
     * @param float $posx Abscissa of the scaling center.
     * @param float $posy Ordinate of the scaling center.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getHorizScaling(float $skx, float $posx, float $posy): string
    {
        return $this->getScaling($skx, 1, $posx, $posy);
    }

    /**
     * Vertical Scaling.
     *
     * @param float $sky  vertical scaling factor.
     * @param float $posx Abscissa of the scaling center.
     * @param float $posy Ordinate of the scaling center.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getVertScaling(float $sky, float $posx, float $posy): string
    {
        return $this->getScaling(1, $sky, $posx, $posy);
    }

    /**
     * Vertical and horizontal proportional Scaling.
     *
     * @param float $skf  Scaling factor.
     * @param float $posx Abscissa of the scaling center.
     * @param float $posy Ordinate of the scaling center.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getPropScaling(float $skf, float $posx, float $posy): string
    {
        return $this->getScaling($skf, $skf, $posx, $posy);
    }

    /**
     * Rotation.
     *
     * @param float $angle Angle in degrees for counter-clockwise rotation.
     * @param float $posx  Abscissa of the rotation center.
     * @param float $posy  Ordinate of the rotation center.
     *
     * @return string Transformation string
     */
    public function getRotation(float $angle, float $posx, float $posy): string
    {
        $posy = ($this->pageh - $posy) * $this->kunit;
        $posx *= $this->kunit;
        $ctm = [];
        $ctm[0] = \cos($this->degToRad($angle));
        $ctm[1] = \sin($this->degToRad($angle));
        $ctm[2] = -$ctm[1];
        $ctm[3] = $ctm[0];
        $ctm[4] = $posx + ($ctm[1] * $posy) - ($ctm[0] * $posx);
        $ctm[5] = $posy - ($ctm[0] * $posy) - ($ctm[1] * $posx);
        return $this->getTransformation($ctm);
    }

    /**
     * Horizontal Mirroring.
     *
     * @param float $posx Abscissa of the mirroring line.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getHorizMirroring(float $posx): string
    {
        return $this->getScaling(-1, 1, $posx, 0);
    }

    /**
     * Vertical Mirroring.
     *
     * @param float $posy Ordinate of the mirroring line.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getVertMirroring(float $posy): string
    {
        return $this->getScaling(1, -1, 0, $posy);
    }

    /**
     * Point reflection mirroring.
     *
     * @param float $posx Abscissa of the mirroring point.
     * @param float $posy Ordinate of the mirroring point.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getPointMirroring(float $posx, float $posy): string
    {
        return $this->getScaling(-1, -1, $posx, $posy);
    }

    /**
     * Reflection against a straight line through point (x, y) with the gradient angle (angle).
     *
     * @param float $ang  Gradient angle in degrees of the straight line.
     * @param float $posx Abscissa of the mirroring point.
     * @param float $posy Ordinate of the mirroring point.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getReflection(float $ang, float $posx, float $posy): string
    {
        return $this->getScaling(-1, 1, $posx, $posy) . $this->getRotation(-2 * ($ang - 90), $posx, $posy);
    }

    /**
     * Translate graphic object horizontally and vertically.
     *
     * @param float $trx Movement to the right.
     * @param float $try Movement to the bottom.
     *
     * @return string Transformation string
     */
    public function getTranslation(float $trx, float $try): string
    {
        //calculate elements of transformation matrix
        $ctm = [1, 0, 0, 1, $trx * $this->kunit, -$try * $this->kunit];
        return $this->getTransformation($ctm);
    }

    /**
     * Translate graphic object horizontally.
     *
     * @param float $trx Movement to the right.
     *
     * @return string Transformation string
     */
    public function getHorizTranslation(float $trx): string
    {
        return $this->getTranslation($trx, 0);
    }

    /**
     * Translate graphic object vertically.
     *
     * @param float $try Movement to the bottom.
     *
     * @return string Transformation string
     */
    public function getVertTranslation(float $try): string
    {
        return $this->getTranslation(0, $try);
    }

    /**
     * Skew.
     *
     * @param float $angx Angle in degrees between -90 (skew to the left) and 90 (skew to the right)
     * @param float $angy Angle in degrees between -90 (skew to the bottom) and 90 (skew to the top)
     * @param float $posx Abscissa of the skewing center.
     * @param float $posy Ordinate of the skewing center.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getSkewing(float $angx, float $angy, float $posx, float $posy): string
    {
        if ($angx <= -90 || $angx >= 90 || $angy <= -90 || $angy >= 90) {
            throw new GraphException('Angle values must be between -90 and +90 degrees.');
        }

        $posy = ($this->pageh - $posy) * $this->kunit;
        $posx *= $this->kunit;
        $ctm = [];
        $ctm[0] = 1;
        $ctm[1] = \tan($this->degToRad($angy));
        $ctm[2] = \tan($this->degToRad($angx));
        $ctm[3] = 1;
        $ctm[4] = -$ctm[2] * $posy;
        $ctm[5] = -$ctm[1] * $posx;
        return $this->getTransformation($ctm);
    }

    /**
     * Skew horizontally.
     *
     * @param float $angx Angle in degrees between -90 (skew to the left) and 90 (skew to the right)
     * @param float $posx Abscissa of the skewing center.
     * @param float $posy Ordinate of the skewing center.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getHorizSkewing(float $angx, float $posx, float $posy): string
    {
        return $this->getSkewing($angx, 0, $posx, $posy);
    }

    /**
     * Skew vertically.
     *
     * @param float $angy Angle in degrees between -90 (skew to the bottom) and 90 (skew to the top)
     * @param float $posx Abscissa of the skewing center.
     * @param float $posy Ordinate of the skewing center.
     *
     * @return string Transformation string
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getVertSkewing(float $angy, float $posx, float $posy): string
    {
        return $this->getSkewing(0, $angy, $posx, $posy);
    }

    /**
     * Get the product of two Transformation Matrix.
     *
     * @param TTMatrix $tma First  Transformation Matrix.
     * @param TTMatrix $tmb Second Transformation Matrix.
     *
     * @return TTMatrix CTM Transformation Matrix.
     */
    public function getCtmProduct(array $tma, array $tmb): array
    {
        return [
            ($tma[0] * $tmb[0]) + ($tma[2] * $tmb[1]),
            ($tma[1] * $tmb[0]) + ($tma[3] * $tmb[1]),
            ($tma[0] * $tmb[2]) + ($tma[2] * $tmb[3]),
            ($tma[1] * $tmb[2]) + ($tma[3] * $tmb[3]),
            ($tma[0] * $tmb[4]) + ($tma[2] * $tmb[5]) + $tma[4],
            ($tma[1] * $tmb[4]) + ($tma[3] * $tmb[5]) + $tma[5],
        ];
    }

    /**
     * Converts the number in degrees to the radian equivalent.
     * We use this instead of the deg2rad() function to avoid precision problems with hhvm.
     *
     * @param float $deg Angular value in degrees.
     *
     * @return float Angle in radians
     */
    public function degToRad(float $deg): float
    {
        return ($deg * self::MPI) / 180;
    }
}
