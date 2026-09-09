<?php

declare(strict_types=1);

/**
 * Box.php
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
use Com\Tecnick\Pdf\Page\Exception as PageException;

/**
 * Com\Tecnick\Pdf\Page\Box
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfPage
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-page
 *
 * @phpstan-type PageBci array{
 *            'color': string,
 *            'width': float,
 *            'style': string,
 *            'dash': array<float|int>,
 *          }
 *
 * @phpstan-type PageBox array{
 *            'llx': float,
 *            'lly': float,
 *            'urx': float,
 *            'ury': float,
 *            'bci'?: PageBci,
 *          }
 *
 * @phpstan-type PageBciInput array{
 *            'color'?: string,
 *            'width'?: float,
 *            'style'?: string,
 *            'dash'?: array<float|int>,
 *          }
 *
 * @phpstan-type PageBoxInput array{
 *            'llx'?: float,
 *            'lly'?: float,
 *            'urx'?: float,
 *            'ury'?: float,
 *            'bci'?: PageBciInput,
 *          }
 *
 * @phpstan-type PageDataBox array{
 *            'llx': float,
 *            'lly': float,
 *            'urx': float,
 *            'ury': float,
 *            'bci': PageBci,
 *          }
 *
 * @phpstan-type MarginData array{
 *            'CB': float,
 *            'CT': float,
 *            'FT': float,
 *            'HB': float,
 *            'PB': float,
 *            'PL': float,
 *            'PR': float,
 *            'PT': float,
 *        }
 *
 * @phpstan-type PageMarginData array{
 *            'booklet': bool,
 *            'CB': float,
 *            'CT': float,
 *            'FT': float,
 *            'HB': float,
 *            'PB': float,
 *            'PL': float,
 *            'PR': float,
 *            'PT': float,
 *        }
 *
 * @phpstan-type RegionData array{
 *            'RB': float,
 *            'RH': float,
 *            'RL': float,
 *            'RR': float,
 *            'RT': float,
 *            'RW': float,
 *            'RX': float,
 *            'RY': float,
 *            'x' : float,
 *            'y' : float,
 *        }
 *
 * @phpstan-type NoWriteArea array{
 *            'side'?: string,
 *            'xt'?: float,
 *            'yt'?: float,
 *            'xb'?: float,
 *            'yb'?: float,
 *            'x'?: float,
 *            'y'?: float,
 *            'w'?: float,
 *            'h'?: float,
 *        }
 *
 *
 * @phpstan-type TransitionInput array{
 *            'B'?: bool,
 *            'D'?: int,
 *            'Di'?: string|int,
 *            'Dm'?: string,
 *            'Dur'?: float,
 *            'M'?: string,
 *            'S'?: string,
 *            'SS'?: float,
 *        }
 *
 * @phpstan-type TransitionData array{
 *            'B': bool,
 *            'D': int,
 *            'Di': string|int,
 *            'Dm': string,
 *            'Dur': float,
 *            'M': string,
 *            'S': string,
 *            'SS': float,
 *        }
 *
 * @phpstan-type PageData array{
 *        'annotrefs': array<int,int>,
 *        'autobreak': bool,
 *        'box': array<string, PageDataBox>,
 *        'columns': int,
 *        'content': array<string>,
 *        'content_mark': array<int>,
 *        'ContentHeight': float,
 *        'ContentWidth': float,
 *        'FooterHeight': float,
 *        'HeaderHeight': float,
 *        'currentRegion': int,
 *        'format': string,
 *        'group': int,
 *        'height': float,
 *        'margin': PageMarginData,
 *        'n': int,
 *        'num': int,
 *        'orientation': string,
 *        'pagenum': int,
 *        'pheight': float,
 *        'pid': int,
 *        'pwidth': float,
 *        'region': array<int, RegionData>,
 *        'rotation': int,
 *        'time': int,
 *        'transition'?: TransitionData,
 *        'width': float,
 *        'zoom': float,
 *    }
 *
 * @phpstan-type PageInputData array{
 *        'annotrefs'?: array<int,int>,
 *        'autobreak'?: bool,
 *        'box'?: array<string, PageBoxInput>,
 *        'columns'?: int,
 *        'content'?: array<string>|string,
 *        'content_mark'?: array<int>,
 *        'ContentHeight'?: float,
 *        'ContentWidth'?: float,
 *        'FooterHeight'?: float,
 *        'HeaderHeight'?: float,
 *        'currentRegion'?: int,
 *        'format'?: string,
 *        'group'?: int,
 *        'height'?: float,
 *        'margin'?: array{
 *            'booklet'?: bool,
 *            'CB'?: float,
 *            'CT'?: float,
 *            'FT'?: float,
 *            'HB'?: float,
 *            'PB'?: float,
 *            'PL'?: float,
 *            'PR'?: float,
 *            'PT'?: float,
 *        },
 *        'n'?: int,
 *        'num'?: int,
 *        'orientation'?: string,
 *        'pagenum'?: int,
 *        'pheight'?: float,
 *        'pid'?: int,
 *        'pwidth'?: float,
 *        'region'?: array<int, array{
 *            'RB'?: float,
 *            'RH'?: float,
 *            'RL'?: float,
 *            'RR'?: float,
 *            'RT'?: float,
 *            'RW'?: float,
 *            'RX'?: float,
 *            'RY'?: float,
 *            'x' ?: float,
 *            'y' ?: float,
 *        }>,
 *        'rotation'?: int,
 *        'time'?: int,
 *        'transition'?: TransitionInput,
 *        'width'?: float,
 *        'zoom'?: float,
 *    }
 */
abstract class Box extends \Com\Tecnick\Pdf\Page\Mode
{
    /**
     * Unit of measure conversion ratio.
     */
    protected float $kunit = 1.0;

    /**
     * Color object.
     */
    protected Color $col;

    /**
     * Memoized PDF RGB components, keyed by the color representation.
     *
     * @var array<string, string>
     */
    private array $rgbcomponents = [];

    /**
     * Page box names.
     *
     * @var array<string>
     */
    public const BOX = [
        'MediaBox',
        'CropBox',
        'BleedBox',
        'TrimBox',
        'ArtBox',
    ];

    /**
     * Swap X and Y coordinates of page boxes (change page boxes orientation).
     *
     * @param array<string, PageBox> $dims Array of page dimensions.
     *
     * @return array<string, PageBox> Page dimensions.
     */
    public function swapCoordinates(array $dims): array
    {
        foreach (self::BOX as $type) {
            if (!array_key_exists($type, $dims)) {
                continue;
            }

            $llx = $dims[$type]['llx'];
            $lly = $dims[$type]['lly'];
            $urx = $dims[$type]['urx'];
            $ury = $dims[$type]['ury'];

            $dims[$type]['llx'] = $lly;
            $dims[$type]['lly'] = $llx;
            $dims[$type]['urx'] = $ury;
            $dims[$type]['ury'] = $urx;
        }

        return $dims;
    }

    /**
     * Set page boundaries.
     *
     * @param array<string, PageBox> $dims Array of page dimensions to modify.
     * @param string|PageBoxType     $type Box type: MediaBox, CropBox, BleedBox, TrimBox, ArtBox, or enum case.
     * @param float                  $llx  Lower-left x coordinate in user units.
     * @param float                  $lly  Lower-left y coordinate in user units.
     * @param float                  $urx  Upper-right x coordinate in user units.
     * @param float                  $ury  Upper-right y coordinate in user units.
     * @param ?PageBciInput          $bci  BoxColorInfo: guideline style (color, width, style, dash).
     *                                     Missing entries are filled with the default style.
     *
     * @return array<string, PageBox> Page dimensions.
     *
     * @throws PageException
     */
    public function setBox(
        array $dims,
        string|PageBoxType $type,
        float $llx,
        float $lly,
        float $urx,
        float $ury,
        ?array $bci = null,
    ): array {
        if ($type instanceof PageBoxType) {
            $type = $type->value;
        }

        if (!\in_array($type, self::BOX, true)) {
            throw new PageException('unknown page box type: ' . $type);
        }

        $dims[$type] = $this->orderBoxCorners([
            'llx' => $llx,
            'lly' => $lly,
            'urx' => $urx,
            'ury' => $ury,
        ]);
        $dims[$type]['bci'] = $this->normalizeBoxColorInfo($bci);

        return $dims;
    }

    /**
     * Order the two corners of a page box so that llx <= urx and lly <= ury.
     *
     * @param PageBox $box Page box.
     *
     * @return PageBox Page box with ordered corners.
     */
    protected function orderBoxCorners(array $box): array
    {
        if ($box['llx'] > $box['urx']) {
            [$box['llx'], $box['urx']] = [$box['urx'], $box['llx']];
        }

        if ($box['lly'] > $box['ury']) {
            [$box['lly'], $box['ury']] = [$box['ury'], $box['lly']];
        }

        return $box;
    }

    /**
     * Clamp a page box inside the given bounding box.
     *
     * @param PageDataBox $box    Page box to clamp.
     * @param PageDataBox $bounds Bounding box.
     *
     * @return PageDataBox Clamped page box.
     */
    protected function clampBox(array $box, array $bounds): array
    {
        $box['llx'] = \min(\max($box['llx'], $bounds['llx']), $bounds['urx']);
        $box['urx'] = \min(\max($box['urx'], $bounds['llx']), $bounds['urx']);
        $box['lly'] = \min(\max($box['lly'], $bounds['lly']), $bounds['ury']);
        $box['ury'] = \min(\max($box['ury'], $bounds['lly']), $bounds['ury']);

        return $box;
    }

    /**
     * Clamp every page box other than the MediaBox inside the MediaBox.
     * Nothing is rendered outside the MediaBox and the reader intersects the other
     * boxes with it.
     *
     * @param array<string, PageDataBox> $box Page boxes.
     *
     * @return array<string, PageDataBox> Page boxes.
     */
    protected function clampBoxesToMediaBox(array $box): array
    {
        $media = $box['MediaBox'] ?? null;
        if ($media === null) {
            // @codeCoverageIgnoreStart
            return $box;

            // @codeCoverageIgnoreEnd
        }

        foreach ($box as $type => $dims) {
            if ($type === 'MediaBox') {
                continue;
            }

            $box[$type] = $this->clampBox($dims, $media);
        }

        return $box;
    }

    /**
     * Returns the default BoxColorInfo guideline style.
     *
     * @return PageBci Default BoxColorInfo.
     */
    protected function getDefaultBoxColorInfo(): array
    {
        return [
            'color' => '#000000',
            'width' => 1.0 / $this->kunit,
            'style' => 'S', // S = solid; D = dash
            'dash' => [3],
        ];
    }

    /**
     * Fill the missing entries of a BoxColorInfo with the default guideline style.
     *
     * @param ?PageBciInput $bci BoxColorInfo to complete, or null for the default one.
     *
     * @return PageBci Complete BoxColorInfo.
     */
    protected function normalizeBoxColorInfo(?array $bci): array
    {
        $def = $this->getDefaultBoxColorInfo();
        if ($bci === null) {
            return $def;
        }

        return [
            'color' => $bci['color'] ?? $def['color'],
            // A width of 0 hides the guideline: it is a value, not a missing entry.
            'width' => \max(0.0, $bci['width'] ?? $def['width']),
            'style' => $bci['style'] ?? $def['style'],
            'dash' => $bci['dash'] ?? $def['dash'],
        ];
    }

    /**
     * Complete the caller-supplied page boxes with the shape the output methods expect:
     * every box gets four coordinates and a full BoxColorInfo.
     *
     * @param array<string, PageBoxInput> $box Page boxes.
     *
     * @return array<string, PageDataBox> Page boxes.
     *
     * @throws PageException
     */
    protected function normalizeBoxes(array $box): array
    {
        $out = [];
        foreach ($box as $type => $dims) {
            if (!\in_array($type, self::BOX, true)) {
                throw new PageException('unknown page box type: ' . $type);
            }

            $ordered = $this->orderBoxCorners([
                'llx' => $dims['llx'] ?? 0.0,
                'lly' => $dims['lly'] ?? 0.0,
                'urx' => $dims['urx'] ?? 0.0,
                'ury' => $dims['ury'] ?? 0.0,
            ]);

            $out[$type] = [
                'llx' => $ordered['llx'],
                'lly' => $ordered['lly'],
                'urx' => $ordered['urx'],
                'ury' => $ordered['ury'],
                'bci' => $this->normalizeBoxColorInfo($dims['bci'] ?? null),
            ];
        }

        return $out;
    }

    /**
     * Initialize page boxes.
     *
     * @param float $width  Page width in points.
     * @param float $height Page height in points.
     *
     * @return array<string, PageBox> Page boxes.
     *
     * @throws PageException
     */
    public function setPageBoxes(float $width, float $height): array
    {
        $dims = [];
        foreach (self::BOX as $type) {
            $dims = $this->setBox($dims, $type, 0, 0, $width, $height);
        }

        return $dims;
    }

    /**
     * Returns the PDF command to output the specified page boxes.
     *
     * @param array<string, array{
     *            'llx': float,
     *            'lly': float,
     *            'urx': float,
     *            'ury': float,
     *          }> $dims Array of page dimensions.
     */
    protected function getBox(array $dims): string
    {
        $out = '';
        foreach (self::BOX as $box) {
            if (empty($dims[$box])) {
                // @codeCoverageIgnoreStart
                continue;

                // @codeCoverageIgnoreEnd
            }

            $out .= \sprintf(
                '/%s [%F %F %F %F]' . "\n",
                $box,
                $dims[$box]['llx'],
                $dims[$box]['lly'],
                $dims[$box]['urx'],
                $dims[$box]['ury'],
            );
        }

        return $out;
    }

    /**
     * Returns the DeviceRGB components of a color, or an empty string if the
     * color cannot be resolved.
     *
     * The lookup resolves spot colors without registering them, so it adds no
     * Separation color space to the document resources. Results are memoized per
     * color representation: a spot color redefined after the first lookup keeps
     * its previous components.
     *
     * @param string $color HTML, CSS or Spot color to parse.
     */
    protected function getRgbComponents(string $color): string
    {
        return $this->rgbcomponents[$color] ??= $this->col->getPdfRgbComponents($color);
    }

    /**
     * Returns the PDF command to output the specified page BoxColorInfo.
     *
     * @param array<string, array{
     *            'bci': PageBci,
     *          }> $dims Array of page dimensions.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     */
    protected function getBoxColorInfo(array $dims): string
    {
        $out = '/BoxColorInfo <<' . "\n";
        foreach (self::BOX as $box) {
            if (!array_key_exists($box, $dims)) {
                // The box is not part of this page dictionary.
                continue;
            }

            $out .= '/' . $box . ' <<' . "\n";
            $color = empty($dims[$box]['bci']['color']) ? '' : $this->getRgbComponents($dims[$box]['bci']['color']);
            if ($color !== '') {
                $out .= '/C [' . $color . ']' . "\n";
            }

            // A width of 0 hides the guideline and is emitted: omitting the entry
            // would let the reader apply the default width of 1.
            $width = $dims[$box]['bci']['width'];
            $out .= \sprintf('/W %F' . "\n", \max(0.0, $width) * $this->kunit);

            if (!empty($dims[$box]['bci']['style'])) {
                $mode = \strtoupper($dims[$box]['bci']['style'][0]);
                if ($mode !== 'D') {
                    $mode = 'S';
                }

                $out .= '/S /' . $mode . "\n";
            }

            if (!empty($dims[$box]['bci']['dash'])) {
                $out .= '/D [';
                foreach ($dims[$box]['bci']['dash'] as $dash) {
                    $out .= \sprintf(' %F', (float) $dash * $this->kunit);
                }

                $out .= ' ]' . "\n";
            }

            $out .= '>>' . "\n";
        }

        return $out . ('>>' . "\n");
    }
}
