<?php

/**
 * RegionTest.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 *
 * This file is part of tc-lib-pdf-page software library.
 */

namespace Test;

use Com\Tecnick\Color\Pdf;
use Com\Tecnick\Pdf\Page\Page;

/**
 * Page Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 */
class RegionTest extends TestUtil
{
    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    protected function getTestObject(): \Com\Tecnick\Pdf\Page\Page
    {
        $pdf = new Pdf();
        $encrypt = $this->getEncryptObject();
        return new Page('mm', $pdf, $encrypt, false, false);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testRegion(): void
    {
        $page = $this->getTestObject();
        $page->add([
            'columns' => 3,
        ]);

        $res = $page->selectRegion(1);
        $exp = [
            'RX' => 70,
            'RY' => 0,
            'RW' => 70,
            'RH' => 297,
            'RL' => 140,
            'RR' => 70,
            'RT' => 297,
            'RB' => 0,
            'x' => 70,
            'y' => 0,
        ];
        $this->bcAssertEqualsWithDelta($exp, $res);

        $res = $page->getRegion();
        $this->bcAssertEqualsWithDelta($exp, $res);

        $res = $page->getNextRegion();
        $this->bcAssertEqualsWithDelta(2, $res['currentRegion']);

        $res = $page->getNextRegion();
        $this->bcAssertEqualsWithDelta(0, $res['currentRegion']);

        $page->setCurrentPage(0);
        $res = $page->getNextRegion();
        $this->bcAssertEqualsWithDelta(0, $res['currentRegion']);

        $res = $page->checkRegionBreak(1000);
        $this->bcAssertEqualsWithDelta(1, $res['currentRegion']);

        $res = $page->checkRegionBreak();
        $this->bcAssertEqualsWithDelta(1, $res['currentRegion']);

        $page->setX(13)->setY(17);
        $this->bcAssertEqualsWithDelta(13, $page->getX());
        $this->bcAssertEqualsWithDelta(17, $page->getY());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testRegionBoundaries(): void
    {
        $page = $this->getTestObject();
        $page->add([
            'columns' => 3,
        ]);

        $region = $page->getRegion();

        $res = $page->isYOutRegion(null, 1);
        $this->assertFalse($res);
        $res = $page->isYOutRegion(-1);
        $this->assertTrue($res);
        $res = $page->isYOutRegion($region['RY']);
        $this->assertFalse($res);
        $res = $page->isYOutRegion(0);
        $this->assertFalse($res);
        $res = $page->isYOutRegion(100);
        $this->assertFalse($res);
        $res = $page->isYOutRegion(297);
        $this->assertFalse($res);
        $res = $page->isYOutRegion($region['RT']);
        $this->assertFalse($res);
        $res = $page->isYOutRegion(298);
        $this->assertTrue($res);

        $page->getNextRegion();
        $region = $page->getRegion();

        $res = $page->isXOutRegion(null, 1);
        $this->assertFalse($res);
        $res = $page->isXOutRegion(69);
        $this->assertTrue($res);
        $res = $page->isXOutRegion($region['RX']);
        $this->assertFalse($res);
        $res = $page->isXOutRegion(70);
        $this->assertFalse($res);
        $res = $page->isXOutRegion(90);
        $this->assertFalse($res);
        $res = $page->isXOutRegion(140);
        $this->assertFalse($res);
        $res = $page->isXOutRegion($region['RL']);
        $this->assertFalse($res);
        $res = $page->isXOutRegion(141);
        $this->assertTrue($res);

        $pid = $page->getPageID();
        $this->assertEquals(0, $pid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetNoWriteRegionsSideRectangle(): void
    {
        $page = $this->getTestObject();
        $page->add();

        // Obstacle on the right side blocking x in [150, 210] for y in [100, 200].
        $res = $page->setNoWriteRegions([
            ['xt' => 150, 'yt' => 100, 'xb' => 150, 'yb' => 200, 'side' => 'R'],
        ], 50.0);

        $this->bcAssertEqualsWithDelta(3, $res['columns']);

        $exp = [
            [
                'RW' => 210,
                'RX' => 0,
                'RL' => 210,
                'RR' => 0,
                'RH' => 100,
                'RY' => 0,
                'RT' => 100,
                'RB' => 197,
                'x' => 0,
                'y' => 0,
            ],
            [
                'RW' => 150,
                'RX' => 0,
                'RL' => 150,
                'RR' => 60,
                'RH' => 100,
                'RY' => 100,
                'RT' => 200,
                'RB' => 97,
                'x' => 0,
                'y' => 100,
            ],
            [
                'RW' => 210,
                'RX' => 0,
                'RL' => 210,
                'RR' => 0,
                'RH' => 97,
                'RY' => 200,
                'RT' => 297,
                'RB' => 0,
                'x' => 0,
                'y' => 200,
            ],
        ];
        $this->bcAssertEqualsWithDelta($exp, $res['region']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testBuildWritableRegionsSlantedSegment(): void
    {
        $page = $this->getTestObject();
        $page->add();

        // Slanted right-side segment from (100,100) to (150,200): a staircase approximation.
        $res = $page->buildWritableRegions([
            ['xt' => 100, 'yt' => 100, 'xb' => 150, 'yb' => 200, 'side' => 'R'],
        ], 50.0);

        $exp = [
            ['RX' => 0, 'RY' => 0, 'RW' => 210, 'RH' => 100],
            ['RX' => 0, 'RY' => 100, 'RW' => 100, 'RH' => 50],
            ['RX' => 0, 'RY' => 150, 'RW' => 125, 'RH' => 50],
            ['RX' => 0, 'RY' => 200, 'RW' => 210, 'RH' => 97],
        ];
        $this->bcAssertEqualsWithDelta($exp, $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testBuildWritableRegionsFloatingBoxKeepsWidestSide(): void
    {
        $page = $this->getTestObject();
        $page->add();

        // Floating obstacle blocking x in [60, 100] for y in [100, 150]:
        // the band splits into [0,60] (w=60) and [100,210] (w=110); the widest is kept.
        $res = $page->buildWritableRegions([
            ['x' => 60, 'y' => 100, 'w' => 40, 'h' => 50],
        ], 50.0);

        $exp = [
            ['RX' => 0, 'RY' => 0, 'RW' => 210, 'RH' => 100],
            ['RX' => 100, 'RY' => 100, 'RW' => 110, 'RH' => 50],
            ['RX' => 0, 'RY' => 150, 'RW' => 210, 'RH' => 147],
        ];
        $this->bcAssertEqualsWithDelta($exp, $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testNoWriteRegionsAddGetRemove(): void
    {
        $page = $this->getTestObject();
        $page->add();

        $page->setNoWriteRegions([
            ['xt' => 150, 'yt' => 100, 'xb' => 150, 'yb' => 200, 'side' => 'R'],
        ], 50.0);
        $this->assertCount(1, $page->getNoWriteRegions());

        // The added left-side obstacle pushes the bottom region's left edge to x=60.
        $res = $page->addNoWriteRegion(['xt' => 60, 'yt' => 230, 'xb' => 60, 'yb' => 270, 'side' => 'L']);
        $this->assertCount(2, $page->getNoWriteRegions());
        $this->bcAssertEqualsWithDelta(3, $res['columns']);
        $expAdded = [
            [
                'RW' => 210,
                'RX' => 0,
                'RL' => 210,
                'RR' => 0,
                'RH' => 100,
                'RY' => 0,
                'RT' => 100,
                'RB' => 197,
                'x' => 0,
                'y' => 0,
            ],
            [
                'RW' => 150,
                'RX' => 0,
                'RL' => 150,
                'RR' => 60,
                'RH' => 100,
                'RY' => 100,
                'RT' => 200,
                'RB' => 97,
                'x' => 0,
                'y' => 100,
            ],
            [
                'RW' => 150,
                'RX' => 60,
                'RL' => 210,
                'RR' => 0,
                'RH' => 97,
                'RY' => 200,
                'RT' => 297,
                'RB' => 0,
                'x' => 60,
                'y' => 200,
            ],
        ];
        $this->bcAssertEqualsWithDelta($expAdded, $res['region']);

        // Removing the first (right) area leaves only the left-side area.
        $page->removeNoWriteRegion(0);
        $this->bcAssertEqualsWithDelta([[
            'xt' => 60,
            'yt' => 230,
            'xb' => 60,
            'yb' => 270,
            'side' => 'L',
        ]], $page->getNoWriteRegions());

        // Removing the last area leaves the page with a single full content region.
        $res = $page->removeNoWriteRegion(0);
        $this->assertCount(0, $page->getNoWriteRegions());
        $this->bcAssertEqualsWithDelta(1, $res['columns']);
        $this->bcAssertEqualsWithDelta([[
            'RW' => 210,
            'RX' => 0,
            'RL' => 210,
            'RR' => 0,
            'RH' => 297,
            'RY' => 0,
            'RT' => 297,
            'RB' => 0,
            'x' => 0,
            'y' => 0,
        ]], $res['region']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testNoWriteRegionsResetOnClonedPage(): void
    {
        $page = $this->getTestObject();
        $page->add();

        $res = $page->setNoWriteRegions([
            ['xt' => 150, 'yt' => 100, 'xb' => 150, 'yb' => 200, 'side' => 'R'],
        ], 50.0);
        $this->assertGreaterThan(1, $res['columns']);

        // Cloning the page (e.g. an automatic page break) must NOT inherit the no-write bands:
        // the new page starts with a single default full-content region.
        $cloned = $page->add();
        $this->bcAssertEqualsWithDelta(1, $cloned['columns']);
        $this->assertCount(0, $page->getNoWriteRegions($cloned['pid']));
        $this->bcAssertEqualsWithDelta([[
            'RW' => 210,
            'RX' => 0,
            'RL' => 210,
            'RR' => 0,
            'RH' => 297,
            'RY' => 0,
            'RT' => 297,
            'RB' => 0,
            'x' => 0,
            'y' => 0,
        ]], $cloned['region']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testNoWriteRegionsValidation(): void
    {
        $page = $this->getTestObject();
        $page->add();

        try {
            $page->setNoWriteRegions([], 0.0);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('band height', $e->getMessage());
        }

        try {
            $page->buildWritableRegions([], -5.0);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('band height', $e->getMessage());
        }

        try {
            $page->addNoWriteRegion(['x' => 10, 'y' => 10, 'w' => 10, 'h' => 10]);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('setNoWriteRegions', $e->getMessage());
        }

        try {
            $page->removeNoWriteRegion(7);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 7', $e->getMessage());
        }
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testDefensiveChecksOnMissingPageAfterForcedSanitizeId(): void
    {
        $pdf = new Pdf();
        $encrypt = $this->getEncryptObject();
        $page = new class('mm', $pdf, $encrypt, false, false) extends Page {
            public bool $forceSanitizePageId = false;
            public int $forcedPid = 0;

            protected function sanitizePageID(int $pid = -1): int
            {
                if ($this->forceSanitizePageId) {
                    return $this->forcedPid;
                }

                return parent::sanitizePageID($pid);
            }
        };
        $page->add();
        $page->forceSanitizePageId = true;
        $page->forcedPid = 99;

        try {
            $page->getPage(99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->setPagePHeight(10, 99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->setPagePWidth(10, 99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->selectRegion(0, 99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->getNextRegion(99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->isAutoPageBreakEnabled(99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->enableAutoPageBreak(true, 99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->setX(1, 99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }

        try {
            $page->setY(1, 99);
            $this->fail('Expected exception was not thrown.');
        } catch (\Com\Tecnick\Pdf\Page\Exception $e) {
            $this->assertStringContainsString('index 99', $e->getMessage());
        }
    }

    /**
     * An out-of-range region index must clamp to the nearest valid region instead
     * of selecting a non-existent index (which previously threw).
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSelectRegionClampsOutOfRangeIndex(): void
    {
        $page = $this->getTestObject();
        $page->add([
            'columns' => 3,
        ]);

        // Index beyond the last region (valid: 0..2) clamps to the last region.
        $res = $page->selectRegion(99);
        $this->bcAssertEqualsWithDelta(2, $page->getPage()['currentRegion']);
        $this->bcAssertEqualsWithDelta($res, $page->getRegion());

        // Negative index clamps to the first region.
        $page->selectRegion(-5);
        $this->bcAssertEqualsWithDelta(0, $page->getPage()['currentRegion']);
    }
}
