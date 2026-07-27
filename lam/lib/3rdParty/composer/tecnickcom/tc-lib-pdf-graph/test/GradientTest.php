<?php

/**
 * GradientTest.php
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
 * Gradient Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
class GradientTest extends TestUtil
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

    public function testGetClippingRect(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('2.250000 71.250000 5.250000 -8.250000 re' . "\n" . 'W n' . "\n", $draw->getClippingRect(
            3,
            5,
            7,
            11,
        ));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetGradientTransform(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('5.250000 0.000000 0.000000 8.250000 2.250000 63.000000 cm'
        . "\n", $draw->getGradientTransform(3, 5, 7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetLinearGradient(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('q'
        . "\n"
        . '2.250000 71.250000 5.250000 -8.250000 re'
        . "\n"
        . 'W n'
        . "\n"
        . '5.250000 0.000000 0.000000 8.250000 2.250000 63.000000 cm'
        . "\n"
        . '/Sh1 sh'
        . "\n"
        . 'Q'
        . "\n", $draw->getLinearGradient(3, 5, 7, 11, 'red', 'green', [1, 2, 3, 4]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetRadialGradient(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('q'
        . "\n"
        . '2.250000 71.250000 5.250000 -8.250000 re'
        . "\n"
        . 'W n'
        . "\n"
        . '5.250000 0.000000 0.000000 8.250000 2.250000 63.000000 cm'
        . "\n"
        . '/Sh1 sh'
        . "\n"
        . 'Q'
        . "\n", $draw->getRadialGradient(3, 5, 7, 11, 'red', 'green', [0.6, 0.5, 0.4, 0.3, 1]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetGradientPDFA(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            0.75,
            80,
            100,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            true,
        );
        $this->assertEquals('', $draw->getGradient(2, [], [], '', false));

        $this->assertNull($draw->getLastGradientID());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetGradient(): void
    {
        $draw = $this->getTestObject();
        $stops = [
            [
                'color' => 'red',
                'exponent' => 1.0,
                'offset' => 0.0,
                'opacity' => 0.5,
            ],
            [
                'color' => 'blue',
                'exponent' => 1.0,
                'offset' => 0.2,
                'opacity' => 0.6,
            ],
            [
                'color' => '#98fb98',
                'exponent' => 1.0,
                'offset' => 0.47,
                'opacity' => 0.7,
            ],
            [
                'color' => 'rgb(64,128,191)',
                'exponent' => 1.0,
                'offset' => 0.8,
                'opacity' => 0.8,
            ],
            [
                'color' => 'skyblue',
                'exponent' => 1.0,
                'offset' => 1.0,
                'opacity' => 0.9,
            ],
        ];
        $this->assertEquals('/TGS1 gs' . "\n" . '/Sh1 sh' . "\n", $draw->getGradient(
            2,
            [0, 0, 1, 0],
            $stops,
            '',
            false,
        ));

        $exp = [
            1 => [
                'type' => 2,
                'coords' => [
                    0 => 0,
                    1 => 0,
                    2 => 1,
                    3 => 0,
                ],
                'antialias' => false,
                'colors' => [
                    0 => [
                        'color' => 'red',
                        'exponent' => 1,
                        'offset' => 0,
                        'opacity' => 0.5,
                    ],
                    1 => [
                        'color' => 'blue',
                        'exponent' => 1,
                        'offset' => 0.20,
                        'opacity' => 0.60,
                    ],
                    2 => [
                        'color' => '#98fb98',
                        'exponent' => 1,
                        'offset' => 0.47,
                        'opacity' => 0.70,
                    ],
                    3 => [
                        'color' => 'rgb(64,128,191)',
                        'exponent' => 1,
                        'offset' => 0.80,
                        'opacity' => 0.80,
                    ],
                    4 => [
                        'color' => 'skyblue',
                        'exponent' => 1,
                        'offset' => 1,
                        'opacity' => 0.90,
                    ],
                ],
                'transparency' => true,
                'background' => null,
                'colspace' => 'DeviceCMYK',
                'id' => 0,
                'pattern' => 0,
                'stream' => '',
            ],
        ];
        $this->bcAssertEqualsWithDelta($exp, $draw->getGradientsArray());

        $this->assertEquals(1, $draw->getLastGradientID());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetGradientInvalidStops(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->getGradient(2, [0, 0, 1, 0], [], '', false);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetAlphaStringNonstrokingConversion(): void
    {
        $draw = $this->getTestObject();
        $this->assertSame('/GS1 gs' . "\n", $draw->getAlpha(0.5, 'Normal', '0.4'));
        $this->assertSame('/GS2 gs' . "\n", $draw->getAlpha(0.5, 'Normal', 'bad'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshPDFA(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            0.75,
            80,
            100,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            true,
        );
        $this->assertEquals('', $draw->getCoonsPatchMesh(3, 5, 7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMesh(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('q'
        . "\n"
        . '2.250000 71.250000 5.250000 -8.250000 re'
        . "\n"
        . 'W n'
        . "\n"
        . '5.250000 0.000000 0.000000 8.250000 2.250000 63.000000 cm'
        . "\n"
        . '/Sh1 sh'
        . "\n"
        . 'Q'
        . "\n", $draw->getCoonsPatchMesh(3, 5, 7, 11));

        $patch_array = [
            0 => [
                'f' => 0,
                'points' => [
                    0 => 0.0,
                    1 => 0.0,
                    2 => 0.33,
                    3 => 0.0,
                    4 => 0.67,
                    5 => 0.0,
                    6 => 1.0,
                    7 => 0.0,
                    8 => 1.0,
                    9 => 0.33,
                    10 => 0.80,
                    11 => 0.67,
                    12 => 1.0,
                    13 => 1.0,
                    14 => 0.67,
                    15 => 0.800,
                    16 => 0.33,
                    17 => 1.8,
                    18 => 0.0,
                    19 => 1.0,
                    20 => 0.0,
                    21 => 0.67,
                    22 => 0.0,
                    23 => 0.33,
                ],
                'colors' => [
                    0 => [
                        'red' => 1,
                        'green' => 1,
                        'blue' => 0,
                    ],
                    1 => [
                        'red' => 0,
                        'green' => 0,
                        'blue' => 1,
                    ],
                    2 => [
                        'red' => 0,
                        'green' => 1,
                        'blue' => 0,
                    ],
                    3 => [
                        'red' => 1,
                        'green' => 0,
                        'blue' => 0,
                    ],
                ],
            ],
            1 => [
                'f' => 2,
                'points' => [
                    0 => 0.0,
                    1 => 1.33,
                    2 => 0.0,
                    3 => 1.67,
                    4 => 0.0,
                    5 => 2.0,
                    6 => 0.33,
                    7 => 2.0,
                    8 => 0.67,
                    9 => 2.0,
                    10 => 1.0,
                    11 => 2.0,
                    12 => 1.0,
                    13 => 1.67,
                    14 => 1.5,
                    15 => 1.33,
                ],
                'colors' => [
                    0 => [
                        'red' => 0,
                        'green' => 0,
                        'blue' => 0,
                    ],
                    1 => [
                        'red' => 1,
                        'green' => 0,
                        'blue' => 1,
                    ],
                ],
            ],
            2 => [
                'f' => 3,
                'points' => [
                    0 => 1.33,
                    1 => 0.80,
                    2 => 1.67,
                    3 => 1.5,
                    4 => 2.0,
                    5 => 1.0,
                    6 => 2.0,
                    7 => 1.33,
                    8 => 2.0,
                    9 => 1.67,
                    10 => 2.0,
                    11 => 2.0,
                    12 => 1.66,
                    13 => 2.0,
                    14 => 1.33,
                    15 => 2.0,
                ],
                'colors' => [
                    0 => [
                        'red' => 0,
                        'green' => 1,
                        'blue' => 1,
                    ],
                    1 => [
                        'red' => 0,
                        'green' => 0,
                        'blue' => 0,
                    ],
                ],
            ],
            3 => [
                'f' => 1,
                'points' => [
                    0 => 2.0,
                    1 => 0.67,
                    2 => 2.0,
                    3 => 0.33,
                    4 => 2.0,
                    5 => 0.0,
                    6 => 1.67,
                    7 => 0.0,
                    8 => 1.33,
                    9 => 0.0,
                    10 => 1.0,
                    11 => 0.0,
                    12 => 1.0,
                    13 => 0.33,
                    14 => 0.80,
                    15 => 0.67,
                ],
                'colors' => [
                    0 => [
                        'red' => 0,
                        'green' => 0,
                        'blue' => 0,
                    ],
                    1 => [
                        'red' => 0,
                        'green' => 0,
                        'blue' => 1,
                    ],
                ],
            ],
        ];

        $this->assertEquals('q'
        . "\n"
        . '7.500000 41.250000 142.500000 -150.000000 re'
        . "\n"
        . 'W n'
        . "\n"
        . '142.500000 0.000000 0.000000 150.000000 7.500000 -108.750000 cm'
        . "\n"
        . '/Sh2 sh'
        . "\n"
        . 'Q'
        . "\n", $draw->getCoonsPatchMesh(10, 45, 190, 200, $patch_array, 0, 2));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetColorRegistrationBar(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getColorRegistrationBar(50, 70, 40, 40);
        $this->assertEquals(
            'q'
            . "\n"
            . '37.500000 22.500000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 18.750000 cm'
            . "\n"
            . '/Sh1 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '37.500000 18.750000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 15.000000 cm'
            . "\n"
            . '/Sh2 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '37.500000 15.000000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 11.250000 cm'
            . "\n"
            . '/Sh3 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '37.500000 11.250000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 7.500000 cm'
            . "\n"
            . '/Sh4 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '37.500000 7.500000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 3.750000 cm'
            . "\n"
            . '/Sh5 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '37.500000 3.750000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 0.000000 cm'
            . "\n"
            . '/Sh6 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '37.500000 0.000000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 -3.750000 cm'
            . "\n"
            . '/Sh7 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '37.500000 -3.750000 30.000000 -3.750000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '30.000000 0.000000 0.000000 3.750000 37.500000 -7.500000 cm'
            . "\n"
            . '/Sh8 sh'
            . "\n"
            . 'Q'
            . "\n",
            $res,
        );

        $res = $draw->getColorRegistrationBar(50, 70, 40, 40, true);
        $this->assertEquals(
            'q'
            . "\n"
            . '37.500000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 37.500000 -7.500000 cm'
            . "\n"
            . '/Sh9 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '41.250000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 41.250000 -7.500000 cm'
            . "\n"
            . '/Sh10 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '45.000000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 45.000000 -7.500000 cm'
            . "\n"
            . '/Sh11 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '48.750000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 48.750000 -7.500000 cm'
            . "\n"
            . '/Sh12 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '52.500000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 52.500000 -7.500000 cm'
            . "\n"
            . '/Sh13 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '56.250000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 56.250000 -7.500000 cm'
            . "\n"
            . '/Sh14 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '60.000000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 60.000000 -7.500000 cm'
            . "\n"
            . '/Sh15 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '63.750000 22.500000 3.750000 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '3.750000 0.000000 0.000000 30.000000 63.750000 -7.500000 cm'
            . "\n"
            . '/Sh16 sh'
            . "\n"
            . 'Q'
            . "\n",
            $res,
        );

        $res = $draw->getColorRegistrationBar(50, 70, 40, 40, true, [
            [''],
            ['g(50%)'],
            ['rgb(50%,50%,50%)'],
            ['cmyk(50%,50%,50,50%)'],
            ['rgb(100%,0%,0%)'],
            ['red', 'white'],
            ['black', 'black'],
            ['g(11%)', 'g(11%)'],
            ['rgb(30%,50%,70%)', 'rgb(170%,150%,130%)'],
            ['cmyk(10%,20%,30,40%)', 'cmyk(100%,90%,80,70%)'],
            [],
        ]);

        $this->assertEquals(
            'q'
            . "\n"
            . '0.500000 g'
            . "\n"
            . '40.227273 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'f'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '0.500000 0.500000 0.500000 rg'
            . "\n"
            . '42.954545 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'f'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '0.500000 0.500000 0.500000 0.500000 k'
            . "\n"
            . '45.681818 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'f'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '1.000000 0.000000 0.000000 rg'
            . "\n"
            . '48.409091 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'f'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '51.136364 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '2.727273 0.000000 0.000000 30.000000 51.136364 -7.500000 cm'
            . "\n"
            . '/Sh17 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '0.000000 0.000000 0.000000 1.000000 k'
            . "\n"
            . '53.863636 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'f'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '0.110000 g'
            . "\n"
            . '56.590909 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'f'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '59.318182 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '2.727273 0.000000 0.000000 30.000000 59.318182 -7.500000 cm'
            . "\n"
            . '/Sh18 sh'
            . "\n"
            . 'Q'
            . "\n"
            . 'q'
            . "\n"
            . '62.045455 22.500000 2.727273 -30.000000 re'
            . "\n"
            . 'W n'
            . "\n"
            . '2.727273 0.000000 0.000000 30.000000 62.045455 -7.500000 cm'
            . "\n"
            . '/Sh19 sh'
            . "\n"
            . 'Q'
            . "\n",
            $res,
        );

        $res = $draw->getColorRegistrationBar(50, 70, 40, 40, false, []);
        $this->assertEquals('', $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCropMark(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getCropMark(3, 5, 7, 11, '');
        $this->assertEquals('', $res);

        $res = $draw->getCropMark(3, 5, 7, 11, 'TBLR');
        $this->assertEquals(
            'q'
            . "\n"
            . '2.250000 79.500000 m'
            . "\n"
            . '2.250000 73.312500 l'
            . "\n"
            . 'S'
            . "\n"
            . '2.250000 69.187500 m'
            . "\n"
            . '2.250000 63.000000 l'
            . "\n"
            . 'S'
            . "\n"
            . '-3.000000 71.250000 m'
            . "\n"
            . '0.937500 71.250000 l'
            . "\n"
            . 'S'
            . "\n"
            . '3.562500 71.250000 m'
            . "\n"
            . '7.500000 71.250000 l'
            . "\n"
            . 'S'
            . "\n"
            . 'Q'
            . "\n",
            $res,
        );

        $style = [
            'lineWidth' => 0.3,
            'lineColor' => 'black',
            'lineCap' => 'butt',
            'lineJoin' => 'miter',
        ];

        $res = $draw->getCropMark(3, 5, 7, 11, 'TBLR', $style);
        $this->assertEquals(
            'q'
            . "\n"
            . '0.225000 w'
            . "\n"
            . '0 J'
            . "\n"
            . '0 j'
            . "\n"
            . '/CS1 CS 1.000000 SCN'
            . "\n"
            . '2.250000 79.500000 m'
            . "\n"
            . '2.250000 73.312500 l'
            . "\n"
            . 'S'
            . "\n"
            . '2.250000 69.187500 m'
            . "\n"
            . '2.250000 63.000000 l'
            . "\n"
            . 'S'
            . "\n"
            . '-3.000000 71.250000 m'
            . "\n"
            . '0.937500 71.250000 l'
            . "\n"
            . 'S'
            . "\n"
            . '3.562500 71.250000 m'
            . "\n"
            . '7.500000 71.250000 l'
            . "\n"
            . 'S'
            . "\n"
            . 'Q'
            . "\n",
            $res,
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetOverprint(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getOverprint();
        $this->assertEquals('/GS1 gs' . "\n", $res);

        $res = $draw->getOverprint(false, true, 1);
        $this->assertEquals('/GS2 gs' . "\n", $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetAlpha(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getAlpha();
        $this->assertEquals('/GS1 gs' . "\n", $res);

        $res = $draw->getAlpha(0.5, '/Missing', 0.4, true);
        $this->assertEquals('/GS2 gs' . "\n", $res);
    }

    /**
     * An empty blend mode must default to Normal without emitting a PHP warning
     * (regression test for accessing $bmv[0] on an empty string).
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetAlphaEmptyBlendMode(): void
    {
        $draw = $this->getTestObject();

        // record whether any PHP warning is emitted while building the alpha
        $warned = false;
        \set_error_handler(static function () use (&$warned): bool {
            $warned = true;
            return true;
        }, E_WARNING);
        try {
            $res = $draw->getAlpha(0.5, '', 0.4);
        } finally {
            \restore_error_handler();
        }

        $this->assertFalse($warned, 'getAlpha() with an empty blend mode must not emit a warning');
        $this->assertEquals('/GS1 gs' . "\n", $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetGradientInvalidColor(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $stops = [
            [
                'color' => 'not-a-valid-color',
                'exponent' => 1.0,
                'opacity' => 1.0,
            ],
        ];
        $draw->getGradient(2, [0, 0, 1, 0], $stops, '', false);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshWithCoordsPdfa(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(
            0.75,
            80,
            100,
            new \Com\Tecnick\Color\Pdf(),
            $this->getEncryptObject(),
            true,
        );
        $this->assertEquals('', $draw->getCoonsPatchMeshWithCoords(3, 5, 7, 11));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshWithCoordsInvalidLowerLeft(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->getCoonsPatchMeshWithCoords(3, 5, 7, 11, 'not-a-valid-color');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshWithCoordsInvalidLowerRight(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->getCoonsPatchMeshWithCoords(3, 5, 7, 11, 'yellow', 'not-a-valid-color');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshWithCoordsInvalidUpperRight(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->getCoonsPatchMeshWithCoords(3, 5, 7, 11, 'yellow', 'blue', 'not-a-valid-color');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshWithCoordsInvalidUpperLeft(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->getCoonsPatchMeshWithCoords(3, 5, 7, 11, 'yellow', 'blue', 'green', 'not-a-valid-color');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCropMarkInvalidType(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getCropMark(3, 5, 7, 11, 'TX');
        $this->assertEquals(
            'q' . "\n" . '2.250000 79.500000 m' . "\n" . '2.250000 73.312500 l' . "\n" . 'S' . "\n" . 'Q' . "\n",
            $res,
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshInvalidCoordsLess(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        // coords_max < coords_min must throw
        $draw->getCoonsPatchMesh(3, 5, 7, 11, [], 1.0, 0.5);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetCoonsPatchMeshInvalidCoordsEqual(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        // coords_max == coords_min would cause division by zero
        $draw->getCoonsPatchMesh(3, 5, 7, 11, [], 1.0, 1.0);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetGradientStopsDefaultsAndAutoOffset(): void
    {
        $draw = $this->getTestObject();
        // middle stop has no explicit offset, exponent, or opacity
        $stops = [
            ['color' => 'red', 'offset' => 0.0],
            ['color' => 'green'],
            ['color' => 'blue', 'offset' => 1.0],
        ];
        $draw->getGradient(2, [0, 0, 1, 0], $stops, '', false);
        $gradients = $draw->getGradientsArray();
        $this->assertCount(1, $gradients);
        if (!isset($gradients[1]['colors'][1])) {
            $this->fail('Expected middle gradient stop to be generated');
        }

        $mid = $gradients[1]['colors'][1];
        // auto-offset for the middle stop should be 0.5
        $this->bcAssertEqualsWithDelta(0.5, $mid['offset'] ?? -1.0, 0.01);
        // default exponent and opacity should be applied
        $this->assertEquals(1, $mid['exponent'] ?? null);
        $this->assertEquals(1, $mid['opacity'] ?? null);
    }
}
