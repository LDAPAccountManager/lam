<?php

declare(strict_types=1);

/**
 * HanXin.php
 *
 * @since       2026-09-02
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
use Com\Tecnick\Barcode\Type\Square\HanXin\Data;
use Com\Tecnick\Barcode\Type\Square\HanXin\Encode;
use Com\Tecnick\Barcode\Type\Square\HanXin\HanXinEccLevel;

/**
 * Com\Tecnick\Barcode\Type\Square\HanXin
 *
 * HanXin Barcode type class
 * Han Xin Code (GB/T 21049, ISO/IEC 20830)
 *
 * Matrix symbology with four position detection patterns, a stepped alignment
 * pattern and eighty four symbol versions.
 *     Symbol sizes:                23x23 to 189x189 modules, in steps of two
 *     Error correction levels:     L1, L2, L3 and L4
 *     Maximum data characters:     7827 digits, 4350 text characters or 3261 bytes
 *
 * The encoder uses the numeric, Text, binary byte, common Chinese character
 * region one and two and GB 18030 two and four byte region modes. The ECI,
 * Unicode, GS1 and URI modes are not available.
 *
 * @since       2026-09-02
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class HanXin extends \Com\Tecnick\Barcode\Type\Square
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'HANXIN';

    /**
     * Error correction level, from 1 to 4, that is L1 to L4.
     */
    protected int $level = 1;

    /**
     * Symbol version, from 1 to 84, or 0 to select the smallest symbol able to
     * carry the data.
     */
    protected int $version = 0;

    /**
     * Data mask pattern reference, from 0 to 3, or a negative value to select
     * the one with the lowest penalty score.
     */
    protected int $mask = -1;

    /**
     * Set extra (optional) parameters:
     *     1: LEVEL   - error correction level: L1, L2, L3 or L4
     *     2: VERSION - symbol version: 1 to 84, or 0 for automatic
     *     3: MASK    - mask pattern reference: 0 to 3, or empty for automatic
     */
    protected function setParameters(): void
    {
        parent::setParameters();

        $eccLevel = HanXinEccLevel::fromLoose(\strval($this->params[0] ?? ''));
        $this->params[0] = $eccLevel->value;
        $this->level = $eccLevel->getLevel();

        $version = (int) ($this->params[1] ?? 0);
        if ($version < 1 || $version > Data::VERSION_MAX) {
            $version = 0;
        }

        $this->params[1] = $version;
        $this->version = $version;

        $mask = -1;
        if (($this->params[2] ?? '') !== '' && $this->params[2] >= 0 && $this->params[2] <= 3) {
            $mask = (int) $this->params[2];
        }

        $this->params[2] = $mask;
        $this->mask = $mask;
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
            $encode = new Encode($this->code, $this->level, $this->version, $this->mask);
        } catch (BarcodeException $barcodeException) {
            throw new BarcodeException('HANXIN: ' . $barcodeException->getMessage());
        }

        $this->version = $encode->getVersion();
        $this->processBinarySequence($encode->getGrid());
    }
}
