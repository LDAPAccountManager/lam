<?php

declare(strict_types=1);

/**
 * RCFourSixteen.php
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
 * Com\Tecnick\Pdf\Encrypt\Type\RCFourSixteen
 *
 * RC4-40 is the standard encryption algorithm used in PDF format
 * The key length is 16 bytes (128 bits)
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class RCFourSixteen
{
    /**
     * Encrypt the data
     *
     * @param string $data Data string to encrypt
     * @param string $key  Encryption key
     *
     * @return string encrypted text
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function encrypt(string $data, string $key): string
    {
        $rcFour = new RCFour();
        return $rcFour->encrypt($data, $key, 'RC4');
    }
}
