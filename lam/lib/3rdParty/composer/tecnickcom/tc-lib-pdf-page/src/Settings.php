<?php

declare(strict_types=1);

/**
 * Settings.php
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

use Com\Tecnick\Pdf\Encrypt\Encrypt;

/**
 * Com\Tecnick\Pdf\Page\Settings
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
 * @phpstan-import-type PageDataBox from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type MarginData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type RegionData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type TransitionData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type TransitionInput from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type PageData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type PageInputData from \Com\Tecnick\Pdf\Page\Box
 *
 * @SuppressWarnings("PHPMD.TooManyPublicMethods")
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Settings extends \Com\Tecnick\Pdf\Page\Box
{
    /**
     * Epsilon precision used to compare floating point values.
     */
    public const EPS = 0.0001;

    /**
     * Maximum number of equal vertical columns a page can be split into.
     */
    public const MAX_COLUMNS = 1000;

    /**
     * Alias for total number of pages in a group.
     *
     * @var string
     */
    public const PAGE_TOT = '~#PT';

    /**
     * Alias for page number.
     *
     * @var string
     */
    public const PAGE_NUM = '~#PN';

    /**
     * Array of pages (stack).
     *
     * @var array<int, PageData>
     */
    protected array $page = [];

    /**
     * Current page ID.
     */
    protected int $pid = -1;

    /**
     * Maximum page ID.
     */
    protected int $pmaxid = -1;

    /**
     * Count pages in each group.
     *
     * @var array<int, int>
     */
    protected array $group = [
        0 => 0,
    ];

    /**
     * Encrypt object.
     */
    protected Encrypt $enc;

    /**
     * True when the conformance mode forbids transparency (PDF/A-1, PDF/X-1a, PDF/X-3).
     *
     * Suppresses the page transparency group.
     */
    protected bool $notransparency = false;

    /**
     * Page boxes excluded from the page dictionary, keyed by box name.
     *
     * @var array<string, bool>
     */
    protected array $omittedboxes = [];

    /**
     * Enable stream compression.
     */
    protected bool $compress = true;

    /**
     * True if the signature approval is enabled (for incremental updates).
     */
    protected bool $sigapp = false;

    /**
     * Per-page flag indicating whether the page uses transparency, keyed by page index.
     * In 'auto' mode a page flagged false omits the per-page transparency /Group;
     * unset pages emit it.
     *
     * @var array<int, bool>
     */
    protected array $pagetransparency = [];

    /**
     * Policy for emitting the per-page transparency /Group on standard pages:
     * 'auto' (every standard page except those explicitly flagged as opaque,
     * default), 'always' (every standard page) or 'never'.
     */
    protected string $transparencygroupmode = 'auto';

    /**
     * Reserved Object ID for the resource dictionary.
     */
    protected int $rdoid = 1;

    /**
     * Root object ID.
     */
    protected int $rootoid = 0;

    /**
     * Return the current page ID.
     *
     * @return int Page ID.
     */
    public function getPageID(): int
    {
        return $this->pid;
    }

    /**
     * Sanitize or set the page number.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizePageNumber(array &$data): void
    {
        if (!empty($data['num'])) {
            $data['num'] = \max(0, (int) $data['num']);
        }
    }

    /**
     * Sanitize or set the page modification time.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeTime(array &$data): void
    {
        $data['time'] = empty($data['time']) ? \time() : \max(0, (int) $data['time']);
    }

    /**
     * Sanitize or set the page group.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeGroup(array &$data): void
    {
        $data['group'] = empty($data['group']) ? 0 : \max(0, $data['group']);
    }

    /**
     * Sanitize or set the page content.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeContent(array &$data): void
    {
        if (!\array_key_exists('content', $data) || $data['content'] === []) {
            $data['content'] = [''];
            return;
        }

        if (\is_string($data['content'])) {
            $data['content'] = [$data['content']];
            return;
        }

        $data['content'] = \array_values($data['content']);
    }

    /**
     * Sanitize or set the annotation references.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeAnnotRefs(array &$data): void
    {
        if (empty($data['annotrefs'])) {
            $data['annotrefs'] = [];
        }
    }

    /**
     * Sanitize or set the page rotation.
     * The number of degrees by which the page shall be rotated clockwise when displayed or printed.
     * The value shall be a multiple of 90; it is normalized to the [0, 360) range.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeRotation(array &$data): void
    {
        // The multiple-of-90 test runs on the float value: the modulo operator
        // converts its operands to int first, losing a fractional value.
        $rotation = $data['rotation'] ?? 0;
        if (\fmod((float) $rotation, 90.0) !== 0.0) {
            $data['rotation'] = 0;
            return;
        }

        $data['rotation'] = (((int) $rotation % 360) + 360) % 360;
    }

    /**
     * Sanitize or set the page preferred zoom (magnification) factor.
     * The factor is a magnification, so any non-positive value falls back to 1.0.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeZoom(array &$data): void
    {
        $zoom = $data['zoom'] ?? 0.0;
        $data['zoom'] = $zoom > 0.0 ? $zoom : 1.0;
    }

    /**
     * Sanitize or set the page transitions.
     *
     * @param PageInputData $data Page data.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     */
    public function sanitizeTransitions(array &$data): void
    {
        if (empty($data['transition'])) {
            return;
        }

        $transition = $data['transition'];

        // display duration before advancing the page: only a positive duration
        // makes the reader auto-advance, any other value drops the entry.
        if (($transition['Dur'] ?? 0) <= 0) {
            unset($transition['Dur']);
        }

        // transition style
        $styles = [
            'Split',
            'Blinds',
            'Box',
            'Wipe',
            'Dissolve',
            'Glitter',
            'R',
            'Fly',
            'Push',
            'Cover',
            'Uncover',
            'Fade',
        ];
        if (empty($transition['S']) || !\in_array($transition['S'], $styles, true)) {
            $transition['S'] = 'R';
        }

        // duration of the transition effect, in seconds: it cannot be negative
        $transition['D'] = \max(0, $transition['D'] ?? 1);

        $data['transition'] = $this->pruneFlyTransitionEntries($this->pruneTransitionDirections($transition));
    }

    /**
     * Drop the direction entries the transition style does not use.
     *
     * @param TransitionInput $transition Transition data.
     *
     * @return TransitionInput Transition data.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function pruneTransitionDirections(array $transition): array
    {
        $style = $transition['S'] ?? '';

        // dimension in which the specified transition effect shall occur
        if (
            empty($transition['Dm'])
            || !\in_array($style, ['Split', 'Blinds'], true)
            || !\in_array($transition['Dm'], ['H', 'V'], true)
        ) {
            unset($transition['Dm']);
        }

        // direction of motion for the specified transition effect
        if (
            empty($transition['M'])
            || !\in_array($style, ['Split', 'Box', 'Fly'], true)
            || !\in_array($transition['M'], ['I', 'O'], true)
        ) {
            unset($transition['M']);
        }

        // direction in which the specified transition effect shall move
        if (
            empty($transition['Di'])
            || !\in_array($style, ['Wipe', 'Glitter', 'Fly', 'Cover', 'Uncover', 'Push'], true)
            || !\in_array($transition['Di'], ['None', 0, 90, 180, 270, 315], true)
            || \in_array($transition['Di'], [90, 180], true) && $style !== 'Wipe'
            || $transition['Di'] === 315 && $style !== 'Glitter'
            || $transition['Di'] === 'None' && $style !== 'Fly'
        ) {
            unset($transition['Di']);
        }

        return $transition;
    }

    /**
     * Drop the entries that apply to the Fly style only.
     *
     * @param TransitionInput $transition Transition data.
     *
     * @return TransitionInput Transition data.
     */
    protected function pruneFlyTransitionEntries(array $transition): array
    {
        if (($transition['S'] ?? '') !== 'Fly') {
            unset($transition['SS'], $transition['B']);
            return $transition;
        }

        // starting or ending scale: it must be greater than 0, any other value
        // drops the entry so the reader applies the default of 1.
        if (($transition['SS'] ?? 0) <= 0) {
            unset($transition['SS']);
        }

        // If true, the area that shall be flown in is rectangular and opaque
        $transition['B'] = !empty($transition['B']);

        return $transition;
    }

    /**
     * Sanitize or set the page margins.
     *
     * @param PageInputData $data Page data.
     * @param-out PageInputData $data
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     *
     * @throws Exception
     */
    public function sanitizeMargins(array &$data): void
    {
        $defmargin = $this->getDefaultMargins();
        $marginData = is_array($data['margin'] ?? null) ? $data['margin'] : [];

        $dataWidth = $data['width'] ?? 0.0;
        $dataHeight = $data['height'] ?? 0.0;
        $booklet = !empty($marginData['booklet']);
        if (!$this->hasUsablePageSize($dataWidth, $dataHeight)) {
            [$data['width'], $data['height'], $data['orientation']] = $this->getPageFormatSize('A4', 'P');
            $data['width'] /= $this->kunit;
            $data['height'] /= $this->kunit;
            $dataWidth = $data['width'];
            $dataHeight = $data['height'];
        }

        $margin = $this->normalizeMargins($marginData, $defmargin, $dataWidth, $dataHeight, $booklet);

        $data['margin'] = ['booklet' => $booklet] + $margin;

        $data['ContentWidth'] = $dataWidth - $margin['PL'] - $margin['PR'];
        $data['ContentHeight'] = $dataHeight - $margin['CT'] - $margin['CB'];
        $data['HeaderHeight'] = $margin['HB'] - $margin['PT'];
        $data['FooterHeight'] = $margin['FT'] - $margin['PB'];
    }

    /**
     * Returns the default margins, all set to zero.
     *
     * @return MarginData
     */
    protected function getDefaultMargins(): array
    {
        return [
            'CB' => 0.0,
            'CT' => 0.0,
            'FT' => 0.0,
            'HB' => 0.0,
            'PB' => 0.0,
            'PL' => 0.0,
            'PR' => 0.0,
            'PT' => 0.0,
        ];
    }

    /**
     * Normalize the margin input: bounds, booklet swap, implicit CT/CB and constraints.
     *
     * @param array<string, scalar|null> $marginData     Caller-supplied margins.
     * @param MarginData                 $defaultMargins Default margins.
     *
     * @return MarginData
     */
    protected function normalizeMargins(
        array $marginData,
        array $defaultMargins,
        float $dataWidth,
        float $dataHeight,
        bool $booklet,
    ): array {
        $hasCT = \array_key_exists('CT', $marginData);
        $hasCB = \array_key_exists('CB', $marginData);

        $margin = $this->getNormalizedMarginInput($marginData, $defaultMargins);
        $margin = $this->applyMarginBounds($margin, $dataWidth, $dataHeight);
        $margin = $this->applyBookletMarginSwap($margin, $booklet);
        $margin = $this->applyImplicitCtCb($margin, $hasCT, $hasCB);

        return $this->applyMarginConstraints($margin, $dataWidth, $dataHeight);
    }

    /**
     * Cast the scalar margin entries to float and replace the others with the defaults.
     *
     * @param array<string, scalar|null> $marginData     Caller-supplied margins.
     * @param MarginData                 $defaultMargins Default margins.
     *
     * @return MarginData
     */
    protected function getNormalizedMarginInput(array $marginData, array $defaultMargins): array
    {
        $margin = $defaultMargins;
        foreach ($defaultMargins as $key => $default) {
            $marginValue = $marginData[$key] ?? null;
            if (is_scalar($marginValue)) {
                $margin[$key] = (float) $marginValue;
                continue;
            }

            $margin[$key] = $default;
        }

        return $margin;
    }

    /**
     * Clamp each margin into the [0, page width] or [0, page height] range.
     *
     * @param MarginData $margin Margins.
     *
     * @return MarginData
     */
    protected function applyMarginBounds(array $margin, float $dataWidth, float $dataHeight): array
    {
        $marginBounds = [
            'PL' => $dataWidth,
            'PR' => $dataWidth,
            'PT' => $dataHeight,
            'HB' => $dataHeight,
            'CT' => $dataHeight,
            'CB' => $dataHeight,
            'FT' => $dataHeight,
            'PB' => $dataHeight,
        ];

        foreach ($marginBounds as $type => $max) {
            $margin[$type] = empty($margin[$type]) ? 0.0 : \min(\max(0.0, $margin[$type]), $max);
        }

        return $margin;
    }

    /**
     * Swap the left and right page margins on the odd pages of a booklet.
     *
     * @param MarginData $margin Margins.
     *
     * @return MarginData
     */
    protected function applyBookletMarginSwap(array $margin, bool $booklet): array
    {
        // The margins belong to the page add() is about to append, which lands at
        // index pmaxid + 1. The gutter is mirrored on odd page indexes.
        if ($booklet && (($this->pmaxid + 1) % 2) === 1) {
            $tmp = $margin['PL'];
            $margin['PL'] = $margin['PR'];
            $margin['PR'] = $tmp;
        }

        return $margin;
    }

    /**
     * Derive the content top and bottom margins that the caller did not provide.
     *
     * @param MarginData $margin Margins.
     *
     * @return MarginData
     */
    protected function applyImplicitCtCb(array $margin, bool $hasCT, bool $hasCB): array
    {
        if (!$hasCT) {
            $margin['CT'] = \max($margin['PT'], $margin['HB']);
        }

        if (!$hasCB && ($margin['PB'] > 0.0 || $margin['FT'] > 0.0)) {
            $margin['CB'] = \max($margin['PB'], $margin['FT']);
        }

        return $margin;
    }

    /**
     * Constrain the margins so that PL + PR fits the page width, CT + CB fits the
     * page height, PT <= HB <= CT and PB <= FT <= CB.
     *
     * @param MarginData $margin Margins.
     *
     * @return MarginData
     */
    protected function applyMarginConstraints(array $margin, float $dataWidth, float $dataHeight): array
    {
        $margin['PR'] = \min($margin['PR'], $dataWidth - $margin['PL']);
        $margin['HB'] = \max($margin['HB'], $margin['PT']);
        $margin['CT'] = \max($margin['CT'], $margin['HB']);
        $margin['CB'] = \min($margin['CB'], $dataHeight - $margin['CT']);
        $margin['FT'] = \min($margin['FT'], $margin['CB']);
        $margin['PB'] = \min($margin['PB'], $margin['FT']);

        return $margin;
    }

    /**
     * Sanitize or set the page regions (columns).
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeRegions(array &$data): void
    {
        $pageWidth = (float) ($data['width'] ?? 0);
        $pageHeight = (float) ($data['height'] ?? 0);
        $marginLeft = (float) ($data['margin']['PL'] ?? 0);
        $marginTop = (float) ($data['margin']['CT'] ?? 0);
        $marginRight = (float) ($data['margin']['PR'] ?? 0);
        $marginBottom = (float) ($data['margin']['CB'] ?? 0);
        // sanitizeMargins() provides the content area; derive it from the page size
        // and the margins when this method is called on its own.
        $contentWidth = $data['ContentWidth'] ?? ($pageWidth - $marginLeft - $marginRight);
        $contentHeight = $data['ContentHeight'] ?? ($pageHeight - $marginTop - $marginBottom);

        if (!empty($data['columns'])) {
            // set equal columns
            $data['region'] = [];
            $numColumns = \min(\max(1, (int) $data['columns']), self::MAX_COLUMNS);
            $width = $contentWidth / $numColumns;
            for ($idx = 0; $idx < $numColumns; ++$idx) {
                $data['region'][] = [ // @phpstan-ignore parameterByRef.type
                    'RX' => $marginLeft + ($idx * $width),
                    'RY' => $marginTop,
                    'RW' => $width,
                    'RH' => $contentHeight,
                ];
            }
        }

        if (empty($data['region'])) {
            // default single region
            $data['region'] = [[ // @phpstan-ignore parameterByRef.type
                'RX' => $marginLeft,
                'RY' => $marginTop,
                'RW' => $contentWidth,
                'RH' => $contentHeight,
            ]];
        }

        // The regions are collected into a fresh list: 'columns' is their count and
        // selectRegion() addresses them by an index in the [0, columns - 1] range.
        $regions = [];
        foreach ($data['region'] ?? [] as $val) {
            $regions[] = $this->normalizeRegionData(
                $val,
                $contentWidth,
                $contentHeight,
                $pageWidth,
                $pageHeight,
                $marginRight,
                $marginBottom,
            );
        }

        $data['region'] = $regions; // @phpstan-ignore parameterByRef.type
        $data['columns'] = \count($regions); // @phpstan-ignore parameterByRef.type

        if (!\array_key_exists('autobreak', $data)) {
            $data['autobreak'] = true; // @phpstan-ignore parameterByRef.type
        }
    }

    /**
     * Clamp a region inside the content area and compute its derived edges.
     *
     * @param array<string, float|int> $region Region.
     *
     * @return RegionData
     */
    protected function normalizeRegionData(
        array $region,
        float $contentWidth,
        float $contentHeight,
        float $pageWidth,
        float $pageHeight,
        float $marginRight,
        float $marginBottom,
    ): array {
        $regionWidth = (float) ($region['RW'] ?? 0);
        $regionHeight = (float) ($region['RH'] ?? 0);
        $regionX = (float) ($region['RX'] ?? 0);
        $regionY = (float) ($region['RY'] ?? 0);

        // The origin is clamped against the clamped extent and the derived edges are
        // computed from the clamped values, so RL = RX + RW and RT = RY + RH hold.
        $rw = \min(\max(0.0, $regionWidth), $contentWidth);
        $rx = \min(\max(0.0, $regionX), \max(0.0, $pageWidth - $marginRight - $rw));
        $rl = $rx + $rw;
        $rr = $pageWidth - $rl;
        $rh = \min(\max(0.0, $regionHeight), $contentHeight);
        $ry = \min(\max(0.0, $regionY), \max(0.0, $pageHeight - $marginBottom - $rh));
        $rt = $ry + $rh;
        $rb = $pageHeight - $rt;

        return [
            'RW' => $rw,
            'RX' => $rx,
            'RL' => $rl,
            'RR' => $rr,
            'RH' => $rh,
            'RY' => $ry,
            'RT' => $rt,
            'RB' => $rb,
            'x' => $rx,
            'y' => $ry,
        ];
    }

    /**
     * Sanitize or set the page boxes containing the page boundaries.
     *
     * Every entry of PageInputData['box'] is optional and is completed first.
     *
     * @param PageInputData $data Page data.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     *
     * @throws Exception
     */
    public function sanitizeBoxData(array &$data): void
    {
        /** @var array<string, PageBox> $box */
        $box = $this->normalizeBoxes($data['box'] ?? []);

        if (empty($box)) {
            if (!$this->hasUsablePageSize($data['pwidth'] ?? null, $data['pheight'] ?? null)) {
                [$data['pwidth'], $data['pheight'], $data['orientation']] = $this->getPageFormatSize('A4', 'P');
            }

            $box = $this->setPageBoxes($data['pwidth'] ?? 0, $data['pheight'] ?? 0);
        }

        if (!empty($box)) {
            $this->applyMediaBoxFormat($data, $box);
            $box = $this->completeBoxChain($box, $data['pwidth'] ?? 0, $data['pheight'] ?? 0);
        }

        $data['box'] = $box;

        [$mbwidth, $mbheight] = $this->getBoxSpan($box, 'MediaBox');
        $orientation = $this->getPageOrientation($mbwidth, $mbheight);
        if (empty($data['orientation'])) {
            $data['orientation'] = $orientation;
        }

        if (($data['orientation'] ?? '') !== $orientation) {
            $data['box'] = $this->swapCoordinates($box);
        }
    }

    /**
     * Returns the horizontal and vertical extent of a page box.
     *
     * @param array<string, PageBox> $box  Page boxes.
     * @param string                 $type Page box name.
     *
     * @return array{0: float, 1: float} Box width and height in points.
     */
    protected function getBoxSpan(array $box, string $type): array
    {
        return [
            \abs(($box[$type]['urx'] ?? 0) - ($box[$type]['llx'] ?? 0)),
            \abs(($box[$type]['ury'] ?? 0) - ($box[$type]['lly'] ?? 0)),
        ];
    }

    /**
     * Resolve the 'MediaBox' pseudo-format: the page size is taken from the MediaBox and
     * the format is re-sanitized with the box-derived dimensions.
     *
     * @param PageInputData          $data Page data.
     * @param array<string, PageBox> $box  Page boxes.
     *
     * @throws Exception
     */
    protected function applyMediaBoxFormat(array &$data, array $box): void
    {
        if (($data['format'] ?? '') !== 'MediaBox') {
            return;
        }

        [$width, $height] = $this->getBoxSpan($box, 'MediaBox');
        $data['format'] = '';
        $data['width'] = $width / $this->kunit;
        $data['height'] = $height / $this->kunit;
        $this->sanitizePageFormat($data);
    }

    /**
     * Give the page a MediaBox, fill every box the caller left out with the one it
     * inherits, and clamp them all inside the MediaBox.
     *
     * @param array<string, PageBox> $box     Page boxes.
     * @param float                  $pwidth  Page width in points.
     * @param float                  $pheight Page height in points.
     *
     * @return array<string, PageDataBox> Page boxes.
     *
     * @throws Exception
     */
    protected function completeBoxChain(array $box, float $pwidth, float $pheight): array
    {
        if (empty($box['MediaBox'])) {
            $box = $this->setBox($box, 'MediaBox', 0, 0, $pwidth, $pheight);
        }

        $box = $this->inheritMissingBox($box, 'CropBox', 'MediaBox');
        $box = $this->inheritMissingBox($box, 'BleedBox', 'CropBox');
        $box = $this->inheritMissingBox($box, 'TrimBox', 'CropBox');
        $box = $this->inheritMissingBox($box, 'ArtBox', 'CropBox');

        /** @var array<string, PageDataBox> $box */
        return $this->clampBoxesToMediaBox($box);
    }

    /**
     * Copy the source box into the target box when the target is not set.
     *
     * @param array<string, PageBox> $box       Page boxes.
     * @param string                 $targetBox Name of the box to fill.
     * @param string                 $sourceBox Name of the box to copy from.
     *
     * @return array<string, PageBox>
     *
     * @throws Exception
     */
    protected function inheritMissingBox(array $box, string $targetBox, string $sourceBox): array
    {
        if (!empty($box[$targetBox])) {
            return $box;
        }

        return $this->setBox(
            $box,
            $targetBox,
            $box[$sourceBox]['llx'] ?? 0,
            $box[$sourceBox]['lly'] ?? 0,
            $box[$sourceBox]['urx'] ?? 0,
            $box[$sourceBox]['ury'] ?? 0,
        );
    }

    /**
     * Sanitize or set the page format.
     *
     * The pseudo-format 'MediaBox' means that the page size is taken from the MediaBox:
     * it is left untouched here and resolved by sanitizeBoxData(), which then re-enters
     * this method with the box-derived dimensions and a cleared format.
     *
     * @param PageInputData $data Page data.
     *
     * @throws Exception
     */
    public function sanitizePageFormat(array &$data): void
    {
        $this->ensurePageOrientation($data);

        $format = $data['format'] ?? '';

        if ($format !== '' && $format !== 'MediaBox') {
            $this->applyNamedPageFormat($data);
        }

        if (empty($data['format'])) {
            $this->applyCustomPageFormat($data);
        }

        // convert values in points
        $data['pwidth'] = ($data['width'] ?? 0.0) * $this->kunit;
        $data['pheight'] = ($data['height'] ?? 0.0) * $this->kunit;
    }

    /**
     * Whether a pair of page dimensions can be used as a page size.
     * A missing, zero or negative dimension is unusable and makes the caller fall
     * back to the default format.
     */
    protected function hasUsablePageSize(?float $width, ?float $height): bool
    {
        return $width !== null && $width > 0.0 && $height !== null && $height > 0.0;
    }

    /**
     * Set the page orientation to the empty string (auto) when it is not given.
     *
     * @param PageInputData $data Page data.
     */
    protected function ensurePageOrientation(array &$data): void
    {
        if (empty($data['orientation'])) {
            $data['orientation'] = '';
        }
    }

    /**
     * Set the page size from the named page format.
     *
     * @param PageInputData $data Page data.
     *
     * @throws Exception
     */
    protected function applyNamedPageFormat(array &$data): void
    {
        [$data['pwidth'], $data['pheight'], $data['orientation']] = $this->getPageFormatSize(
            $data['format'] ?? '',
            $data['orientation'] ?? '',
        );
        $data['width'] = $data['pwidth'] / $this->kunit;
        $data['height'] = $data['pheight'] / $this->kunit;
    }

    /**
     * Set the 'CUSTOM' format from the given page size, or fall back to A4 portrait.
     *
     * @param PageInputData $data Page data.
     *
     * @throws Exception
     */
    protected function applyCustomPageFormat(array &$data): void
    {
        $width = $data['width'] ?? null;
        $height = $data['height'] ?? null;

        $data['format'] = 'CUSTOM';
        if (!$this->hasUsablePageSize($width, $height)) {
            // default page format
            $data['format'] = 'A4';
            $data['orientation'] = 'P';
            $this->sanitizePageFormat($data);
            return;
        }

        [$data['width'], $data['height'], $data['orientation']] = $this->getPageOrientedSize(
            (float) $width,
            (float) $height,
            $data['orientation'] ?? 'P',
        );
    }
}
