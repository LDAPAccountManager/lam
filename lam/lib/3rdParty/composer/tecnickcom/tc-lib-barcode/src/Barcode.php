<?php

declare(strict_types=1);

/**
 * Barcode.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Barcode
 *
 * Barcode factory class
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Barcode
{
    /**
     * Maximum accepted length in bytes of the barcode payload.
     * Longer payloads are rejected by getBarcodeObj().
     */
    public const MAX_CODE_LENGTH = 30_000;

    /**
     * List of supported Barcode Types with description.
     *
     * @var array<string, string>
     */
    public const BARCODETYPES = [
        'AUSPOST' => 'Australia Post 4-State Customer Barcode',
        'C128' => 'CODE 128',
        'C128A' => 'CODE 128 A',
        'C128B' => 'CODE 128 B',
        'C128C' => 'CODE 128 C',
        'C16K' => 'CODE 16K (stacked CODE 128)',
        'C32' => 'CODE 32 (Italian Pharmacode - IMH - Radix 32)',
        'C49' => 'CODE 49 (ANSI/AIM BC6)',
        'C39' => 'CODE 39 - ANSI MH10.8M-1983 - USD-3 - 3 of 9.',
        'C39+' => 'CODE 39 + CHECKSUM',
        'C39E' => 'CODE 39 EXTENDED',
        'C39E+' => 'CODE 39 EXTENDED + CHECKSUM',
        'C93' => 'CODE 93 - USS-93',
        'CODABAR' => 'CODABAR',
        'CODE11' => 'CODE 11',
        'DATABAR' => 'GS1 DataBar Omnidirectional (ISO/IEC 24724)',
        'DATABAREXP' => 'GS1 DataBar Expanded (ISO/IEC 24724)',
        'DATABAREXPSTACK' => 'GS1 DataBar Expanded Stacked (ISO/IEC 24724)',
        'DATABARLIMITED' => 'GS1 DataBar Limited (ISO/IEC 24724)',
        'DATABARSTACK' => 'GS1 DataBar Stacked (ISO/IEC 24724)',
        'DATABARSTACKOMNI' => 'GS1 DataBar Stacked Omnidirectional (ISO/IEC 24724)',
        'DATABARTRUNC' => 'GS1 DataBar Truncated (ISO/IEC 24724)',
        'EAN13' => 'EAN 13',
        'EAN2' => 'EAN 2-Digits UPC-Based Extension',
        'EAN5' => 'EAN 5-Digits UPC-Based Extension',
        'EAN8' => 'EAN 8',
        'GS114' => 'GS1-14 - EAN-14 - SCC-14 (GS1-128 with the Application Identifier 01)',
        'GS1128' => 'GS1-128 (CODE 128 with GS1 Application Identifiers)',
        'HIBC128' => 'HIBC in CODE 128 (ANSI/HIBC 2.6 and ANSI/HIBC 1.3)',
        'HIBC39' => 'HIBC in CODE 39 (ANSI/HIBC 2.6 and ANSI/HIBC 1.3)',
        'I25' => 'Interleaved 2 of 5',
        'I25+' => 'Interleaved 2 of 5 + CHECKSUM',
        'IDENTCODE' => 'Deutsche Post Identcode',
        'IMB' => 'IMB - Intelligent Mail Barcode - Onecode - USPS-B-3200',
        'IMBPRE' => 'IMB - Intelligent Mail Barcode pre-processed',
        'ITF14' => 'ITF-14 (GTIN-14 - GS1 General Specifications)',
        'JPPOST' => 'Japan Post Customer Barcode',
        'KIX' => 'KIX (Klant index - Customer index)',
        'LEITCODE' => 'Deutsche Post Leitcode',
        'LOGMARS' => 'LOGMARS (CODE 39 profile of MIL-STD-1189B)',
        'LRAW' => '1D RAW MODE (comma-separated rows of 01 strings)',
        'MAILMARK' => 'Royal Mail Mailmark 4-state barcode (types C and L)',
        'MSI' => 'MSI (Variation of Plessey code)',
        'MSI+' => 'MSI + CHECKSUM (modulo 11)',
        'PHARMA' => 'PHARMACODE',
        'PHARMA2T' => 'PHARMACODE TWO-TRACKS',
        'PLANET' => 'PLANET',
        'PLESSEY' => 'Plessey Code',
        'POSTNET' => 'POSTNET',
        'PZN' => 'PZN (Pharmazentralnummer - IFA coding system)',
        'RMS4CC' => 'RMS4CC (Royal Mail 4-state Customer Bar Code)',
        'S25' => 'Standard 2 of 5',
        'S25+' => 'Standard 2 of 5 + CHECKSUM',
        'S25DATALOGIC' => '2 of 5 Datalogic (China Post Code)',
        'S25IATA' => '2 of 5 IATA (Computer Identics 2 of 5)',
        'S25MATRIX' => '2 of 5 Matrix',
        'SSCC18' => 'SSCC-18 (GS1-128 with the Application Identifier 00)',
        'TELEPEN' => 'Telepen (full ASCII)',
        'UPCA' => 'UPC-A',
        'UPCE' => 'UPC-E',
        'AZTEC' => 'AZTEC Code (ISO/IEC 24778:2008)',
        'AZTECRUNE' => 'AZTEC Rune (ISO/IEC 24778:2008 Annex A)',
        'DATAMATRIX' => 'DATAMATRIX (ISO/IEC 16022)',
        'DMRE' => 'Data Matrix Rectangular Extension (ISO/IEC 21471)',
        'HANXIN' => 'Han Xin Code (GB/T 21049, ISO/IEC 20830)',
        'HIBCAZ' => 'HIBC in AZTEC Code (ANSI/HIBC 2.6 and ANSI/HIBC 1.3)',
        'HIBCDM' => 'HIBC in DATAMATRIX (ANSI/HIBC 2.6 and ANSI/HIBC 1.3)',
        'HIBCQR' => 'HIBC in QR-CODE (ANSI/HIBC 2.6 and ANSI/HIBC 1.3)',
        'MICROQR' => 'Micro QR Code (ISO/IEC 18004)',
        'PDF417' => 'PDF417 (ISO/IEC 15438:2006)',
        'PDF417C' => 'Compact PDF417 - truncated (ISO/IEC 15438:2006)',
        'QRCODE' => 'QR-CODE',
        'SRAW' => '2D RAW MODE (comma-separated rows of 01 strings)',
    ];

    /**
     * Get the barcode object
     *
     * @param string|BarcodeType        $type    Barcode type (leading token, optionally followed by
     *                                           comma-separated extra parameters), or a BarcodeType enum case
     * @param string                    $code    Barcode content
     * @param int                       $width   Barcode width in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each column.
     * @param int                       $height  Barcode height in user units (excluding padding).
     *                                           A negative value indicates the multiplication
     *                                           factor for each row.
     * @param string                    $color   Foreground color in Web notation
     *                                           (color name, or hexadecimal code, or CSS syntax)
     *                                           or PDF spot color name
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *                                           (top, right, bottom, left) in user units. A
     *                                           negative value indicates the multiplication
     *                                           factor for each row or column.
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color parsing errors
     */
    public function getBarcodeObj(
        string|BarcodeType $type,
        string $code,
        int $width = -1,
        int $height = -1,
        string $color = 'black',
        array $padding = [0, 0, 0, 0],
    ): Model {
        if ($type instanceof BarcodeType) {
            $type = $type->value;
        }

        if (\strlen($code) > self::MAX_CODE_LENGTH) {
            throw new BarcodeException(
                'The barcode payload is too long: ' . \strlen($code) . ' bytes (maximum ' . self::MAX_CODE_LENGTH . ')',
            );
        }

        // extract extra parameters (if any)
        $params = \explode(',', $type);
        $type = \array_shift($params);

        return (
            $this->getLinearObj($type, $code, $width, $height, $color, $params, $padding) ?? $this->getSquareObj(
                $type,
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ) ?? throw new BarcodeException('Unsupported barcode type: ' . $type)
        );
    }

    /**
     * Get the barcode object of a linear type, or null when the type is not a linear one.
     *
     * @param string                    $type    Barcode type token
     * @param string                    $code    Barcode content
     * @param int                       $width   Barcode width in user units (excluding padding)
     * @param int                       $height  Barcode height in user units (excluding padding)
     * @param string                    $color   Foreground color
     * @param array<int|float|string>   $params  Extra parameters for the specified barcode type
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color parsing errors
     */
    protected function getLinearObj(
        string $type,
        string $code,
        int $width,
        int $height,
        string $color,
        array $params,
        array $padding,
    ): ?Model {
        return (
            $this->getLinearObjAtoH(
                $type,
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ) ?? $this->getLinearObjItoZ($type, $code, $width, $height, $color, $params, $padding)
        );
    }

    /**
     * Get the barcode object of a linear type whose token precedes I, or null.
     *
     * The linear types are split over two methods because a single match crosses
     * the linter Halstead volume threshold.
     *
     * @param string                    $type    Barcode type token
     * @param string                    $code    Barcode content
     * @param int                       $width   Barcode width in user units (excluding padding)
     * @param int                       $height  Barcode height in user units (excluding padding)
     * @param string                    $color   Foreground color
     * @param array<int|float|string>   $params  Extra parameters for the specified barcode type
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color parsing errors
     */
    protected function getLinearObjAtoH(
        string $type,
        string $code,
        int $width,
        int $height,
        string $color,
        array $params,
        array $padding,
    ): ?Model {
        return match ($type) {
            'AUSPOST' => new \Com\Tecnick\Barcode\Type\Linear\AustraliaPost(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
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
            'C16K' => new \Com\Tecnick\Barcode\Type\Linear\CodeOneSixK(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C49' => new \Com\Tecnick\Barcode\Type\Linear\CodeFourNine(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'C32' => new \Com\Tecnick\Barcode\Type\Linear\CodeThreeTwo(
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
            'DATABAR' => new \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarOmni(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DATABAREXP' => new \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarExpanded(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DATABAREXPSTACK' => new \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarExpandedStacked(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DATABARLIMITED' => new \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarLimited(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DATABARSTACK' => new \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarStacked(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DATABARSTACKOMNI' => new \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarStackedOmni(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DATABARTRUNC' => new \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarTruncated(
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
            'GS114' => new \Com\Tecnick\Barcode\Type\Linear\GsOneOneFour(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'GS1128' => new \Com\Tecnick\Barcode\Type\Linear\GsOneOneTwoEight(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'HIBC128' => new \Com\Tecnick\Barcode\Type\Linear\HibcOneTwoEight(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'HIBC39' => new \Com\Tecnick\Barcode\Type\Linear\HibcThreeNine(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            default => null,
        };
    }

    /**
     * Get the barcode object of a linear type whose token starts with I or later, or null.
     *
     * @param string                    $type    Barcode type token
     * @param string                    $code    Barcode content
     * @param int                       $width   Barcode width in user units (excluding padding)
     * @param int                       $height  Barcode height in user units (excluding padding)
     * @param string                    $color   Foreground color
     * @param array<int|float|string>   $params  Extra parameters for the specified barcode type
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color parsing errors
     */
    protected function getLinearObjItoZ(
        string $type,
        string $code,
        int $width,
        int $height,
        string $color,
        array $params,
        array $padding,
    ): ?Model {
        return match ($type) {
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
            'IDENTCODE' => new \Com\Tecnick\Barcode\Type\Linear\Identcode(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'IMB' => new \Com\Tecnick\Barcode\Type\Linear\Imb($code, $width, $height, $color, $params, $padding),
            'IMBPRE' => new \Com\Tecnick\Barcode\Type\Linear\ImbPre($code, $width, $height, $color, $params, $padding),
            'ITF14' => new \Com\Tecnick\Barcode\Type\Linear\ItfOneFour(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'JPPOST' => new \Com\Tecnick\Barcode\Type\Linear\JapanPost(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'KIX' => new \Com\Tecnick\Barcode\Type\Linear\KlantIndex($code, $width, $height, $color, $params, $padding),
            'LEITCODE' => new \Com\Tecnick\Barcode\Type\Linear\Leitcode(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'LOGMARS' => new \Com\Tecnick\Barcode\Type\Linear\Logmars(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'LRAW' => new \Com\Tecnick\Barcode\Type\Linear\Raw($code, $width, $height, $color, $params, $padding),
            'MAILMARK' => new \Com\Tecnick\Barcode\Type\Linear\Mailmark(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
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
            'PLESSEY' => new \Com\Tecnick\Barcode\Type\Linear\Plessey(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'POSTNET' => new \Com\Tecnick\Barcode\Type\Linear\Postnet(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'PZN' => new \Com\Tecnick\Barcode\Type\Linear\Pzn($code, $width, $height, $color, $params, $padding),
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
            'S25DATALOGIC' => new \Com\Tecnick\Barcode\Type\Linear\DatalogicTwoOfFive(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'S25IATA' => new \Com\Tecnick\Barcode\Type\Linear\IataTwoOfFive(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'S25MATRIX' => new \Com\Tecnick\Barcode\Type\Linear\MatrixTwoOfFive(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'SSCC18' => new \Com\Tecnick\Barcode\Type\Linear\SsccOneEight(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'TELEPEN' => new \Com\Tecnick\Barcode\Type\Linear\Telepen(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'UPCA' => new \Com\Tecnick\Barcode\Type\Linear\UpcA($code, $width, $height, $color, $params, $padding),
            'UPCE' => new \Com\Tecnick\Barcode\Type\Linear\UpcE($code, $width, $height, $color, $params, $padding),
            default => null,
        };
    }

    /**
     * Get the barcode object of a 2D type, or null when the type is not a 2D one.
     *
     * @param string                    $type    Barcode type token
     * @param string                    $code    Barcode content
     * @param int                       $width   Barcode width in user units (excluding padding)
     * @param int                       $height  Barcode height in user units (excluding padding)
     * @param string                    $color   Foreground color
     * @param array<int|float|string>   $params  Extra parameters for the specified barcode type
     * @param array{int, int, int, int} $padding Additional padding to add around the barcode
     *
     * @throws BarcodeException in case of error
     * @throws \Com\Tecnick\Color\Exception in case of color parsing errors
     */
    protected function getSquareObj(
        string $type,
        string $code,
        int $width,
        int $height,
        string $color,
        array $params,
        array $padding,
    ): ?Model {
        return match ($type) {
            'AZTEC' => new \Com\Tecnick\Barcode\Type\Square\Aztec($code, $width, $height, $color, $params, $padding),
            'AZTECRUNE' => new \Com\Tecnick\Barcode\Type\Square\AztecRune(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DATAMATRIX' => new \Com\Tecnick\Barcode\Type\Square\Datamatrix(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'DMRE' => new \Com\Tecnick\Barcode\Type\Square\Dmre($code, $width, $height, $color, $params, $padding),
            'HANXIN' => new \Com\Tecnick\Barcode\Type\Square\HanXin($code, $width, $height, $color, $params, $padding),
            'HIBCAZ' => new \Com\Tecnick\Barcode\Type\Square\HibcAztec(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'HIBCDM' => new \Com\Tecnick\Barcode\Type\Square\HibcDatamatrix(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'HIBCQR' => new \Com\Tecnick\Barcode\Type\Square\HibcQrCode(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'MICROQR' => new \Com\Tecnick\Barcode\Type\Square\MicroQrCode(
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
            'PDF417C' => new \Com\Tecnick\Barcode\Type\Square\PdfFourOneSevenCompact(
                $code,
                $width,
                $height,
                $color,
                $params,
                $padding,
            ),
            'QRCODE' => new \Com\Tecnick\Barcode\Type\Square\QrCode($code, $width, $height, $color, $params, $padding),
            'SRAW' => new \Com\Tecnick\Barcode\Type\Square\Raw($code, $width, $height, $color, $params, $padding),
            default => null,
        };
    }
}
