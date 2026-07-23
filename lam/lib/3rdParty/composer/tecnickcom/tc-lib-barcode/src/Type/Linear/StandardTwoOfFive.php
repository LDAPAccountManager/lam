<?php

declare(strict_types=1);

/**
 * StandardTwoOfFive.php
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
 * Com\Tecnick\Barcode\Type\Linear\StandardTwoOfFive;
 *
 * StandardTwoOfFive Barcode type class
 * Standard 2 of 5
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class StandardTwoOfFive extends \Com\Tecnick\Barcode\Type\Linear\StandardTwoOfFiveCheck
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'S25';

    /**
     * Format code
     */
    protected function formatCode(): void
    {
        $this->extcode = $this->code;
    }
}
