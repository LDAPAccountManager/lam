<?php

declare(strict_types=1);

/**
 * Identcode.php
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
 * Com\Tecnick\Barcode\Type\Linear\Identcode;
 *
 * Identcode Barcode type class
 * Deutsche Post Identcode
 *
 * Twelve digits: two for the originating freight centre, three for the customer
 * identifier, six for the consignment number and the check digit.
 *
 * Identcode is a trademark of Deutsche Post DHL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Identcode extends \Com\Tecnick\Barcode\Type\Linear\DeutschePost
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'IDENTCODE';

    /**
     * Total number of digits, including the check digit
     */
    protected const CODE_LENGTH = 12;
}
