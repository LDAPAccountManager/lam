<?php

declare(strict_types=1);

/**
 * Leitcode.php
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
 * Com\Tecnick\Barcode\Type\Linear\Leitcode;
 *
 * Leitcode Barcode type class
 * Deutsche Post Leitcode
 *
 * Fourteen digits: five for the postal code, three for the street code, three
 * for the house number, two for the product code and the check digit.
 *
 * Leitcode is a trademark of Deutsche Post DHL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Leitcode extends \Com\Tecnick\Barcode\Type\Linear\DeutschePost
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'LEITCODE';

    /**
     * Total number of digits, including the check digit
     */
    protected const CODE_LENGTH = 14;
}
