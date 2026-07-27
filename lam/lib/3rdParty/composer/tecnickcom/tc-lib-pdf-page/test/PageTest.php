<?php

/**
 * PageTest.php
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
class PageTest extends TestUtil
{
    /**
     * PDF marker for the per-page transparency group.
     */
    private const TRANSPARENCY_GROUP = '/Group << /Type /Group /S /Transparency /CS /DeviceRGB >>';

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    protected function getTestObject(): \Com\Tecnick\Pdf\Page\Page
    {
        $pdf = new \Com\Tecnick\Color\Pdf();
        $encrypt = $this->getEncryptObject();
        return new \Com\Tecnick\Pdf\Page\Page('mm', $pdf, $encrypt, false, true, false);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    protected function getPdfaTestObject(): \Com\Tecnick\Pdf\Page\Page
    {
        $pdf = new \Com\Tecnick\Color\Pdf();
        $encrypt = $this->getEncryptObject();
        return new \Com\Tecnick\Pdf\Page\Page('mm', $pdf, $encrypt, true, true, false);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetKUnit(): void
    {
        $page = $this->getTestObject();
        $this->bcAssertEqualsWithDelta(2.83464566929134, $page->getKUnit(), 0.001);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testEnableSignatureApproval(): void
    {
        $page = $this->getTestObject();
        $res = $page->enableSignatureApproval(true);
        $this->assertNotNull($res); // @phpstan-ignore method.alreadyNarrowedType
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testAdd(): void
    {
        $page = $this->getTestObject();
        // 1
        $res = $page->add();

        $box = [
            'llx' => 0,
            'lly' => 0,
            'urx' => 595.2765,
            'ury' => 841.890,
            'bci' => [
                'color' => '#000000',
                'width' => 0.353,
                'style' => 'S',
                'dash' => [
                    0 => 3,
                ],
            ],
        ];

        $exp = [
            'group' => 0,
            'rotation' => 0,
            'zoom' => 1,
            'orientation' => 'P',
            'format' => 'A4',
            'pheight' => 841.890,
            'pwidth' => 595.2765,
            'width' => 210,
            'height' => 297,
            'box' => [
                'MediaBox' => $box,
                'CropBox' => $box,
                'BleedBox' => $box,
                'TrimBox' => $box,
                'ArtBox' => $box,
            ],
            'margin' => [
                'booklet' => false,
                'PL' => 0,
                'PR' => 0,
                'PT' => 0,
                'HB' => 0,
                'CT' => 0,
                'CB' => 0,
                'FT' => 0,
                'PB' => 0,
            ],
            'ContentWidth' => 210,
            'ContentHeight' => 297,
            'HeaderHeight' => 0,
            'FooterHeight' => 0,
            'region' => [[
                'RX' => 0,
                'RY' => 0,
                'RW' => 210,
                'RH' => 297,
                'RL' => 210,
                'RR' => 0.0,
                'RT' => 297,
                'RB' => 0.0,
                'x' => 0.0,
                'y' => 0.0,
            ]],
            'currentRegion' => 0,
            'columns' => 1,
            'content' => [
                0 => '',
            ],
            'annotrefs' => [],
            'content_mark' => [
                0 => 0,
            ],
            'autobreak' => true,
        ];

        unset($res['time']);
        $exp['pid'] = 0;
        $this->bcAssertEqualsWithDelta($exp, $res);

        // 2
        $res = $page->add();
        unset($res['time']);
        $exp['pid'] = 1;
        $this->bcAssertEqualsWithDelta($exp, $res);

        // 3
        $res = $page->add([
            'group' => 1,
        ]);
        unset($res['time']);
        $exp['pid'] = 2;
        $exp['group'] = 1;
        $this->bcAssertEqualsWithDelta($exp, $res);

        // 3
        $res = $page->add([
            'columns' => 2,
        ]);
        unset($res['time']);
        $exp['pid'] = 3;
        $exp['group'] = 0;
        $exp['columns'] = 2;
        $exp['region'] = [
            0 => [
                'RX' => 0,
                'RY' => 0,
                'RW' => 105,
                'RH' => 297,
                'RL' => 105,
                'RR' => 105,
                'RT' => 297,
                'RB' => 0,
                'x' => 0,
                'y' => 0,
            ],
            1 => [
                'RX' => 105,
                'RY' => 0,
                'RW' => 105,
                'RH' => 297,
                'RL' => 210,
                'RR' => 0.0,
                'RT' => 297,
                'RB' => 0,
                'x' => 105,
                'y' => 0,
            ],
        ];
        $this->bcAssertEqualsWithDelta($exp, $res);

        $pid = $page->getPageID();
        $this->assertEquals(3, $pid);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetNextPage(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->add();
        $page->add();

        $page->setCurrentPage(2);
        $page->getNextPage();
        $page->enableAutoPageBreak(false);
        $page->getNextPage();
        $page->enableAutoPageBreak(true);
        $page->getNextPage();
        $page->getNextPage();

        $this->assertCount(6, $page->getPages());
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testDelete(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->add();
        $this->assertCount(3, $page->getPages());
        $res = $page->delete(1);
        $this->assertCount(2, $page->getPages());
        $this->assertArrayHasKey('time', $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testDeleteEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $page = $this->getTestObject();
        $page->delete(2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testPop(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->add();
        $this->assertCount(3, $page->getPages());
        $res = $page->pop();
        $this->assertCount(2, $page->getPages());
        $this->assertArrayHasKey('time', $res);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testMove(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add([
            'group' => 1,
        ]);
        $page->add([
            'group' => 2,
        ]);
        $page->add([
            'group' => 3,
        ]);

        $this->assertEquals($page->getPage(3), $page->getPage());

        $page->move(3, 0);
        $this->assertCount(4, $page->getPages());

        $res = $page->getPage(0);
        $this->assertEquals(3, $res['group']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testMoveEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $page = $this->getTestObject();
        $page->move(1, 2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetPageEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $page = $this->getTestObject();
        $page->getPage(2);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testContent(): void
    {
        $testObj = $this->getTestObject();
        $testObj->add();
        $testObj->addContent('Lorem');
        $testObj->addContent('ipsum');
        $testObj->addContentMark();
        $testObj->addContent('dolor');
        $testObj->addContent('sit');
        $testObj->addContent('amet');

        $this->assertEquals('amet', $testObj->popContent());

        $page = $testObj->getPage();
        $this->assertEquals([0, 3], $page['content_mark']);
        $this->assertEquals(['', 'Lorem', 'ipsum', 'dolor', 'sit'], $page['content']);

        $testObj->popContentToLastMark();
        $page = $testObj->getPage();
        $this->assertEquals([0], $page['content_mark']);
        $this->assertEquals(['', 'Lorem', 'ipsum'], $page['content']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPages(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->addContent('TEST1');
        $page->add();
        $page->addContent('TEST2');
        $page->add([
            'group' => 1,
            'transition' => [
                'Dur' => 2,
                'D' => 3,
                'Dm' => 'V',
                'S' => 'Glitter',
                'M' => 'O',
                'Di' => 315,
                'SS' => 1.3,
                'B' => true,
            ],
            'annotrefs' => [10, 20],
        ]);
        $page->addContent('TEST2');

        $pon = 0;
        $out = $page->getPdfPages($pon);
        $this->assertEquals(1, $page->getResourceDictObjID());
        $this->assertEquals(2, $page->getRootObjID());
        $this->assertStringContainsString('<< /Type /Pages /Kids [ 3 0 R 4 0 R 5 0 R ] /Count 3 >>', $out);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testaddAnnotRef(): void
    {
        $testObj = $this->getTestObject();
        $testObj->add();
        $testObj->addAnnotRef(13);
        $testObj->addAnnotRef(17);

        $page = $testObj->getPage();
        $this->assertEquals(13, $page['annotrefs'][0] ?? null);
        $this->assertEquals(17, $page['annotrefs'][1] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetPagePHeight(): void
    {
        $testObj = $this->getTestObject();
        $testObj->add();
        $page = $testObj->getPage();
        $this->assertEquals(841, (int) $page['pheight']);
        $testObj->setPagePHeight(123.4);
        $page = $testObj->getPage();
        $this->assertEquals(123.4, $page['pheight']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetPagePWidth(): void
    {
        $testObj = $this->getTestObject();
        $testObj->add();
        $page = $testObj->getPage();
        $this->assertEquals(595, (int) $page['pwidth']);
        $testObj->setPagePWidth(123.4);
        $page = $testObj->getPage();
        $this->assertEquals(123.4, $page['pwidth']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testAddAnnotRefDuplicate(): void
    {
        $testObj = $this->getTestObject();
        $testObj->add();
        $testObj->addAnnotRef(42);
        $testObj->addAnnotRef(42);

        $page = $testObj->getPage();
        $this->assertCount(1, $page['annotrefs']);
        $this->assertEquals(42, $page['annotrefs'][0] ?? null);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testPopContentEmptyEx(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $testObj = $this->getTestObject();
        $testObj->add();
        $testObj->popContent();
        $testObj->popContent();
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testPopContentToLastMarkEmpty(): void
    {
        $testObj = $this->getTestObject();
        $testObj->add();
        $testObj->popContent();
        $testObj->popContentToLastMark();
        $page = $testObj->getPage();
        $this->assertEmpty($page['content']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetPageTransparencyReturnsStatic(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $this->assertSame($page, $page->setPageTransparency(true, 0));
        $this->assertSame($page, $page->setPageTransparency(false, 0));
        // -1 targets the current page, mirroring the rest of the page API.
        $this->assertSame($page, $page->setPageTransparency(false, -1));
    }

    /**
     * setPageTransparency() validates the page index like every other
     * page-targeting method.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetPageTransparencyInvalidPageThrows(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Page\Exception::class);
        $page = $this->getTestObject();
        $page->add();
        $page->setPageTransparency(false, 99);
    }

    /**
     * The transparency mode is matched case-insensitively.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testSetPageTransparencyGroupModeIsCaseInsensitive(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->setPageTransparency(true, 0);
        $page->setPageTransparencyGroupMode('NEVER');

        $pon = 0;
        $out = $page->getPdfPages($pon);
        $this->assertStringNotContainsString(self::TRANSPARENCY_GROUP, $out);
    }

    /**
     * Per-page transparency flags stay aligned with the page they were set on
     * after a page is deleted and the stack is reindexed.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testSetPageTransparencyFollowsPageAfterDelete(): void
    {
        $page = $this->getTestObject();
        $page->add(); // 0
        $page->add(); // 1
        $page->add(); // 2
        // Flag the last page opaque, then remove the first page so it becomes index 1.
        $page->setPageTransparency(false, 2);
        $page->delete(0);

        $pon = 0;
        $out = $page->getPdfPages($pon);
        // Two pages remain; the opaque flag must still suppress exactly one group.
        $this->assertEquals(1, substr_count($out, self::TRANSPARENCY_GROUP));
    }

    /**
     * Deleting a page keeps the current-page pointer valid.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testDeleteKeepsCurrentPageValid(): void
    {
        $page = $this->getTestObject();
        $page->add(); // 0
        $page->add(); // 1
        $page->add(); // 2 (current)
        $page->delete(0);

        // The current-page pointer must stay within the reindexed stack so that
        // getPage() resolves instead of throwing "page ... do not exist".
        $this->assertGreaterThanOrEqual(0, $page->getPageID());
        $this->assertLessThanOrEqual(1, $page->getPageID());
        $current = $page->getPage();
        $this->assertArrayHasKey('time', $current);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testSetPageTransparencyGroupModeReturnsStatic(): void
    {
        $page = $this->getTestObject();
        $this->assertSame($page, $page->setPageTransparencyGroupMode('auto'));
    }

    /**
     * By default (auto mode) pages that are never flagged keep emitting the
     * transparency group, preserving backward-compatible output.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransparencyAutoDefault(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();

        $pon = 0;
        $out = $page->getPdfPages($pon);
        $this->assertEquals(2, substr_count($out, self::TRANSPARENCY_GROUP));
    }

    /**
     * In auto mode, a page flagged as not using transparency omits the group,
     * while unflagged pages keep emitting it.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransparencyAutoFlaggedFalse(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->setPageTransparency(false, 0);

        $pon = 0;
        $out = $page->getPdfPages($pon);
        // page 0 omits the group, page 1 (unflagged) keeps it.
        $this->assertEquals(1, substr_count($out, self::TRANSPARENCY_GROUP));
    }

    /**
     * In auto mode, a page explicitly flagged as using transparency emits the
     * group.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransparencyAutoFlaggedTrue(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->setPageTransparency(false, 0);
        $page->setPageTransparency(true, 1);

        $pon = 0;
        $out = $page->getPdfPages($pon);
        $this->assertEquals(1, substr_count($out, self::TRANSPARENCY_GROUP));
    }

    /**
     * The 'always' mode emits the group on every standard page, even those
     * flagged as not using transparency.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransparencyAlways(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->setPageTransparency(false, 0);
        $page->setPageTransparency(false, 1);
        $page->setPageTransparencyGroupMode('always');

        $pon = 0;
        $out = $page->getPdfPages($pon);
        $this->assertEquals(2, substr_count($out, self::TRANSPARENCY_GROUP));
    }

    /**
     * The 'never' mode never emits the group, even on pages flagged as using
     * transparency.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransparencyNever(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->setPageTransparency(true, 0);
        $page->setPageTransparencyGroupMode('never');

        $pon = 0;
        $out = $page->getPdfPages($pon);
        $this->assertStringNotContainsString(self::TRANSPARENCY_GROUP, $out);
    }

    /**
     * Unknown modes are treated as 'auto'.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransparencyUnknownModeIsAuto(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->setPageTransparency(false, 0);
        $page->setPageTransparencyGroupMode('bogus');

        $pon = 0;
        $out = $page->getPdfPages($pon);
        // behaves like auto: page 0 omits, page 1 keeps the group.
        $this->assertEquals(1, substr_count($out, self::TRANSPARENCY_GROUP));
    }

    /**
     * PDF/A documents never emit the transparency group, regardless of the
     * configured mode or per-page flags.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransparencyPdfa(): void
    {
        $page = $this->getPdfaTestObject();
        $page->add();
        $page->add();
        $page->setPageTransparency(true, 0);
        $page->setPageTransparencyGroupMode('always');

        $pon = 0;
        $out = $page->getPdfPages($pon);
        $this->assertStringNotContainsString(self::TRANSPARENCY_GROUP, $out);
    }

    /**
     * Numeric transition entries (/D, /Di, /SS) must be emitted as PDF numbers,
     * not as name objects, while name entries (/S) stay names.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransitionNumericEntries(): void
    {
        $page = $this->getTestObject();
        $page->add([
            'transition' => [
                'Dur' => 2,
                'D' => 3,
                'S' => 'Glitter',
                'Di' => 315,
                'SS' => 1.3,
                'B' => true,
            ],
        ]);
        $page->addContent('TEST');

        $pon = 0;
        $out = $page->getPdfPages($pon);

        $this->bcAssertStringContainsString('/D 3' . "\n", $out);
        $this->bcAssertStringContainsString('/Di 315' . "\n", $out);
        $this->bcAssertStringContainsString('/SS 1.300000' . "\n", $out);
        $this->bcAssertStringContainsString('/S /Glitter' . "\n", $out);
        $this->bcAssertStringContainsString('/B true' . "\n", $out);

        // Regression guard: numeric keys must not be rendered as name objects.
        $this->assertStringNotContainsString('/D /3', $out);
        $this->assertStringNotContainsString('/Di /315', $out);
        $this->assertStringNotContainsString('/SS /', $out);
    }

    /**
     * The /Di direction may legitimately be the name /None (Fly transitions).
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function testGetPdfPagesTransitionDiNoneIsName(): void
    {
        $page = $this->getTestObject();
        $page->add([
            'transition' => [
                'S' => 'Fly',
                'Di' => 'None',
                'SS' => 0.5,
            ],
        ]);
        $page->addContent('TEST');

        $pon = 0;
        $out = $page->getPdfPages($pon);

        $this->bcAssertStringContainsString('/S /Fly' . "\n", $out);
        $this->bcAssertStringContainsString('/Di /None' . "\n", $out);
        $this->bcAssertStringContainsString('/SS 0.500000' . "\n", $out);
    }

    /**
     * delete() reindexes the stack, so the embedded 'pid' of every remaining page
     * must keep matching its array index.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testDeleteReindexesPageIds(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->add();
        $page->delete(0);

        foreach ($page->getPages() as $idx => $data) {
            $this->assertSame($idx, $data['pid']);
        }
    }

    /**
     * move() reindexes the stack, so the embedded 'pid' of every page must keep
     * matching its array index.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testMoveReindexesPageIds(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->add();
        $page->add();
        $page->move(3, 0);

        foreach ($page->getPages() as $idx => $data) {
            $this->assertSame($idx, $data['pid']);
        }
    }

    /**
     * getNextPage() must decide auto-break based on the queried page, not the
     * current-page pointer when they differ.
     *
     * @throws \Com\Tecnick\Pdf\Page\Exception
     */
    public function testGetNextPageUsesQueriedPageAutoBreak(): void
    {
        $page = $this->getTestObject();
        $page->add();
        $page->add();
        $page->enableAutoPageBreak(true, 0);
        $page->enableAutoPageBreak(false, 1);

        // Current pointer is page 0 (auto-break on), but page 1 (auto-break off)
        // is queried: no new page must be appended.
        $page->setCurrentPage(0);
        $page->getNextRegion(1);

        $this->assertCount(2, $page->getPages());
    }
}
