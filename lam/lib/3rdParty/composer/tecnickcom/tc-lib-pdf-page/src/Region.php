<?php

declare(strict_types=1);

/**
 * Region.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 *
 * This file is part of tc-lib-pdf-page software library.
 */

namespace Com\Tecnick\Pdf\Page;

use Com\Tecnick\Pdf\Page\Exception as PageException;

/**
 * Com\Tecnick\Pdf\Page\Region
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 *
 * @phpstan-import-type RegionData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type PageData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type PageInputData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type NoWriteArea from \Com\Tecnick\Pdf\Page\Box
 *
 * A page region defines the writable area of the page.
 */
abstract class Region extends \Com\Tecnick\Pdf\Page\Settings
{
    /**
     * Stored no-write areas per page, used to (re)build the writable regions.
     * Keyed by page ID.
     *
     * @var array<int, array{band: float, areas: array<int, NoWriteArea>}>
     */
    protected array $nowrite = [];

    /**
     * Add a new page.
     *
     * @param PageInputData $data Page data:
     *     annotrefs   : array containing the annotation object references.
     *     autobreak   : true to automatically add a page when the content reaches the breaking point.
     *     box         : array containing page box boundaries and settings (@see setBox).
     *     columns     : number of equal vertical columns, if set it will automatically populate the region array.
     *     content     : string containing the raw page content.
     *     format      : page format name, or alternatively you can set width and height as below.
     *     group       : page group number.
     *     height      : page height.
     *     margin      : page margins:
     *                   - booklet : true to enable booklet mode
     *                   - PL : page left margin measured from the left page edge
     *                   - PR : page right margin measured from the right page edge
     *                   - PT : page top or header top measured distance from the top page edge
     *                   - HB : header bottom measured from the top page edge
     *                   - CT : content top measured from the top page edge
     *                   - CB : content bottom (page breaking point) measured from the top page edge
     *                   - FT : footer top measured from the bottom page edge
     *                   - PB : page bottom (footer bottom) measured from the bottom page edge
     *     num         : if set overwrites the page number.
     *     orientation : page orientation ('P' or 'L').
     *     region      : array containing the ordered list of rectangular areas where it is allowed to write,
     *                   each region is defined by:
     *                   - RX : horizontal coordinate of top-left corner
     *                   - RY : vertical coordinate of top-left corner
     *                   - RW : region width
     *                   - RH : region height
     *     rotation    : the number of degrees by which the page shall be rotated clockwise when displayed or printed.
     *     time        : UTC page modification time in seconds.
     *     transition  : array containing page transition data (@see getPageTransition).
     *     width       : page width.
     *     zoom        : preferred zoom (magnification) factor: 1.0 = 100%.
     *
     * NOTE: if $data is null, then the last page format will be cloned.
     *
     * @return PageData Page data with additional Page ID property 'pid'.
     *
     * @throws PageException
     */
    public function add(array $data = []): array
    {
        $cloneLastPage = $data === [] && $this->pmaxid >= 0;
        if ($cloneLastPage) {
            // clone last page data
            $data = $this->getPage($this->pmaxid);
            unset($data['time'], $data['content'], $data['annotrefs'], $data['pagenum']);
            if (!empty($this->nowrite[$this->pmaxid]['areas'])) {
                // No-write regions are page-specific: a cloned page (e.g. created when the text
                // flow reaches the bottom of a no-write page with automatic page break enabled)
                // starts with the default full-content region instead of inheriting the bands.
                unset($data['region'], $data['columns']);
                $this->sanitizeRegions($data);
            }
        }

        if (!$cloneLastPage) {
            $this->sanitizeNewPageData($data);
        }

        $this->sanitizeCommonPageData($data);
        $data['content_mark'] = [0];
        $data['currentRegion'] = 0;
        $data['pid'] = ++$this->pmaxid;
        $this->pid = $data['pid'];
        $data['group'] ??= 0;

        /** @var PageData $data */
        $this->page[$this->pid] = $data; // @phpstan-ignore assign.propertyType
        $group = (int) $data['group'];
        $this->group[$group] = ($this->group[$group] ?? 0) + 1;

        return $data;
    }

    /**
     * @param PageInputData $data
     *
     * @throws PageException
     */
    protected function sanitizeNewPageData(array &$data): void
    {
        $this->sanitizeGroup($data);
        $this->sanitizeRotation($data);
        $this->sanitizeZoom($data);
        $this->sanitizePageFormat($data);
        $this->sanitizeBoxData($data);
        $this->sanitizeTransitions($data);
        $this->sanitizeMargins($data);
        $this->sanitizeRegions($data);
    }

    /**
     * @param PageInputData $data
     */
    protected function sanitizeCommonPageData(array &$data): void
    {
        $this->sanitizeTime($data);
        $this->sanitizeContent($data);
        $this->sanitizeAnnotRefs($data);
        $this->sanitizePageNumber($data);
    }

    /**
     * Set the current page number (move to the specified page).
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    public function setCurrentPage(int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        $this->pid = $pid;
        return $this->getPage($this->pid);
    }

    /**
     * Returns the specified page data.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    public function getPage(int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        $page = $this->page[$pid] ?? null;
        if ($page === null) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        return $page;
    }

    /**
     * Overrides the page height and returns the current value in points.
     *
     * @param float $pheight new page height in internal points.
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return float original page height in points.
     *
     * @throws PageException
     */
    public function setPagePHeight(float $pheight, int $pid = -1): float
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        $ret = $this->page[$pid]['pheight'];
        $this->page[$pid]['pheight'] = $pheight;
        $this->page[$pid]['height'] = $pheight / $this->kunit;
        return $ret;
    }

    /**
     * Overrides the page width and returns the current value in points.
     *
     * @param float $pwidth new page width in internal points.
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return float original page width in points.
     *
     * @throws PageException
     */
    public function setPagePWidth(float $pwidth, int $pid = -1): float
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        $ret = $this->page[$pid]['pwidth'];
        $this->page[$pid]['pwidth'] = $pwidth;
        $this->page[$pid]['width'] = $pwidth / $this->kunit;
        return $ret;
    }

    /**
     * Check if the specified page ID exist.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return int Page ID.
     *
     * @throws PageException
     */
    protected function sanitizePageID(int $pid = -1): int
    {
        if ($pid < 0) {
            $pid = $this->pid;
        }

        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        return $pid;
    }

    /**
     * Select the specified page region.
     *
     * @param int $idr ID of the region.
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return RegionData Selected region data.
     *
     * @throws PageException
     */
    public function selectRegion(int $idr, int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        $columns = (int) $this->page[$pid]['columns'];
        // 'columns' is the region count, so the last valid index is columns - 1.
        $this->page[$pid]['currentRegion'] = \min(\max(0, $idr), \max(0, $columns - 1));
        return $this->getRegion($pid);
    }

    /**
     * Returns the current region data.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return RegionData Region.
     *
     * @throws PageException
     */
    public function getRegion(int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        $page = $this->getPage($pid);
        $currentRegion = $page['currentRegion'];
        $regions = $page['region'];
        if (!\array_key_exists($currentRegion, $regions)) {
            throw new PageException('The current region with index ' . $currentRegion . ' do not exist.');
        }

        return $regions[$currentRegion];
    }

    /**
     * Returns the next page data.
     * Creates a new page if required and page break is enabled.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    public function getNextPage(int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        if ($pid < $this->pmaxid) {
            $this->pid = ++$pid;
            return $this->getPage($this->pid);
        }

        if (!$this->isAutoPageBreakEnabled($pid)) {
            return $this->setCurrentPage($pid);
        }

        return $this->add();
    }

    /**
     * Returns the page data with the next selected region.
     * If there are no more regions available, then the first region on the next page is selected.
     * A new page is added if required.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return PageData Current page data.
     *
     * @throws PageException
     */
    public function getNextRegion(int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        $nextid = (int) $this->page[$pid]['currentRegion'] + 1;
        if (\array_key_exists($nextid, $this->page[$pid]['region'])) {
            $this->page[$pid]['currentRegion'] = $nextid;
            return $this->getPage($pid);
        }

        return $this->getNextPage($pid);
    }

    /**
     * Move to the next page region if required.
     *
     * @param float  $height Height of the block to add.
     * @param ?float $ypos   Starting Y position or NULL for current position.
     * @param int    $pid    Page index. Omit or set it to -1 for the current page ID.
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    public function checkRegionBreak(float $height = 0.0, ?float $ypos = null, int $pid = -1): array
    {
        if ($this->isYOutRegion($ypos, $height, $pid)) {
            return $this->getNextRegion($pid);
        }

        return $this->getPage($pid);
    }

    /**
     * Return the auto-page-break status.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return bool True if the auto page break is enabled, false otherwise.
     *
     * @throws PageException
     */
    public function isAutoPageBreakEnabled(int $pid = -1): bool
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        return $this->page[$pid]['autobreak'];
    }

    /**
     * Enable or disable automatic page break.
     *
     * @param bool $isenabled Set this to true to enable automatic page break.
     * @param int  $pid       page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function enableAutoPageBreak(bool $isenabled = true, int $pid = -1): void
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        $this->page[$pid]['autobreak'] = $isenabled;
    }

    /**
     * Check if the specified position is outside the region.
     *
     * @param float  $pos Position.
     * @param 'RX'|'RY' $min ID of the min region value to check.
     * @param int    $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    private function isOutRegion(float $pos, string $min, int $pid = -1): bool
    {
        $region = $this->getRegion($pid);
        $minpos = $region['RY'];
        $maxpos = $region['RT'];

        if ($min === 'RX') {
            $minpos = $region['RX'];
            $maxpos = $region['RL'];
        }

        return $pos < ($minpos - self::EPS) || $pos > ($maxpos + self::EPS);
    }

    /**
     * Check if the specified vertical position is outside the region.
     *
     * @param ?float $posy   Y position or NULL for current position.
     * @param float  $height Additional height to add.
     * @param int    $pid    page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function isYOutRegion(?float $posy = null, float $height = 0.0, int $pid = -1): bool
    {
        if ($posy === null) {
            $posy = $this->getY();
        }

        return $this->isOutRegion($posy + $height, 'RY', $pid);
    }

    /**
     * Check if the specified horizontal position is outside the region.
     *
     * @param ?float $posx  X position or NULL for current position.
     * @param float  $width Additional width to add.
     * @param int    $pid   page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function isXOutRegion(?float $posx = null, float $width = 0.0, int $pid = -1): bool
    {
        if ($posx === null) {
            $posx = $this->getX();
        }

        return $this->isOutRegion($posx + $width, 'RX', $pid);
    }

    /**
     * Return the absolute horizontal cursor position for the current region.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function getX(int $pid = -1): float
    {
        $region = $this->getRegion($pid);
        return $region['x'];
    }

    /**
     * Return the absolute vertical cursor position for the current region.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function getY(int $pid = -1): float
    {
        $region = $this->getRegion($pid);
        return $region['y'];
    }

    /**
     * Set the absolute horizontal cursor position for the current region.
     *
     * @param float $xpos X position relative to the page coordinates.
     * @param int   $pid  page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function setX(float $xpos, int $pid = -1): static
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        $currentRegion = (int) $this->page[$pid]['currentRegion'];
        if (!\array_key_exists($currentRegion, $this->page[$pid]['region'])) {
            throw new PageException('The current region with index ' . $currentRegion . ' do not exist.');
        }

        $this->page[$pid]['region'][$currentRegion]['x'] = $xpos;
        return $this;
    }

    /**
     * Set the absolute vertical cursor position for the current region.
     *
     * @param float $ypos Y position relative to the page coordinates.
     * @param int   $pid  page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function setY(float $ypos, int $pid = -1): static
    {
        $pid = $this->sanitizePageID($pid);
        if (!\array_key_exists($pid, $this->page)) {
            throw new PageException('The page with index ' . $pid . ' do not exist.');
        }

        $currentRegion = (int) $this->page[$pid]['currentRegion'];
        if (!\array_key_exists($currentRegion, $this->page[$pid]['region'])) {
            throw new PageException('The current region with index ' . $currentRegion . ' do not exist.');
        }

        $this->page[$pid]['region'][$currentRegion]['y'] = $ypos;
        return $this;
    }

    /**
     * Build the ordered list of writable rectangular regions that avoid the given no-write areas.
     *
     * This approximates the legacy TCPDF no-write page regions: the content area is sliced into
     * horizontal bands of height $bandHeight, and for each band the widest rectangle that does
     * not overlap any no-write area is returned. Vertically-contiguous bands that share the same
     * horizontal extent are merged, so the area above and below an obstacle collapses to a single
     * tall region and only the obstacle span is finely banded. The result is a stack of
     * axis-aligned rectangles ordered top-to-bottom, suitable as the 'region' value of add() or
     * as the input of setNoWriteRegions().
     *
     * Each no-write area can be defined in one of two ways (page coordinates, user units):
     *  - side-anchored segment (legacy form), where the obstacle spans from a page side to a
     *    vertical, possibly slanted, segment:
     *      'xt','yt' : segment top point, 'xb','yb' : segment bottom point,
     *      'side'    : 'L' to block from the left page edge to the segment, 'R' for the right.
     *  - free rectangle (floating obstacle, e.g. an image):
     *      'x','y' : top-left corner, 'w' : width, 'h' : height.
     *
     * NOTE: the text engine fills one region before moving to the next, so when a floating area
     * splits a band into two writable columns only the widest one is kept; text does not wrap on
     * both sides of a floating obstacle.
     *
     * @param array<int, NoWriteArea> $noWriteAreas No-write areas (page coordinates, user units).
     * @param float                   $bandHeight   Height of each horizontal slice (must be > 0).
     * @param int                     $pid          Page index. Omit or set it to -1 for the current page.
     *
     * @return array<int, array{RX: float, RY: float, RW: float, RH: float}> Ordered writable regions.
     *
     * @throws PageException
     */
    public function buildWritableRegions(array $noWriteAreas, float $bandHeight, int $pid = -1): array
    {
        if ($bandHeight <= self::EPS) {
            throw new PageException('The band height must be a positive value.');
        }

        $page = $this->getPage($pid);

        $cx0 = $page['margin']['PL'];
        $cy0 = $page['margin']['CT'];
        $cx1 = $cx0 + $page['ContentWidth'];
        $cy1 = $cy0 + $page['ContentHeight'];

        $norm = $this->normalizeNoWriteAreas($noWriteAreas, $cx0, $cx1);

        return $this->bandWritableRegions($norm, $cx0, $cy0, $cx1, $cy1, $bandHeight);
    }

    /**
     * Set the no-write page regions, replacing any previously stored areas for the page.
     * The writable regions are (re)built and assigned to the page. Mirrors the legacy
     * setPageRegions() behaviour using rectangular regions.
     *
     * @param array<int, NoWriteArea> $noWriteAreas No-write areas (page coordinates, user units).
     * @param float                   $bandHeight   Height of each horizontal slice (must be > 0).
     * @param int                     $pid          Page index. Omit or set it to -1 for the current page.
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    public function setNoWriteRegions(array $noWriteAreas, float $bandHeight, int $pid = -1): array
    {
        if ($bandHeight <= self::EPS) {
            throw new PageException('The band height must be a positive value.');
        }

        $pid = $this->sanitizePageID($pid);
        $this->nowrite[$pid] = [
            'band' => $bandHeight,
            'areas' => \array_values($noWriteAreas),
        ];

        return $this->applyNoWriteRegions($pid);
    }

    /**
     * Append a single no-write area to the page and rebuild the writable regions.
     * setNoWriteRegions() must have been called first to set the band height.
     *
     * @param NoWriteArea $noWriteArea No-write area (page coordinates, user units).
     * @param int         $pid         Page index. Omit or set it to -1 for the current page.
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    public function addNoWriteRegion(array $noWriteArea, int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        if (!isset($this->nowrite[$pid]) || $this->nowrite[$pid]['band'] <= self::EPS) {
            throw new PageException('Call setNoWriteRegions() to set the band height before adding a region.');
        }

        $this->nowrite[$pid]['areas'][] = $noWriteArea;
        return $this->applyNoWriteRegions($pid);
    }

    /**
     * Return the no-write areas currently stored for the page.
     *
     * @param int $pid Page index. Omit or set it to -1 for the current page.
     *
     * @return array<int, NoWriteArea> No-write areas.
     *
     * @throws PageException
     */
    public function getNoWriteRegions(int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        return $this->nowrite[$pid]['areas'] ?? [];
    }

    /**
     * Remove the no-write area with the given index and rebuild the writable regions.
     *
     * @param int $index Index of the no-write area to remove.
     * @param int $pid   Page index. Omit or set it to -1 for the current page.
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    public function removeNoWriteRegion(int $index, int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        if (!isset($this->nowrite[$pid]['areas'][$index])) {
            throw new PageException('The no-write region with index ' . $index . ' do not exist.');
        }

        unset($this->nowrite[$pid]['areas'][$index]);
        $this->nowrite[$pid]['areas'] = \array_values($this->nowrite[$pid]['areas']);
        return $this->applyNoWriteRegions($pid);
    }

    /**
     * (Re)build the writable regions for the page from its stored no-write areas and assign them.
     *
     * @param int $pid Page index (already sanitized).
     *
     * @return PageData Page data.
     *
     * @throws PageException
     */
    private function applyNoWriteRegions(int $pid): array
    {
        $store = $this->nowrite[$pid] ?? ['band' => 0.0, 'areas' => []];
        $page = $this->getPage($pid);

        $raw = $this->buildWritableRegions($store['areas'], $store['band'], $pid);

        if ($raw === []) {
            // Everything is blocked: fall back to the full content region so text can still flow.
            $raw = [[
                'RX' => $page['margin']['PL'],
                'RY' => $page['margin']['CT'],
                'RW' => $page['ContentWidth'],
                'RH' => $page['ContentHeight'],
            ]];
        }

        $regions = [];
        foreach ($raw as $reg) {
            $regions[] = $this->normalizeRegionData(
                $reg,
                $page['ContentWidth'],
                $page['ContentHeight'],
                $page['width'],
                $page['height'],
                $page['margin']['PR'],
                $page['margin']['CB'],
            );
        }

        $this->page[$pid]['region'] = $regions;
        $this->page[$pid]['columns'] = \count($regions);
        $this->page[$pid]['currentRegion'] = 0;

        return $this->getPage($pid);
    }

    /**
     * Convert the public no-write area definitions into internal occupied-edge records.
     * Each record stores the occupied X interval as two linear functions of Y over [y0, y1]:
     * a left edge (lt -> lb) and a right edge (rt -> rb).
     *
     * @param array<int, NoWriteArea> $areas No-write areas.
     * @param float                   $cx0   Left edge of the content area.
     * @param float                   $cx1   Right edge of the content area.
     *
     * @return array<int, array{y0: float, y1: float, lt: float, lb: float, rt: float, rb: float}>
     */
    private function normalizeNoWriteAreas(array $areas, float $cx0, float $cx1): array
    {
        $norm = [];
        foreach ($areas as $area) {
            if (isset($area['side'])) {
                $yt = (float) ($area['yt'] ?? 0);
                $yb = (float) ($area['yb'] ?? 0);
                $xt = (float) ($area['xt'] ?? 0);
                $xb = (float) ($area['xb'] ?? 0);
                if ($yb < $yt) {
                    [$yt, $yb] = [$yb, $yt];
                    [$xt, $xb] = [$xb, $xt];
                }

                if (\strtoupper($area['side']) === 'L') {
                    // Obstacle on the left: it occupies from the left content edge to the segment.
                    $norm[] = ['y0' => $yt, 'y1' => $yb, 'lt' => $cx0, 'lb' => $cx0, 'rt' => $xt, 'rb' => $xb];
                } else {
                    // Obstacle on the right: it occupies from the segment to the right content edge.
                    $norm[] = ['y0' => $yt, 'y1' => $yb, 'lt' => $xt, 'lb' => $xb, 'rt' => $cx1, 'rb' => $cx1];
                }

                continue;
            }

            $xpos = (float) ($area['x'] ?? 0);
            $ypos = (float) ($area['y'] ?? 0);
            $width = (float) ($area['w'] ?? 0);
            $height = (float) ($area['h'] ?? 0);
            $norm[] = [
                'y0' => $ypos,
                'y1' => $ypos + $height,
                'lt' => $xpos,
                'lb' => $xpos,
                'rt' => $xpos + $width,
                'rb' => $xpos + $width,
            ];
        }

        return $norm;
    }

    /**
     * Slice the content height into bands and compute the widest writable rectangle per band,
     * merging vertically-contiguous bands that share the same horizontal extent.
     *
     * @param array<int, array{y0: float, y1: float, lt: float, lb: float, rt: float, rb: float}> $norm
     * @param float $cx0        Left edge of the content area.
     * @param float $cy0        Top edge of the content area.
     * @param float $cx1        Right edge of the content area.
     * @param float $cy1        Bottom edge of the content area.
     * @param float $bandHeight Height of each horizontal slice.
     *
     * @return array<int, array{RX: float, RY: float, RW: float, RH: float}>
     */
    private function bandWritableRegions(
        array $norm,
        float $cx0,
        float $cy0,
        float $cx1,
        float $cy1,
        float $bandHeight,
    ): array {
        $regions = [];
        /** @var array{RX: float, RY: float, RW: float, RH: float}|null $pending */
        $pending = null;
        $ypos = $cy0;
        while ($ypos < ($cy1 - self::EPS)) {
            $yTop = $ypos;
            $yBot = \min($ypos + $bandHeight, $cy1);
            $ypos = $yBot;

            $best = $this->widestBandInterval($norm, $cx0, $cx1, $yTop, $yBot);
            if ($best === null) {
                // Fully blocked band: leaves a gap and breaks the vertical merge.
                if ($pending !== null) {
                    $regions[] = $pending;
                    $pending = null;
                }

                continue;
            }

            $rposx = $best[0];
            $rwidth = $best[1] - $best[0];
            if (
                $pending !== null
                && \abs($pending['RX'] - $rposx) <= self::EPS
                && \abs($pending['RW'] - $rwidth) <= self::EPS
                && \abs($pending['RY'] + $pending['RH'] - $yTop) <= self::EPS
            ) {
                // Same horizontal extent and vertically contiguous: extend the pending region.
                $pending['RH'] += $yBot - $yTop;
                continue;
            }

            if ($pending !== null) {
                $regions[] = $pending;
            }

            $pending = ['RX' => $rposx, 'RY' => $yTop, 'RW' => $rwidth, 'RH' => $yBot - $yTop];
        }

        if ($pending !== null) {
            $regions[] = $pending;
        }

        return $regions;
    }

    /**
     * Return the widest writable X interval [left, right] for a single band, or null if the band
     * is fully blocked. The occupied span of each overlapping area is taken at its widest within
     * the band so that no text can overlap a slanted edge.
     *
     * @param array<int, array{y0: float, y1: float, lt: float, lb: float, rt: float, rb: float}> $norm
     * @param float $cx0  Left edge of the content area.
     * @param float $cx1  Right edge of the content area.
     * @param float $yTop Top of the band.
     * @param float $yBot Bottom of the band.
     *
     * @return array{0: float, 1: float}|null
     */
    private function widestBandInterval(array $norm, float $cx0, float $cx1, float $yTop, float $yBot): ?array
    {
        $free = [[$cx0, $cx1]];
        foreach ($norm as $nwa) {
            $ovlTop = \max($yTop, $nwa['y0']);
            $ovlBot = \min($yBot, $nwa['y1']);
            if ($ovlBot <= ($ovlTop + self::EPS)) {
                continue;
            }

            $occl = \max($cx0, \min(
                $this->lerpEdge($nwa['lt'], $nwa['lb'], $nwa['y0'], $nwa['y1'], $ovlTop),
                $this->lerpEdge($nwa['lt'], $nwa['lb'], $nwa['y0'], $nwa['y1'], $ovlBot),
            ));
            $occr = \min($cx1, \max(
                $this->lerpEdge($nwa['rt'], $nwa['rb'], $nwa['y0'], $nwa['y1'], $ovlTop),
                $this->lerpEdge($nwa['rt'], $nwa['rb'], $nwa['y0'], $nwa['y1'], $ovlBot),
            ));
            if ($occr <= ($occl + self::EPS)) {
                continue;
            }

            $free = $this->subtractInterval($free, $occl, $occr);
        }

        $best = null;
        foreach ($free as $interval) {
            $fwidth = $interval[1] - $interval[0];
            if ($fwidth <= self::EPS) {
                continue;
            }

            if ($best === null || $fwidth > ($best[1] - $best[0])) {
                $best = $interval;
            }
        }

        return $best;
    }

    /**
     * Linearly interpolate an edge X value at the vertical position $yy between (y0 -> vt) and
     * (y1 -> vb).
     */
    private function lerpEdge(float $vt, float $vb, float $y0, float $y1, float $yy): float
    {
        $span = $y1 - $y0;
        if ($span <= self::EPS) {
            return $vt;
        }

        return $vt + (($vb - $vt) * (($yy - $y0) / $span));
    }

    /**
     * Subtract the occupied interval [occl, occr] from a set of disjoint free intervals.
     *
     * @param array<int, array{0: float, 1: float}> $free Free intervals.
     * @param float                                 $occl Left edge of the occupied interval.
     * @param float                                 $occr Right edge of the occupied interval.
     *
     * @return array<int, array{0: float, 1: float}> Remaining free intervals.
     */
    private function subtractInterval(array $free, float $occl, float $occr): array
    {
        $out = [];
        foreach ($free as $interval) {
            $fleft = $interval[0];
            $fright = $interval[1];
            if ($occr <= ($fleft + self::EPS) || $occl >= ($fright - self::EPS)) {
                $out[] = [$fleft, $fright];
                continue;
            }

            if ($occl > ($fleft + self::EPS)) {
                $out[] = [$fleft, $occl];
            }

            if ($occr < ($fright - self::EPS)) {
                $out[] = [$occr, $fright];
            }
        }

        return $out;
    }
}
