<?php

declare(strict_types=1);

/**
 * HibcAztec.php
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

namespace Com\Tecnick\Barcode\Type\Square;

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\HibcPayload;

/**
 * Com\Tecnick\Barcode\Type\Square\HibcAztec
 *
 * HibcAztec Barcode type class
 * HIBC in Aztec Code (ANSI/HIBC 2.6 and ANSI/HIBC 1.3)
 *
 * Aztec Code symbol carrying a Health Industry Bar Code data structure. The
 * input is the data structure without its modulo 43 check character, which is
 * appended by Hibc. The data structure is encoded as it is, without the data
 * format envelope of ISO/IEC 15434.
 *
 * HIBC and HIBCC are trademarks of the Health Industry Business
 * Communications Council.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class HibcAztec extends \Com\Tecnick\Barcode\Type\Square\Aztec
{
    use HibcPayload;

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'HIBCAZ';

    /**
     * Get the bars array
     *
     * @throws BarcodeException in case of error
     */
    protected function setBars(): void
    {
        $this->extcode = $this->getHibcExtendedCode($this->code);
        parent::setBars();
    }
}
