<?php

declare(strict_types=1);

/**
 * PdfFourOneSevenCompact.php
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 *
 * This file is part of tc-lib-barcode software library.
 */

namespace Com\Tecnick\Barcode\Type\Square;

/**
 * Com\Tecnick\Barcode\Type\Square\PdfFourOneSevenCompact
 *
 * PdfFourOneSevenCompact Barcode type class
 * Compact PDF417, also known as truncated PDF417 (ISO/IEC 15438:2006)
 *
 * PDF417 symbol with the right row indicator column removed and the stop
 * pattern reduced to a single narrow bar, which saves 34 modules per row.
 * It is intended for clean printing environments, where the loss of the right
 * row indicator does not compromise scanning.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2015-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class PdfFourOneSevenCompact extends \Com\Tecnick\Barcode\Type\Square\PdfFourOneSeven
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'PDF417C';

    /**
     * Truncated symbol: the right row indicator and the stop pattern are
     * replaced by a single narrow bar.
     */
    protected bool $truncated = true;
}
