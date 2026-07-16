<?php

/**
 * AESnopadTest.php
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

use Com\Tecnick\Pdf\Encrypt\Type\AESnopad;

/**
 * AESnopad encryption Test
 *
 * Verifies that pad() extends data to the next multiple of BLOCKSIZE without
 * truncating, so that stream data longer than one AES block is not silently
 * discarded before encryption.
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class AESnopadTest extends TestUtil
{
    /**
     * 32-byte key used throughout (exact multiple of BLOCKSIZE, no padding applied).
     */
    private const KEY256 = '0123456789abcdef0123456789abcdef';

    protected function getTestObject(): AESnopad
    {
        return new AESnopad();
    }

    /**
     * AESnopad::encrypt() returns padded_len bytes of ciphertext (no IV prefix).
     * padded_len = ceil(n / 16) * 16, but if n % 16 == 0 then padded_len = n.
     *
     * The formula below matches that behaviour for all cases tested.
     */
    private function expectedCiphertextLen(int $plainLen): int
    {
        $rem = $plainLen % AESnopad::BLOCKSIZE;
        return $rem === 0 ? $plainLen : $plainLen + (AESnopad::BLOCKSIZE - $rem);
    }

    // --- output-length tests (validate pad() indirectly) ---

    public function testEncryptOutputLenShortData(): void
    {
        // 5 bytes → padded to 16
        $enc = $this->getTestObject()->encrypt(\str_repeat('x', 5), self::KEY256);
        $this->assertSame($this->expectedCiphertextLen(5), \strlen($enc));
    }

    public function testEncryptOutputLenExactlyOneBlock(): void
    {
        // 16 bytes → already a multiple, no extra padding → 16
        $enc = $this->getTestObject()->encrypt(\str_repeat('x', 16), self::KEY256);
        $this->assertSame($this->expectedCiphertextLen(16), \strlen($enc));
    }

    public function testEncryptOutputLenJustOverOneBlock(): void
    {
        // 17 bytes → padded to 32
        // This test would fail with the old code (which truncated data to 16 bytes,
        // producing only 16 bytes of ciphertext regardless of input length).
        $enc = $this->getTestObject()->encrypt(\str_repeat('x', 17), self::KEY256);
        $this->assertSame($this->expectedCiphertextLen(17), \strlen($enc));
    }

    public function testEncryptOutputLenTwoBlocks(): void
    {
        // 32 bytes → padded to 32
        $enc = $this->getTestObject()->encrypt(\str_repeat('x', 32), self::KEY256);
        $this->assertSame($this->expectedCiphertextLen(32), \strlen($enc));
    }

    public function testEncryptOutputLenJustOverTwoBlocks(): void
    {
        // 33 bytes → padded to 48
        $enc = $this->getTestObject()->encrypt(\str_repeat('x', 33), self::KEY256);
        $this->assertSame($this->expectedCiphertextLen(33), \strlen($enc));
    }

    public function testEncryptOutputLenLargeData(): void
    {
        // 100 bytes → padded to 112
        $enc = $this->getTestObject()->encrypt(\str_repeat('x', 100), self::KEY256);
        $this->assertSame($this->expectedCiphertextLen(100), \strlen($enc));
    }

    /**
     * Longer input must produce longer ciphertext.
     * With the old bug, both short and long inputs produced 16-byte ciphertext.
     */
    public function testCiphertextGrowsWithPlaintext(): void
    {
        $aesnopad = $this->getTestObject();
        $key = self::KEY256;

        $short = $aesnopad->encrypt(\str_repeat('a', 5), $key);
        $long = $aesnopad->encrypt(\str_repeat('a', 100), $key);

        $this->assertGreaterThan(\strlen($short), \strlen($long));
    }

    // --- AES-128-CBC variant ---

    public function testEncryptAes128OutputLen(): void
    {
        // 17 bytes with aes-128-cbc → padded to 32
        $enc = $this->getTestObject()->encrypt(\str_repeat('x', 17), self::KEY256, AESnopad::IVECT, 'aes-128-cbc');
        $this->assertSame($this->expectedCiphertextLen(17), \strlen($enc));
    }

    // --- deterministic output with fixed IV ---

    public function testEncryptDeterministicWithFixedIv(): void
    {
        $aesnopad = $this->getTestObject();
        $data = \str_repeat('x', 32);
        $key = self::KEY256;

        $enc1 = $aesnopad->encrypt($data, $key, AESnopad::IVECT, 'aes-256-cbc');
        $enc2 = $aesnopad->encrypt($data, $key, AESnopad::IVECT, 'aes-256-cbc');
        $this->assertSame($enc1, $enc2);
    }

    // --- exception paths ---

    public function testCheckCipherInvalidName(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        $this->getTestObject()->checkCipher('des-cbc');
    }

    public function testEncryptInvalidCipher(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        $this->getTestObject()->encrypt('data', self::KEY256, AESnopad::IVECT, 'des-cbc');
    }

    public function testCheckCipherUnavailable(): void
    {
        $missingCipher = null;
        $available = \openssl_get_cipher_methods();
        foreach (AESnopad::VALID_CIPHERS as $cipher) {
            if (\in_array($cipher, $available, true)) {
                continue;
            }

            $missingCipher = $cipher;
            break;
        }

        if ($missingCipher === null) {
            $this->markTestSkipped('All AESnopad valid ciphers are available on this runtime.');
        }

        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        $this->getTestObject()->checkCipher($missingCipher);
    }

    public function testDecryptInvalidCiphertextLength(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        $this->getTestObject()->decrypt('short', self::KEY256, AESnopad::IVECT, 'aes-256-cbc');
    }
}
