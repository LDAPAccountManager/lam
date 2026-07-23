<?php

declare(strict_types=1);

/**
 * EanFive.php
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

/**
 * Com\Tecnick\Barcode\Type\Linear\EanFive;
 *
 * EanFive Barcode type class
 * EAN 5-Digits UPC-Based Extension
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class EanFive extends \Com\Tecnick\Barcode\Type\Linear\EanTwo
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'EAN5';

    /**
     * Fixed code length
     */
    protected int $code_length = 5;

    /**
     * Map parities
     *
     * @var array<int|string, array<string>>
     */
    protected const PARITIES = [
        '0' => ['B', 'B', 'A', 'A', 'A'],
        '1' => ['B', 'A', 'B', 'A', 'A'],
        '2' => ['B', 'A', 'A', 'B', 'A'],
        '3' => ['B', 'A', 'A', 'A', 'B'],
        '4' => ['A', 'B', 'B', 'A', 'A'],
        '5' => ['A', 'A', 'B', 'B', 'A'],
        '6' => ['A', 'A', 'A', 'B', 'B'],
        '7' => ['A', 'B', 'A', 'B', 'A'],
        '8' => ['A', 'B', 'A', 'A', 'B'],
        '9' => ['A', 'A', 'B', 'A', 'B'],
    ];

    /**
     * Calculate checksum
     *
     * @param string $code Code to represent.
     *
     * @return int char checksum.
     */
    protected function getChecksum(string $code): int
    {
        return (
            ((3 * ((int) $code[0] + (int) $code[2] + (int) $code[4])) + (9 * ((int) $code[1] + (int) $code[3]))) % 10
        );
    }
}
