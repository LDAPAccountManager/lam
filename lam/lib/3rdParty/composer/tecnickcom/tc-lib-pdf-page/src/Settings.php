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
 * @phpstan-import-type MarginData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type RegionData from \Com\Tecnick\Pdf\Page\Box
 * @phpstan-import-type TransitionData from \Com\Tecnick\Pdf\Page\Box
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
     * True if we are in PDF/A mode.
     */
    protected bool $pdfa = false;

    /**
     * Enable stream compression.
     */
    protected bool $compress = true;

    /**
     * True if the signature approval is enabled (for incremental updates).
     */
    protected bool $sigapp = false;

    /**
     * Per-page flag indicating whether the page actually uses transparency.
     *
     * Maps a page index to true/false. When a page is flagged false the
     * per-page transparency /Group is omitted (a fully-opaque page does not
     * need it, and the group would otherwise force conforming interpreters onto
     * the slower transparency-compositing path). Unset pages default to emitting
     * the group, preserving backward-compatible output.
     *
     * @var array<int, bool>
     */
    protected array $pagetransparency = [];

    /**
     * Policy for emitting the per-page transparency /Group on standard pages:
     * 'auto' (only on pages flagged as using transparency, default), 'always'
     * (every standard page) or 'never'.
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
     * The value shall be a multiple of 90.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeRotation(array &$data): void
    {
        $data['rotation'] = empty($data['rotation']) || ($data['rotation'] % 90) !== 0 ? 0 : (int) $data['rotation'];
    }

    /**
     * Sanitize or set the page preferred zoom (magnification) factor.
     *
     * @param PageInputData $data Page data.
     */
    public function sanitizeZoom(array &$data): void
    {
        $data['zoom'] = empty($data['zoom']) ? 1.0 : $data['zoom'];
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

        // display duration before advancing page
        if (empty($transition['Dur'])) {
            unset($transition['Dur']);
        }

        if (!empty($transition['Dur'])) {
            $transition['Dur'] = \max(0, $transition['Dur']);
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

        // duration of the transition effect, in seconds
        $transition['D'] ??= 1;

        // dimension in which the specified transition effect shall occur
        if (
            empty($transition['Dm'])
            || !\in_array($transition['S'] ?? '', ['Split', 'Blinds'], true)
            || !\in_array($transition['Dm'], ['H', 'V'], true)
        ) {
            unset($transition['Dm']);
        }

        // direction of motion for the specified transition effect
        if (
            empty($transition['M'])
            || !\in_array($transition['S'] ?? '', ['Split', 'Box', 'Fly'], true)
            || !\in_array($transition['M'], ['I', 'O'], true)
        ) {
            unset($transition['M']);
        }

        // direction in which the specified transition effect shall moves
        if (
            empty($transition['Di'])
            || !\in_array($transition['S'] ?? '', ['Wipe', 'Glitter', 'Fly', 'Cover', 'Uncover', 'Push'], true)
            || !\in_array($transition['Di'], ['None', 0, 90, 180, 270, 315], true)
            || \in_array($transition['Di'], [90, 180], true) && ($transition['S'] ?? '') !== 'Wipe'
            || $transition['Di'] === 315 && ($transition['S'] ?? '') !== 'Glitter'
            || $transition['Di'] === 'None' && ($transition['S'] ?? '') !== 'Fly'
        ) {
            unset($transition['Di']);
        }

        // If true, the area that shall be flown in is rectangular and opaque
        $transition['B'] = !empty($transition['B']);
        $data['transition'] = $transition;
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
        if (empty($data['margin'])) {
            if (empty($data['width']) || empty($data['height'])) {
                [$data['width'], $data['height'], $data['orientation']] = $this->getPageFormatSize('A4', 'P');
                $data['width'] /= $this->kunit;
                $data['height'] /= $this->kunit;
                $dataWidth = $data['width'];
                $dataHeight = $data['height'];
            }
        }

        $margin = $this->normalizeMargins($marginData, $defmargin, $dataWidth, $dataHeight, $booklet);

        $data['margin'] = ['booklet' => $booklet] + $margin;

        $data['ContentWidth'] = $dataWidth - $margin['PL'] - $margin['PR'];
        $data['ContentHeight'] = $dataHeight - $margin['CT'] - $margin['CB'];
        $data['HeaderHeight'] = $margin['HB'] - $margin['PT'];
        $data['FooterHeight'] = $margin['FT'] - $margin['PB'];
    }

    /**
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
     * @param array<string, scalar|null> $marginData
     * @param MarginData $defaultMargins
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
     * @param array<string, scalar|null> $marginData
     * @param MarginData $defaultMargins
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
     * @param MarginData $margin
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
     * @param MarginData $margin
     *
     * @return MarginData
     */
    protected function applyBookletMarginSwap(array $margin, bool $booklet): array
    {
        if ($booklet && ($this->pid % 2) === 0) {
            // swap margins on odd pages
            // NOTE: $this->pid is the previous page (0 indexed).
            $tmp = $margin['PL'];
            $margin['PL'] = $margin['PR'];
            $margin['PR'] = $tmp;
        }

        return $margin;
    }

    /**
     * @param MarginData $margin
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
     * @param MarginData $margin
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
        $contentWidth = (float) ($data['ContentWidth'] ?? 0);
        $contentHeight = (float) ($data['ContentHeight'] ?? 0);
        $pageWidth = (float) ($data['width'] ?? 0);
        $pageHeight = (float) ($data['height'] ?? 0);
        $marginLeft = (float) ($data['margin']['PL'] ?? 0);
        $marginTop = (float) ($data['margin']['CT'] ?? 0);
        $marginRight = (float) ($data['margin']['PR'] ?? 0);
        $marginBottom = (float) ($data['margin']['CB'] ?? 0);

        if (!empty($data['columns'])) {
            // set equal columns
            $data['region'] = [];
            $numColumns = \max(1, (int) $data['columns']);
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

        $regions = $data['region'] ?? [];
        $columnCount = 0;
        foreach ($regions as $key => $val) {
            $regions[$key] = $this->normalizeRegionData(
                $val,
                $contentWidth,
                $contentHeight,
                $pageWidth,
                $pageHeight,
                $marginRight,
                $marginBottom,
            );
            ++$columnCount;
        }

        $data['region'] = $regions; // @phpstan-ignore parameterByRef.type
        $data['columns'] = $columnCount; // @phpstan-ignore parameterByRef.type

        if (!\array_key_exists('autobreak', $data)) {
            $data['autobreak'] = true; // @phpstan-ignore parameterByRef.type
        }
    }

    /**
     * @param array<string, float|int> $region
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

        $rw = \min(\max(0.0, $regionWidth), $contentWidth);
        $rx = \min(\max(0.0, $regionX), $pageWidth - $marginRight - $regionWidth);
        $rl = $regionX + $regionWidth;
        $rr = $pageWidth - $regionX - $regionWidth;
        $rh = \min(\max(0.0, $regionHeight), $contentHeight);
        $ry = \min(\max(0.0, $regionY), $pageHeight - $marginBottom - $regionHeight);
        $rt = $regionY + $regionHeight;
        $rb = $pageHeight - $regionY - $regionHeight;

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
        $box = $data['box'] ?? [];

        if (empty($box)) {
            if (empty($data['pwidth']) || empty($data['pheight'])) {
                [$data['pwidth'], $data['pheight'], $data['orientation']] = $this->getPageFormatSize('A4', 'P');
            }

            $box = $this->setPageBoxes($data['pwidth'] ?? 0, $data['pheight'] ?? 0);
        }

        if (!empty($box)) {
            if (\array_key_exists('format', $data) && $data['format'] !== '' && $data['format'] === 'MediaBox') {
                $data['format'] = '';
                $data['width'] = \abs(($box['MediaBox']['urx'] ?? 0) - ($box['MediaBox']['llx'] ?? 0)) / $this->kunit;
                $data['height'] = \abs(($box['MediaBox']['ury'] ?? 0) - ($box['MediaBox']['lly'] ?? 0)) / $this->kunit;
                $this->sanitizePageFormat($data);
            }

            if (empty($box['MediaBox'])) {
                $box = $this->setBox($box, 'MediaBox', 0, 0, $data['pwidth'] ?? 0, $data['pheight'] ?? 0);
            }

            $box = $this->inheritMissingBox($box, 'CropBox', 'MediaBox');
            $box = $this->inheritMissingBox($box, 'BleedBox', 'CropBox');
            $box = $this->inheritMissingBox($box, 'TrimBox', 'CropBox');
            $box = $this->inheritMissingBox($box, 'ArtBox', 'CropBox');
        }

        $data['box'] = $box;

        $orientation = $this->getPageOrientation(
            \abs(($box['MediaBox']['urx'] ?? 0) - ($box['MediaBox']['llx'] ?? 0)),
            \abs(($box['MediaBox']['ury'] ?? 0) - ($box['MediaBox']['lly'] ?? 0)),
        );
        if (empty($data['orientation'])) {
            $data['orientation'] = $orientation;
        }

        if (($data['orientation'] ?? '') !== $orientation) {
            $data['box'] = $this->swapCoordinates($box);
        }
    }

    /**
     * @param array<string, PageBox> $box
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
     * @param PageInputData $data Page data.
     *
     * @throws Exception
     */
    public function sanitizePageFormat(array &$data): void
    {
        $this->ensurePageOrientation($data);

        if (!empty($data['format'])) {
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
     * @param PageInputData $data
     */
    protected function ensurePageOrientation(array &$data): void
    {
        if (empty($data['orientation'])) {
            $data['orientation'] = '';
        }
    }

    /**
     * @param PageInputData $data
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
     * @param PageInputData $data
     *
     * @throws Exception
     */
    protected function applyCustomPageFormat(array &$data): void
    {
        $data['format'] = 'CUSTOM';
        if (empty($data['width']) || empty($data['height'])) {
            // default page format
            $data['format'] = 'A4';
            $data['orientation'] = 'P';
            $this->sanitizePageFormat($data);
            return;
        }

        [$data['width'], $data['height'], $data['orientation']] = $this->getPageOrientedSize(
            $data['width'],
            $data['height'],
            $data['orientation'] ?? 'P',
        );
    }
}
