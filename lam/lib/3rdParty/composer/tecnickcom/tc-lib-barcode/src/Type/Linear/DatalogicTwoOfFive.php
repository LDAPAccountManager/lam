<?php

declare(strict_types=1);

/**
 * DatalogicTwoOfFive.php
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
 * Com\Tecnick\Barcode\Type\Linear\DatalogicTwoOfFive;
 *
 * DatalogicTwoOfFive Barcode type class
 * 2 of 5 Datalogic (China Post Code)
 *
 * Variant of 2 of 5 Matrix with the same digit patterns and a different pair of
 * start and stop patterns: two narrow bars for the start, a wide bar and a
 * narrow bar for the stop. The check digit is optional and is not added.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class DatalogicTwoOfFive extends \Com\Tecnick\Barcode\Type\Linear\MatrixTwoOfFive
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'S25DATALOGIC';

    /**
     * Start pattern, including the separator space that follows it
     *
     * @var string
     */
    protected const START = '1010';

    /**
     * Stop pattern
     *
     * @var string
     */
    protected const STOP = '11101';
}
