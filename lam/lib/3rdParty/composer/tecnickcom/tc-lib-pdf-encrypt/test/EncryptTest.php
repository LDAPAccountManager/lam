<?php

/**
 * EncryptTest.php
 *
 * @since     2011-05-23
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

/**
 * Encrypt Test
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class EncryptTest extends TestUtil
{
    // Coverage note: src/Compute.php hash2B() line ~295
    //   `throw new EncException('AES-128-CBC encryption failed in hash2B')` is a defensive guard;
    //   openssl_encrypt() never returns false for valid block-aligned AES-128-CBC inputs with
    //   a correct 16-byte key and IV — this branch is unreachable under normal PHP/OpenSSL conditions.
    //
    // Coverage note: src/Compute.php getEncryptedRecipientBytes()
    //   The `tempnam() === false` / `file_put_contents() === false` guards (and the
    //   accompanying unlink() cleanup on those error paths) require a filesystem failure
    //   that cannot be reliably induced in unit tests. The happy-path try/finally cleanup
    //   and encryptRecipientEnvelope() are exercised by the public-key tests.
    //
    // Coverage note: src/Encrypt.php convertStringToHexString() line ~246
    //   `return ''` after `preg_split('//', ...)` guards against the impossible case where
    //   preg_split returns false; the regex '//\'' is always valid and never returns false.

    public function testEncryptException(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'));
            $encrypt->encrypt('WRONG');
        });
    }

    public function testEncryptModeException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 5);
    }

    public function testEncryptThree(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta');
        $result = $encrypt->encrypt(3, 'alpha');
        $this->assertEquals(32, \strlen($result));
    }

    public function testEncryptWithAesEncoderName(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta');
        $result = $encrypt->encrypt('AES', 'alpha', '0123456789abcdef0123456789abcdef');
        $this->assertGreaterThan(16, \strlen($result));
    }

    public function testEncryptPubThree(): void
    {
        $pubkeys = [[
            'c' => __DIR__ . '/data/cert.pem',
            'p' => ['print'],
        ]];
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta', $pubkeys);
        $result = $encrypt->encrypt(3, 'alpha');
        $this->assertEquals(32, \strlen($result));
    }

    public function testEncryptPubNoP(): void
    {
        $pubkeys = [[
            'c' => __DIR__ . '/data/cert.pem',
            'p' => ['print'],
        ]];
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta', $pubkeys);
        $result = $encrypt->encrypt(3, 'alpha');
        $this->assertEquals(32, \strlen($result));
    }

    public function testEncryptPubException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);
        new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta', [[
            'c' => __FILE__,
            'p' => ['print'],
        ]]);
    }

    public function testEncryptPubUnreadableCertificateException(): void
    {
        $this->bcExpectException(\Com\Tecnick\Pdf\Encrypt\Exception::class);

        \set_error_handler(static fn(): bool => true);
        try {
            new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta', [[
                'c' => __DIR__ . '/data/does-not-exist.pem',
                'p' => ['print'],
            ]]);
        } finally {
            \restore_error_handler();
        }
    }

    public function testEncryptRc4ThroughOpenSslWhenAvailable(): void
    {
        if (!\in_array('RC4', \openssl_get_cipher_methods(), true)) {
            $this->markTestSkipped('OpenSSL RC4 cipher is not available on this runtime.');
        }

        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta');
        $result = $encrypt->encrypt('RC4', 'alpha', '0123456789abcdef');
        $this->assertSame(5, \strlen($result));
    }

    public function testEncryptModZeroPub(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $pubkeys = [[
                'c' => __DIR__ . '/data/cert.pem',
                'p' => ['print'],
            ]];
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
                true,
                \md5('file_id'),
                0,
                ['print'],
                'alpha',
                'beta',
                $pubkeys,
            );
            $result = $encrypt->encrypt(1, 'alpha');
            // Check for "error:0308010C:digital envelope routines::unsupported" when using OpenSSL 3.
            // \var_dump(\openssl_error_string());
            $this->assertEquals(5, \strlen($result));
        });
    }

    /** Issue 6: RC4 mode 0 must emit a deprecation notice. */
    public function testRc4DeprecationModeZero(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/RC4 encryption.*deprecated.*cryptographically broken/i', function (): void {
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 0, ['print'], 'alpha', 'beta');
            $result = $encrypt->encrypt(0, 'alpha');
            $this->assertGreaterThan(0, \strlen($result));
        });
    }

    /** Issue 6: RC4 mode 1 must emit a deprecation notice. */
    public function testRc4DeprecationModeOne(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/RC4 encryption.*deprecated.*cryptographically broken/i', function (): void {
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 1, ['print'], 'alpha', 'beta');
            $result = $encrypt->encrypt(1, 'alpha');
            $this->assertGreaterThan(0, \strlen($result));
        });
    }

    /** Issue 5: mode 0 + pubkeys must emit the upgrade deprecation notice. */
    public function testPubKeyModeZeroDeprecation(): void
    {
        $this->bcAssertUserDeprecationMessageMatches('/Public-key encryption requires at least RC4-128/i', function (): void {
            $pubkeys = [[
                'c' => __DIR__ . '/data/cert.pem',
                'p' => ['print'],
            ]];
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
                true,
                \md5('file_id'),
                0,
                ['print'],
                'alpha',
                'beta',
                $pubkeys,
            );
            // After promotion to mode 1, the resulting encryption data must reflect mode 1
            $data = $encrypt->getEncryptionData();
            $this->assertEquals(1, $data['mode']);
            $this->assertEquals(2, $data['V']);
        });
    }

    /** Issue 2: AES-256 perms bytes 12-15 must be random (not 'nick'). */
    public function testPermsRandomBytes(): void
    {
        $encrypt1 = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta');
        $encrypt2 = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 3, ['print'], 'alpha', 'beta');
        $data1 = $encrypt1->getEncryptionData();
        $data2 = $encrypt2->getEncryptionData();
        // The 16-byte AES-encrypted perms block (AESnopad strips the PKCS7 padding block)
        $this->assertEquals(16, \strlen($data1['perms']));
        $this->assertEquals(16, \strlen($data2['perms']));
        // Two independently generated perms values should almost certainly differ (random bytes 12-15)
        // Note: 1 in 2^32 chance of collision is acceptable to document rather than retry.
        $this->assertNotEquals($data1['perms'], $data2['perms'], 'perms bytes should be random each time');
    }

    /** Issue 3: AES-256 with EncryptMetadata=false must store the flag. */
    public function testEncryptMetadataFalse(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(
            true,
            \md5('file_id'),
            3,
            ['print'],
            'alpha',
            'beta',
            null,
            false, // encryptMetadata = false
        );
        $data = $encrypt->getEncryptionData();
        $this->assertFalse($data['EncryptMetadata']);
    }

    /** Issue 4: AES-256 R6 (mode 4) encrypt round-trip. */
    public function testEncryptFour(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 4, ['print'], 'alpha', 'beta');
        $result = $encrypt->encrypt(4, 'alpha');
        $this->assertEquals(32, \strlen($result));
    }

    /** Issue 4: AES-256 R6 (mode 4) encryptdata must have V=6 and mode=4. */
    public function testEncryptFourSettings(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 4, ['print'], 'alpha', 'beta');
        $data = $encrypt->getEncryptionData();
        $this->assertEquals(4, $data['mode']);
        $this->assertEquals(6, $data['V']);
        $this->assertEquals(256, $data['Length']);
        $this->assertEquals('AESV3', $data['CF']['CFM']);
        $this->assertEquals(48, \strlen($data['U']));
        $this->assertEquals(48, \strlen($data['O']));
        $this->assertEquals(32, \strlen($data['UE']));
        $this->assertEquals(32, \strlen($data['OE']));
        $this->assertEquals(16, \strlen($data['perms']));
    }

    /** Issue 4: AES-256 R6 (mode 4) public-key encryption. */
    public function testEncryptPubFour(): void
    {
        $pubkeys = [[
            'c' => __DIR__ . '/data/cert.pem',
            'p' => ['print'],
        ]];
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 4, ['print'], 'alpha', 'beta', $pubkeys);
        $result = $encrypt->encrypt(4, 'alpha');
        $this->assertEquals(32, \strlen($result));
    }

    public function testGetEncryptionData(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $permissions = ['print'];
            $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 0, $permissions, 'alpha', 'beta');
            $result = $encrypt->getEncryptionData();
            $this->assertEquals(2_147_422_008, $result['protection']);
            $this->assertEquals(1, $result['V']);
            $this->assertEquals(40, $result['Length']);
            $this->assertEquals('V2', $result['CF']['CFM']);
        });
    }

    public function testGetObjectKey(): void
    {
        $permissions = ['print', 'modify', 'copy', 'annot-forms', 'fill-forms', 'extract', 'assemble', 'print-high'];

        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 2, $permissions, 'alpha', 'beta');
        $result = $encrypt->getObjectKey(123);
        $this->assertEquals('93879594941619c98047c404192b977d', \bin2hex($result));
    }

    public function testGetUserPermissionCode(): void
    {
        $permissions = [
            'owner',
            'print',
            'modify',
            'copy',
            'annot-forms',
            'fill-forms',
            'extract',
            'assemble',
            'print-high',
        ];

        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();
        $result = $encrypt->getUserPermissionCode($permissions, 0);
        $this->assertEquals(2_147_421_954, $result);
    }

    public function testGetUserPermissionCodeIgnoreInvalidPermission(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();
        $result = $encrypt->getUserPermissionCode(['invalid-permission'], 0);
        $this->assertEquals(2_147_422_012, $result);
    }

    public function testConvertHexStringToString(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();

        $result = $encrypt->convertHexStringToString('');
        $this->assertEquals('', $result);

        $result = $encrypt->convertHexStringToString('68656c6c6f20776f726c64');
        $this->assertEquals('hello world', $result);

        $result = $encrypt->convertHexStringToString('68656c6c6f20776f726c642');
        $this->assertEquals('hello world ', $result);
    }

    public function testConvertStringToHexString(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();

        $result = $encrypt->convertStringToHexString('');
        $this->assertEquals('', $result);

        $result = $encrypt->convertStringToHexString('hello world');
        $this->assertEquals('68656c6c6f20776f726c64', $result);
    }

    public function testEncodeNameObject(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();

        $result = $encrypt->encodeNameObject('');
        $this->assertEquals('', $result);

        $result = $encrypt->encodeNameObject('059akzAKZ#_=-');
        $this->assertEquals('059akzAKZ#_=-', $result);

        $result = $encrypt->encodeNameObject('059[]{}+~*akzAKZ#_=-');
        $this->assertEquals('059#5B#5D#7B#7D#2B#7E#2AakzAKZ#_=-', $result);
    }

    public function testEscapeString(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();

        $result = $encrypt->escapeString('');
        $this->assertEquals('', $result);

        $result = $encrypt->escapeString('hello world');
        $this->assertEquals('hello world', $result);

        $result = $encrypt->escapeString('(hello world) slash \\' . \chr(13));
        $this->assertEquals('\\(hello world\\) slash \\\\\r', $result);
    }

    public function testEncryptStringDisabled(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();

        $result = $encrypt->encryptString('');
        $this->assertEquals('', $result);

        $result = $encrypt->encryptString('hello world');
        $this->assertEquals('hello world', $result);

        $result = $encrypt->encryptString('(hello world) slash \\' . \chr(13) . \chr(250));
        $this->assertEquals('(hello world) slash \\' . \chr(13) . \chr(250), $result);
    }

    public function testEncryptStringEnabled(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $permissions = [
                'print',
                'modify',
                'copy',
                'annot-forms',
                'fill-forms',
                'extract',
                'assemble',
                'print-high',
            ];

            $enc = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 0, $permissions, 'alpha');
            $result = $enc->encryptString('(hello world) slash \\' . \chr(13));
            $this->assertEquals('728cc693be1e4c1fb6b7e7b2a34644ad', \md5($result));

            $enc = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 1, $permissions, 'alpha', 'beta');
            $result = $enc->encryptString('(hello world) slash \\' . \chr(13));
            $this->assertEquals('258ad774ddeec21b3b439a720df18e0d', \md5($result));
        });
    }

    public function testEscapeDataStringDisabled(): void
    {
        $encrypt = new \Com\Tecnick\Pdf\Encrypt\Encrypt();

        $result = $encrypt->escapeDataString('');
        $this->assertEquals('()', $result);

        $result = $encrypt->escapeDataString('hello world');
        $this->assertEquals('(hello world)', $result);

        $result = $encrypt->escapeDataString('(hello world) slash \\' . \chr(13));
        $this->assertEquals('(\\(hello world\\) slash \\\\\r)', $result);
    }

    public function testEscapeDataStringEnabled(): void
    {
        $this->bcRunIgnoringUserDeprecations(function (): void {
            $permissions = [
                'print',
                'modify',
                'copy',
                'annot-forms',
                'fill-forms',
                'extract',
                'assemble',
                'print-high',
            ];

            $enc = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 0, $permissions, 'alpha');
            $result = $enc->escapeDataString('(hello world) slash \\' . \chr(13));
            $this->assertEquals('24f60765c1c07a44fc3c9b44d2f55dbc', \md5($result));

            $enc = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 1, $permissions, 'alpha', 'beta');
            $result = $enc->escapeDataString('(hello world) slash \\' . \chr(13));
            $this->assertEquals('ebc28272f4aff661fa0b7764d791fb79', \md5($result));
        });
    }

    public function testGetFormattedDate(): void
    {
        $permissions = ['print', 'modify', 'copy', 'annot-forms', 'fill-forms', 'extract', 'assemble', 'print-high'];

        $enc = new \Com\Tecnick\Pdf\Encrypt\Encrypt(false);
        $result = $enc->getFormattedDate();
        $this->assertEquals('(D:', \substr($result, 0, 3));
        $this->assertEquals("+00'00')", \substr($result, -8));

        $this->bcRunIgnoringUserDeprecations(function () use ($permissions): void {
            $enc = new \Com\Tecnick\Pdf\Encrypt\Encrypt(true, \md5('file_id'), 0, $permissions, 'alpha');
            $result = $enc->getFormattedDate();
            $this->assertNotEmpty($result);
        });
    }
}
