<?php

declare(strict_types=1);

/**
 * RCFour.php
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
 * Com\Tecnick\Pdf\Encrypt\Type\RCFour
 *
 * RC4 is the standard encryption algorithm used in PDF format
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class RCFour
{
    /**
     * List of valid openssl cyphers for RC4 encryption.
     *
     * @var array<string>
     */
    public const VALID_CIPHERS = [
        'RC4',
        'RC4-40',
    ];

    /**
     * Encrypt the data using the RC4 (Rivest Cipher 4, also known as ARC4 or ARCFOUR) algorithm.
     * RC4 is one of the standard encryption algorithm used in PDF format.
     * If possible, please use AES encryption instead as this is insecure.
     *
     * @param string $data Data string to encrypt
     * @param string $key  Encryption key
     * @param string $mode Cipher
     *
     * @return string encrypted text
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function encrypt(string $data, string $key, string $mode = ''): string
    {
        if ($mode === '') {
            $mode = \strlen($key) > 5 ? 'RC4' : 'RC4-40';
        }

        if (!\in_array($mode, self::VALID_CIPHERS, strict: true)) {
            throw new EncException('invalid cipher: ' . $mode);
        }

        if (!\in_array($mode, \openssl_get_cipher_methods(), strict: true)) {
            return $this->rc4($data, $key);
        }

        $enc = \openssl_encrypt($data, $mode, $key, OPENSSL_RAW_DATA | OPENSSL_ZERO_PADDING);
        if ($enc === false) {
            throw new EncException('openssl_encrypt failed');
        }

        return $enc;
    }

    /**
     * Returns the input text encrypted using RC4 algorithm and the specified key.
     * This function is used when the openssl extension is not available.
     *
     * @param string $data Data string to encrypt
     * @param string $key  Encryption key
     *
     * @return string encrypted text
     */
    protected function rc4(string $data, string $key): string
    {
        $rc4 = $this->initRc4State($key);
        return $this->applyRc4Stream($data, $rc4);
    }

    /**
     * @return array<int, int>
     */
    protected function initRc4State(string $key): array
    {
        $pkey = \str_repeat($key, \max(1, (int) ((256 / \strlen($key)) + 1)));
        /** @var array<int, int> $rc4 */
        $rc4 = \range(0, 255);
        $pos = 0;
        for ($idx = 0; $idx < 256; ++$idx) {
            $val = $rc4[$idx] ?? 0;
            $pos = ($pos + $val + \ord($pkey[$idx])) % 256;
            $rc4[$idx] = $rc4[$pos] ?? 0;
            $rc4[$pos] = $val;
        }

        return $rc4;
    }

    /**
     * @param array<int, int> $rc4
     */
    protected function applyRc4Stream(string $data, array $rc4): string
    {
        $len = \strlen($data);
        $posa = 0;
        $posb = 0;
        $out = '';
        for ($idx = 0; $idx < $len; ++$idx) {
            $posa = ($posa + 1) % 256;
            $val = $rc4[$posa] ?? 0;
            $posb = ($posb + $val) % 256;
            $rc4[$posa] = $rc4[$posb] ?? 0;
            $rc4[$posb] = $val;
            $pkey = $rc4[(($rc4[$posa] ?? 0) + ($rc4[$posb] ?? 0)) % 256] ?? 0;
            $out .= \chr((\ord($data[$idx]) ^ $pkey) & 0xFF);
        }

        return $out;
    }
}
