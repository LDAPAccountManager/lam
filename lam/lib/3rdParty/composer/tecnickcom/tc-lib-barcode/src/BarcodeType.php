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
    case C128 = 'C128';

    case C128A = 'C128A';

    case C128B = 'C128B';

    case C128C = 'C128C';

    case C39 = 'C39';

    case C39Plus = 'C39+';

    case C39E = 'C39E';

    case C39EPlus = 'C39E+';

    case C93 = 'C93';

    case CODABAR = 'CODABAR';

    case CODE11 = 'CODE11';

    case EAN13 = 'EAN13';

    case EAN2 = 'EAN2';

    case EAN5 = 'EAN5';

    case EAN8 = 'EAN8';

    case I25 = 'I25';

    case I25Plus = 'I25+';

    case IMB = 'IMB';

    case IMBPRE = 'IMBPRE';

    case KIX = 'KIX';

    case LRAW = 'LRAW';

    case MSI = 'MSI';

    case MSIPlus = 'MSI+';

    case PHARMA = 'PHARMA';

    case PHARMA2T = 'PHARMA2T';

    case PLANET = 'PLANET';

    case POSTNET = 'POSTNET';

    case RMS4CC = 'RMS4CC';

    case S25 = 'S25';

    case S25Plus = 'S25+';

    case UPCA = 'UPCA';

    case UPCE = 'UPCE';

    case AZTEC = 'AZTEC';

    case DATAMATRIX = 'DATAMATRIX';

    case PDF417 = 'PDF417';

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
