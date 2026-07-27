<?php

/**
 * DecryptTest.php
 *
 * @since     2026-04-30
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * This file is part of tc-lib-pdf-encrypt software library.
 */

namespace Test;

use Com\Tecnick\Pdf\Encrypt\Decrypt;
use Com\Tecnick\Pdf\Encrypt\Encrypt;

/**
 * Decrypt test
 *
 * Coverage notes (unreachable / untestable defensive guards):
 *   - Decrypt::tryDecryptRecipient() `$tmpIn === false || $tmpOut === false`:
 *     tempnam() failure requires a filesystem-level fault; cannot be reliably
 *     induced in unit tests.
 *   - Decrypt::tryDecryptRecipient() `file_put_contents === false`:
 *     same as above.
 *   - AESnopad::decrypt() `$dec === false`:
 *     openssl_decrypt() cannot return false for well-formed AES-CBC ciphertext
 *     with a valid key; this guard protects against hypothetical extension failures.
 *   - Decrypt::decryptAes() `$dec === false` return '':
 *     same reasoning; openssl_decrypt is called with correct key/iv/cipher.
 *
 * @since     2026-04-30
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class DecryptTest extends TestUtil
{
    /** Build a Decrypt object from an Encrypt instance's encryption data. */
    private function decryptFromEncrypt(Encrypt $enc): Decrypt
    {
        return new Decrypt($enc->getEncryptionData());
    }

    // -------------------------------------------------------------------------
    // Mode 2 (AES-128) — standard password authentication
    // -------------------------------------------------------------------------

    public function testAuthenticateUserMode2(): void
    {
        $enc = new Encrypt(true, \md5('file'), 2, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('userpass'));
        $this->assertNotEmpty($dec->getDocumentKey());
    }

    public function testAuthenticateOwnerMode2(): void
    {
        $enc = new Encrypt(true, \md5('file'), 2, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('ownerpass'));
        $this->assertNotEmpty($dec->getDocumentKey());
    }

    public function testAuthenticateWrongPasswordMode2(): void
    {
        $enc = new Encrypt(true, \md5('file'), 2, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertFalse($dec->authenticate('wrongpassword'));
        // Key must remain empty after failed authentication.
        $this->assertSame('', $dec->getDocumentKey());
    }

    // -------------------------------------------------------------------------
    // Mode 3 (AES-256 R5) — standard password authentication
    // -------------------------------------------------------------------------

    public function testAuthenticateUserMode3(): void
    {
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('userpass'));
        $this->assertEquals(32, \strlen($dec->getDocumentKey()));
    }

    public function testAuthenticateOwnerMode3(): void
    {
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('ownerpass'));
        $this->assertEquals(32, \strlen($dec->getDocumentKey()));
    }

    public function testAuthenticateWrongPasswordMode3(): void
    {
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertFalse($dec->authenticate('wrong'));
    }

    // -------------------------------------------------------------------------
    // Mode 4 (AES-256 R6 / PDF 2.0) — standard password authentication
    // -------------------------------------------------------------------------

    public function testAuthenticateUserMode4(): void
    {
        $enc = new Encrypt(true, \md5('file'), 4, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('userpass'));
        $this->assertEquals(32, \strlen($dec->getDocumentKey()));
    }

    public function testAuthenticateOwnerMode4(): void
    {
        $enc = new Encrypt(true, \md5('file'), 4, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('ownerpass'));
        $this->assertEquals(32, \strlen($dec->getDocumentKey()));
    }

    public function testAuthenticateWrongPasswordMode4(): void
    {
        $enc = new Encrypt(true, \md5('file'), 4, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertFalse($dec->authenticate('wrong'));
    }

    // -------------------------------------------------------------------------
    // Modes 0 and 1 (RC4 — deprecated but must still authenticate correctly)
    // -------------------------------------------------------------------------

    public function testAuthenticateUserMode0(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/RC4 encryption.*deprecated/i', function (): void {
            $enc = new Encrypt(true, \md5('file'), 0, ['print'], 'userpass', 'ownerpass');
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('userpass'));
            $this->assertNotEmpty($dec->getDocumentKey());
        });
    }

    public function testAuthenticateOwnerMode0(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/RC4 encryption.*deprecated/i', function (): void {
            $enc = new Encrypt(true, \md5('file'), 0, ['print'], 'userpass', 'ownerpass');
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('ownerpass'));
            $this->assertNotEmpty($dec->getDocumentKey());
        });
    }

    public function testAuthenticateUserMode1(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/RC4 encryption.*deprecated/i', function (): void {
            $enc = new Encrypt(true, \md5('file'), 1, ['print'], 'userpass', 'ownerpass');
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('userpass'));
            $this->assertNotEmpty($dec->getDocumentKey());
        });
    }

    public function testAuthenticateOwnerMode1(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/RC4 encryption.*deprecated/i', function (): void {
            $enc = new Encrypt(true, \md5('file'), 1, ['print'], 'userpass', 'ownerpass');
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('ownerpass'));
            $this->assertNotEmpty($dec->getDocumentKey());
        });
    }

    // -------------------------------------------------------------------------
    // decryptString round-trips
    // -------------------------------------------------------------------------

    /**
     * RC4 modes are symmetric: encrypt(encrypt(data, key)) = data.
     * The plaintext is recovered exactly (no padding).
     */
    public function testDecryptStringRoundtripMode0(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/RC4 encryption.*deprecated/i', function (): void {
            $enc = new Encrypt(true, \md5('file'), 0, ['print'], 'alpha', 'beta');
            $plaintext = 'hello world';
            $ciphertext = $enc->encryptString($plaintext, 1);
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('alpha'));
            $this->assertSame($plaintext, $dec->decryptString($ciphertext, 1));
        });
    }

    /**
     * AES-128: IV-prefixed stream; PKCS#7 padding is stripped so the exact
     * plaintext is recovered.
     */
    public function testDecryptStringRoundtripMode2(): void
    {
        $enc = new Encrypt(true, \md5('file'), 2, ['print'], 'alpha', 'beta');
        $plaintext = 'hello world';
        $ciphertext = $enc->encryptString($plaintext, 1);
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('alpha'));
        $this->assertSame($plaintext, $dec->decryptString($ciphertext, 1));
    }

    /**
     * AES-256 R5: full document key used; exact plaintext recovered.
     */
    public function testDecryptStringRoundtripMode3(): void
    {
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], 'alpha', 'beta');
        $plaintext = 'hello world';
        $ciphertext = $enc->encryptString($plaintext, 1);
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('alpha'));
        $this->assertSame($plaintext, $dec->decryptString($ciphertext, 1));
    }

    /**
     * AES-256 R6: same as R5 but with hash2B key derivation.
     */
    public function testDecryptStringRoundtripMode4(): void
    {
        $enc = new Encrypt(true, \md5('file'), 4, ['print'], 'alpha', 'beta');
        $plaintext = 'hello world';
        $ciphertext = $enc->encryptString($plaintext, 1);
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('alpha'));
        $this->assertSame($plaintext, $dec->decryptString($ciphertext, 1));
    }

    /**
     * Block-aligned plaintext (exactly 16 bytes) must round-trip exactly: the
     * PKCS#7 scheme appends a full extra padding block on encryption that must
     * be removed on decryption.
     */
    public function testDecryptStringRoundtripBlockAligned(): void
    {
        foreach ([2, 3, 4] as $mode) {
            $enc = new Encrypt(true, \md5('file'), $mode, ['print'], 'alpha', 'beta');
            $plaintext = \str_repeat('A', 16);
            $ciphertext = $enc->encryptString($plaintext, 7);
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('alpha'));
            $this->assertSame($plaintext, $dec->decryptString($ciphertext, 7), "mode {$mode}");
        }
    }

    /**
     * Empty plaintext must round-trip to an empty string for all AES modes.
     */
    public function testDecryptStringRoundtripEmpty(): void
    {
        foreach ([2, 3, 4] as $mode) {
            $enc = new Encrypt(true, \md5('file'), $mode, ['print'], 'alpha', 'beta');
            $ciphertext = $enc->encryptString('', 3);
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('alpha'));
            $this->assertSame('', $dec->decryptString($ciphertext, 3), "mode {$mode}");
        }
    }

    /**
     * Without authentication the key is empty; decryptString returns data that
     * differs from the original plaintext (garbage decrypt, not the correct value).
     */
    public function testDecryptStringWithoutAuthProducesGarbage(): void
    {
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], 'userpass', 'ownerpass');
        $dec = $this->decryptFromEncrypt($enc);
        // Key is cleared in constructor; without authenticate(), key is empty.
        $ciphertext = $enc->encryptString('hello world', 1);
        $result = $dec->decryptString($ciphertext, 1);
        // Without the correct key the output must differ from the plaintext.
        $this->assertStringNotContainsString('hello world', $result);
    }

    /**
     * decryptString with too-short AES data (≤ 16 bytes) returns empty string.
     */
    public function testDecryptStringAesTooShortData(): void
    {
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], 'alpha', 'beta');
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('alpha'));
        $this->assertSame('', $dec->decryptString(\str_repeat('x', 16), 0));
    }

    // -------------------------------------------------------------------------
    // getDocumentKey after failed/successful authentication
    // -------------------------------------------------------------------------

    public function testGetDocumentKeyAfterFailedAuth(): void
    {
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], 'userpass', 'ownerpass');
        // Store the real key, then construct Decrypt with an overwritten empty key
        $data = $enc->getEncryptionData();
        $data['key'] = '';
        $dec = new Decrypt($data);
        $this->assertFalse($dec->authenticate('wrong'));
        $this->assertSame('', $dec->getDocumentKey());
    }

    // -------------------------------------------------------------------------
    // Public-key mode authentication
    // -------------------------------------------------------------------------

    public function testAuthenticatePublicKeyMode3(): void
    {
        $certPath = __DIR__ . '/data/cert.pem';
        $pubkeys = [['c' => $certPath, 'p' => ['print']]];
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], '', '', $pubkeys);
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertTrue($dec->authenticate('', $certPath));
        $this->assertEquals(32, \strlen($dec->getDocumentKey()));
    }

    public function testAuthenticatePublicKeyMode1(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            // Mode 1 pubkey silently promotes mode 0 → 1 (covered elsewhere).
            $certPath = __DIR__ . '/data/cert.pem';
            $pubkeys = [['c' => $certPath, 'p' => ['print']]];
            $enc = new Encrypt(true, \md5('file'), 1, ['print'], '', '', $pubkeys);
            $dec = $this->decryptFromEncrypt($enc);
            $this->assertTrue($dec->authenticate('', $certPath));
            $this->assertNotEmpty($dec->getDocumentKey());
        });
    }

    public function testAuthenticatePublicKeyEmptyPathReturnsFalse(): void
    {
        $certPath = __DIR__ . '/data/cert.pem';
        $pubkeys = [['c' => $certPath, 'p' => ['print']]];
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], '', '', $pubkeys);
        $dec = $this->decryptFromEncrypt($enc);
        $this->assertFalse($dec->authenticate('', ''));
    }

    public function testAuthenticatePublicKeyWrongKeyReturnsFalse(): void
    {
        $certPath = __DIR__ . '/data/cert.pem';
        $pubkeys = [['c' => $certPath, 'p' => ['print']]];
        $enc = new Encrypt(true, \md5('file'), 3, ['print'], '', '', $pubkeys);
        $dec = $this->decryptFromEncrypt($enc);
        // Use the test PHP file as a "wrong" key — openssl_pkcs7_decrypt will fail.
        $this->assertFalse($dec->authenticate('', __FILE__));
    }

    /**
     * Cover the `hex2bin() === false` branch in findDecryptedRecipientSeed().
     *
     * When a Recipients entry contains non-hexadecimal characters, hex2bin()
     * returns false and the entry is skipped via `continue`.  With no valid
     * entries the method returns null and authenticate() returns false.
     */
    public function testAuthenticatePublicKeyInvalidHexRecipientReturnsFalse(): void
    {
        $certPath = __DIR__ . '/data/cert.pem';
        // Manually build an encryptdata array in pubkey mode whose Recipients
        // list contains only a string that is not valid hex (non-hex characters
        // cause hex2bin() to return false).
        $data = [
            'V' => 6,
            'Length' => 256,
            'O' => \str_repeat('x', 32),
            'U' => \str_repeat('x', 48),
            'P' => 0,
            'fileid' => \md5('test'),
            'mode' => 3,
            'pubkey' => true,
            'Recipients' => ['ZZZZINVALID!!'], // hex2bin returns false for non-hex chars
        ];
        $dec = new \Com\Tecnick\Pdf\Encrypt\Decrypt($data);
        $this->assertFalse($dec->authenticate('', $certPath));
    }

    // -------------------------------------------------------------------------
    // AESnopad::decrypt() direct tests
    // -------------------------------------------------------------------------

    public function testAesnopadDecryptRoundtrip32Bytes(): void
    {
        $aesnopad = new \Com\Tecnick\Pdf\Encrypt\Type\AESnopad();
        $key = \str_repeat('k', 32);
        $plaintext = \str_repeat('p', 32); // exact 32-byte payload (e.g. file key)
        $ciphertext = $aesnopad->encrypt($plaintext, $key);
        $decrypted = $aesnopad->decrypt($ciphertext, $key);
        $this->assertSame($plaintext, $decrypted);
    }

    public function testAesnopadDecryptRoundtripAes128(): void
    {
        $aesnopad = new \Com\Tecnick\Pdf\Encrypt\Type\AESnopad();
        $key = \str_repeat('k', 16);
        $plaintext = \str_repeat('p', 16);
        $ivect = \Com\Tecnick\Pdf\Encrypt\Type\AESnopad::IVECT;
        $ciphertext = $aesnopad->encrypt($plaintext, $key, $ivect, 'aes-128-cbc');
        $decrypted = $aesnopad->decrypt($ciphertext, $key, $ivect, 'aes-128-cbc');
        $this->assertSame($plaintext, $decrypted);
    }

    public function testAesnopadDecryptInvalidCipherThrows(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        $aesnopad = new \Com\Tecnick\Pdf\Encrypt\Type\AESnopad();
        $aesnopad->decrypt('data', 'key', \Com\Tecnick\Pdf\Encrypt\Type\AESnopad::IVECT, 'des-cbc');
    }
}
