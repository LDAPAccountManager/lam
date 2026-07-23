<?php

/**
 * RCFourTest.php
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

namespace Test;

/**
 * RC4 encryption Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class RCFourTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Encrypt\Type\RCFour
    {
        return new \Com\Tecnick\Pdf\Encrypt\Type\RCFour();
    }

    public function testEncrypt40(): void
    {
        $rcFour = $this->getTestObject();
        $data = 'alpha';
        $key = '12345'; // 5 bytes = 40 bit KEY

        $enc_a = $rcFour->encrypt($data, $key, '');
        $enc_b = $rcFour->encrypt($data, $key, 'RC4-40');
        $this->assertEquals($enc_a, $enc_b);

        $rcFourFive = new \Com\Tecnick\Pdf\Encrypt\Type\RCFourFive();
        $enc_c = $rcFourFive->encrypt($data, $key);
        $this->assertEquals($enc_a, $enc_c);
    }

    public function testEncrypt128(): void
    {
        $rcFour = $this->getTestObject();
        $data = 'alpha';
        $key = '0123456789abcdef'; // 16 bytes = 128 bit KEY

        $enc_a = $rcFour->encrypt($data, $key);
        $enc_b = $rcFour->encrypt($data, $key, 'RC4');
        $this->assertEquals($enc_a, $enc_b);

        $rcFourSixteen = new \Com\Tecnick\Pdf\Encrypt\Type\RCFourSixteen();
        $enc_c = $rcFourSixteen->encrypt($data, $key);
        $this->assertEquals($enc_a, $enc_c);
    }

    public function testEncryptException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        $rcFour = $this->getTestObject();
        $rcFour->encrypt('alpha', '12345', 'ERROR');
    }
}
