<?php

/**
 * TransformTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * This file is part of tc-lib-pdf-graph software library.
 */

namespace Test;

/**
 * Transform Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * @throws \Com\Tecnick\Pdf\Graph\Exception
 */
class TransformTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Graph\Draw
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            1,
            0,
            0,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            false,
        );
        $this->assertEquals(-1, $draw->getTransformIndex());
        $this->assertEquals('q' . "\n", $draw->getStartTransform());
        return $draw;
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetStartStopTransform(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            1,
            0,
            0,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            false,
        );
        $this->assertEquals(-1, $draw->getTransformIndex());
        $this->assertEquals('q' . "\n", $draw->getStartTransform());
        $this->assertEquals(0, $draw->getTransformIndex());

        $tmx = [0.1, 1.2, 2.3, 3.4, 4.5, 5.6];
        $this->assertEquals(
            '0.100000 1.200000 2.300000 3.400000 4.500000 5.600000 cm' . "\n",
            $draw->getTransformation($tmx),
        );

        $this->bcAssertEqualsWithDelta(
            [
                0 => [
                    0 => [0.1, 1.2, 2.3, 3.4, 4.5, 5.6],
                ],
            ],
            $draw->getTransformStack(),
            0.0001,
            '',
        );

        $this->assertEquals('Q' . "\n", $draw->getStopTransform());
        $this->assertEquals(-1, $draw->getTransformIndex());
        $this->assertEquals('', $draw->getStopTransform());
        $this->assertEquals(-1, $draw->getTransformIndex());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetTransform(): void
    {
        $draw = $this->getTestObject();
        $tmx = [0.1, 1.2, 2.3, 3.4, 4.5, 5.6];
        $this->assertEquals(
            '0.100000 1.200000 2.300000 3.400000 4.500000 5.600000 cm' . "\n",
            $draw->getTransformation($tmx),
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testSetPageHeight(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            1,
            0,
            0,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            false,
        );
        $prev = $draw->setPageHeight(100);
        $this->assertEquals(0, (int) $prev);
        $this->assertEquals('q' . "\n", $draw->getStartTransform());
        $this->assertEquals('3.000000 0.000000 0.000000 5.000000 -14.000000 -356.000000 cm' . "\n", $draw->getScaling(
            3,
            5,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testSetPageWidth(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            1,
            0,
            0,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            false,
        );
        $prev = $draw->setPageWidth(100);
        $this->assertEquals(0, (int) $prev);
        $this->assertEquals('q' . "\n", $draw->getStartTransform());
        $this->assertEquals('3.000000 0.000000 0.000000 5.000000 -14.000000 44.000000 cm' . "\n", $draw->getScaling(
            3,
            5,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testSetKUnit(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            1,
            0,
            0,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            false,
        );
        $draw->setKUnit(0.75);
        $this->assertEquals('q' . "\n", $draw->getStartTransform());
        $this->assertEquals('3.000000 0.000000 0.000000 5.000000 -10.500000 33.000000 cm' . "\n", $draw->getScaling(
            3,
            5,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetScaling(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('3.000000 0.000000 0.000000 5.000000 -14.000000 44.000000 cm' . "\n", $draw->getScaling(
            3,
            5,
            7,
            11,
        ));
        $this->assertEquals('3.000000 0.000000 0.000000 3.000000 -14.000000 22.000000 cm' . "\n", $draw->getScaling(
            3,
            3,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetScalingEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->getScaling(0, 0, 7, 11);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetHorizScaling(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('3.000000 0.000000 0.000000 1.000000 -14.000000 0.000000 cm' . "\n", $draw->getHorizScaling(
            3,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetVertScaling(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('1.000000 0.000000 0.000000 5.000000 0.000000 44.000000 cm' . "\n", $draw->getVertScaling(
            5,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetPropScaling(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('3.000000 0.000000 0.000000 3.000000 -14.000000 22.000000 cm' . "\n", $draw->getPropScaling(
            3,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRotation(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('0.707107 0.707107 -0.707107 0.707107 -5.727922 -8.171573 cm' . "\n", $draw->getRotation(
            45,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetHorizMirroring(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals(
            '-1.000000 0.000000 0.000000 1.000000 14.000000 0.000000 cm' . "\n",
            $draw->getHorizMirroring(7),
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetVertMirroring(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals(
            '1.000000 0.000000 0.000000 -1.000000 0.000000 -22.000000 cm' . "\n",
            $draw->getVertMirroring(11),
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetPointMirroring(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('-1.000000 0.000000 0.000000 -1.000000 14.000000 -22.000000 cm'
        . "\n", $draw->getPointMirroring(7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetReflection(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('-1.000000 0.000000 0.000000 1.000000 14.000000 0.000000 cm'
        . "\n"
        . '0.000000 1.000000 -1.000000 0.000000 -4.000000 -18.000000 cm'
        . "\n", $draw->getReflection(45, 7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetTranslation(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('1.000000 0.000000 0.000000 1.000000 3.000000 -5.000000 cm' . "\n", $draw->getTranslation(
            3,
            5,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetHorizTranslation(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals(
            '1.000000 0.000000 0.000000 1.000000 3.000000 0.000000 cm' . "\n",
            $draw->getHorizTranslation(3),
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetVertTranslation(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals(
            '1.000000 0.000000 0.000000 1.000000 0.000000 -5.000000 cm' . "\n",
            $draw->getVertTranslation(5),
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetSkewing(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('1.000000 0.087489 0.052408 1.000000 0.576486 -0.612421 cm' . "\n", $draw->getSkewing(
            3,
            5,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetSkewingEx(): void
    {
        $draw = $this->getTestObject();
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw->getSkewing(90, -90, 7, 11);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetHorizSkewing(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('1.000000 0.000000 0.052408 1.000000 0.576486 0.000000 cm' . "\n", $draw->getHorizSkewing(
            3,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetVertSkewing(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('1.000000 0.087489 0.000000 1.000000 0.000000 -0.612421 cm' . "\n", $draw->getVertSkewing(
            5,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCtmProduct(): void
    {
        $draw = $this->getTestObject();
        $tma = [3.1, 5.2, 7.3, 11.4, 13.5, 17.6];
        $tmb = [19.1, 23.2, 29.3, 31.4, 37.5, 41.6];
        $ctm = $draw->getCtmProduct($tma, $tmb);
        $this->bcAssertEqualsWithDelta([228.570, 363.800, 320.050, 510.320, 433.430, 686.840], $ctm, 0.001);
    }
}
