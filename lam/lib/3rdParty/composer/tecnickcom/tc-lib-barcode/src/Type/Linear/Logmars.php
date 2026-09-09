<?php

declare(strict_types=1);

/**
 * Logmars.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Linear;

use Com\Tecnick\Barcode\Exception as BarcodeException;

/**
 * Com\Tecnick\Barcode\Type\Linear\Logmars;
 *
 * Logmars Barcode type class
 * LOGMARS (MIL-STD-1189B)
 *
 * CODE 39 profile of the US Department of Defense Logistics Applications of
 * Automated Marking and Reading Symbols. The code is restricted to the 43
 * character set of CODE 39, lowercase letters are folded to uppercase, and the
 * modulo 43 check character defined by the standard is always appended.
 * The standard also constrains density, wide to narrow ratio and bar height,
 * which are set by the caller through the barcode width and height.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Logmars extends \Com\Tecnick\Barcode\Type\Linear\CodeThreeNineCheck
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'LOGMARS';

    /**
     * Format code
     *
     * @throws BarcodeException if the code is empty or contains a character
     *                          outside the CODE 39 character set
     */
    protected function formatCode(): void
    {
        if ($this->code === '') {
            throw new BarcodeException('Empty input');
        }

        $this->validateNoStartStop($this->code);
        $code = \strtoupper($this->code);
        $clen = \strlen($code);
        for ($pos = 0; $pos < $clen; ++$pos) {
            if (!\array_key_exists($code[$pos], $this::CHBAR)) {
                throw new BarcodeException('Invalid character: ' . (\ord($code[$pos]) & 0xFF));
            }
        }

        $this->extcode = '*' . $code . $this->getChecksum($code) . '*';
    }
}
