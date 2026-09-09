<?php

declare(strict_types=1);

/**
 * MatrixTwoOfFive.php
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

/**
 * Com\Tecnick\Barcode\Type\Linear\MatrixTwoOfFive;
 *
 * MatrixTwoOfFive Barcode type class
 * 2 of 5 Matrix
 *
 * Discrete two width symbology that carries data in both the bars and the
 * spaces. Each digit is three bars and two spaces, two of the five elements
 * being wide, and each character is followed by a narrow separator space.
 * The check digit is optional and is not added.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class MatrixTwoOfFive extends \Com\Tecnick\Barcode\Type\Linear\StandardTwoOfFive
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'S25MATRIX';

    /**
     * Map characters to barcodes, including the separator space that follows
     * each character
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '1011100010',
        '1' => '1110101110',
        '2' => '1000101110',
        '3' => '1110001010',
        '4' => '1011101110',
        '5' => '1110111010',
        '6' => '1000111010',
        '7' => '1010001110',
        '8' => '1110100010',
        '9' => '1000100010',
    ];

    /**
     * Start pattern, including the separator space that follows it
     *
     * @var string
     */
    protected const START = '11101010';

    /**
     * Stop pattern
     *
     * @var string
     */
    protected const STOP = '1110101';
}
