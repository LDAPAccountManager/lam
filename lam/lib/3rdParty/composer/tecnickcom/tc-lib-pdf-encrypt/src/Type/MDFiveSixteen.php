<?php

declare(strict_types=1);

/**
 * MDFiveSixteen.php
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * This file is part of tc-lib-pdf-encrypt software library.
 */

namespace Com\Tecnick\Pdf\Encrypt\Type;

/**
 * Com\Tecnick\Pdf\Encrypt\Type\MDFiveSixteen
 *
 * MD5-16
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class MDFiveSixteen
{
    /**
     * Encrypt the data
     *
     * @param string $data Data string to encrypt
     *
     * @return string encrypted text
     */
    public function encrypt(string $data, string $_key = 'H*'): string
    {
        return \pack('H*', \md5($data));
    }
}
