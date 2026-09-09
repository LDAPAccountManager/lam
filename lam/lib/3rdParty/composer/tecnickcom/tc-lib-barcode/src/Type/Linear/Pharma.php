<?php

declare(strict_types=1);

/**
 * Pharma.php
 *
 * @since       2015-02-21
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
 * Com\Tecnick\Barcode\Type\Linear\Pharma;
 *
 * Pharma Barcode type class
 * PHARMACODE
 *
 * PHARMA-CODE is a trademark of Laetus GmbH.
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Pharma extends \Com\Tecnick\Barcode\Type\Linear
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'PHARMA';

    /**
     * Lowest encodable value (3 bars).
     *
     * @var int
     */
    protected const MIN_VALUE = 3;

    /**
     * Highest encodable value (16 bars).
     *
     * @var int
     */
    protected const MAX_VALUE = 131_070;

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (
            !\ctype_digit($this->code)
            || \strlen(\ltrim($this->code, '0')) > \strlen((string) $this::MAX_VALUE)
            || (int) $this->code < $this::MIN_VALUE
            || (int) $this->code > $this::MAX_VALUE
        ) {
            throw new BarcodeException(
                'Invalid barcode value: the code must be an integer between '
                . $this::MIN_VALUE
                . ' and '
                . $this::MAX_VALUE,
            );
        }

        $seq = '';
        $code = (int) $this->code;
        while ($code > 0) {
            if (($code % 2) === 0) {
                $seq .= '11100';
                $code -= 2;
                $code = \intdiv($code, 2);
                continue;
            }

            $seq .= '100';
            --$code;
            $code = \intdiv($code, 2);
        }

        $seq = \substr($seq, 0, -2);
        $seq = \strrev($seq);
        $this->processBinarySequence($this->getRawCodeRows($seq));
    }
}
