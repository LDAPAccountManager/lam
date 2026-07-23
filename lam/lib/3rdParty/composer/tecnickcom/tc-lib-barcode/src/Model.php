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
 */
interface Model
{
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
     */
    public function setSize(int $width, int $height, array $padding = [0, 0, 0, 0]): static;

    /**
     * Set the color of the bars.
     * An empty or transparent foreground color is rejected with a BarcodeException.
     *
     * @param string $color Foreground color in Web notation (color name, or hexadecimal code, or CSS syntax)
     *
     * @throws ColorException in case of color error
     * @throws BarcodeException in case of empty or transparent color
     */
    public function setColor(string $color): static;

    /**
     * Set the background color
     *
     * @param string $color Background color in Web notation (color name, or hexadecimal code, or CSS syntax)
     *
     * @throws ColorException in case of color error
     */
    public function setBackgroundColor(string $color): static;

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
    public function getArray(): array;

    /**
     * Get the extended code (code + checksum)
     */
    public function getExtendedCode(): string;

    /**
     * Get the barcode as SVG image object
     *
     * @param string|null $filename The file name without extension (optional).
     *                              Only allows alphanumeric characters, underscores and hyphens.
     *                              Defaults to a md5 hash of the data.
     *                              The file extension is always '.svg'.
     */
    public function getSvg(?string $filename = null): void;

    /**
     * Get the barcode as inline SVG code.
     *
     * @return string Inline SVG code.
     */
    public function getInlineSvgCode(): string;

    /**
     * Get the barcode as SVG code, including the XML declaration.
     *
     * @return string SVG code
     */
    public function getSvgCode(): string;

    /**
     * Get an HTML representation of the barcode.
     *
     * @return string HTML code (DIV block)
     */
    public function getHtmlDiv(): string;

    /**
     * Get Barcode as PNG Image (requires GD or Imagick library)
     *
     * @param string|null $filename The file name without extension (optional).
     *                              Only allows alphanumeric characters, underscores and hyphens.
     *                              Defaults to a md5 hash of the data.
     *                              The file extension is always '.png'.
     */
    public function getPng(?string $filename = null): void;

    /**
     * Get the barcode as PNG image (requires GD or Imagick library)
     *
     * @param bool $imagick If true try to use the Imagick extension
     *
     * @return string PNG image data
     */
    public function getPngData(bool $imagick = true): string;

    /**
     * Get the barcode as PNG image (requires Imagick library)
     *
     * @throws BarcodeException if the Imagick library is not installed
     */
    public function getPngDataImagick(): string;

    /**
     * Get the barcode as GD image object (requires GD library)
     *
     * @throws BarcodeException if the GD library is not installed
     */
    public function getGd(): \GdImage;

    /**
     * Get a raw barcode string representation using characters
     *
     * @param string $space_char Character or string to use for filling empty spaces
     * @param string $bar_char   Character or string to use for filling bars
     */
    public function getGrid(string $space_char = '0', string $bar_char = '1'): string;

    /**
     * Get a raw barcode grid array
     *
     * @param string $space_char Character or string to use for filling empty spaces
     * @param string $bar_char   Character or string to use for filling bars
     *
     * @return array<int, array<int, string>>
     */
    public function getGridArray(string $space_char = '0', string $bar_char = '1'): array;

    /**
     * Get the array containing all the formatted bars coordinates
     *
     * @return array<int, array{float, float, float, float}>
     */
    public function getBarsArrayXYXY(): array;

    /**
     * Get the array containing all the formatted bars coordinates
     *
     * @return array<int, array{float, float, float, float}>
     */
    public function getBarsArrayXYWH(): array;
}
