<?php

/**
 * AESTest.php
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
 * AES encryption Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class AESTest extends TestUtil
{
    protected function getTestObject(): \Com\Tecnick\Pdf\Encrypt\Type\AES
    {
        return new \Com\Tecnick\Pdf\Encrypt\Type\AES();
    }

    public function testEncrypt128(): void
    {
        $aes = $this->getTestObject();
        $data = 'alpha';
        $key = '0123456789abcdef'; // 16 bytes = 128 bit KEY

        $enc_a = $aes->encrypt($data, $key);
        $enc_b = $aes->encrypt($data, $key, 'aes-128-cbc');
        $this->assertEquals(\strlen($enc_a), \strlen($enc_b));

        $aesSixteen = new \Com\Tecnick\Pdf\Encrypt\Type\AESSixteen();
        $enc_c = $aesSixteen->encrypt($data, $key);
        $this->assertEquals(\strlen($enc_a), \strlen($enc_c));
    }

    public function testEncrypt256(): void
    {
        $aes = $this->getTestObject();
        $data = 'alpha';
        $key = '0123456789abcdef0123456789abcdef'; // 32 bytes = 256 bit KEY

        $enc_a = $aes->encrypt($data, $key, '');
        $enc_b = $aes->encrypt($data, $key, 'aes-256-cbc');
        $this->assertEquals(\strlen($enc_a), \strlen($enc_b));

        $aesThirtytwo = new \Com\Tecnick\Pdf\Encrypt\Type\AESThirtytwo();
        $enc_c = $aesThirtytwo->encrypt($data, $key);
        $this->assertEquals(\strlen($enc_a), \strlen($enc_c));
    }

    /**
     * AES::encrypt() output = 16-byte IV + PKCS#7-padded ciphertext.
     * padded_len = ceil((n + 1)/16) * 16, so aligned input gets one full block.
     * Total = padded_len + 16.
     *
     * With the old truncation bug, pad() always produced 16 bytes, so every
     * plaintext — no matter how long — produced a 32-byte output.  These tests
     * verify that the output size grows correctly with the plaintext length.
     */
    public function testEncrypt128LongData(): void
    {
        $aes = $this->getTestObject();
        $key = '0123456789abcdef'; // 16 bytes = 128 bit KEY

        // 17 bytes → padded to 32 → 32 ciphertext + 16 IV = 48
        $enc17 = $aes->encrypt(\str_repeat('x', 17), $key, 'aes-128-cbc');
        $this->assertSame(48, \strlen($enc17));

        // 32 bytes → padded to 48 (full PKCS#7 block) → 48 + 16 = 64
        $enc32 = $aes->encrypt(\str_repeat('x', 32), $key, 'aes-128-cbc');
        $this->assertSame(64, \strlen($enc32));

        // 33 bytes → padded to 48 → 48 + 16 = 64
        $enc33 = $aes->encrypt(\str_repeat('x', 33), $key, 'aes-128-cbc');
        $this->assertSame(64, \strlen($enc33));

        // Short input must produce shorter output than long input.
        $encShort = $aes->encrypt('alpha', $key, 'aes-128-cbc'); // 5 bytes → 32
        $this->assertGreaterThan(\strlen($encShort), \strlen($enc33));
    }

    public function testEncrypt256LongData(): void
    {
        $aes = $this->getTestObject();
        $key = '0123456789abcdef0123456789abcdef'; // 32 bytes = 256 bit KEY

        // 17 bytes → padded to 32 → 32 ciphertext + 16 IV = 48
        $enc17 = $aes->encrypt(\str_repeat('x', 17), $key, 'aes-256-cbc');
        $this->assertSame(48, \strlen($enc17));

        // 32 bytes → padded to 48 (full PKCS#7 block) → 48 + 16 = 64
        $enc32 = $aes->encrypt(\str_repeat('x', 32), $key, 'aes-256-cbc');
        $this->assertSame(64, \strlen($enc32));

        // 100 bytes → padded to 112 → 112 + 16 = 128
        $enc100 = $aes->encrypt(\str_repeat('x', 100), $key, 'aes-256-cbc');
        $this->assertSame(128, \strlen($enc100));

        $aesThirtytwo = new \Com\Tecnick\Pdf\Encrypt\Type\AESThirtytwo();
        $enc100b = $aesThirtytwo->encrypt(\str_repeat('x', 100), $key);
        $this->assertSame(\strlen($enc100), \strlen($enc100b));
    }

    public function testEncryptException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        $aes = $this->getTestObject();
        $aes->encrypt('alpha', '12345', 'ERROR');
    }
}
