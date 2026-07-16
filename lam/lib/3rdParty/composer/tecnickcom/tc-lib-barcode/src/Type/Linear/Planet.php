<?php

declare(strict_types=1);

/**
 * Planet.php
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
 * Com\Tecnick\Barcode\Type\Linear\Planet;
 *
 * Planet Barcode type class
 * PLANET
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Planet extends \Com\Tecnick\Barcode\Type\Linear\Postnet
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'PLANET';

    /**
     * Map characters to barcodes
     *
     * @var array<int|string, string>
     */
    protected const CHBAR = [
        '0' => '11222',
        '1' => '22211',
        '2' => '22121',
        '3' => '22112',
        '4' => '21221',
        '5' => '21212',
        '6' => '21122',
        '7' => '12221',
        '8' => '12212',
        '9' => '12122',
    ];
}
