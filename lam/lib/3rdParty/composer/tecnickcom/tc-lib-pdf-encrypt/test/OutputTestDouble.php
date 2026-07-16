<?php

/**
 * OutputTestDouble.php
 *
 * @since     2026-04-19
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * This file is part of tc-lib-pdf-encrypt software library.
 */

namespace Test;

class OutputTestDouble extends \Com\Tecnick\Pdf\Encrypt\Output
{
    public function callSetMissingValues(): void
    {
        $this->setMissingValues();
    }
}
