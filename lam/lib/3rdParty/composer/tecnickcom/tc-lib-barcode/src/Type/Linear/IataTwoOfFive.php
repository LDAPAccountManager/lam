<?php

declare(strict_types=1);

/**
 * IataTwoOfFive.php
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
 * Com\Tecnick\Barcode\Type\Linear\IataTwoOfFive;
 *
 * IataTwoOfFive Barcode type class
 * 2 of 5 IATA (Computer Identics 2 of 5 - Airline 2 of 5)
 *
 * Discrete two width symbology with the same digit patterns as Standard 2 of 5,
 * where only the bars carry data. It differs in the start pattern, a narrow bar
 * and a narrow bar, and in the stop pattern, a wide bar and a narrow bar.
 * The check digit is optional and is not added.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class IataTwoOfFive extends \Com\Tecnick\Barcode\Type\Linear\StandardTwoOfFive
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'S25IATA';

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
