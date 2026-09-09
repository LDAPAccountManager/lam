<?php

declare(strict_types=1);

/**
 * Gradient.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * This file is part of tc-lib-pdf-graph software library.
 */

namespace Com\Tecnick\Pdf\Graph;

use Com\Tecnick\Color\ColorModelType;
use Com\Tecnick\Color\Model as ColorModel;
use Com\Tecnick\Pdf\Graph\Exception as GraphException;

/**
 * Com\Tecnick\Pdf\Graph\Gradient
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfGraph
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-graph
 *
 * @phpstan-import-type GradientData from \Com\Tecnick\Pdf\Graph\Base
 * @phpstan-import-type StyleDataOpt from \Com\Tecnick\Pdf\Graph\Base
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Gradient extends \Com\Tecnick\Pdf\Graph\Raw
{
    /**
     * Returns the gradients array
     *
     * @return array<int, GradientData>
     */
    public function getGradientsArray(): array
    {
        return $this->gradients;
    }

    /**
     * Draws a basic rectangle
     *
     * @param float              $posx   Abscissa of upper-left corner.
     * @param float              $posy   Ordinate of upper-left corner.
     * @param float              $width  Width.
     * @param float              $height Height.
     * @param string|PathPaintOp $mode   Mode of rendering. @see getPathPaintOp()
     * @param StyleDataOpt       $style  Style.
     *
     * @return string PDF command
     */
    public function getBasicRect(
        float $posx,
        float $posy,
        float $width,
        float $height,
        string|PathPaintOp $mode = 'S',
        array $style = [],
    ): string {
        return $this->getStyleCmd($style) . $this->getRawRect($posx, $posy, $width, $height, $mode);
    }

    /**
     * Get a linear colour gradient command.
     *
     * @param float        $posx       Abscissa of the top left corner of the rectangle.
     * @param float        $posy       Ordinate of the top left corner of the rectangle.
     * @param float        $width      Width of the rectangle.
     * @param float        $height     Height of the rectangle.
     * @param string       $colorstart Starting color.
     * @param string       $colorend   Ending color.
     * @param array<float> $coords     Gradient vector (x1, y1, x2, y2).
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getLinearGradient(
        float $posx,
        float $posy,
        float $width,
        float $height,
        string $colorstart,
        string $colorend,
        array $coords = [0, 0, 1, 0],
    ): string {
        return $this->getTwoStopGradient(2, $posx, $posy, $width, $height, $colorstart, $colorend, $coords);
    }

    /**
     * Get a radial colour gradient command.
     *
     * @param float        $posx       Abscissa of the top left corner of the rectangle.
     * @param float        $posy       Ordinate of the top left corner of the rectangle.
     * @param float        $width      Width of the rectangle.
     * @param float        $height     Height of the rectangle.
     * @param string       $colorstart Starting color.
     * @param string       $colorend   Ending color.
     * @param array<float> $coords     Array of the form (fx, fy, cx, cy, r) where
     *                                 (fx, fy) is the starting point of the
     *                                 gradient with $colorstart (which should be
     *                                 inside the circle), (cx, cy) is the center of the
     *                                 circle with $colorend, and r is the radius
     *                                 of the circle.
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getRadialGradient(
        float $posx,
        float $posy,
        float $width,
        float $height,
        string $colorstart,
        string $colorend,
        array $coords = [0.5, 0.5, 0.5, 0.5, 1],
    ): string {
        return $this->getTwoStopGradient(3, $posx, $posy, $width, $height, $colorstart, $colorend, $coords);
    }

    /**
     * Get a two-stop colour gradient command clipped to a rectangular area.
     *
     * @param int          $type       Shading type: 2 = axial, 3 = radial.
     * @param float        $posx       Abscissa of the top left corner of the rectangle.
     * @param float        $posy       Ordinate of the top left corner of the rectangle.
     * @param float        $width      Width of the rectangle.
     * @param float        $height     Height of the rectangle.
     * @param string       $colorstart Starting color.
     * @param string       $colorend   Ending color.
     * @param array<float> $coords     Gradient coordinates.
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     */
    private function getTwoStopGradient(
        int $type,
        float $posx,
        float $posy,
        float $width,
        float $height,
        string $colorstart,
        string $colorend,
        array $coords,
    ): string {
        return (
            $this->getStartTransform()
            . $this->getClippingRect($posx, $posy, $width, $height)
            . $this->getGradientTransform($posx, $posy, $width, $height)
            . $this->getGradient(
                $type,
                $coords,
                [
                    [
                        'color' => $colorstart,
                        'offset' => 0.0,
                        'opacity' => 1.0,
                    ],
                    [
                        'color' => $colorend,
                        'exponent' => 1.0,
                        'offset' => 1.0,
                        'opacity' => 1.0,
                    ],
                ],
                '',
                false,
            )
            . $this->getStopTransform()
        );
    }

    /**
     * Rectangular clipping area.
     *
     * @param float $posx   Abscissa of the top left corner of the rectangle.
     * @param float $posy   Ordinate of the top left corner of the rectangle.
     * @param float $width  Width of the rectangle.
     * @param float $height Height of the rectangle.
     * @param bool  $eoclip If true, set clipping path using even-odd rule.
     *
     * @return string PDF command
     */
    public function getClippingRect(float $posx, float $posy, float $width, float $height, bool $eoclip = false): string
    {
        $mode = $eoclip ? 'CEO' : 'CNZ';
        return $this->getRawRect($posx, $posy, $width, $height, $mode);
    }

    /**
     * Returns the transformation command that maps a gradient onto a rectangular area.
     *
     * @param float $posx   Abscissa of the top left corner of the rectangle.
     * @param float $posy   Ordinate of the top left corner of the rectangle.
     * @param float $width  Width of the rectangle.
     * @param float $height Height of the rectangle.
     *
     * @return string PDF command
     */
    public function getGradientTransform(float $posx, float $posy, float $width, float $height): string
    {
        $ctm = [
            $width * $this->kunit,
            0,
            0,
            $height * $this->kunit,
            $posx * $this->kunit,
            ($this->pageh - ($posy + $height)) * $this->kunit,
        ];
        return $this->getTransformation($ctm);
    }

    /**
     * Get a color gradient PDF command.
     * When a stop is not fully opaque the command also selects the soft-mask ExtGState
     * of the gradient, so the caller is responsible for wrapping it between "q" and "Q".
     *
     * @param int          $type      Type of gradient: 2 = Axial shading; 3 = Radial shading.
     * @param array<float> $coords    Array of coordinates:
     *                                (x0, y0, x1, y1) for type 2,
     *                                (fx, fy, cx, cy, r) for type 3.
     * @param array<int, array{
     *            'color': string,
     *            'exponent'?: float,
     *            'opacity'?: float,
     *            'offset'?: float,
     *        }>          $stops     Gradient color stops (at least two entries):
     *                                color = color;
     *                                offset = (0 to 1) location along the gradient vector;
     *                                opacity = (0 to 1) stop opacity;
     *                                exponent = exponent of the exponential interpolation function
     *                                (default = 1) used between the previous stop and this one,
     *                                so the value set on the first stop is unused.
     * @param string       $bgcolor   Background color.
     * @param bool         $antialias Flag indicating whether to filter the
     *                                shading function to prevent aliasing artifacts.
     *
     * @return string PDF command
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getGradient(
        int $type,
        array $coords,
        array $stops,
        string $bgcolor,
        bool $antialias = false,
    ): string {
        $mincoords = match ($type) {
            2 => 4,
            3 => 5,
            default => throw new GraphException('Unsupported gradient type: ' . $type),
        };

        if (\count($coords) < $mincoords) {
            throw new GraphException('A type ' . $type . ' gradient requires ' . $mincoords . ' coordinates');
        }

        $stops = \array_values($stops);
        if (\count($stops) < 2) {
            throw new GraphException('A gradient requires at least two color stops');
        }

        foreach ($stops as $stop) {
            if (!$this->pdfColor->getColorObject($stop['color']) instanceof ColorModel) {
                throw new GraphException('Invalid color: ' . $stop['color']);
            }
        }

        // the shading color space is the one of the first stop
        $model = $this->pdfColor->getColorObject($stops[0]['color']);
        if (!$model instanceof ColorModel) {
            throw new GraphException('Invalid color: ' . $stops[0]['color']);
        }

        $background = $this->pdfColor->getColorObject($bgcolor);
        if ($bgcolor !== '' && !$background instanceof ColorModel) {
            throw new GraphException('Invalid background color: ' . $bgcolor);
        }

        $colspace = match (ColorModelType::tryFrom($model->getType())) {
            ColorModelType::Cmyk => 'DeviceCMYK',
            ColorModelType::Gray => 'DeviceGray',
            default => 'DeviceRGB',
        };

        $ngr = $this->getNextGradientKey();
        $this->gradients[$ngr] = $this->getGradientStops([
            'antialias' => $antialias,
            'background' => $background,
            'colors' => [],
            'colspace' => $colspace,
            'coords' => $coords,
            'id' => 0,
            'pattern' => 0,
            'stream' => '',
            'transparency' => false,
            'type' => $type,
        ], $stops);

        $out = '';
        if ($this->gradients[$ngr]['transparency']) {
            // paint luminosity gradient
            $out .= '/TGS' . $ngr . ' gs' . "\n";
        }

        // paint the gradient
        $out .= '/Sh' . $ngr . ' sh' . "\n";

        return $out;
    }

    /**
     * Returns the key of the last registered gradient, to be used with XObjects.
     * This is the key accepted by getOutGradientResourcesByKeys(), not the PDF object number.
     *
     * @return ?int
     */
    public function getLastGradientID(): ?int
    {
        return \array_key_last($this->gradients);
    }

    /**
     * Returns the next free key of the gradients array.
     */
    private function getNextGradientKey(): int
    {
        $lastkey = \array_key_last($this->gradients);
        return $lastkey === null ? 1 : $lastkey + 1;
    }

    /**
     * Process the gradient stops.
     *
     * @param GradientData $grad Array containing gradient info
     * @param array<int, array{
     *            'color': string,
     *            'exponent'?: float,
     *            'opacity'?: float,
     *            'offset'?: float,
     *        }>          $stops Gradient color stops:
     *                            color = color;
     *                            offset = (0 to 1) location along the gradient vector;
     *                            opacity = (0 to 1) stop opacity;
     *                            exponent = exponent of the exponential interpolation function
     *                            (default = 1) used between the previous stop and this one,
     *                            so the value set on the first stop is unused.
     *
     * @return GradientData Gradient array.
     */
    protected function getGradientStops(array $grad, array $stops): array
    {
        // the offset interpolation requires contiguous keys starting at zero
        $stops = \array_values($stops);
        $num_stops = \count($stops);
        $last_stop_id = $num_stops - 1;
        $prevoffset = 0.0;

        foreach ($stops as $key => $stop) {
            $grad['colors'][$key] = ['color' => $stop['color']];
            $grad['colors'][$key]['exponent'] = 1.0;
            if (array_key_exists('exponent', $stop)) {
                // exponent for the interpolation function
                $grad['colors'][$key]['exponent'] = $stop['exponent'];
            }

            // A stop opacity is painted through a soft mask, so it is dropped when the
            // conformance mode forbids transparency: the shading stays, fully opaque.
            $grad['colors'][$key]['opacity'] = 1.0;
            if (array_key_exists('opacity', $stop) && !$this->notransparency) {
                // the opacity is a DeviceGray component of the soft mask shading
                $opacity = \max(0.0, \min(1.0, $stop['opacity']));
                $grad['colors'][$key]['opacity'] = $opacity;
                $grad['transparency'] = $grad['transparency'] || $opacity < 1.0;
            }

            // offset represents a location along the gradient vector
            $offset = array_key_exists('offset', $stop) ? $stop['offset'] : null;
            if ($offset === null) {
                $offset = match ($key) {
                    0 => 0.0,
                    $last_stop_id => 1.0,
                    default => $prevoffset + ((1.0 - $prevoffset) / ($num_stops - $key)),
                };
            }

            // the offsets are the /Bounds of the stitching function:
            // they stay within the [0, 1] domain and never decrease
            $offset = \max($prevoffset, \min(1.0, $offset));
            $grad['colors'][$key]['offset'] = $offset;
            $prevoffset = $offset;
        }

        return $grad;
    }

    /**
     * Paints a coons patch mesh.
     *
     * @param float        $posx       Abscissa of the top left corner of the rectangle.
     * @param float        $posy       Ordinate of the top left corner of the rectangle.
     * @param float        $width      Width of the rectangle.
     * @param float        $height     Height of the rectangle.
     * @param string       $colll      Lower-Left corner color.
     * @param string       $collr      Lower-Right corner color.
     * @param string       $colur      Upper-Right corner color.
     * @param string       $colul      Upper-Left corner color.
     * @param array<float> $coords     Coordinates of the 12 Bezier control points of the patch.
     * @param float        $coords_min Minimum value used by the coordinates:
     *                                 smaller values are cut to coords_min.
     * @param float        $coords_max Maximum value used by the coordinates:
     *                                 greater values are cut to coords_max.
     * @param bool         $antialias  Flag indicating whether to filter the
     *                                 shading function to prevent aliasing artifacts.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getCoonsPatchMeshWithCoords(
        float $posx,
        float $posy,
        float $width,
        float $height,
        string $colll = 'yellow',
        string $collr = 'blue',
        string $colur = 'green',
        string $colul = 'red',
        array $coords = [
            0.00,
            0.00,
            0.33,
            0.00,
            0.67,
            0.00,
            1.00,
            0.00,
            1.00,
            0.33,
            1.00,
            0.67,
            1.00,
            1.00,
            0.67,
            1.00,
            0.33,
            1.00,
            0.00,
            1.00,
            0.00,
            0.67,
            0.00,
            0.33,
        ],
        float $coords_min = 0.0,
        float $coords_max = 1.0,
        bool $antialias = false,
    ): string {
        // convert the simple array to a multi patch array
        $patch_array = [
            0 => [
                'f' => 0,
                'points' => $coords,
                'colors' => [],
            ],
        ];

        $colllobj = $this->pdfColor->getColorObject($colll);
        if (!$colllobj instanceof ColorModel) {
            throw new GraphException('Invalid Lower-Left corner color');
        }

        $patch_array[0]['colors'][0] = $colllobj->toRgbArray();

        $collrobj = $this->pdfColor->getColorObject($collr);
        if (!$collrobj instanceof ColorModel) {
            throw new GraphException('Invalid Lower-Right corner color');
        }

        $patch_array[0]['colors'][1] = $collrobj->toRgbArray();

        $colurobj = $this->pdfColor->getColorObject($colur);
        if (!$colurobj instanceof ColorModel) {
            throw new GraphException('Invalid Upper-Right corner color');
        }

        $patch_array[0]['colors'][2] = $colurobj->toRgbArray();

        $colulobj = $this->pdfColor->getColorObject($colul);
        if (!$colulobj instanceof ColorModel) {
            throw new GraphException('Invalid Upper-Left corner color');
        }

        $patch_array[0]['colors'][3] = $colulobj->toRgbArray();

        return $this->getCoonsPatchMesh(
            $posx,
            $posy,
            $width,
            $height,
            $patch_array,
            $coords_min,
            $coords_max,
            $antialias,
        );
    }

    /**
     * Paints a coons patch mesh.
     *
     * @param float        $posx       Abscissa of the top left corner of the rectangle.
     * @param float        $posy       Ordinate of the top left corner of the rectangle.
     * @param float        $width      Width of the rectangle.
     * @param float        $height     Height of the rectangle.
     * @param array<array{
     *            'f': int,
     *            'points': array<float>,
     *            'colors': array<int, array<string, float>>,
     *        }>          $patch_array One entry for each patch:
     *                                 f = position of the patch
     *                                 (0 = first patch; 1, 2, 3 = right, top and left);
     *                                 points = pairs of coordinates (normally from 0 to 1)
     *                                 of the Bezier control points that define the patch:
     *                                 12 pairs for the first patch, 8 pairs for the following
     *                                 ones, ignoring the coordinates already defined by the
     *                                 preceding patch. The first pair is the lower left edge
     *                                 point, the next is its right control point (control
     *                                 point 2), then the other points follow counter-clockwise
     *                                 around the patch in the order: control point 1, edge
     *                                 point, control point 2. The last pair is the left control
     *                                 point (control point 1) of the first edge point;
     *                                 colors = 4 colors for the first patch,
     *                                 2 colors for the following ones.
     * @param float        $coords_min Minimum value used by the coordinates:
     *                                 smaller values are cut to coords_min.
     * @param float        $coords_max Maximum value used by the coordinates:
     *                                 greater values are cut to coords_max.
     * @param bool         $antialias  Flag indicating whether to filter the
     *                                 shading function to prevent aliasing artifacts.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.ExcessiveParameterList")
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getCoonsPatchMesh(
        float $posx,
        float $posy,
        float $width,
        float $height,
        array $patch_array = [],
        float $coords_min = 0.0,
        float $coords_max = 1.0,
        bool $antialias = false,
    ): string {
        if ($coords_max <= $coords_min) {
            throw new GraphException('coords_max must be greater than coords_min');
        }

        $ngr = $this->getNextGradientKey();
        $this->gradients[$ngr] = [
            'antialias' => $antialias,
            'colors' => [],
            'background' => null,
            'colspace' => 'DeviceRGB',
            'coords' => [],
            'id' => 0,
            'pattern' => 0,
            'stream' => '',
            'transparency' => false,
            'type' => 6, //coons patch mesh
        ];

        $bpcd = 65_535; // 16 bits per coordinate
        $span = $coords_max - $coords_min;
        $stream = '';

        foreach ($patch_array as $par) {
            $stream .= \chr($par['f'] & 0xFF); // start with the edge flag as 8 bit
            foreach ($par['points'] as $point) {
                // each point as 16 bit
                $val = (int) \max(0, \min($bpcd, (($point - $coords_min) / $span) * $bpcd));
                $stream .= \chr(\intdiv($val, 256)) . \chr($val % 256);
            }

            foreach ($par['colors'] as $color) {
                // each color component as 8 bit
                $red = $color['red'] ?? 0.0;
                $green = $color['green'] ?? 0.0;
                $blue = $color['blue'] ?? 0.0;
                $stream .=
                    \chr((int) \floor($red * 255) & 0xFF)
                    . \chr((int) \floor($green * 255) & 0xFF)
                    . \chr((int) \floor($blue * 255) & 0xFF);
            }
        }

        $this->gradients[$ngr]['stream'] = $stream;

        return (
            $this->getStartTransform()
            . $this->getClippingRect($posx, $posy, $width, $height)
            . $this->getGradientTransform($posx, $posy, $width, $height)
            . '/Sh'
            . $ngr
            . ' sh'
            . "\n"
            . $this->getStopTransform()
        );
    }

    /**
     * Paints registration bars with color transitions
     *
     * @param float                            $posx     Abscissa of the top left corner of the rectangle.
     * @param float                            $posy     Ordinate of the top left corner of the rectangle.
     * @param float                            $width    Width of the rectangle.
     * @param float                            $height   Height of the rectangle.
     * @param bool                             $vertical If true prints bar vertically, otherwise horizontally.
     * @param array<int, string|array<string>> $colors   Array of colors to print: each entry is a color
     *                                                   string or an array of two transition colors.
     *
     * @return string PDF command
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     *
     * @throws \Com\Tecnick\Pdf\Graph\Exception
     */
    public function getColorRegistrationBar(
        float $posx,
        float $posy,
        float $width,
        float $height,
        bool $vertical = false,
        array $colors = [
            // GRAY : black   to white
            ['g(0%)',               'g(100%)'],
            // RGB  : red     to white
            ['rgb(100%,0%,0%)',     'rgb(100%,100%,100%)'],
            // RGB  : green   to white
            ['rgb(0%,100%,0%)',     'rgb(100%,100%,100%)'],
            // RGB  : blue    to white
            ['rgb(0%,0%,100%)',     'rgb(100%,100%,100%)'],
            // CMYK : cyan    to white
            ['cmyk(100%,0%,0%,0%)', 'cmyk(0%,0%,0%,0%)'],
            // CMYK : magenta to white
            ['cmyk(0%,100%,0%,0%)', 'cmyk(0%,0%,0%,0%)'],
            // CMYK : yellow  to white
            ['cmyk(0%,0%,100%,0%)', 'cmyk(0%,0%,0%,0%)'],
            // CMYK : black   to white
            ['cmyk(0%,0%,0%,100%)', 'cmyk(0%,0%,0%,0%)'],
        ],
    ): string {
        $numbars = \count($colors);
        if ($numbars <= 0) {
            return '';
        }

        // set bar measures
        // default (non-vertical)
        $coords = [0, 0, 1, 0];
        $wbr = $width;
        $hbr = $height / $numbars;
        $xdt = 0.0;
        $ydt = $hbr;

        if ($vertical) {
            $coords = [0, 1, 0, 0]; // coordinates for gradient transition
            $wbr = $width / $numbars; // bar width
            $hbr = $height; // bar height
            $xdt = $wbr; // delta x
            $ydt = 0.0; // delta y
        }

        $xbr = $posx;
        $ybr = $posy;

        $out = '';
        foreach ($colors as $color) {
            // a single color string is a bar with no transition
            $color = \is_string($color) ? [$color, $color] : $color;
            if (isset($color[0]) && $color[0] !== '') {
                if (!isset($color[1])) {
                    $color[1] = $color[0];
                }

                if ($color[0] === $color[1]) {
                    // colored rectangle
                    $out .=
                        $this->getStartTransform()
                        . $this->getDeviceColorCmd($color[0])
                        . $this->getBasicRect($xbr, $ybr, $wbr, $hbr, 'F')
                        . $this->getStopTransform();
                } else {
                    // color gradient
                    $out .= $this->getLinearGradient($xbr, $ybr, $wbr, $hbr, $color[0], $color[1], $coords);
                }
            }

            $xbr += $xdt;
            $ybr += $ydt;
        }

        return $out;
    }

    /**
     * Get a crop-mark.
     *
     * @param float        $posx   Abscissa of the crop-mark center.
     * @param float        $posy   Ordinate of the crop-mark center.
     * @param float        $width  Width of the crop-mark.
     * @param float        $height Height of the crop-mark.
     * @param string       $type   Type of crop mark - one symbol per type:
     *                             T = TOP, B = BOTTOM, L = LEFT, R = RIGHT
     * @param StyleDataOpt $style  Line style to apply.
     *
     * @return string PDF command
     */
    public function getCropMark(
        float $posx,
        float $posy,
        float $width,
        float $height,
        string $type = 'TBLR',
        array $style = [],
    ): string {
        $crops = \array_unique(\str_split(\strtoupper($type), 1));
        $space_ratio = 4;
        $dhw = $width / $space_ratio; // horizontal space to leave before the intersection point
        $dvh = $height / $space_ratio; // vertical space to leave before the intersection point

        $out = '';
        foreach ($crops as $crop) {
            // offsets from the crop-mark center to the segment endpoints
            $delta = match ($crop) {
                'T' => [0.0, -$height, 0.0, -$dvh],
                'B' => [0.0, $dvh, 0.0, $height],
                'L' => [-$width, 0.0, -$dhw, 0.0],
                'R' => [$dhw, 0.0, $width, 0.0],
                default => null,
            };

            if ($delta === null) {
                continue;
            }

            $out .=
                $this->getRawPoint($posx + $delta[0], $posy + $delta[1])
                . $this->getRawLine($posx + $delta[2], $posy + $delta[3])
                . $this->getPathPaintOp('S');
        }

        if ($out === '') {
            return '';
        }

        return $this->getStartTransform() . $this->getStyleCmd($style) . $out . $this->getStopTransform();
    }

    /**
     * Get overprint mode for stroking (OP) and non-stroking (op) painting operations.
     * (Check the "Entries in a Graphics State Parameter Dictionary" on PDF 32000-1:2008).
     *
     * @param bool      $stroking    If true apply overprint for stroking operations.
     * @param bool|null $nonstroking If true apply overprint for painting operations other than stroking.
     * @param int       $mode        Overprint mode:
     *                               0 = each source colour component value replaces the value
     *                               previously painted for the corresponding device colorant;
     *                               1 = a tint value of 0.0 for a source colour component leaves
     *                               the corresponding component of the previously painted colour
     *                               unchanged.
     *
     * @return string PDF command
     */
    public function getOverprint(bool $stroking = true, ?bool $nonstroking = null, int $mode = 0): string
    {
        if ($nonstroking === null) {
            $nonstroking = $stroking;
        }

        return $this->getExtGState([
            'OP' => $stroking,
            'op' => $nonstroking,
            'OPM' => \max(0, \min(1, $mode)),
        ]);
    }

    /**
     * Set alpha for stroking (CA) and non-stroking (ca) operations.
     *
     * @param float            $stroking    Alpha value for stroking operations:
     *                                      real value from 0 (transparent) to 1 (opaque).
     * @param string|BlendMode $bmv         Blend mode, one of the following: Normal, Multiply,
     *                                      Screen, Overlay, Darken, Lighten, ColorDodge, ColorBurn,
     *                                      HardLight, SoftLight, Difference, Exclusion, Hue,
     *                                      Saturation, Color, Luminosity.
     * @param float|string     $nonstroking Alpha value for non-stroking operations:
     *                                      real value from 0 (transparent) to 1 (opaque).
     * @param bool             $ais         Alpha-Is-Shape flag set on the /AIS ExtGState key.
     *
     * @return string PDF command
     */
    public function getAlpha(
        float $stroking = 1,
        string|BlendMode $bmv = 'Normal',
        float|string $nonstroking = '',
        bool $ais = false,
    ): string {
        if ($nonstroking === '') {
            $nonstroking = $stroking;
        }

        $bmv = BlendMode::fromLoose($bmv)->value;

        if (\is_string($nonstroking)) {
            $nonstroking = \is_numeric($nonstroking) ? (float) $nonstroking : $stroking;
        }

        return $this->getExtGState([
            'CA' => \max(0.0, \min(1.0, $stroking)),
            'ca' => \max(0.0, \min(1.0, $nonstroking)),
            'BM' => '/' . $bmv,
            'AIS' => $ais,
        ]);
    }
}
