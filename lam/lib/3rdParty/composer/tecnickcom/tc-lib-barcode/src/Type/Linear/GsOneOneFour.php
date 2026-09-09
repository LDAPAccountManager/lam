<?php

declare(strict_types=1);

/**
 * GsOneOneFour.php
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
 * Com\Tecnick\Barcode\Type\Linear\GsOneOneFour;
 *
 * GsOneOneFour Barcode type class
 * GS1-14 - EAN-14 - SCC-14 (GS1 General Specifications)
 *
 * GS1-128 symbol carrying the single element string of the Global Trade Item
 * Number, Application Identifier (01). The input is the plain digit string: a
 * code of the full length must carry a valid check digit, a shorter one is
 * left-padded with zeros and the check digit is appended.
 *
 * GS1 and GS1-128 are registered trademarks of GS1 AISBL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class GsOneOneFour extends \Com\Tecnick\Barcode\Type\Linear\SsccOneEight
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'GS114';

    /**
     * Application Identifier of the element string
     *
     * @var string
     */
    protected const APPID = '01';

    /**
     * Fixed code length, check digit included
     */
    protected const CODE_LENGTH = 14;
}
