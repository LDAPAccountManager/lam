<?php

declare(strict_types=1);

/**
 * PharmaTwoTracks.php
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
 * Com\Tecnick\Barcode\Type\Linear\PharmaTwoTracks;
 *
 * PharmaTwoTracks Barcode type class
 * PHARMACODE TWO-TRACKS
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class PharmaTwoTracks extends \Com\Tecnick\Barcode\Type\Linear
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'PHARMA2T';

    /**
     * Set the bars array.
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        if (!\ctype_digit($this->code) || (int) $this->code < 1) {
            throw new BarcodeException('Invalid barcode value: the code must be a positive integer');
        }

        $seq = '';
        $code = (int) $this->code;

        do {
            switch ($code % 3) {
                case 0:
                    $seq .= '3';
                    $code = ($code - 3) / 3;
                    break;
                case 1:
                    $seq .= '1';
                    $code = ($code - 1) / 3;
                    break;
                case 2:
                    $seq .= '2';
                    $code = ($code - 2) / 3;
            }
        } while ($code !== 0);

        $seq = \strrev($seq);
        $this->ncols = 0;
        $this->nrows = 2;
        $this->bars = [];
        $len = \strlen($seq);
        for ($pos = 0; $pos < $len; ++$pos) {
            switch ($seq[$pos]) {
                case '1':
                    $this->bars[] = [$this->ncols, 1, 1, 1];
                    break;
                case '2':
                    $this->bars[] = [$this->ncols, 0, 1, 1];
                    break;
                case '3':
                    $this->bars[] = [$this->ncols, 0, 1, 2];
                    break;
            }

            $this->ncols += 2;
        }

        --$this->ncols;
    }
}
