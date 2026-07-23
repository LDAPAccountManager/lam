<?php

declare(strict_types=1);

/**
 * KlantIndex.php
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\KlantIndex;
 *
 * KlantIndex Barcode type class
 * KIX (Klant index - Customer index)
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class KlantIndex extends \Com\Tecnick\Barcode\Type\Linear\RoyalMailFourCc
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'KIX';

    /**
     * Format code
     *
     * @throws BarcodeException in case of error
     */
    protected function formatCode(): void
    {
        $code = \strtoupper($this->code);
        $len = \strlen($code);
        for ($pos = 0; $pos < $len; ++$pos) {
            if (!\array_key_exists($code[$pos], $this::CHBAR)) {
                throw new BarcodeException('Invalid character: ' . (\ord($code[$pos]) & 0xFF));
            }
        }

        $this->extcode = $code;
    }

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->ncols = 0;
        $this->nrows = 3;
        $this->bars = [];
        $this->getCoreBars();
    }
}
