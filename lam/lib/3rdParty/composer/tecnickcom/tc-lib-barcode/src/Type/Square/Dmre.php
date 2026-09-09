<?php

declare(strict_types=1);

/**
 * Dmre.php
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

use Com\Tecnick\Barcode\Type\Square\Dmre\DmreSize;

/**
 * Com\Tecnick\Barcode\Type\Square\Dmre
 *
 * Dmre Barcode type class
 * Data Matrix Rectangular Extension (ISO/IEC 21471)
 *
 * Rectangular symbol sizes added to Data Matrix ECC 200 for narrow marking
 * areas. The encodation, the symbol character placement and the Reed-Solomon
 * error correction are those of ISO/IEC 16022; only the symbol sizes differ.
 *
 * @since       2026-09-01
 * @category    Library
 * @package     Barcode
 * @author      Nicola Asuni <info@tecnick.com>
 * @copyright   2010-2026 Nicola Asuni - Tecnick.com LTD
 * @license     https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link        https://github.com/tecnickcom/tc-lib-barcode
 */
class Dmre extends \Com\Tecnick\Barcode\Type\Square\Datamatrix
{
    /**
     * Barcode format
     *
     * @var string
     */
    protected const FORMAT = 'DMRE';

    /**
     * Number of data codewords of the largest symbol, the 26x64.
     *
     * @var int
     */
    protected const MAXCDW = 118;

    /**
     * Key of the DMRE symbol sizes in the symbol attribute table.
     *
     * @var string
     */
    protected const SHAPE = 'E';

    /**
     * Set extra (optional) parameters:
     *     1: MODE: N=default, GS1 = the FNC1 codeword is added in the first position.
     *     2: ENCODING: ASCII (default), C40, TXT, X12, EDIFACT, BASE256.
     *     3: SIZE: symbol size as rows by columns, as in 26x64; the smallest size that fits by default.
     */
    protected function setParameters(): void
    {
        parent::setParameters();
        $this->shape = self::SHAPE;
        $this->setModeAndEncoding(0);
        $this->size = DmreSize::fromLoose(\strval($this->params[2] ?? ''))->value;
    }
}
