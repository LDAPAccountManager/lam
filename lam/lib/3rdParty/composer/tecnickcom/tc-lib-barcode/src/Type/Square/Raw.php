<?php

declare(strict_types=1);

/**
 * Raw.php
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

namespace Com\Tecnick\Barcode\Type\Square;

/**
 * Com\Tecnick\Barcode\Type\Square\Raw
 *
 * Raw Barcode type class
 * RAW MODE (comma-separated rows)
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Raw extends \Com\Tecnick\Barcode\Type\Raw
{
    /**
     * Barcode type
     *
     * @var string
     */
    protected const TYPE = 'square';

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'SRAW';
}
