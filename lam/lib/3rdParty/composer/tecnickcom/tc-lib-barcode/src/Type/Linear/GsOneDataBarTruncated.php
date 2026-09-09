<?php

declare(strict_types=1);

/**
 * GsOneDataBarTruncated.php
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
 * Com\Tecnick\Barcode\Type\Linear\GsOneDataBarTruncated;
 *
 * GsOneDataBarTruncated Barcode type class
 * GS1 DataBar Truncated (ISO/IEC 24724)
 *
 * Reduced height variation of GS1 DataBar Omnidirectional, 96 modules wide by
 * 13 modules high. The encoding is identical.
 *
 * GS1 and GS1 DataBar are registered trademarks of GS1 AISBL.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class GsOneDataBarTruncated extends \Com\Tecnick\Barcode\Type\Linear\GsOneDataBarOmni
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DATABARTRUNC';

    /**
     * Symbol height in modules
     */
    protected const SYMBOL_HEIGHT = 13;
}
