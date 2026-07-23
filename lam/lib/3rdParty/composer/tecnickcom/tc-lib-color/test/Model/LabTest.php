<?php

/**
 * LabTest.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Test\Model;

use Test\TestUtil;

/**
 * Lab Color class test
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class LabTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Color\Model\Lab
    {
        return new \Com\Tecnick\Color\Model\Lab([
            'lstar' => 52,
            'astar' => 0,
            'bstar' => -39,
            'alpha' => 0.85,
        ]);
    }

    public function testGetType(): void
    {
        $lab = $this->getTestObject();
        $this->assertEquals('LAB', $lab->getType());
    }

    public function testGetArray(): void
    {
        $lab = $this->getTestObject();
        $this->assertEquals(
            [
                'L' => 52,
                'a' => 0,
                'b' => -39,
                'A' => 0.85,
            ],
            $lab->getArray(),
        );
    }

    public function testGetPDFacArray(): void
    {
        $lab = $this->getTestObject();
        $this->bcAssertEqualsWithDelta(
            [
                0.25,
                0.50,
                0.75,
            ],
            $lab->getPDFacArray(),
            0.03,
        );
    }

    public function testGetNormalizedArray(): void
    {
        $lab = new \Com\Tecnick\Color\Model\Lab([
            'lstar' => 51.6,
            'astar' => 0.4,
            'bstar' => -38.7,
            'alpha' => 0.85,
        ]);

        $this->assertEquals(
            [
                'L' => 52.0,
                'a' => 0.0,
                'b' => -39.0,
                'A' => 0.85,
            ],
            $lab->getNormalizedArray(255),
        );
    }

    public function testGetCssColor(): void
    {
        $lab = $this->getTestObject();
        $this->assertSame('lab(52% 0 -39 / 0.85)', $lab->getCssColor());

        $opaque = new \Com\Tecnick\Color\Model\Lab([
            'lstar' => 52,
            'astar' => 0,
            'bstar' => -39,
            'alpha' => 1,
        ]);
        $this->assertSame('lab(52% 0 -39)', $opaque->getCssColor());
    }

    public function testGetJsPdfColor(): void
    {
        $lab = $this->getTestObject();
        $this->assertSame('["RGB",0.252784,0.499848,0.747328]', $lab->getJsPdfColor());
    }

    public function testGetJsPdfTransparentColor(): void
    {
        $transparent = new \Com\Tecnick\Color\Model\Lab([
            'lstar' => 52,
            'astar' => 0,
            'bstar' => -39,
            'alpha' => 0,
        ]);

        $this->assertSame('["T"]', $transparent->getJsPdfColor());
    }

    public function testGetComponentsString(): void
    {
        $lab = $this->getTestObject();
        $this->assertSame('52.000000 0.000000 -39.000000', $lab->getComponentsString());
    }

    public function testGetPdfColor(): void
    {
        $lab = $this->getTestObject();
        $this->assertSame('0.252784 0.499848 0.747328 rg' . "\n", $lab->getPdfColor());
        $this->assertSame('0.252784 0.499848 0.747328 RG' . "\n", $lab->getPdfColor(true));
    }

    public function testToGrayArray(): void
    {
        $lab = $this->getTestObject();
        $this->bcAssertEqualsWithDelta(
            [
                'gray' => 0.465,
                'alpha' => 0.85,
            ],
            $lab->toGrayArray(),
            0.02,
        );
    }

    public function testToRgbArray(): void
    {
        $lab = $this->getTestObject();
        $this->bcAssertEqualsWithDelta(
            [
                'red' => 0.25,
                'green' => 0.50,
                'blue' => 0.75,
                'alpha' => 0.85,
            ],
            $lab->toRgbArray(),
            0.03,
        );
    }

    public function testToLabArray(): void
    {
        $lab = $this->getTestObject();
        $this->bcAssertEqualsWithDelta(
            [
                'lstar' => 52,
                'astar' => 0,
                'bstar' => -39,
                'alpha' => 0.85,
            ],
            $lab->toLabArray(),
            0.01,
        );
    }

    public function testToCmykArray(): void
    {
        $lab = $this->getTestObject();
        $this->bcAssertEqualsWithDelta(
            [
                'cyan' => 0.666,
                'magenta' => 0.333,
                'yellow' => 0,
                'key' => 0.25,
                'alpha' => 0.85,
            ],
            $lab->toCmykArray(),
            0.05,
        );
    }

    public function testToHslArray(): void
    {
        $lab = $this->getTestObject();
        $this->bcAssertEqualsWithDelta(
            [
                'hue' => 0.583,
                'saturation' => 0.5,
                'lightness' => 0.5,
                'alpha' => 0.85,
            ],
            $lab->toHslArray(),
            0.05,
        );
    }

    public function testToRgbArrayLowLightnessBranch(): void
    {
        $lab = new \Com\Tecnick\Color\Model\Lab([
            'lstar' => 2,
            'astar' => 0,
            'bstar' => 0,
            'alpha' => 1,
        ]);

        $rgb = $lab->toRgbArray();
        $this->assertGreaterThan(0.0, $rgb['red'] ?? 0.0);
        $this->assertLessThan(0.03, $rgb['red'] ?? 0.0);
        $this->assertEqualsWithDelta($rgb['red'] ?? 0.0, $rgb['green'] ?? 0.0, 0.0001);
        $this->assertEqualsWithDelta($rgb['green'] ?? 0.0, $rgb['blue'] ?? 0.0, 0.0001);
        $this->assertSame(1.0, $rgb['alpha'] ?? 0.0);
    }

    public function testInvertColor(): void
    {
        $lab = $this->getTestObject();
        $lab->invertColor();
        $this->bcAssertEqualsWithDelta(
            [
                'red' => 0.75,
                'green' => 0.50,
                'blue' => 0.25,
                'alpha' => 0.85,
            ],
            $lab->toRgbArray(),
            0.05,
        );
    }

    public function testSetComponentValueForLabSpecificComponents(): void
    {
        $model = new class(['lstar' => 0.425, 'astar' => -0.1225, 'bstar' => 0.3375, 'alpha' => 1]) extends
            \Com\Tecnick\Color\Model {
            protected $type = 'TEST';
            protected float $cmp_lstar = 0.0;
            protected float $cmp_astar = 0.0;
            protected float $cmp_bstar = 0.0;

            public function getArray(): array
            {
                return [];
            }

            public function getPDFacArray(): array
            {
                return [];
            }

            public function getNormalizedArray(int $max): array
            {
                return [];
            }

            public function getCssColor(): string
            {
                return '';
            }

            public function getJsPdfColor(): string
            {
                return '';
            }

            public function getComponentsString(): string
            {
                return '';
            }

            public function getPdfColor(bool $stroke = false): string
            {
                return '';
            }

            public function toGrayArray(): array
            {
                return [];
            }

            public function toRgbArray(): array
            {
                return [];
            }

            public function toHslArray(): array
            {
                return [];
            }

            public function toCmykArray(): array
            {
                return [];
            }

            public function toLabArray(): array
            {
                return [
                    'lstar' => $this->cmp_lstar,
                    'astar' => $this->cmp_astar,
                    'bstar' => $this->cmp_bstar,
                    'alpha' => $this->cmp_alpha,
                ];
            }

            public function invertColor(): self
            {
                return $this;
            }
        };

        $this->assertEqualsWithDelta(
            [
                'lstar' => 0.425,
                'astar' => 0.0,
                'bstar' => 0.3375,
                'alpha' => 1.0,
            ],
            $model->toLabArray(),
            0.0001,
        );
    }
}
