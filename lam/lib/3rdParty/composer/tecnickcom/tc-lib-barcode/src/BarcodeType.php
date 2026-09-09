<?php

declare(strict_types=1);

/**
 * BarcodeType.php
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\BarcodeType
 *
 * Backed enum for the supported barcode symbologies. The backing value of each
 * case is the leading type token accepted by Barcode::getBarcodeObj() (before
 * any comma-separated extra parameters).
 *
 * @since       2026-07-17
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
enum BarcodeType: string
{
    case AUSPOST = 'AUSPOST';

    case C128 = 'C128';

    case C128A = 'C128A';

    case C128B = 'C128B';

    case C128C = 'C128C';

    case C16K = 'C16K';

    case C32 = 'C32';

    case C49 = 'C49';

    case C39 = 'C39';

    case C39Plus = 'C39+';

    case C39E = 'C39E';

    case C39EPlus = 'C39E+';

    case C93 = 'C93';

    case CODABAR = 'CODABAR';

    case CODE11 = 'CODE11';

    case DATABAR = 'DATABAR';

    case DATABAREXP = 'DATABAREXP';

    case DATABAREXPSTACK = 'DATABAREXPSTACK';

    case DATABARLIMITED = 'DATABARLIMITED';

    case DATABARSTACK = 'DATABARSTACK';

    case DATABARSTACKOMNI = 'DATABARSTACKOMNI';

    case DATABARTRUNC = 'DATABARTRUNC';

    case EAN13 = 'EAN13';

    case EAN2 = 'EAN2';

    case EAN5 = 'EAN5';

    case EAN8 = 'EAN8';

    case GS114 = 'GS114';

    case GS1128 = 'GS1128';

    case HIBC128 = 'HIBC128';

    case HIBC39 = 'HIBC39';

    case I25 = 'I25';

    case I25Plus = 'I25+';

    case IDENTCODE = 'IDENTCODE';

    case IMB = 'IMB';

    case IMBPRE = 'IMBPRE';

    case ITF14 = 'ITF14';

    case JPPOST = 'JPPOST';

    case KIX = 'KIX';

    case LEITCODE = 'LEITCODE';

    case LOGMARS = 'LOGMARS';

    case LRAW = 'LRAW';

    case MAILMARK = 'MAILMARK';

    case MSI = 'MSI';

    case MSIPlus = 'MSI+';

    case PHARMA = 'PHARMA';

    case PHARMA2T = 'PHARMA2T';

    case PLANET = 'PLANET';

    case PLESSEY = 'PLESSEY';

    case POSTNET = 'POSTNET';

    case PZN = 'PZN';

    case RMS4CC = 'RMS4CC';

    case S25 = 'S25';

    case S25Plus = 'S25+';

    case S25DATALOGIC = 'S25DATALOGIC';

    case S25IATA = 'S25IATA';

    case S25MATRIX = 'S25MATRIX';

    case SSCC18 = 'SSCC18';

    case TELEPEN = 'TELEPEN';

    case UPCA = 'UPCA';

    case UPCE = 'UPCE';

    case AZTEC = 'AZTEC';

    case AZTECRUNE = 'AZTECRUNE';

    case DATAMATRIX = 'DATAMATRIX';

    case DMRE = 'DMRE';

    case HANXIN = 'HANXIN';

    case HIBCAZ = 'HIBCAZ';

    case HIBCDM = 'HIBCDM';

    case HIBCQR = 'HIBCQR';

    case MICROQR = 'MICROQR';

    case PDF417 = 'PDF417';

    case PDF417C = 'PDF417C';

    case QRCODE = 'QRCODE';

    case SRAW = 'SRAW';

    /**
     * Resolve a loose barcode type token to the matching enum case.
     *
     * Accepts the exact leading type token (as validated by getBarcodeObj) or an
     * enum instance (returned unchanged). Unknown values throw.
     *
     * @param string|self $value Barcode type token or enum case.
     *
     * @throws BarcodeException if the value does not match a known barcode type.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new BarcodeException('Unsupported barcode type: ' . $value);
    }
}
