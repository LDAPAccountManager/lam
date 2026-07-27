<?php

/**
 * PathPaintOpTest.php
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

namespace Test;

use Com\Tecnick\Pdf\Graph\PathPaintOp;

/**
 * PathPaintOp enum test
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 */
class PathPaintOpTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Graph\Draw
    {
        return new \Com\Tecnick\Pdf\Graph\Draw(1, 0, 0, new \Com\Tecnick\Color\Pdf(), $this->getEncryptObject(), false);
    }

    public function testCaseBackingValues(): void
    {
        $this->assertSame('S', PathPaintOp::Stroke->value);
        $this->assertSame('f', PathPaintOp::Fill->value);
        $this->assertSame('h f', PathPaintOp::CloseFill->value);
        $this->assertSame('B*', PathPaintOp::FillStrokeEvenOdd->value);
        $this->assertSame('W n', PathPaintOp::Clip->value);
        $this->assertSame('W* n', PathPaintOp::ClipEvenOdd->value);
        $this->assertCount(14, PathPaintOp::cases());
    }

    public function testFromLooseCanonical(): void
    {
        $this->assertSame(PathPaintOp::Stroke, PathPaintOp::fromLoose('S'));
        $this->assertSame(PathPaintOp::Fill, PathPaintOp::fromLoose('f'));
        $this->assertSame(PathPaintOp::NoOp, PathPaintOp::fromLoose('n'));
    }

    public function testFromLooseResolvesAliases(): void
    {
        $this->assertSame(PathPaintOp::Stroke, PathPaintOp::fromLoose('D'));
        $this->assertSame(PathPaintOp::CloseStroke, PathPaintOp::fromLoose('h S'));
        $this->assertSame(PathPaintOp::Fill, PathPaintOp::fromLoose('F'));
        $this->assertSame(PathPaintOp::FillStroke, PathPaintOp::fromLoose('FD'));
        $this->assertSame(PathPaintOp::FillStroke, PathPaintOp::fromLoose('DF'));
        $this->assertSame(PathPaintOp::FillStrokeEvenOdd, PathPaintOp::fromLoose('F*D'));
        $this->assertSame(PathPaintOp::CloseFillStroke, PathPaintOp::fromLoose('df'));
        $this->assertSame(PathPaintOp::Clip, PathPaintOp::fromLoose('CNZ'));
        $this->assertSame(PathPaintOp::ClipEvenOdd, PathPaintOp::fromLoose('CEO'));
    }

    public function testFromLooseFallsBackToStroke(): void
    {
        $this->assertSame(PathPaintOp::Stroke, PathPaintOp::fromLoose('nonsense'));
        $this->assertSame(PathPaintOp::Stroke, PathPaintOp::fromLoose(''));
    }

    public function testFromLoosePassesThroughEnumInstance(): void
    {
        $this->assertSame(PathPaintOp::Clip, PathPaintOp::fromLoose(PathPaintOp::Clip));
    }

    public function testFromLooseRoundTrip(): void
    {
        foreach (PathPaintOp::cases() as $case) {
            $this->assertSame($case, PathPaintOp::fromLoose($case->value));
        }
    }

    /**
     * The widened getPathPaintOp() accepts a PathPaintOp enum for both $mode and
     * $default, staying consistent with the legacy string call.
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testGetPathPaintOpAcceptsEnum(): void
    {
        $draw = $this->getTestObject();
        $this->assertSame("f\n", $draw->getPathPaintOp(PathPaintOp::Fill));
        $this->assertSame("B\n", $draw->getPathPaintOp(PathPaintOp::FillStroke));
        $this->assertSame($draw->getPathPaintOp('CEO'), $draw->getPathPaintOp(PathPaintOp::ClipEvenOdd));
        // enum used as the $default when the mode is unknown/empty
        $this->assertSame("b\n", $draw->getPathPaintOp('', PathPaintOp::CloseFillStroke));
    }

    /**
     * A drawing method that funnels $mode through the string predicates accepts
     * a PathPaintOp enum too (getPolygon normalizes it once).
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function testDrawingMethodAcceptsEnum(): void
    {
        $draw = $this->getTestObject();
        $fromEnum = $draw->getRect(0, 0, 10, 10, PathPaintOp::Fill);
        $fromString = $draw->getRect(0, 0, 10, 10, 'f');
        $this->assertNotSame('', $fromEnum);
        $this->assertSame($fromString, $fromEnum);
    }
}
