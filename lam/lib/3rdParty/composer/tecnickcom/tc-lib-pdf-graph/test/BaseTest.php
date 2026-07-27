<?php

/**
 * BaseTest.php
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
 * Base Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
class BaseTest extends TestUtil
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
    public function testGetOutResourcesByKeysSkipInvalidEntries(): void
    {
        $draw = $this->getTestObject();

        $draw->getOverprint();
        $draw->getGradient(
            2,
            [0, 0, 1, 0],
            [
                ['color' => 'red', 'offset' => 0.0],
                ['color' => 'blue', 'offset' => 1.0],
            ],
            '',
            false,
        );

        $this->assertSame(' /ExtGState << >>' . "\n", $draw->getOutExtGStateResourcesByKeys([999]));
        $this->assertSame('', $draw->getOutGradientResourcesByKeys([999]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGradientShadersHandleMalformedStopsAndFloatExponent(): void
    {
        $draw = $this->getTestObject();
        $draw->getGradient(
            2,
            [0, 0, 1, 0],
            [
                ['color' => 'red', 'offset' => 0.0, 'opacity' => 0.5],
                ['color' => 'blue', 'offset' => 1.0, 'opacity' => 0.6, 'exponent' => 1.5],
            ],
            '',
            false,
        );

        $outFloatExponent = $draw->getOutGradientShaders($draw->getObjectNumber());
        $this->assertStringContainsString('/N 1.5', $outFloatExponent);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetOutGradientShadersSkipsMissingOpacityPattern(): void
    {
        $draw = new class(0.75, 80, 100, new \Com\Tecnick\Color\Pdf(), $this->getEncryptObject(), false) extends
            \Com\Tecnick\Pdf\Graph\Draw {
            /**
             * @param array<int, array{
             *     antialias: bool,
             *     background: ?\Com\Tecnick\Color\Model,
             *     colors: array<int, array{color: string, exponent?: float, offset?: float, opacity?: float}>,
             *     colspace: string,
             *     coords: array<float>,
             *     id: int,
             *     pattern: int,
             *     stream: string,
             *     transparency: bool,
             *     type: int,
             * }> $grads
             */
            public function setGradientsForTest(array $grads): void
            {
                $this->gradients = $grads;
            }

            protected function getOutGradientCols(array $grad, string $type): string
            {
                if ($type === 'opacity') {
                    return '';
                }

                return parent::getOutGradientCols($grad, $type);
            }
        };

        $draw->setGradientsForTest([
            1 => [
                'antialias' => false,
                'background' => null,
                'colors' => [
                    0 => ['color' => 'red', 'offset' => 0.0, 'opacity' => 0.5, 'exponent' => 1.0],
                    1 => ['color' => 'blue', 'offset' => 1.0, 'opacity' => 0.6, 'exponent' => 1.0],
                ],
                'colspace' => 'DeviceRGB',
                'coords' => [0.0, 0.0, 1.0, 0.0],
                'id' => 0,
                'pattern' => 0,
                'stream' => '',
                'transparency' => true,
                'type' => 2,
            ],
        ]);

        $out = $draw->getOutGradientShaders(10);
        $this->assertNotSame('', $out);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetOutExtGState(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getOutExtGState(10);
        $this->assertEquals('', $res);

        $draw->getOverprint();
        $draw->getAlpha();

        $res = $draw->getOutExtGState(10);
        $this->assertEquals(
            '11 0 obj'
            . "\n"
            . '<< /Type /ExtGState /OP true /op true /OPM 0.000000 >>'
            . "\n"
            . 'endobj'
            . "\n"
            . '12 0 obj'
            . "\n"
            . '<< /Type /ExtGState /CA 1.000000 /ca 1.000000 /BM /Normal /AIS false >>'
            . "\n"
            . 'endobj'
            . "\n",
            $res,
        );

        $this->assertEquals(12, $draw->getObjectNumber());

        $this->assertEquals(2, $draw->getLastExtGStateID());
    }

    /**
     * @SuppressWarnings("PHPMD.LongVariable")
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetOutExtGStateResourcesEmpty(): void
    {
        $draw = $this->getTestObject();
        $outExtGStateResources = $draw->getOutExtGStateResources();
        $this->assertEquals('', $outExtGStateResources);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetOutGradientResourcesEmpty(): void
    {
        $draw = $this->getTestObject();
        $outGradientResources = $draw->getOutGradientResources();
        $this->assertEquals('', $outGradientResources);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */

    public function testGetOutGradientShaders(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getOutGradientShaders(10);
        $this->assertEquals('', $res);

        $draw->getCoonsPatchMeshWithCoords(3, 5, 7, 11);
        $draw->getOutGradientShaders(11);
        $this->assertEquals(13, $draw->getObjectNumber());

        $res = $draw->getOutGradientResources();
        $this->assertEquals(' /Pattern << /p1 13 0 R >>' . "\n" . ' /Shading << /Sh1 12 0 R >>' . "\n", $res);

        $nres = $draw->getOutGradientResourcesByKeys([]);
        $this->assertEmpty($nres);

        $resx = $draw->getOutGradientResourcesByKeys([1]);
        $this->assertEquals(' /Pattern << /p1 13 0 R >>' . "\n" . ' /Shading << /Sh1 12 0 R >>' . "\n", $resx);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */

    public function testGetOutShaders(): void
    {
        $draw = $this->getTestObject();
        $stops = [
            [
                'color' => 'red',
                'exponent' => 1,
                'opacity' => 0.5,
            ],
            [
                'color' => 'blue',
                'exponent' => 1,
                'offset' => 0.2,
                'opacity' => 0.6,
            ],
            [
                'color' => '#98fb98',
                'exponent' => 1,
                'opacity' => 0.7,
            ],
            [
                'color' => 'rgb(64,128,191)',
                'exponent' => 1,
                'offset' => 0.8,
                'opacity' => 0.8,
            ],
            [
                'color' => 'skyblue',
                'exponent' => 1,
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

        $draw->getOverprint();
        $draw->getAlpha();
        $draw->getOutExtGState($draw->getObjectNumber());

        $draw->getOutGradientShaders($draw->getObjectNumber());
        $this->assertEquals(19, $draw->getObjectNumber());

        $res = $draw->getOutExtGStateResources();
        $this->assertEquals(' /ExtGState << /GS1 1 0 R /GS2 2 0 R /TGS1 19 0 R >>' . "\n", $res);

        $nres = $draw->getOutExtGStateResourcesByKeys([]);
        $this->assertEmpty($nres);

        $resx = $draw->getOutExtGStateResourcesByKeys([1, 2]);
        $this->assertEquals(' /ExtGState << /GS1 1 0 R /GS2 2 0 R >>' . "\n", $resx);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */

    public function testGetOutShadersRadial(): void
    {
        $draw = $this->getTestObject();
        $out = $draw->getGradient(
            3,
            [0.6, 0.5, 0.4, 0.3, 1],
            [
                [
                    'color' => 'red',
                    'exponent' => 1,
                    'offset' => 0,
                ],
                [
                    'color' => 'green',
                    'exponent' => 1,
                    'offset' => 1,
                ],
            ],
            'white',
            true,
        );

        $this->assertEquals('/Sh1 sh' . "\n", $out);

        $this->assertEquals(0, $draw->getObjectNumber());
        $draw->getOutGradientShaders($draw->getObjectNumber());
        $this->assertEquals(4, $draw->getObjectNumber());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */

    public function testGetOutGradientShadersInvalidColor(): void
    {
        $draw = $this->getTestObject();
        $stops = [
            [
                'color' => 'red',
                'exponent' => 1.0,
                'opacity' => 1.0,
            ],
            [
                'color' => 'not-a-valid-color',
                'exponent' => 1.0,
                'opacity' => 1.0,
            ],
            [
                'color' => 'blue',
                'exponent' => 1.0,
                'opacity' => 1.0,
            ],
        ];
        $draw->getGradient(2, [0, 0, 1, 0], $stops, '', false);
        $res = $draw->getOutGradientShaders($draw->getObjectNumber());
        $this->assertNotEmpty($res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetTransparencyExtGStateNamesEmpty(): void
    {
        $draw = $this->getTestObject();
        $this->assertSame([], $draw->getTransparencyExtGStateNames());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetTransparencyExtGStateNamesIgnoresOpaque(): void
    {
        $draw = $this->getTestObject();

        // Overprint carries no alpha/blend/soft-mask information.
        $draw->getOverprint();
        // A fully opaque alpha (CA = ca = 1, BM = Normal, no SMask) must be ignored.
        $draw->getAlpha();

        $this->assertSame([], $draw->getTransparencyExtGStateNames());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetTransparencyExtGStateNamesDetectsTransparency(): void
    {
        $draw = $this->getTestObject();

        // GS1: stroking alpha below 1 (CA).
        $draw->getAlpha(0.5, 'Normal', 1.0);
        // GS2: non-stroking alpha below 1 (ca).
        $draw->getAlpha(1.0, 'Normal', 0.4);
        // GS3: non-Normal blend mode.
        $draw->getAlpha(1.0, 'Multiply', 1.0);
        // GS4: an explicit soft mask.
        $draw->getExtGState(['SMask' => '/Foo']);
        // GS5: fully opaque, must be skipped.
        $draw->getAlpha();

        $this->assertSame(['GS1', 'GS2', 'GS3', 'GS4'], $draw->getTransparencyExtGStateNames());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetTransparencyExtGStateNamesUsesAssignedName(): void
    {
        $draw = new class(0.75, 80, 100, new \Com\Tecnick\Color\Pdf(), $this->getEncryptObject(), false) extends
            \Com\Tecnick\Pdf\Graph\Draw {
            /**
             * @param array<int, array{
             *     'n': int,
             *     'name': string,
             *     'parms': array<string, int|float|bool|string>,
             * }> $extgstates
             */
            public function setExtGStatesForTest(array $extgstates): void
            {
                $this->extgstates = $extgstates;
            }
        };

        $draw->setExtGStatesForTest([
            // Named transparent entry: the assigned name is returned verbatim.
            3 => ['n' => 0, 'name' => 'NAMEDGS', 'parms' => ['ca' => 0.25]],
            // Unnamed transparent entry: the name falls back to "GS" . key.
            4 => ['n' => 0, 'name' => '', 'parms' => ['CA' => 0.5]],
            // Named but opaque entry: must be skipped.
            5 => ['n' => 0, 'name' => 'OPAQUE', 'parms' => ['ca' => 1.0]],
        ]);

        $this->assertSame(['NAMEDGS', 'GS4'], $draw->getTransparencyExtGStateNames());
    }

    /**
     * Axial (type 2) and radial (type 3) shading dictionaries must terminate
     * with a real newline before "endobj", not a literal backslash-n.
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGradientShadingDictionaryUsesRealNewline(): void
    {
        $draw = $this->getTestObject();
        // axial shading
        $draw->getGradient(
            2,
            [0, 0, 1, 0],
            [
                ['color' => 'red', 'offset' => 0.0],
                ['color' => 'blue', 'offset' => 1.0],
            ],
            '',
            false,
        );
        // radial shading
        $draw->getGradient(
            3,
            [0.5, 0.5, 0.5, 0.5, 1],
            [
                ['color' => 'red', 'offset' => 0.0],
                ['color' => 'green', 'offset' => 1.0],
            ],
            '',
            false,
        );
        $out = $draw->getOutGradientShaders($draw->getObjectNumber());

        // the shading dictionary must end with ">>" followed by a real newline and "endobj"
        $this->assertStringContainsString('/Extend [true true] >>' . "\n" . 'endobj', $out);
        // it must NOT contain a literal backslash-n sequence (single-quoted: backslash + n)
        $this->assertStringNotContainsString('>>\n', $out);
    }

    /**
     * A transparent gradient registers a luminosity soft-mask ExtGState that
     * must be reported as carrying actual transparency.
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetTransparencyExtGStateNamesDetectsGradientSoftMask(): void
    {
        $draw = $this->getTestObject();
        // a gradient with opacity < 1 generates a luminosity soft mask
        $draw->getGradient(
            2,
            [0, 0, 1, 0],
            [
                ['color' => 'red', 'offset' => 0.0, 'opacity' => 0.5],
                ['color' => 'blue', 'offset' => 1.0, 'opacity' => 0.5],
            ],
            '',
            false,
        );
        // the TGS ExtGState only exists after the shaders have been generated
        $this->assertSame([], $draw->getTransparencyExtGStateNames());

        $draw->getOutGradientShaders($draw->getObjectNumber());

        $this->assertContains('TGS1', $draw->getTransparencyExtGStateNames());
    }
}
