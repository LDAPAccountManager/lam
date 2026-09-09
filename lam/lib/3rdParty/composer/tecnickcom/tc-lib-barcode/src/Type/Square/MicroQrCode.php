<?php

declare(strict_types=1);

/**
 * MicroQrCode.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\Square\MicroQrCode\Encode;
use Com\Tecnick\Barcode\Type\Square\MicroQrCode\MicroQrEccLevel;
use Com\Tecnick\Barcode\Type\Square\MicroQrCode\MicroQrEncodingMode;

/**
 * Com\Tecnick\Barcode\Type\Square\MicroQrCode
 *
 * MicroQrCode Barcode type class
 * Micro QR Code (ISO/IEC 18004)
 *
 * Matrix symbology with a single finder pattern, four symbol sizes and a fixed
 * error correction level per size.
 *     Symbol sizes:                M1 11x11, M2 13x13, M3 15x15, M4 17x17 modules
 *     Error correction levels:     L, M and Q; M1 carries error detection only
 *     Maximum data characters:     35 digits, 21 alphanumeric or 15 bytes
 *
 * QR Code is a registered trademark of DENSO WAVE INCORPORATED.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class MicroQrCode extends \Com\Tecnick\Barcode\Type\Square
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'MICROQR';

    /**
     * Error correction level: L, M, Q, or an empty string to select the
     * smallest symbol able to carry the data.
     */
    protected string $level = '';

    /**
     * Symbol version, from 1 to 4 for M1 to M4, or 0 to select the smallest
     * symbol able to carry the data.
     */
    protected int $version = 0;

    /**
     * Encodation mode, or a negative value to select the mode that yields the
     * shortest bit stream.
     */
    protected int $mode = -1;

    /**
     * Set extra (optional) parameters:
     *     1: LEVEL   - error correction level: L, M, Q, or empty for automatic
     *     2: VERSION - symbol version: 1 to 4 for M1 to M4, or 0 for automatic
     *     3: HINT    - encodation mode: NM=numeric, AN=alphanumeric, 8B=8bit, or empty for automatic
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        $eccLevel = MicroQrEccLevel::fromLoose(\strval($this->params[0] ?? ''));
        $this->params[0] = $eccLevel->value;
        $this->level = $eccLevel->value;

        $version = (int) ($this->params[1] ?? 0);
        if ($version < 1 || $version > 4) {
            $version = 0;
        }

        $this->params[1] = $version;
        $this->version = $version;

        $encMode = MicroQrEncodingMode::fromLoose(\strval($this->params[2] ?? ''));
        $this->params[2] = $encMode->value;
        $this->mode = $encMode->getMode();
    }

    /**
     * Get the bars array
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (\strlen($this->code) === 0) {
            throw new BarcodeException('Empty input');
        }

        try {
            $encode = new Encode($this->code, $this->level, $this->version, $this->mode);
            $this->processBinarySequence($encode->getGrid());
        } catch (BarcodeException $barcodeException) {
            throw new BarcodeException('MICROQR: ' . $barcodeException->getMessage());
        }
    }
}
