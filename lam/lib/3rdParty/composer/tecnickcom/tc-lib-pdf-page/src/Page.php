<?php

declare(strict_types=1);

/**
 * Page.php
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

namespace Com\Tecnick\Pdf\Page;

use Com\Tecnick\Color\Pdf as Color;
use Com\Tecnick\Pdf\Encrypt\Encrypt;
use Com\Tecnick\Pdf\Encrypt\Exception as EncryptException;
use Com\Tecnick\Pdf\Page\Exception as PageException;

/**
 * Com\Tecnick\Pdf\Page\Page
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 *
 * @phpstan-import-type PageBci from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type PageBox from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type MarginData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type RegionData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type TransitionData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type PageData from \Com\Tecnick\Pdf\Page\Box
 *
 * @SuppressWarnings("PHPMD.TooManyPublicMethods")
 */
class Page extends \Com\Tecnick\Pdf\Page\Region
{
    /**
     * Initialize page data.
     *
     * @param string|Unit $unit  Unit of measure ('pt', 'mm', 'cm', 'in') or a Unit enum case.
     * @param Color   $color    Color object.
     * @param Encrypt $encrypt  Encrypt object.
     * @param bool    $pdfa     True if we are in PDF/A mode.
     * @param bool    $compress Set to false to disable stream compression.
     * @param bool    $sigapp   True if the signature approval is enabled (for incremental updates).
     *
     * @throws PageException
     */
    public function __construct(
        string|Unit $unit,
        Color $color,
        Encrypt $encrypt,
        bool $pdfa = false,
        bool $compress = true,
        bool $sigapp = false,
    ) {
        $this->kunit = $this->getUnitRatio($unit);
        $this->col = $color;
        $this->enc = $encrypt;
        $this->pdfa = $pdfa;
        $this->compress = $compress;
        $this->sigapp = $sigapp;
    }

    /**
     * Get the unit ratio.
     *
     * @return float Unit Ratio.
     */
    public function getKUnit(): float
    {
        return $this->kunit;
    }

    /**
     * Enable Signature Approval.
     *
     * @param bool $sigapp True if the signature approval is enabled (for incremental updates).
     */
    public function enableSignatureApproval(bool $sigapp): static
    {
        $this->sigapp = $sigapp;
        return $this;
    }

    /**
     * Record whether the given page actually uses transparency.
     *
     * When a page is flagged false, the per-page transparency /Group is omitted
     * for that page in getPdfPages(). This lets the document assembler flatten
     * fully-opaque pages (friendlier to conservative print interpreters) without
     * affecting pages that genuinely blend. Pages never flagged keep emitting
     * the group, preserving backward-compatible output.
     *
     * @param bool $hasTransparency True if the page uses actual transparency.
     * @param int  $pid             Page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function setPageTransparency(bool $hasTransparency = true, int $pid = -1): static
    {
        $pid = $this->sanitizePageID($pid);
        $this->pagetransparency[$pid] = $hasTransparency;
        return $this;
    }

    /**
     * Set the policy for emitting the per-page transparency /Group on standard
     * (non PDF/A) pages.
     *
     * - 'auto'   : opt-out policy (default). The group is emitted on every
     *              standard page except those explicitly flagged as opaque via
     *              setPageTransparency(false, $pid). There is no automatic
     *              transparency detection, so pages that are never flagged keep
     *              emitting the group, preserving backward-compatible output.
     * - 'always' : emit the group on every standard page (legacy behaviour).
     * - 'never'  : never emit the group.
     *
     * The mode is matched case-insensitively and unknown values are treated as 'auto'.
     *
     * @param string|TransparencyGroupMode $mode One of 'auto', 'always', 'never', or a TransparencyGroupMode case.
     */
    public function setPageTransparencyGroupMode(string|TransparencyGroupMode $mode): static
    {
        if ($mode instanceof TransparencyGroupMode) {
            $mode = $mode->value;
        }

        $this->transparencygroupmode = \strtolower($mode);
        return $this;
    }

    /**
     * Whether the per-page transparency /Group must be emitted for a page,
     * according to the configured mode and the page's recorded transparency.
     *
     * @param int $pid Page index.
     */
    protected function emitPageTransparencyGroup(int $pid): bool
    {
        return match ($this->transparencygroupmode) {
            'always' => true,
            'never' => false,
            default => $this->pagetransparency[$pid] ?? true,
        };
    }

    /**
     * Remove the specified page.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @return PageData Removed page.
     *
     * @throws PageException
     */
    public function delete(int $pid = -1): array
    {
        $pid = $this->sanitizePageID($pid);
        $page = $this->getPage($pid);
        $group = $page['group'];
        $groupCount = $this->group[$group] ?? 0;
        if ($groupCount > 0) {
            $this->group[$group] = $groupCount - 1;
        }

        unset($this->page[$pid]);
        $this->page = \array_values($this->page); // reindex array
        $this->reindexPageIds();

        // Keep the per-page side maps aligned with the reindexed page stack:
        // drop the deleted entry and shift the entries above it down by one.
        $transparency = [];
        foreach ($this->pagetransparency as $idx => $flag) {
            if ($idx === $pid) {
                continue;
            }

            $transparency[$idx > $pid ? $idx - 1 : $idx] = $flag;
        }

        $this->pagetransparency = $transparency;

        $nowrite = [];
        foreach ($this->nowrite as $idx => $area) {
            if ($idx === $pid) {
                continue;
            }

            $nowrite[$idx > $pid ? $idx - 1 : $idx] = $area;
        }

        $this->nowrite = $nowrite;

        --$this->pmaxid;

        // Keep the current-page pointer valid and tracking the same page where possible.
        if ($this->pid > $pid) {
            --$this->pid;
        }

        if ($this->pid > $this->pmaxid) {
            $this->pid = $this->pmaxid;
        }

        return $page;
    }

    /**
     * Remove and return last page.
     *
     * @return PageData Removed page.
     *
     * @throws PageException
     */
    public function pop(): array
    {
        return $this->delete($this->pmaxid);
    }

    /**
     * Move a page to a previous position.
     *
     * @param int $from Index of the page to move.
     * @param int $new  Destination index.
     *
     * @throws PageException
     */
    public function move(int $from, int $new): void
    {
        $page = $this->page[$from] ?? null;
        if ($from <= $new || $new < 0 || $from > $this->pmaxid || !is_array($page)) {
            throw new PageException('The new position must be lower than the starting position');
        }

        $pages = $this->page;
        unset($pages[$from]);
        $pages = \array_values($pages);
        \array_splice($pages, $new, 0, [$page]);

        /** @var array<int, PageData> $pages */
        $this->page = $pages;
        $this->reindexPageIds();

        // Keep the per-page side maps and the current-page pointer aligned with
        // the reordered page stack.
        $transparency = [];
        foreach ($this->pagetransparency as $idx => $flag) {
            $transparency[$this->movedIndex($idx, $from, $new)] = $flag;
        }

        $this->pagetransparency = $transparency;

        $nowrite = [];
        foreach ($this->nowrite as $idx => $area) {
            $nowrite[$this->movedIndex($idx, $from, $new)] = $area;
        }

        $this->nowrite = $nowrite;

        $this->pid = $this->movedIndex($this->pid, $from, $new);
    }

    /**
     * Compute the new index of a page after move($from, $new) is applied.
     *
     * @param int $idx  Original page index.
     * @param int $from Index of the moved page.
     * @param int $new  Destination index.
     *
     * @return int The index the page occupies after the move.
     */
    private function movedIndex(int $idx, int $from, int $new): int
    {
        if ($idx === $from) {
            return $new;
        }

        if ($idx >= $new && $idx < $from) {
            return $idx + 1;
        }

        return $idx;
    }

    /**
     * Re-sync the embedded 'pid' field of every page with its index in the stack.
     *
     * The page stack is reindexed with array_values() after a delete or move, so the
     * 'pid' stored inside each page (set once at add() time) would otherwise drift out
     * of sync with the array key callers use to address the page.
     */
    private function reindexPageIds(): void
    {
        foreach (\array_keys($this->page) as $idx) {
            $this->page[$idx]['pid'] = $idx;
        }
    }

    /**
     * Returns the array (stack) containing all pages data.
     *
     * @return array<int, PageData> Pages.
     */
    public function getPages(): array
    {
        return $this->page;
    }

    /**
     * Add Annotation references.
     *
     * @param int $oid Annotation object IDs.
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function addAnnotRef(int $oid, int $pid = -1): void
    {
        $pid = $this->sanitizePageID($pid);
        $annotrefs = $this->page[$pid]['annotrefs'] ?? [];

        if (\in_array($oid, $annotrefs, strict: true)) {
            return;
        }

        $annotrefs[] = $oid;
        $this->page[$pid]['annotrefs'] = $annotrefs;
    }

    /**
     * Add page content.
     *
     * @param string $content Page content.
     * @param int    $pid     Page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function addContent(string $content, int $pid = -1): void
    {
        $pid = $this->sanitizePageID($pid);

        $pageContent = $this->page[$pid]['content'] ?? [''];
        $pageContent[] = $content;
        $this->page[$pid]['content'] = $pageContent;
    }

    /**
     * Remove and return last page content.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function popContent(int $pid = -1): string
    {
        $pid = $this->sanitizePageID($pid);

        $pageContent = $this->page[$pid]['content'] ?? null;
        if ($pageContent === null) {
            throw new PageException('Page content is empty');
        }

        $page = \array_pop($pageContent);
        if ($page === null) {
            throw new PageException('Page content is empty');
        }

        $this->page[$pid]['content'] = $pageContent;

        return $page;
    }

    /**
     * Add page content mark.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function addContentMark(int $pid = -1): void
    {
        $pid = $this->sanitizePageID($pid);

        $pageContent = $this->page[$pid]['content'] ?? [''];
        $contentMark = $this->page[$pid]['content_mark'] ?? [0];

        $contentMark[] = \count($pageContent);

        $this->page[$pid]['content'] = $pageContent;
        $this->page[$pid]['content_mark'] = $contentMark;
    }

    /**
     * Remove the last marked page content.
     *
     * @param int $pid page index. Omit or set it to -1 for the current page ID.
     *
     * @throws PageException
     */
    public function popContentToLastMark(int $pid = -1): void
    {
        $pid = $this->sanitizePageID($pid);

        $pageContent = $this->page[$pid]['content'] ?? null;
        if (empty($pageContent)) {
            return;
        }

        $contentMark = $this->page[$pid]['content_mark'] ?? [0];

        $mark = \array_pop($contentMark);
        $this->page[$pid]['content_mark'] = $contentMark;
        $this->page[$pid]['content'] = \array_slice($pageContent, 0, (int) ($mark ?? 0), true);
    }

    /**
     * Returns the PDF command to output all page sections.
     *
     * @param int $pon Current PDF object number.
     *
     * @return string PDF command.
     *
     * @throws EncryptException
     */
    public function getPdfPages(int &$pon): string
    {
        $out = $this->getPageRootObj($pon);
        foreach ($this->page as $num => $page) {
            if (!array_key_exists('num', $page)) {
                $page['num'] = $this->getPageNumInGroup($num, $page);
            }

            $this->page[$num]['num'] = $page['num'];

            $content = $this->replacePageTemplates($page);
            $out .= $this->getPageContentObj($pon, $content);
            $contentobjid = $pon;

            $out .=
                $page['n']
                . ' 0 obj'
                . "\n"
                . '<<'
                . "\n"
                . '/Type /Page'
                . "\n"
                . '/Parent '
                . $this->rootoid
                . ' 0 R'
                . "\n";
            if (!$this->pdfa && $this->emitPageTransparencyGroup($num)) {
                $out .= '/Group << /Type /Group /S /Transparency /CS /DeviceRGB >>' . "\n";
            }

            if (!$this->sigapp) {
                $out .= '/LastModified ' . $this->enc->getFormattedDate($page['time'], $pon) . "\n";
            }

            [$boxdims, $boxinfo] = $this->getPageBoxData($page);

            $out .=
                '/Resources '
                . $this->rdoid
                . ' 0 R'
                . "\n"
                . $this->getBox($boxdims)
                . $this->getBoxColorInfo($boxinfo)
                . '/Contents '
                . $contentobjid
                . ' 0 R'
                . "\n"
                . '/Rotate '
                . $page['rotation']
                . "\n";

            $out .= \sprintf('/PZ %F' . "\n", $page['zoom']);

            $out .= $this->getPageTransition($page) . $this->getAnnotationRef($page) . '>>' . "\n" . 'endobj' . "\n";
        }

        return $out;
    }

    /**
     * @param int $num Page index.
     * @param PageData $page
     */
    protected function getPageNumInGroup(int $num, array $page): int
    {
        $pnum = 1 + $num;
        if ($num <= 0) {
            return $pnum;
        }

        $prevPage = $this->page[$num - 1] ?? null;
        if (!is_array($prevPage)) {
            return $pnum;
        }

        return $prevPage['group'] === $page['group'] ? 1 + $prevPage['num'] : 1;
    }

    /**
     * @param PageData $page
     *
     * @return array{0: array<string, array{llx: float, lly: float, urx: float, ury: float}>, 1: array<string, array{bci: PageBci}>}
     */
    protected function getPageBoxData(array $page): array
    {
        $boxdims = [];
        $boxinfo = [];
        foreach ($page['box'] as $name => $box) {
            $boxdims[$name] = [
                'llx' => $box['llx'],
                'lly' => $box['lly'],
                'urx' => $box['urx'],
                'ury' => $box['ury'],
            ];

            $boxinfo[$name] = [
                'bci' => $box['bci'],
            ];
        }

        return [$boxdims, $boxinfo];
    }

    /**
     * Returns the reserved Object ID for the Resource dictionary.
     *
     * @return int Resource dictionary Object ID.
     */
    public function getResourceDictObjID(): int
    {
        return $this->rdoid;
    }

    /**
     * Returns the root object ID.
     *
     * @return int Root Object ID.
     */
    public function getRootObjID(): int
    {
        return $this->rootoid;
    }

    /**
     * Returns the PDF command to output the page content.
     *
     * @param array<string, mixed> $page Page data.
     *
     * @return string PDF command.
     */
    protected function getPageTransition(array $page): string
    {
        if (!array_key_exists('transition', $page) || !is_array($page['transition']) || $page['transition'] === []) {
            return '';
        }

        $transition = $page['transition'];
        /** @var array<string, bool|int|float|string> $transition */

        $entries = ['B', 'D', 'Di', 'Dm', 'M', 'S', 'SS'];
        // Entries that are PDF name objects. Everything else among $entries is a
        // number (/D, /SS, numeric /Di) or a boolean (/B). /Di may also be the
        // name /None, which is handled explicitly below.
        $nameKeys = ['S', 'Dm', 'M'];
        $out = '';
        $out .= \sprintf('/Dur %F' . "\n", (float) ($transition['Dur'] ?? 0.0));

        $out .= '/Trans <<' . "\n" . '/Type /Trans' . "\n";
        foreach ($transition as $key => $val) {
            if (!\in_array($key, $entries, strict: true)) {
                continue;
            }

            if (\is_bool($val)) {
                $out .= '/' . $key . ' ' . ($val ? 'true' : 'false') . "\n";
                continue;
            }

            if (\in_array($key, $nameKeys, strict: true) || $val === 'None') {
                $out .= '/' . $key . ' /' . (string) $val . "\n";
                continue;
            }

            if (\is_float($val)) {
                $out .= \sprintf('/%s %F' . "\n", $key, $val);
                continue;
            }

            $out .= \sprintf('/%s %d' . "\n", $key, (int) $val);
        }

        return $out . '>>' . "\n";
    }

    /**
     * Get references to page annotations.
     *
     * @param PageData $page Page data.
     *
     * @return string PDF command.
     */
    protected function getAnnotationRef(array $page): string
    {
        if (empty($page['annotrefs'])) {
            return '';
        }

        $out = '/Annots [ ';
        \sort($page['annotrefs']);
        foreach ($page['annotrefs'] as $val) {
            $out .= (int) $val . ' 0 R ';
        }

        return $out . ']' . "\n";
    }

    /**
     * Returns the PDF command to output the page content.
     *
     * @param int    $pon     Current PDF object number.
     * @param string $content Page content.
     *
     * @return string PDF command.
     *
     * @throws EncryptException
     */
    protected function getPageContentObj(int &$pon, string $content = ''): string
    {
        $out = ++$pon . ' 0 obj' . "\n" . '<<';
        if ($this->compress) {
            $out .= ' /Filter /FlateDecode';
            $cmpr = \gzcompress($content);
            if ($cmpr !== false) {
                $content = $cmpr;
            }
        }

        $stream = $this->enc->encryptString($content, $pon);
        return (
            $out
            . ' /Length '
            . \strlen($stream)
            . ' >>'
            . "\n"
            . 'stream'
            . "\n"
            . $stream
            . "\n"
            . 'endstream'
            . "\n"
            . 'endobj'
            . "\n"
        );
    }

    /**
     * Returns the PDF command to output the page root object.
     *
     * @param int $pon Current PDF object number.
     *
     * @return string PDF command.
     */
    protected function getPageRootObj(int &$pon): string
    {
        $this->rdoid = ++$pon; // reserve object ID for the resource dictionary
        $this->rootoid = ++$pon;
        $out = $this->rootoid . ' 0 obj' . "\n";
        $out .= '<< /Type /Pages /Kids [ ';
        $numpages = \count($this->page);
        for ($pid = 0; $pid < $numpages; ++$pid) {
            $this->page[$pid]['n'] = ++$pon;
            $out .= $this->page[$pid]['n'] . ' 0 R ';
        }

        return $out . '] /Count ' . $numpages . ' >>' . "\n" . 'endobj' . "\n";
    }

    /**
     * Replace page templates and numbers.
     *
     * @param PageData $data Page data.
     */
    protected function replacePageTemplates(array $data): string
    {
        return \implode("\n", \str_replace(
            [self::PAGE_TOT, self::PAGE_NUM],
            [(string) ($this->group[$data['group']] ?? 0), (string) $data['num']],
            $data['content'],
        ));
    }
}
