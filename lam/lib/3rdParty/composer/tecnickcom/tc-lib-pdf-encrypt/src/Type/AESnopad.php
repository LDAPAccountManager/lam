<?php

declare(strict_types=1);

/**
 * AESnopad.php
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

namespace Com\Tecnick\Pdf\Encrypt\Type;

use Com\Tecnick\Pdf\Encrypt\Exception as EncException;

/**
 * Com\Tecnick\Pdf\Encrypt\Type\AESnopad
 *
 * AES no-padding
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class AESnopad
{
    /**
     * Block size (IV length):
     * \openssl_cipher_iv_length('aes-256-cbc')
     *
     * @var int
     */
    public const BLOCKSIZE = 16;

    /**
     * Initialization Vector (16 bytes)
     *
     * @var string
     */
    public const IVECT = "\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00";

    /**
     * List of valid openssl cyphers for AES encryption.
     *
     * @var array<string>
     */
    public const VALID_CIPHERS = [
        'aes-128-cbc',
        'aes-256-cbc',
    ];

    /**
     * Encrypt the data
     *
     * @param string $data  Data string to encrypt
     * @param string $key   Encryption key
     * @param string $ivect Initialization vector
     * @param string $mode  Cipher
     *
     * @return string Encrypted data string.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function encrypt(
        string $data,
        string $key,
        string $ivect = self::IVECT,
        string $mode = 'aes-256-cbc',
    ): string {
        $this->checkCipher($mode);
        $this->checkKeyLength($key, $mode);

        $enc = \openssl_encrypt(
            $this->pad($data, self::BLOCKSIZE),
            $mode,
            $key,
            OPENSSL_RAW_DATA | OPENSSL_ZERO_PADDING,
            $ivect,
        );

        if ($enc === false) {
            throw new EncException('encryption error: ' . (string) \openssl_error_string());
        }

        return $enc;
    }

    /**
     * Decrypt data that was produced by encrypt().
     *
     * encrypt() zero-pads the plaintext to 16-byte alignment and applies no block
     * padding, so this method decrypts with OPENSSL_ZERO_PADDING and returns the
     * zero-padded plaintext. The caller strips any trailing zero bytes.
     *
     * @param string $data  Encrypted data (produced by encrypt()).
     * @param string $key   Encryption key.
     * @param string $ivect Initialization vector (default: 16 zero bytes).
     * @param string $mode  Cipher (default: aes-256-cbc).
     *
     * @return string Decrypted data string.
     *
     * @throws EncException When decryption fails.
     */
    public function decrypt(
        string $data,
        string $key,
        string $ivect = self::IVECT,
        string $mode = 'aes-256-cbc',
    ): string {
        $this->checkCipher($mode);
        $this->checkKeyLength($key, $mode);

        $dec = \openssl_decrypt($data, $mode, $key, OPENSSL_RAW_DATA | OPENSSL_ZERO_PADDING, $ivect);

        if ($dec === false) {
            throw new EncException('decryption error: ' . (string) \openssl_error_string());
        }

        return $dec;
    }

    /**
     * Zero-pad the input string up to the next multiple of $length.
     *
     * @param string $data   Data to pad
     * @param int    $length Block length
     *
     * @return string Padded string
     */
    protected function pad(string $data, int $length): string
    {
        $rem = \strlen($data) % $length;
        if ($rem !== 0) {
            $data .= \str_repeat("\x00", \max(0, $length - $rem));
        }

        return $data;
    }

    /**
     * Check if the cipher is valid and available.
     *
     * @param string $cipher openSSL cipher name.
     *
     * @throws EncException in case of error.
     */
    public function checkCipher(string $cipher): void
    {
        if (!\in_array($cipher, self::VALID_CIPHERS, strict: true)) {
            throw new EncException('invalid cipher: ' . $cipher);
        }

        if (!\in_array($cipher, \openssl_get_cipher_methods(), strict: true)) {
            throw new EncException('unavailable cipher: ' . $cipher);
        }
    }

    /**
     * Check that the key length matches the one required by the cipher.
     *
     * OpenSSL zero-extends a short key and truncates a long one.
     *
     * @param string $key    Encryption key.
     * @param string $cipher openSSL cipher name.
     *
     * @throws EncException in case of error.
     */
    public function checkKeyLength(string $key, string $cipher): void
    {
        $expected = $cipher === 'aes-128-cbc' ? self::BLOCKSIZE : 2 * self::BLOCKSIZE;
        if (\strlen($key) !== $expected) {
            throw new EncException(
                'invalid key length for ' . $cipher . ': expected ' . $expected . ' bytes, got ' . \strlen($key),
            );
        }
    }
}
