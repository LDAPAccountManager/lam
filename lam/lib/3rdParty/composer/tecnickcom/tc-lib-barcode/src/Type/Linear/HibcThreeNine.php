<?php

declare(strict_types=1);

/**
 * HibcThreeNine.php
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

use Com\Tecnick\Barcode\Exception as BarcodeException;
use Com\Tecnick\Barcode\Type\HibcPayload;

/**
 * Com\Tecnick\Barcode\Type\Linear\HibcThreeNine;
 *
 * HibcThreeNine Barcode type class
 * HIBC in CODE 39 (ANSI/HIBC 2.6 and ANSI/HIBC 1.3)
 *
 * CODE 39 symbol carrying a Health Industry Bar Code data structure. The input
 * is the data structure without its modulo 43 check character, which is
 * appended by Hibc. There is no CODE 39 check character: the modulo 43 check
 * character of the data structure is computed over the same character values.
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
class HibcThreeNine extends \Com\Tecnick\Barcode\Type\Linear\CodeThreeNine
{
    use HibcPayload;

    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'HIBC39';

    /**
     * Format code
     *
     * @throws BarcodeException in case of error
     */
    protected function formatCode(): void
    {
        $this->extcode = $this->getHibcExtendedCode($this->code);
    }
}
