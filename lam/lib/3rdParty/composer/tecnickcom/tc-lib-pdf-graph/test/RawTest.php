<?php

/**
 * RawTest.php
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

namespace Test;

/**
 * Raw Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
class RawTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Graph\Draw
    {
        return new \Com\Tecnick\Pdf\Graph\Draw(
            0.75,
            80,
            100,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            false,
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawPoint(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('2.250000 71.250000 m' . "\n", $draw->getRawPoint(3, 5));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawLine(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('2.250000 71.250000 l' . "\n", $draw->getRawLine(3, 5));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawRect(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('2.250000 71.250000 5.250000 -8.250000 re' . "\n", $draw->getRawRect(3, 5, 7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawCurve(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('2.250000 71.250000 5.250000 66.750000 9.750000 62.250000 c' . "\n", $draw->getRawCurve(
            3,
            5,
            7,
            11,
            13,
            17,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawCurveV(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('2.250000 71.250000 5.250000 66.750000 v' . "\n", $draw->getRawCurveV(3, 5, 7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawCurveY(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('2.250000 71.250000 5.250000 66.750000 y' . "\n", $draw->getRawCurveY(3, 5, 7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawEllipticalArc(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getRawEllipticalArc(0, 0, 0, 0);
        $this->assertEquals('', $res);

        $res = $draw->getRawEllipticalArc(3, 5, 7, 11);
        $this->assertEquals(
            '7.500000 71.250000 m'
            . "\n"
            . '7.500000 73.189135 7.064930 75.067534 6.271733 76.552998 c'
            . "\n"
            . '5.478536 78.038462 4.376901 79.037937 3.161653 79.374664 c'
            . "\n"
            . '1.946405 79.711391 0.693671 79.364277 -0.375000 78.394710 c'
            . "\n"
            . '-1.443671 77.425142 -2.261335 75.893857 -2.683386 74.071666 c'
            . "\n"
            . '-3.105437 72.249475 -3.105437 70.250525 -2.683386 68.428334 c'
            . "\n"
            . '-2.261335 66.606143 -1.443671 65.074858 -0.375000 64.105290 c'
            . "\n"
            . '0.693671 63.135723 1.946405 62.788609 3.161653 63.125336 c'
            . "\n"
            . '4.376901 63.462063 5.478536 64.461538 6.271733 65.947002 c'
            . "\n"
            . '7.064930 67.432466 7.500000 69.310865 7.500000 71.250000 c'
            . "\n",
            $res,
        );

        $bbox = [];
        $res = $draw->getRawEllipticalArc(3, 5, 7, 11, 0, -180, -90, true, 1.8, false, false, true, $bbox);
        $this->assertEquals(
            '2.250000 71.250000 m'
            . "\n"
            . '-3.000000 71.250000 l'
            . "\n"
            . '-3.000000 73.118591 -2.596009 74.932867 -1.854615 76.393791 c'
            . "\n"
            . '-1.113221 77.854714 -0.077525 78.877355 1.081765 79.293155 c'
            . "\n"
            . '2.241055 79.708956 3.456544 79.493745 4.527890 78.682993 c'
            . "\n"
            . '5.599235 77.872242 6.464154 76.513084 6.980087 74.829541 c'
            . "\n"
            . '7.496019 73.145998 7.632972 71.235944 7.368372 69.414202 c'
            . "\n"
            . '7.103771 67.592460 6.453000 65.964938 5.523321 64.799890 c'
            . "\n"
            . '4.593643 63.634843 3.439104 63.000000 2.250000 63.000000 c'
            . "\n"
            . '2.250000 71.250000 l'
            . "\n",
            $res,
        );
        $res = $draw->getRawEllipticalArc(3, 5, 7, 11, 0, 90, 45);
        $this->assertEquals(
            '2.250000 79.500000 m'
            . "\n"
            . '1.084822 79.500000 -0.047846 78.890447 -0.968406 77.767993 c'
            . "\n"
            . '-1.888967 76.645539 -2.546049 75.072821 -2.835467 73.299209 c'
            . "\n"
            . '-3.124884 71.525598 -3.030486 69.650067 -2.567240 67.970002 c'
            . "\n"
            . '-2.103993 66.289938 -1.297750 64.899093 -0.276348 64.018002 c'
            . "\n"
            . '0.745054 63.136911 1.924617 62.814741 3.075308 63.102576 c'
            . "\n"
            . '4.225999 63.390411 5.283605 64.272189 6.080433 65.608093 c'
            . "\n"
            . '6.877260 66.943998 7.368843 68.659482 7.477234 70.482537 c'
            . "\n"
            . '7.585626 72.305591 7.304778 74.134484 6.679223 75.679223 c'
            . "\n",
            $res,
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetVectorsAngle(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getVectorsAngle(0, 0, 0, 0);
        $this->bcAssertEqualsWithDelta(0, $res);

        $res = $draw->getVectorsAngle(0, 1, 0, 1);
        $this->bcAssertEqualsWithDelta(0, $res);

        $res = $draw->getVectorsAngle(1, 1, 2, 2);
        $this->bcAssertEqualsWithDelta(0, $res);

        $res = $draw->getVectorsAngle(1, 0, 0, 1);
        $this->bcAssertEqualsWithDelta(1.57, $res);

        $res = $draw->getVectorsAngle(0, 1, 1, 0);
        $this->bcAssertEqualsWithDelta(-1.57, $res);

        $res = $draw->getVectorsAngle(1, 0, 1, 1);
        $this->bcAssertEqualsWithDelta(0.79, $res);

        $res = $draw->getVectorsAngle(-1, -1, 1, 1);
        $this->bcAssertEqualsWithDelta(M_PI, $res);

        $res = $draw->getVectorsAngle(1, 0, -1, 0);
        $this->bcAssertEqualsWithDelta(M_PI, $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawEllipticalArcBboxPopulated(): void
    {
        $draw = $this->getTestObject();
        $bbox = [];
        // full circle: bbox should be fully populated (no sentinel values remaining)
        $draw->getRawEllipticalArc(3, 5, 7, 11, 0, 0, 360, false, 2, true, true, false, $bbox);
        $this->assertCount(4, $bbox);
        $minx = $bbox[0] ?? PHP_INT_MAX;
        $miny = $bbox[1] ?? PHP_INT_MAX;
        $maxx = $bbox[2] ?? PHP_INT_MIN;
        $maxy = $bbox[3] ?? PHP_INT_MIN;
        $this->assertLessThan(PHP_INT_MAX, $minx); // min-x was updated
        $this->assertLessThan(PHP_INT_MAX, $miny); // min-y was updated
        $this->assertGreaterThan(PHP_INT_MIN, $maxx); // max-x was updated
        $this->assertGreaterThan(PHP_INT_MIN, $maxy); // max-y was updated
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRawEllipticalArcBboxNegativeMaxValues(): void
    {
        $draw = $this->getTestObject();
        // Arc from 45° to 90° with centre at (0, 0) and vertical radius 5.
        // In this range every cy control point is negative (cy = -rdv * sin(ang)).
        // The PHP_INT_MIN initialisation ensures the max values are updated correctly
        // even when all sampled points are negative.
        $bbox = [];
        $draw->getRawEllipticalArc(0, 0, 7, 5, 0, 45, 90, false, 2, true, true, false, $bbox);
        $this->assertCount(4, $bbox);
        $maxx = $bbox[2] ?? PHP_INT_MIN;
        $maxy = $bbox[3] ?? PHP_INT_MIN;
        $this->assertGreaterThan(PHP_INT_MIN, $maxx); // max-x was updated from sentinel
        $this->assertGreaterThan(PHP_INT_MIN, $maxy); // max-y was updated from sentinel
        $this->assertLessThan(0, $maxy); // all cy values in this arc are < 0
    }

    /**
     * A zero vertical radius means "circle" (per the docblock) and must not
     * trigger a division by zero.
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetRawEllipticalArcZeroVerticalRadius(): void
    {
        $draw = $this->getTestObject();
        // rdv = 0 must be treated as a circle (rdv = rdh), not crash.
        $out = $draw->getRawEllipticalArc(3, 5, 7, 0);
        $this->assertNotSame('', $out);
        // identical to passing the horizontal radius as the vertical one
        $this->assertSame($draw->getRawEllipticalArc(3, 5, 7, 7), $out);
    }
}
