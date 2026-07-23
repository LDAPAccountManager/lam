<?php

declare(strict_types=1);

/**
 * InterleavedTwoOfFive.php
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
 * Com\Tecnick\Barcode\Type\Linear\InterleavedTwoOfFive;
 *
 * InterleavedTwoOfFive Barcode type class
 * Interleaved 2 of 5
 *
 * @since       2015-02-21
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class InterleavedTwoOfFive extends \Com\Tecnick\Barcode\Type\Linear\InterleavedTwoOfFiveCheck
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'I25';

    /**
     * Format code
     */
    protected function formatCode(): void
    {
        $this->extcode = $this->code;
    }
}
