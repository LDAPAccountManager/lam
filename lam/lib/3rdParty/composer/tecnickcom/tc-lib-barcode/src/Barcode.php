<?php

declare(strict_types=1);

/**
 * Barcode.php
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

/**
 * Com\Tecnick\Barcode\Barcode
 *
 * Barcode Barcode class
 *
 * @since     2015-02-21
 * @category  Library
 * @package   Barcode
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-barcode
 */
class Barcode
{
    /**
     * Maximum accepted length of the barcode payload.
     *
     * No single barcode symbology can encode anywhere near this much data
     * (the densest, QR version 40-L, tops out at ~7089 numeric digits), so
     * this is a defensive upper bound to reject abusive inputs early, before
     * any encoder spends time and memory on data it can never represent.
     */
    public const MAX_CODE_LENGTH = 30_000;

    /**
     * List of supported Barcode Types with description.
     *
     * @var array<string, string>
     */
    public const BARCODETYPES = [
        'C128' => 'CODE 128',
        'C128A' => 'CODE 128 A',
        'C128B' => 'CODE 128 B',
        'C128C' => 'CODE 128 C',
        'C39' => 'CODE 39 - ANSI MH10.8M-1983 - USD-3 - 3 of 9.',
        'C39+' => 'CODE 39 + CHECKSUM',
        'C39E' => 'CODE 39 EXTENDED',
        'C39E+' => 'CODE 39 EXTENDED + CHECKSUM',
        'C93' => 'CODE 93 - USS-93',
        'CODABAR' => 'CODABAR',
        'CODE11' => 'CODE 11',
        'EAN13' => 'EAN 13',
        'EAN2' => 'EAN 2-Digits UPC-Based Extension',
        'EAN5' => 'EAN 5-Digits UPC-Based Extension',
        'EAN8' => 'EAN 8',
        'I25' => 'Interleaved 2 of 5',
        'I25+' => 'Interleaved 2 of 5 + CHECKSUM',
        'IMB' => 'IMB - Intelligent Mail Barcode - Onecode - USPS-B-3200',
        'IMBPRE' => 'IMB - Intelligent Mail Barcode pre-processed',
        'KIX' => 'KIX (Klant index - Customer index)',
        'LRAW' => '1D RAW MODE (comma-separated rows of 01 strings)',
        'MSI' => 'MSI (Variation of Plessey code)',
        'MSI+' => 'MSI + CHECKSUM (modulo 11)',
        'PHARMA' => 'PHARMACODE',
        'PHARMA2T' => 'PHARMACODE TWO-TRACKS',
        'PLANET' => 'PLANET',
        'POSTNET' => 'POSTNET',
        'RMS4CC' => 'RMS4CC (Royal Mail 4-state Customer Bar Code)',
        'S25' => 'Standard 2 of 5',
        'S25+' => 'Standard 2 of 5 + CHECKSUM',
        'UPCA' => 'UPC-A',
        'UPCE' => 'UPC-E',
        'AZTEC' => 'AZTEC Code (ISO/IEC 24778:2008)',
        'DATAMATRIX' => 'DATAMATRIX (ISO/IEC 16022)',
        'PDF417' => 'PDF417 (ISO/IEC 15438:2006)',
        'QRCODE' => 'QR-CODE',
        'SRAW' => '2D RAW MODE (comma-separated rows of 01 strings)',
    ];

    /**
     * Get the barcode object
     *
     * @param string                    $type    Barcode type
     * @param string                    $code    Barcode content
     * @param int                       $width   Barcode width in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each column.
     * @param int                       $height  Barcode height in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each row.
     * @param string                    $color   Foreground color in Web notation
     *                                           (color name, or hexadecimal code, or CSS syntax)
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *                                           (top, right, bottom, left) in user units. A
     *                                           negative value indicates the multiplication
     *                                           factor for each row or column.
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color parsing errors
     */
    public function getBarcodeObj(
        string $type,
        string $code,
        int $width = -1,
        int $height = -1,
        string $color = 'black',
        array $padding = [0, 0, 0, 0],
    ): Model {
        if (\strlen($code) > self::MAX_CODE_LENGTH) {
            throw new BarcodeException(
                'The barcode payload is too long: ' . \strlen($code) . ' bytes (maximum ' . self::MAX_CODE_LENGTH . ')',
            );
        }

        // extract extra parameters (if any)
        $params = \explode(',', $type);
        $type = \array_shift($params);

        return match ($type) {
            'C128' => new \Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C128A' => new \Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight\CodeOneTwoEightA(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C128B' => new \Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight\CodeOneTwoEightB(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C128C' => new \Com\Tecnick\Barcode\Type\Linear\CodeOneTwoEight\CodeOneTwoEightC(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C39' => new \Com\Tecnick\Barcode\Type\Linear\CodeThreeNine(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C39+' => new \Com\Tecnick\Barcode\Type\Linear\CodeThreeNineCheck(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C39E' => new \Com\Tecnick\Barcode\Type\Linear\CodeThreeNineExt(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C39E+' => new \Com\Tecnick\Barcode\Type\Linear\CodeThreeNineExtCheck(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C93' => new \Com\Tecnick\Barcode\Type\Linear\CodeNineThree(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'CODABAR' => new \Com\Tecnick\Barcode\Type\Linear\Codabar(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'CODE11' => new \Com\Tecnick\Barcode\Type\Linear\CodeOneOne(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'EAN13' => new \Com\Tecnick\Barcode\Type\Linear\EanOneThree(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'EAN2' => new \Com\Tecnick\Barcode\Type\Linear\EanTwo($code, $width, $height, $color, $params, $padding),
            'EAN5' => new \Com\Tecnick\Barcode\Type\Linear\EanFive($code, $width, $height, $color, $params, $padding),
            'EAN8' => new \Com\Tecnick\Barcode\Type\Linear\EanEight($code, $width, $height, $color, $params, $padding),
            'I25' => new \Com\Tecnick\Barcode\Type\Linear\InterleavedTwoOfFive(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'I25+' => new \Com\Tecnick\Barcode\Type\Linear\InterleavedTwoOfFiveCheck(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'IMB' => new \Com\Tecnick\Barcode\Type\Linear\Imb($code, $width, $height, $color, $params, $padding),
            'IMBPRE' => new \Com\Tecnick\Barcode\Type\Linear\ImbPre($code, $width, $height, $color, $params, $padding),
            'KIX' => new \Com\Tecnick\Barcode\Type\Linear\KlantIndex($code, $width, $height, $color, $params, $padding),
            'LRAW' => new \Com\Tecnick\Barcode\Type\Linear\Raw($code, $width, $height, $color, $params, $padding),
            'MSI' => new \Com\Tecnick\Barcode\Type\Linear\Msi($code, $width, $height, $color, $params, $padding),
            'MSI+' => new \Com\Tecnick\Barcode\Type\Linear\MsiCheck($code, $width, $height, $color, $params, $padding),
            'PHARMA' => new \Com\Tecnick\Barcode\Type\Linear\Pharma($code, $width, $height, $color, $params, $padding),
            'PHARMA2T' => new \Com\Tecnick\Barcode\Type\Linear\PharmaTwoTracks(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'PLANET' => new \Com\Tecnick\Barcode\Type\Linear\Planet($code, $width, $height, $color, $params, $padding),
            'POSTNET' => new \Com\Tecnick\Barcode\Type\Linear\Postnet(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'RMS4CC' => new \Com\Tecnick\Barcode\Type\Linear\RoyalMailFourCc(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'S25' => new \Com\Tecnick\Barcode\Type\Linear\StandardTwoOfFive(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'S25+' => new \Com\Tecnick\Barcode\Type\Linear\StandardTwoOfFiveCheck(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'UPCA' => new \Com\Tecnick\Barcode\Type\Linear\UpcA($code, $width, $height, $color, $params, $padding),
            'UPCE' => new \Com\Tecnick\Barcode\Type\Linear\UpcE($code, $width, $height, $color, $params, $padding),
            'AZTEC' => new \Com\Tecnick\Barcode\Type\Square\Aztec($code, $width, $height, $color, $params, $padding),
            'DATAMATRIX' => new \Com\Tecnick\Barcode\Type\Square\Datamatrix(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'PDF417' => new \Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'QRCODE' => new \Com\Tecnick\Barcode\Type\Square\QrCode($code, $width, $height, $color, $params, $padding),
            'SRAW' => new \Com\Tecnick\Barcode\Type\Square\Raw($code, $width, $height, $color, $params, $padding),
            default => throw new BarcodeException('Unsupported barcode type: ' . $type),
        };
    }
}
