<?php

/**
 * StyleTest.php
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
 * Style Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
class StyleTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Graph\Draw
    {
        return new \Com\Tecnick\Pdf\Graph\Draw(1, 0, 0, new \Com\Tecnick\Color\Pdf(), $this->getEncryptObject(), false);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetStyleCmd(): void
    {
        $draw = $this->getTestObject();

        $styleCmd = $draw->getStyleCmd();
        $exp1 = '';
        $this->assertEquals($exp1, $styleCmd);

        $style2 = [
            'lineWidth' => 3,
            'lineCap' => 'round',
            'lineJoin' => 'bevel',
            'miterLimit' => 11,
            'dashArray' => [5, 7],
            'dashPhase' => 0,
            'lineColor' => 'greenyellow',
            'fillColor' => '["RGB",0.250000,0.500000,0.750000]',
        ];
        $res2 = $draw->getStyleCmd($style2);
        $exp2 =
            '3.000000 w'
            . "\n"
            . '1 J'
            . "\n"
            . '2 j'
            . "\n"
            . '11.000000 M'
            . "\n"
            . '[5.000000 7.000000] 0.000000 d'
            . "\n"
            . '0.678431 1.000000 0.184314 RG'
            . "\n"
            . '0.250000 0.500000 0.750000 rg'
            . "\n";
        $this->assertEquals($exp2, $res2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testStyle(): void
    {
        $draw = $this->getTestObject();
        $style = [];
        $res1 = $draw->add($style, true);
        $exp1 =
            '1.000000 w'
            . "\n"
            . '0 J'
            . "\n"
            . '0 j'
            . "\n"
            . '10.000000 M'
            . "\n"
            . '[] 0.000000 d'
            . "\n"
            . '/CS1 CS 1.000000 SCN'
            . "\n"
            . '/CS1 cs 1.000000 scn'
            . "\n";
        $this->assertEquals($exp1, $res1);

        $style = [
            'lineWidth' => 3,
            'lineCap' => 'round',
            'lineJoin' => 'bevel',
            'miterLimit' => 11,
            'dashArray' => [5, 7],
            'dashPhase' => 1,
            'lineColor' => 'greenyellow',
            'fillColor' => '["RGB",0.250000,0.500000,0.750000]',
        ];
        $res2 = $draw->add($style, false);
        $exp2 =
            '3.000000 w'
            . "\n"
            . '1 J'
            . "\n"
            . '2 j'
            . "\n"
            . '11.000000 M'
            . "\n"
            . '[5.000000 7.000000] 1.000000 d'
            . "\n"
            . '0.678431 1.000000 0.184314 RG'
            . "\n"
            . '0.250000 0.500000 0.750000 rg'
            . "\n";
        $this->assertEquals($exp2, $res2);
        $this->assertEquals($style, $draw->getCurrentStyleArray());

        $style = [
            'lineCap' => 'round',
            'lineJoin' => 'bevel',
            'lineColor' => 'transparent',
            'fillColor' => 'cmyk(67,33,0,25)',
        ];
        $res3 = $draw->add($style, true);
        $exp3 =
            '3.000000 w'
            . "\n"
            . '1 J'
            . "\n"
            . '2 j'
            . "\n"
            . '11.000000 M'
            . "\n"
            . '[5.000000 7.000000] 1.000000 d'
            . "\n"
            . '0.670000 0.330000 0.000000 0.250000 k'
            . "\n";
        $this->assertEquals($exp3, $res3);

        $style = [
            'lineCap' => 'round',
            'lineJoin' => 'bevel',
            'lineColor' => 'transparent',
            'fillColor' => 'cmyk(67,33,0,25)',
            'dashArray' => [],
        ];
        $res4 = $draw->add($style, true);
        $exp4 =
            '3.000000 w'
            . "\n"
            . '1 J'
            . "\n"
            . '2 j'
            . "\n"
            . '11.000000 M'
            . "\n"
            . '[] 1.000000 d'
            . "\n"
            . '0.670000 0.330000 0.000000 0.250000 k'
            . "\n";
        $this->assertEquals($exp4, $res4);

        $style = [
            'lineWidth' => 7.123,
        ];
        $res5 = $draw->add($style, false);
        $exp5 = '7.123000 w' . "\n";
        $this->assertEquals($exp5, $res5);

        $res = $draw->pop();
        $this->assertEquals($exp5, $res);

        $res = $draw->pop();
        $this->assertEquals($exp4, $res);

        $res = $draw->pop();
        $this->assertEquals($exp3, $res);

        $res = $draw->pop();
        $this->assertEquals($exp2, $res);

        $res = $draw->pop();
        $this->assertEquals($exp1, $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testStyleEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->pop();
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testSaveRestoreStyle(): void
    {
        $draw = $this->getTestObject();
        $draw->add([
            'lineWidth' => 1,
        ], false);
        $draw->add([
            'lineWidth' => 2,
        ], false);
        $draw->add([
            'lineWidth' => 3,
        ], false);
        $draw->saveStyleStatus();
        $draw->add([
            'lineWidth' => 4,
        ], false);
        $draw->add([
            'lineWidth' => 5,
        ], false);
        $draw->add([
            'lineWidth' => 6,
        ], false);
        $this->assertEquals(
            [
                'lineWidth' => 6,
            ],
            $draw->getCurrentStyleArray(),
        );
        $draw->restoreStyleStatus();
        $this->assertEquals(
            [
                'lineWidth' => 3,
            ],
            $draw->getCurrentStyleArray(),
        );
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testStyleItem(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('butt', $draw->getCurrentStyleItem('lineCap'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testStyleItemEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Graph\Exception::class);
        $draw = $this->getTestObject();
        $draw->getCurrentStyleItem('wrongField');
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetLastStyleProperty(): void
    {
        $draw = $this->getTestObject();
        $draw->add([
            'lineWidth' => 1,
        ], false);
        $draw->add([
            'lineWidth' => 2,
        ], false);
        $draw->add([
            'lineWidth' => 3,
        ], false);
        $this->assertEquals(3, $draw->getLastStyleProperty('lineWidth', 0));
        $draw->add([
            'lineWidth' => 4,
        ], false);
        $this->assertEquals(4, $draw->getLastStyleProperty('lineWidth', 0));
        $this->assertEquals(7, $draw->getLastStyleProperty('unknown', 7));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetPathPaintOp(): void
    {
        $draw = $this->getTestObject();
        $res = $draw->getPathPaintOp('', '');
        $this->assertEquals('', $res);

        $res = $draw->getPathPaintOp('');
        $this->assertEquals('S' . "\n", $res);

        $res = $draw->getPathPaintOp('', 'df');
        $this->assertEquals('b' . "\n", $res);

        $res = $draw->getPathPaintOp('CEO');
        $this->assertEquals('W* n' . "\n", $res);

        $res = $draw->getPathPaintOp('F*D');
        $this->assertEquals('B*' . "\n", $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testIsFillingMode(): void
    {
        $draw = $this->getTestObject();
        $this->assertTrue($draw->isFillingMode('f'));
        $this->assertTrue($draw->isFillingMode('f*'));
        $this->assertTrue($draw->isFillingMode('B'));
        $this->assertTrue($draw->isFillingMode('B*'));
        $this->assertTrue($draw->isFillingMode('b'));
        $this->assertTrue($draw->isFillingMode('b*'));
        $this->assertFalse($draw->isFillingMode('S'));
        $this->assertFalse($draw->isFillingMode('s'));
        $this->assertFalse($draw->isFillingMode('n'));
        $this->assertFalse($draw->isFillingMode(''));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testIsStrokingMode(): void
    {
        $draw = $this->getTestObject();
        $this->assertTrue($draw->isStrokingMode('S'));
        $this->assertTrue($draw->isStrokingMode('s'));
        $this->assertTrue($draw->isStrokingMode('B'));
        $this->assertTrue($draw->isStrokingMode('B*'));
        $this->assertTrue($draw->isStrokingMode('b'));
        $this->assertTrue($draw->isStrokingMode('b*'));
        $this->assertFalse($draw->isStrokingMode('f'));
        $this->assertFalse($draw->isStrokingMode('f*'));
        $this->assertFalse($draw->isStrokingMode('n'));
        $this->assertFalse($draw->isStrokingMode(''));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testIsClosingMode(): void
    {
        $draw = $this->getTestObject();
        $this->assertTrue($draw->isClosingMode('s'));
        $this->assertTrue($draw->isClosingMode('b'));
        $this->assertTrue($draw->isClosingMode('b*'));
        $this->assertFalse($draw->isClosingMode('f'));
        $this->assertFalse($draw->isClosingMode('f*'));
        $this->assertFalse($draw->isClosingMode('S'));
        $this->assertFalse($draw->isClosingMode('B'));
        $this->assertFalse($draw->isClosingMode('B*'));
        $this->assertFalse($draw->isClosingMode('n'));
        $this->assertFalse($draw->isClosingMode(''));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetModeWithoutClose(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('', $draw->getModeWithoutClose(''));
        $this->assertEquals('S', $draw->getModeWithoutClose('s'));
        $this->assertEquals('B', $draw->getModeWithoutClose('b'));
        $this->assertEquals('B*', $draw->getModeWithoutClose('b*'));
        $this->assertEquals('n', $draw->getModeWithoutClose('n'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetModeWithoutFill(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('', $draw->getModeWithoutFill(''));
        $this->assertEquals('', $draw->getModeWithoutFill('f'));
        $this->assertEquals('', $draw->getModeWithoutFill('f*'));
        $this->assertEquals('S', $draw->getModeWithoutFill('B'));
        $this->assertEquals('S', $draw->getModeWithoutFill('B*'));
        $this->assertEquals('s', $draw->getModeWithoutFill('b'));
        $this->assertEquals('s', $draw->getModeWithoutFill('b*'));
        $this->assertEquals('n', $draw->getModeWithoutFill('n'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetModeWithoutStroke(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('', $draw->getModeWithoutStroke(''));
        $this->assertEquals('', $draw->getModeWithoutStroke('S'));
        $this->assertEquals('h', $draw->getModeWithoutStroke('s'));
        $this->assertEquals('f', $draw->getModeWithoutStroke('B'));
        $this->assertEquals('f*', $draw->getModeWithoutStroke('B*'));
        $this->assertEquals('h f', $draw->getModeWithoutStroke('b'));
        $this->assertEquals('h f*', $draw->getModeWithoutStroke('b*'));
        $this->assertEquals('n', $draw->getModeWithoutStroke('n'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetExtGState(): void
    {
        $draw = $this->getTestObject();
        $this->assertEquals('/GS1 gs' . "\n", $draw->getExtGState([
            'A' => 'B',
        ]));
        $this->assertEquals('/GS1 gs' . "\n", $draw->getExtGState([
            'A' => 'B',
        ]));
        $this->assertEquals('/GS2 gs' . "\n", $draw->getExtGState([
            'C' => 'D',
        ]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetExtGStatePdfa(): void
    {
        $draw = new \Com\Tecnick\Pdf\Graph\Draw(1, 0, 0, new \Com\Tecnick\Color\Pdf(), $this->getEncryptObject(), true);
        $this->assertEquals('', $draw->getExtGState([
            'A' => 'B',
        ]));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testRestoreStyleStatusNull(): void
    {
        $draw = $this->getTestObject();
        // stylemark starts as [0]; first restore pops that value
        $draw->restoreStyleStatus();
        // stylemark is now empty; second restore pops null and falls back to 0
        $draw->restoreStyleStatus();
        $this->assertEquals('butt', $draw->getCurrentStyleItem('lineCap'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetCurrentStyleArrayDefaultFallback(): void
    {
        $draw = new class(1, 0, 0, new \Com\Tecnick\Color\Pdf(), $this->getEncryptObject(), false) extends
            \Com\Tecnick\Pdf\Graph\Draw {
            public function setStyleIdForTest(int $styleid): void
            {
                $this->styleid = $styleid;
            }
        };

        $draw->setStyleIdForTest(999);
        $this->assertSame('butt', $draw->getCurrentStyleArray()['lineCap'] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testIsClippingModeInvalid(): void
    {
        $draw = $this->getTestObject();
        $this->assertFalse($draw->isClippingMode('invalid'));
    }

    /**
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */

    public function testGetLineModeWithDashArrayNoDashPhase(): void
    {
        $draw = $this->getTestObject();
        $style = [
            'dashArray' => [3, 5],
        ];
        $res = $draw->getStyleCmd($style);
        $this->assertEquals('[3.000000 5.000000] 0.000000 d' . "\n", $res);
    }
}
