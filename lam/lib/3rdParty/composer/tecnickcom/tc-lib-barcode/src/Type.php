<?php

declare(strict_types=1);

/**
 * Type.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Color\Exception as ColorException;
use Com\Tecnick\Color\Model\Rgb;
use Com\Tecnick\Color\Pdf;

/**
 * Com\Tecnick\Barcode\Type
 *
 * Barcode Type class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Type extends \Com\Tecnick\Barcode\Type\Convert implements Model
{
    /**
     * Initialize a new barcode object
     *
     * @param string                    $code    Barcode content
     * @param int                       $width   Barcode width in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each column.
     * @param int                       $height  Barcode height in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each row.
     * @param string                    $color   Foreground color in Web notation
     *                                           (color name, or hexadecimal code, or CSS syntax)
     * @param array<int|float|string>   $params  Array containing extra parameters for the specified barcode type
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *                                           (top, right, bottom, left) in user units. A
     *                                           negative value indicates the number or rows
     *                                           or columns.
     *
     * @throws BarcodeException in case of error
     * @throws ColorException in case of color error
     */
    public function __construct(
        string $code,
        int $width = -1,
        int $height = -1,
        string $color = 'black',
        array $params = [],
        array $padding = [0, 0, 0, 0],
    ) {
        $this->code = $code;
        $this->extcode = $code;
        $this->params = $params;
        $this->setParameters();
        $this->setBars();
        $this->setSize($width, $height, $padding);
        $this->setColor($color);
    }

    /**
     * Set extra (optional) parameters
     */
    protected function setParameters(): void {}

    /**
     * Set the bars array
     */
    protected function setBars(): void {}

    /**
     * Set the size of the barcode to be exported
     *
     * @param int                       $width   Barcode width in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each column.
     * @param int                       $height  Barcode height in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each row.
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *                                           (top, right, bottom, left) in user units. A
     *                                           negative value indicates the number or rows
     *                                           or columns.
     *
     * @throws BarcodeException in case of an empty barcode or invalid padding
     */
    public function setSize(int $width, int $height, array $padding = [0, 0, 0, 0]): static
    {
        if ($this->ncols <= 0 || $this->nrows <= 0) {
            throw new BarcodeException('Empty barcode: the number of rows and columns must be greater than zero');
        }

        $this->width = $width;
        if ($this->width <= 0) {
            $this->width = \abs(\min(-1, $this->width)) * $this->ncols;
        }

        $this->height = $height;
        if ($this->height <= 0) {
            $this->height = \abs(\min(-1, $this->height)) * $this->nrows;
        }

        $this->width_ratio = $this->width / $this->ncols;
        $this->height_ratio = $this->height / $this->nrows;

        $this->setPadding($padding);

        return $this;
    }

    /**
     * Set the barcode padding
     *
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *                                           (top, right, bottom, left) in user units.
     *                                           A negative value indicates the number or rows or columns.
     *
     * @throws BarcodeException in case of error
     */
    protected function setPadding(array $padding): static
    {
        if (\count($padding) !== 4) {
            throw new BarcodeException('Invalid padding, expecting an array of 4 numbers (top, right, bottom, left)');
        }

        foreach ($padding as $key => $val) {
            $side = match ($key) {
                0 => 'T',
                1 => 'R',
                2 => 'B',
                3 => 'L',
            };
            $ratio = match ($key) {
                0, 2 => $this->height_ratio,
                1, 3 => $this->width_ratio,
            };
            if ($val < 0) {
                $val = \abs(\min(-1, $val)) * $ratio;
            }

            $this->padding[$side] = (int) $val;
        }

        return $this;
    }

    /**
     * @param array<string, float> $rgbcolor
     */
    protected function getRgbComponent(array $rgbcolor, string $channel): float
    {
        return $rgbcolor[$channel] ?? 0.0;
    }

    /**
     * Set the color of the bars.
     * An empty or transparent foreground color is rejected with a BarcodeException.
     *
     * @param string $color Foreground color in Web notation (color name, or hexadecimal code, or CSS syntax)
     *
     * @throws ColorException in case of color error
     * @throws BarcodeException in case of empty or transparent color
     */
    public function setColor(string $color): static
    {
        $colobj = $this->getRgbColorObject($color);
        if (!$colobj instanceof \Com\Tecnick\Color\Model\Rgb) {
            throw new BarcodeException('The foreground color cannot be empty or transparent');
        }

        $this->color_obj = $colobj;
        return $this;
    }

    /**
     * Set the background color
     *
     * @param string $color Background color in Web notation (color name, or hexadecimal code, or CSS syntax)
     *
     * @throws ColorException in case of color error
     */
    public function setBackgroundColor(string $color): static
    {
        $this->bg_color_obj = $this->getRgbColorObject($color);
        return $this;
    }

    /**
     * Get the RGB Color object for the given color representation
     *
     * @param string $color Color in Web notation (color name, or hexadecimal code, or CSS syntax)
     *
     * @throws ColorException in case of color error
     */
    protected function getRgbColorObject(string $color): ?Rgb
    {
        $pdf = new Pdf();
        $cobj = $pdf->getColorObject($color);
        if ($cobj instanceof \Com\Tecnick\Color\Model) {
            return new Rgb($cobj->toRgbArray());
        }

        return null;
    }

    /**
     * Get the barcode raw array
     *
     * @return array{
     *             'type': string,
     *             'format': string,
     *             'params': array<int|float|string>,
     *             'code': string,
     *             'extcode': string,
     *             'ncols': int,
     *             'nrows': int,
     *             'width': int,
     *             'height': int,
     *             'width_ratio': float,
     *             'height_ratio': float,
     *             'padding': array{'T': int, 'R': int, 'B': int, 'L': int},
     *             'full_width': int,
     *             'full_height': int,
     *             'color_obj': Rgb,
     *             'bg_color_obj': ?Rgb,
     *             'bars': array<array{int, int, int, int}>,
     *         }
     */
    public function getArray(): array
    {
        return [
            'type' => $this::TYPE,
            'format' => $this::FORMAT,
            'params' => $this->params,
            'code' => $this->code,
            'extcode' => $this->extcode,
            'ncols' => $this->ncols,
            'nrows' => $this->nrows,
            'width' => $this->width,
            'height' => $this->height,
            'width_ratio' => $this->width_ratio,
            'height_ratio' => $this->height_ratio,
            'padding' => $this->padding,
            'full_width' => $this->width + $this->padding['L'] + $this->padding['R'],
            'full_height' => $this->height + $this->padding['T'] + $this->padding['B'],
            'color_obj' => $this->color_obj,
            'bg_color_obj' => $this->bg_color_obj,
            'bars' => $this->bars,
        ];
    }

    /**
     * Get the extended code (code + checksum)
     */
    public function getExtendedCode(): string
    {
        return $this->extcode;
    }

    /**
     * Sends the data as file to the browser.
     *
     * @param string $data The file data.
     * @param string $mime The file MIME type (i.e. 'application/svg+xml' or 'image/png').
     * @param string $fileext The file extension (i.e. 'svg' or 'png').
     * @param string|null $filename The file name without extension (optional).
     *                              Only allows alphanumeric characters, underscores and hyphens.
     *                              Defaults to a md5 hash of the data.
     *
     * @return void
     */
    protected function getHTTPFile(string $data, string $mime, string $fileext, ?string $filename = null): void
    {
        if (\is_null($filename) || \preg_match('/^[a-zA-Z0-9_\-]{1,250}$/', $filename) !== 1) {
            $filename = \md5($data);
        }

        \header('Content-Type: ' . $mime);
        \header('Cache-Control: private, must-revalidate, post-check=0, pre-check=0, max-age=1');
        \header('Pragma: public');
        \header('Expires: Thu, 04 jan 1973 00:00:00 GMT'); // Date in the past
        \header('Last-Modified: ' . gmdate('D, d M Y H:i:s') . ' GMT');
        \header('Content-Disposition: inline; filename="' . $filename . '.' . $fileext . '";');
        if (($_SERVER['HTTP_ACCEPT_ENCODING'] ?? null) === null) {
            // the content length may vary if the server is using compression
            \header('Content-Length: ' . \strlen($data));
        }

        echo $data;
    }

    /**
     * Get the barcode as SVG image object.
     *
     * @param string|null $filename The file name without extension (optional).
     *                              Only allows alphanumeric characters, underscores and hyphens.
     *                              Defaults to a md5 hash of the data.
     *                              The file extension is always '.svg'.
     */
    public function getSvg(?string $filename = null): void
    {
        $this->getHTTPFile($this->getSvgCode(), 'application/svg+xml', 'svg', $filename);
    }

    /**
     * Get the barcode as inline SVG code.
     *
     * @return string Inline SVG code.
     */
    public function getInlineSvgCode(): string
    {
        // flags for htmlspecialchars
        $hflag = ENT_NOQUOTES;
        if (\defined('ENT_XML1') && \defined('ENT_DISALLOWED')) {
            $hflag = ENT_XML1 | ENT_DISALLOWED;
        }

        $width = \sprintf('%F', $this->width + $this->padding['L'] + $this->padding['R']);
        $height = \sprintf('%F', $this->height + $this->padding['T'] + $this->padding['B']);

        $svg =
            '<svg'
            . ' version="1.2"'
            . ' baseProfile="full"'
            . ' xmlns="http://www.w3.org/2000/svg"'
            . ' xmlns:xlink="http://www.w3.org/1999/xlink"'
            . ' xmlns:ev="http://www.w3.org/2001/xml-events"'
            . ' width="'
            . $width
            . '"'
            . ' height="'
            . $height
            . '"'
            . ' viewBox="0 0 '
            . $width
            . ' '
            . $height
            . '"'
            . '>'
            . "\n"
            . "\t"
            . '<desc>'
            . \htmlspecialchars($this->code, $hflag, 'UTF-8')
            . '</desc>'
            . "\n";
        if ($this->bg_color_obj instanceof \Com\Tecnick\Color\Model\Rgb) {
            $svg .=
                '	<rect x="0" y="0" width="'
                . $width
                . '"'
                . ' height="'
                . $height
                . '"'
                . ' fill="'
                . $this->bg_color_obj->getRgbHexColor()
                . '"'
                . ' stroke="none"'
                . ' stroke-width="0"'
                . ' stroke-linecap="square"'
                . ' />'
                . "\n";
        }

        $svg .=
            '	<g id="bars" fill="'
            . $this->color_obj->getRgbHexColor()
            . '"'
            . ' stroke="none"'
            . ' stroke-width="0"'
            . ' stroke-linecap="square"'
            . '>'
            . "\n";
        $bars = $this->getBarsArrayXYWH();
        foreach ($bars as $bar) {
            $svg .= \sprintf(
                '		<rect x="%F" y="%F" width="%F" height="%F" />' . "\n",
                $bar[0],
                $bar[1],
                $bar[2],
                $bar[3],
            );
        }

        return $svg . ('	</g>' . "\n" . '</svg>' . "\n");
    }

    /**
     * Get the barcode as SVG code, including the XML declaration.
     *
     * @return string SVG code
     */
    public function getSvgCode(): string
    {
        return '<?xml version="1.0" standalone="no" ?>' . "\n" . $this->getInlineSvgCode();
    }

    /**
     * Get an HTML representation of the barcode.
     *
     * @return string HTML code (DIV block)
     */
    public function getHtmlDiv(): string
    {
        $html = \sprintf(
            '<div style="width:%Fpx;height:%Fpx;position:relative;font-size:0;border:none;padding:0;margin:0;',
            $this->width + $this->padding['L'] + $this->padding['R'],
            $this->height + $this->padding['T'] + $this->padding['B'],
        );
        if ($this->bg_color_obj instanceof \Com\Tecnick\Color\Model\Rgb) {
            $html .= 'background-color:' . $this->bg_color_obj->getCssColor() . ';';
        }

        $html .= '">' . "\n";
        $bars = $this->getBarsArrayXYWH();
        foreach ($bars as $bar) {
            $html .= \sprintf(
                '	<div style="background-color:%s;left:%Fpx;top:%Fpx;width:%Fpx;height:%Fpx;position:absolute;border:none;padding:0;margin:0;">&nbsp;</div>'
                . "\n",
                $this->color_obj->getCssColor(),
                $bar[0],
                $bar[1],
                $bar[2],
                $bar[3],
            );
        }

        return $html . ('</div>' . "\n");
    }

    /**
     * Get Barcode as PNG Image (requires GD or Imagick library)
     *
     * @param string|null $filename The file name without extension (optional).
     *                              Only allows alphanumeric characters, underscores and hyphens.
     *                              Defaults to a md5 hash of the data.
     *                              The file extension is always '.png'.
     *
     * @throws BarcodeException in case image generation fails
     */
    public function getPng(?string $filename = null): void
    {
        $this->getHTTPFile($this->getPngData(), 'image/png', 'png', $filename);
    }

    /**
     * Get the barcode as PNG image (requires GD or Imagick library)
     *
     * @param bool $imagick If true try to use the Imagick extension
     *
     * @return string PNG image data
     *
     * @throws BarcodeException in case image generation fails
     */
    public function getPngData(bool $imagick = true): string
    {
        if ($imagick && \extension_loaded('imagick')) {
            return $this->getPngDataImagick();
        }

        $gdImage = $this->getGd();
        \ob_start();
        \imagepng($gdImage);
        $data = \ob_get_clean();
        if ($data === false) {
            throw new BarcodeException('Unable to get PNG data');
        }
        return $data;
    }

    /**
     * Maximum width or height, in pixels, of a rendered barcode image.
     * Guards against pathological size multipliers triggering huge allocations.
     */
    protected const MAX_IMAGE_SIDE = 30_000;

    /**
     * Compute and validate the rendered image dimensions, in pixels.
     *
     * @return array{int, int} [width, height], each at least 1 pixel
     *
     * @throws BarcodeException if the requested image size is too large
     */
    protected function getImageSize(): array
    {
        $width = \max(1, (int) \ceil($this->width + $this->padding['L'] + $this->padding['R']));
        $height = \max(1, (int) \ceil($this->height + $this->padding['T'] + $this->padding['B']));
        if ($width > self::MAX_IMAGE_SIDE || $height > self::MAX_IMAGE_SIDE) {
            throw new BarcodeException(
                'The requested image size ('
                . $width
                . 'x'
                . $height
                . ' px) exceeds the maximum of '
                . self::MAX_IMAGE_SIDE
                . ' px per side',
            );
        }

        return [$width, $height];
    }

    /**
     * Get the barcode as PNG image (requires Imagick library)
     *
     * @throws BarcodeException if the Imagick library is not installed or the image is too large
     */
    public function getPngDataImagick(): string
    {
        $imagick = new \Imagick();
        [$width, $height] = $this->getImageSize();
        $imagick->newImage($width, $height, 'none', 'png');
        $imagickdraw = new \ImagickDraw();
        if ($this->bg_color_obj instanceof \Com\Tecnick\Color\Model\Rgb) {
            $rgbcolor = $this->bg_color_obj->getNormalizedArray(255);
            $imagickdraw->setfillcolor(
                'rgb('
                . (string) $this->getRgbComponent($rgbcolor, 'R')
                . ','
                . (string) $this->getRgbComponent($rgbcolor, 'G')
                . ','
                . (string) $this->getRgbComponent($rgbcolor, 'B')
                . ')',
            );
            $imagickdraw->rectangle(0, 0, $width, $height);
        }

        $rgbcolor = $this->color_obj->getNormalizedArray(255);
        $imagickdraw->setfillcolor(
            'rgb('
            . (string) $this->getRgbComponent($rgbcolor, 'R')
            . ','
            . (string) $this->getRgbComponent($rgbcolor, 'G')
            . ','
            . (string) $this->getRgbComponent($rgbcolor, 'B')
            . ')',
        );
        $bars = $this->getBarsArrayXYXY();
        foreach ($bars as $bar) {
            $imagickdraw->rectangle($bar[0], $bar[1], $bar[2], $bar[3]);
        }

        $imagick->drawimage($imagickdraw);
        return $imagick->getImageBlob();
    }

    /**
     * Apply GD background color/alpha strategy.
     *
     * @throws BarcodeException if background allocation fails
     */
    protected function applyGdBackground(\GdImage $img, int $width, int $height): void
    {
        $bgColorObj = $this->bg_color_obj;
        if ($bgColorObj instanceof \Com\Tecnick\Color\Model\Rgb) {
            $rgbcolor = $bgColorObj->getNormalizedArray(255);
            $bg_color = \imagecolorallocate(
                $img,
                (int) \round($this->getRgbComponent($rgbcolor, 'R')),
                (int) \round($this->getRgbComponent($rgbcolor, 'G')),
                (int) \round($this->getRgbComponent($rgbcolor, 'B')),
            );
            if ($bg_color === false) {
                throw new BarcodeException('Unable to allocate GD background color');
            }
            \imagefilledrectangle($img, 0, 0, $width, $height, $bg_color);
            return;
        }

        $bgobj = clone $this->color_obj;
        $rgbcolor = $bgobj->invertColor()->getNormalizedArray(255);
        $background_color = \imagecolorallocate(
            $img,
            (int) \round($this->getRgbComponent($rgbcolor, 'R')),
            (int) \round($this->getRgbComponent($rgbcolor, 'G')),
            (int) \round($this->getRgbComponent($rgbcolor, 'B')),
        );
        if ($background_color === false) {
            throw new BarcodeException('Unable to allocate default GD background color');
        }
        \imagecolortransparent($img, $background_color);
    }

    /**
     * Get the barcode as GD image object (requires GD library)
     *
     * @throws BarcodeException if the GD library is not installed or the image is too large
     */
    public function getGd(): \GdImage
    {
        [$width, $height] = $this->getImageSize();
        $img = \imagecreate($width, $height);
        if ($img === false) {
            throw new BarcodeException('Unable to create GD image');
        }

        $this->applyGdBackground($img, $width, $height);

        $rgbcolor = $this->color_obj->getNormalizedArray(255);
        $bar_color = \imagecolorallocate(
            $img,
            (int) \round($this->getRgbComponent($rgbcolor, 'R')),
            (int) \round($this->getRgbComponent($rgbcolor, 'G')),
            (int) \round($this->getRgbComponent($rgbcolor, 'B')),
        );
        if ($bar_color === false) {
            throw new BarcodeException('Unable to allocate GD foreground color');
        }
        $bars = $this->getBarsArrayXYXY();
        foreach ($bars as $bar) {
            \imagefilledrectangle(
                $img,
                (int) \floor($bar[0]),
                (int) \floor($bar[1]),
                (int) \floor($bar[2]),
                (int) \floor($bar[3]),
                $bar_color,
            );
        }

        return $img;
    }

    /**
     * Get a raw barcode string representation using characters
     *
     * @param string $space_char Character or string to use for filling empty spaces
     * @param string $bar_char   Character or string to use for filling bars
     */
    public function getGrid(string $space_char = '0', string $bar_char = '1'): string
    {
        $raw = $this->getGridArray($space_char, $bar_char);
        $grid = '';
        foreach ($raw as $row) {
            $grid .= \implode('', $row) . "\n";
        }

        return $grid;
    }

    /**
     * Get the array containing all the formatted bars coordinates
     *
     * @return array<int, array{float, float, float, float}>
     */
    public function getBarsArrayXYXY(): array
    {
        $rect = [];
        foreach ($this->bars as $bar) {
            if ($bar[2] <= 0) {
                continue;
            }

            if ($bar[3] <= 0) {
                continue;
            }

            $rect[] = $this->getBarRectXYXY($bar);
        }

        if ($this->nrows > 1) {
            // reprint rotated to cancel row gaps
            $rot = $this->getRotatedBarArray();
            foreach ($rot as $bar) {
                if ($bar[2] <= 0) {
                    continue;
                }

                if ($bar[3] <= 0) {
                    continue;
                }

                $rect[] = $this->getBarRectXYXY($bar);
            }
        }

        return $rect;
    }

    /**
     * Get the array containing all the formatted bars coordinates
     *
     * @return array<int, array{float, float, float, float}>
     */
    public function getBarsArrayXYWH(): array
    {
        $rect = [];
        foreach ($this->bars as $bar) {
            if ($bar[2] <= 0) {
                continue;
            }

            if ($bar[3] <= 0) {
                continue;
            }

            $rect[] = $this->getBarRectXYWH($bar);
        }

        if ($this->nrows > 1) {
            // reprint rotated to cancel row gaps
            $rot = $this->getRotatedBarArray();
            foreach ($rot as $bar) {
                if ($bar[2] <= 0) {
                    continue;
                }

                if ($bar[3] <= 0) {
                    continue;
                }

                $rect[] = $this->getBarRectXYWH($bar);
            }
        }

        return $rect;
    }
}
