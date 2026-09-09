<?php

declare(strict_types=1);

/**
 * Pdf.php
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 *
 * This file is part of tc-lib-color software library.
 */

namespace Com\Tecnick\Color;

use Com\Tecnick\Color\Exception as ColorException;

/**
 * Com\Tecnick\Color\Pdf
 *
 * PDF Color class
 *
 * Extension points: getPdfColor() and getColorObject(). A subclass overriding
 * either must mirror the signature exactly. Every other method is final.
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Color
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-color
 */
class Pdf extends \Com\Tecnick\Color\Spot implements PdfColorWriterInterface
{
    /**
     * Array of valid JavaScript color names to be used in PDF documents
     */
    public const JSCOLOR = [
        'transparent',
        'black',
        'white',
        'red',
        'green',
        'blue',
        'cyan',
        'magenta',
        'yellow',
        'dkGray',
        'gray',
        'ltGray',
    ];

    /**
     * Convert color to javascript string
     *
     * The JSCOLOR names are matched case-sensitively and emitted verbatim as
     * Acrobat JavaScript identifiers ('color.dkGray'). A name that differs only
     * in case falls through to the color parser and yields a component array.
     *
     * @param string $color color name or color object
     */
    final public function getJsColorString(string $color): string
    {
        if (\in_array($color, self::JSCOLOR, strict: true)) {
            return 'color.' . $color;
        }

        try {
            if (($colobj = $this->getColorObj($color)) instanceof \Com\Tecnick\Color\Model) {
                return $colobj->getJsPdfColor();
            }
        } catch (ColorException $colorException) {
            unset($colorException);
        }

        // default transparent color
        return 'color.' . self::JSCOLOR[0];
    }

    /**
     * Returns a color object from an HTML, CSS or Spot color representation.
     *
     * NOTE: this is a read-only lookup. A spot color referenced here is resolved
     * but NOT registered, so it does not affect the output of
     * getPdfSpotObjects() and getPdfSpotResources(). Use getPdfColor() or
     * addSpotColor() to register a spot color for emission. The returned model
     * is a copy of the registered one.
     *
     * Spot colors are resolved first. Eight of the eleven default spot color
     * names are also CSS color names ('red', 'green', 'blue', 'cyan', 'magenta',
     * 'yellow', 'black', 'white') and resolve to the spot color. They agree with
     * their CSS namesake except 'green': the spot Green is CMYK(1,0,1,0) =
     * #00ff00, while CSS green is #008000. Pass $allowSpot = false to skip the
     * spot lookup and get the device color. 'key', 'all' and 'none' are
     * spot-only names: with $allowSpot = false they yield null.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $allowSpot True to resolve spot colors, false to force a device color.
     */
    public function getColorObject(string $color, bool $allowSpot = true): ?\Com\Tecnick\Color\Model
    {
        if ($allowSpot) {
            try {
                return clone $this->resolveSpotColorData($color)['color'];
            } catch (ColorException $colorException) {
                unset($colorException);
            }
        }

        try {
            return $this->getColorObj($color);
        } catch (ColorException $colorException) {
            unset($colorException);
        }

        return null;
    }

    /**
     * Get the color components format used in PDF documents
     * NOTE: the alpha channel is omitted
     *
     * Spot colors are resolved first. Eight of the eleven default spot color
     * names are also CSS color names ('red', 'green', 'blue', 'cyan', 'magenta',
     * 'yellow', 'black', 'white') and resolve to a Separation color space rather
     * than to DeviceRGB. Pass $allowSpot = false to skip the spot lookup and get
     * a device color. 'key', 'all' and 'none' are spot-only names: with
     * $allowSpot = false they yield an empty string.
     *
     * NOTE: resolving a spot color REGISTERS it, so it is subsequently emitted
     * by getPdfSpotObjects() and getPdfSpotResources(). Call this before
     * getPdfSpotObjects(), so that the returned '/CSn cs' operator references an
     * existing resource. Use getColorObject() for a lookup with no side effect.
     *
     * Returns an empty string both for a transparent color and for input that
     * cannot be parsed.
     *
     * NOTE: $tint is the operand of the 'scn' operator and applies to spot
     * colors only; a device color is always written at full intensity.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $stroke    True for stroking (lines, drawing) and false for non-stroking (text and area filling).
     * @param float  $tint      Intensity of the color (from 0 to 1; 1 = full intensity). Spot colors only.
     * @param bool   $allowSpot True to resolve (and register) spot colors, false to force a device color.
     */
    public function getPdfColor(string $color, bool $stroke = false, float $tint = 1, bool $allowSpot = true): string
    {
        if ($allowSpot) {
            try {
                $col = $this->getSpotColor($color);
                $operator = \sprintf('cs %F scn', \max(0, \min(1, $tint)));
                if ($stroke) {
                    $operator = \strtoupper($operator);
                }

                return \sprintf('/CS%d %s' . "\n", $col['i'], $operator);
            } catch (ColorException $colorException) {
                unset($colorException);
            }
        }

        try {
            $col = $this->getColorObj($color);
            if ($col instanceof \Com\Tecnick\Color\Model) {
                return $col->getPdfColor($stroke);
            }
        } catch (ColorException $colorException) {
            unset($colorException);
        }

        return '';
    }

    /**
     * Get the stroked color components format used in PDF documents.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param float  $tint      Intensity of the color (from 0 to 1; 1 = full intensity). Spot colors only.
     * @param bool   $allowSpot True to resolve (and register) spot colors, false to force a device color.
     */
    final public function getPdfStrokeColor(string $color, float $tint = 1, bool $allowSpot = true): string
    {
        return $this->getPdfColor($color, true, $tint, $allowSpot);
    }

    /**
     * Get the fill color components format used in PDF documents.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param float  $tint      Intensity of the color (from 0 to 1; 1 = full intensity). Spot colors only.
     * @param bool   $allowSpot True to resolve (and register) spot colors, false to force a device color.
     */
    final public function getPdfFillColor(string $color, float $tint = 1, bool $allowSpot = true): string
    {
        return $this->getPdfColor($color, false, $tint, $allowSpot);
    }

    /**
     * Get the RGB color components format used in PDF documents
     *
     * Returns an empty string if the color cannot be resolved.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $allowSpot True to resolve spot colors, false to force a device color.
     */
    final public function getPdfRgbComponents(string $color, bool $allowSpot = true): string
    {
        $model = $this->getColorObject($color, $allowSpot);
        if (!$model instanceof \Com\Tecnick\Color\Model) {
            return '';
        }

        $cmp = $model->toRgbArray();
        return \sprintf('%F %F %F', $cmp['red'] ?? 0.0, $cmp['green'] ?? 0.0, $cmp['blue'] ?? 0.0);
    }

    /**
     * Get the CMYK color components format used in PDF documents.
     *
     * Returns an empty string if the color cannot be resolved.
     *
     * @param string $color     HTML, CSS or Spot color to parse
     * @param bool   $allowSpot True to resolve spot colors, false to force a device color.
     */
    final public function getPdfCmykComponents(string $color, bool $allowSpot = true): string
    {
        $model = $this->getColorObject($color, $allowSpot);
        if (!$model instanceof \Com\Tecnick\Color\Model) {
            return '';
        }

        $cmp = $model->toCmykArray();
        return \sprintf(
            '%F %F %F %F',
            $cmp['cyan'] ?? 0.0,
            $cmp['magenta'] ?? 0.0,
            $cmp['yellow'] ?? 0.0,
            $cmp['key'] ?? 0.0,
        );
    }
}
