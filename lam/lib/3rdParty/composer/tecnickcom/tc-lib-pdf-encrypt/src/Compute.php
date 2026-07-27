<?php

declare(strict_types=1);

/**
 * Compute.php
 *
 * @since     2008-01-02
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * This file is part of tc-lib-pdf-encrypt software library.
 */

namespace Com\Tecnick\Pdf\Encrypt;

use Com\Tecnick\Pdf\Encrypt\Exception as EncException;

/**
 * Com\Tecnick\Pdf\Encrypt\Compute
 *
 * PHP class to generate encryption data
 *
 * @since     2008-01-02
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
abstract class Compute extends \Com\Tecnick\Pdf\Encrypt\Data
{
    /**
     * Encrypt data using the specified encrypt type.
     *
     * @param int|string|false $type   Encrypt type.
     * @param string     $data   Data string to encrypt.
     * @param string     $key    Encryption key.
     * @param int        $objnum Object number.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function encrypt(int|string|false $type, string $data = '', string $key = '', int $objnum = 0): string
    {
        if (!$this->encryptdata['encrypted'] || $type === false) {
            return $data;
        }

        $encoder = self::ENCMAP[$type] ?? null;
        if ($encoder === null) {
            throw new EncException('unknown encryption type: ' . $type);
        }

        if ($key === '' && $type === $this->encryptdata['mode']) {
            if ($this->encryptdata['mode'] < 3) {
                $key = $this->getObjectKey($objnum);
            }

            if ($this->encryptdata['mode'] >= 3) { // mode >= 3: AES-256 (R5 or R6), use the full document key
                $key = $this->encryptdata['key'];
            }
        }

        return $this->encryptWithEncoderName($encoder, $data, $key);
    }

    /**
     * Dispatch to a concrete encryptor class for static analyzers.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithEncoderName(string $encoder, string $data, string $key): string
    {
        return match ($encoder) {
            'RCFourFive' => $this->encryptWithRCFourFive($data, $key),
            'RCFourSixteen' => $this->encryptWithRCFourSixteen($data, $key),
            'AESSixteen' => $this->encryptWithAESSixteen($data, $key),
            'AESThirtytwo' => $this->encryptWithAESThirtytwo($data, $key),
            'RCFour' => $this->encryptWithRCFour($data, $key),
            'AES' => $this->encryptWithAES($data, $key),
            'AESnopad' => $this->encryptWithAESnopad($data, $key),
            'MDFiveSixteen' => $this->encryptWithMDFiveSixteen($data, $key),
            'Seed' => $this->encryptSeed($data, $key),
            default => throw new EncException('unknown encryption type: ' . $encoder),
        };
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithRCFourFive(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\RCFourFive();
        return $enc->encrypt($data, $key);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithRCFourSixteen(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\RCFourSixteen();
        return $enc->encrypt($data, $key);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithAESSixteen(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\AESSixteen();
        return $enc->encrypt($data, $key);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithAESThirtytwo(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\AESThirtytwo();
        return $enc->encrypt($data, $key);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithRCFour(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\RCFour();
        return $enc->encrypt($data, $key);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithAES(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\AES();
        return $enc->encrypt($data, $key);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithAESnopad(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\AESnopad();
        return $enc->encrypt($data, $key);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptWithMDFiveSixteen(string $data, string $key): string
    {
        $enc = new \Com\Tecnick\Pdf\Encrypt\Type\MDFiveSixteen();
        return $enc->encrypt($data, $key);
    }

    /**
     * Encrypt using the Seed generator and normalize random errors.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptSeed(string $data, string $key): string
    {
        try {
            $enc = new \Com\Tecnick\Pdf\Encrypt\Type\Seed();
            return $enc->encrypt($data, $key);
        } catch (\Random\RandomException $e) {
            throw new EncException('Unable to generate random seed', 0, $e);
        }
    }

    /**
     * Compute encryption key depending on object number where the encrypted data is stored.
     * This is used for all strings and streams without crypt filter specifier.
     *
     * @param int $objnum Object number.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function getObjectKey(int $objnum): string
    {
        $objkey = $this->encryptdata['key'] . \pack('VXxx', $objnum);
        if ($this->encryptdata['mode'] === 2) {
            // AES-128 padding
            $objkey .= "\x73\x41\x6C\x54"; // sAlT
        }

        $objkey = \substr($this->encrypt('MD5-16', $objkey, 'H*'), 0, (int) (($this->encryptdata['Length'] / 8) + 5));
        return \substr($objkey, 0, 16);
    }

    /**
     * Convert encryption P value to a string of bytes, low-order byte first.
     *
     * @param int $protection 32bit encryption permission value (P value).
     */
    public function getEncPermissionsString(int $protection): string
    {
        $binprot = \sprintf('%032b', $protection);
        return (
            \chr((int) \bindec(\substr($binprot, 24, 8)) & 0xFF)
            . \chr((int) \bindec(\substr($binprot, 16, 8)) & 0xFF)
            . \chr((int) \bindec(\substr($binprot, 8, 8)) & 0xFF)
            . \chr((int) \bindec(\substr($binprot, 0, 8)) & 0xFF)
        );
    }

    /**
     * Return the permission code used on encryption (P value).
     *
     * @param array<string> $permissions The set of permissions (specify the ones you want to block).
     * @param int   $mode        Encryption strength: 0 = RC4-40 (deprecated); 1 = RC4-128 (deprecated);
     *                           2 = AES-128; 3 = AES-256 R5; 4 = AES-256 R6 (PDF 2.0 / ISO 32000-2).
     */
    public function getUserPermissionCode(array $permissions, int $mode = 0): int
    {
        $protection = 2_147_422_012; // 32 bit: (01111111 11111111 00001111 00111100)
        foreach ($permissions as $permission) {
            $permBit = self::PERMBITS[$permission] ?? null;
            if ($permBit === null) {
                continue;
            }

            if ($mode <= 0 && $permBit > 32) {
                continue;
            }

            // set only valid permissions
            if ($permBit === 2) {
                // the logic for bit 2 is inverted (cleared by default)
                $protection += $permBit;
                continue;
            }

            $protection -= $permBit;
        }

        return $protection;
    }

    /**
     * Compute UE value
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getUEValue(): string
    {
        if ($this->encryptdata['mode'] === 4) { // R6: use Algorithm 2.B
            return $this->encrypt(
                'AESnopad',
                $this->encryptdata['key'],
                $this->hash2B($this->encryptdata['user_password'], $this->encryptdata['UKS']),
            );
        }

        return $this->encrypt(
            'AESnopad',
            $this->encryptdata['key'],
            \hash('sha256', $this->encryptdata['user_password'] . $this->encryptdata['UKS'], true),
        );
    }

    /**
     * Compute OE value
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getOEValue(): string
    {
        if ($this->encryptdata['mode'] === 4) { // R6: use Algorithm 2.B
            return $this->encrypt(
                'AESnopad',
                $this->encryptdata['key'],
                $this->hash2B($this->encryptdata['owner_password'], $this->encryptdata['OKS'], $this->encryptdata['U']),
            );
        }

        return $this->encrypt(
            'AESnopad',
            $this->encryptdata['key'],
            \hash(
                'sha256',
                $this->encryptdata['owner_password'] . $this->encryptdata['OKS'] . $this->encryptdata['U'],
                true,
            ),
        );
    }

    /**
     * Compute U value
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getUValue(): string
    {
        if ($this->encryptdata['mode'] === 0) { // RC4-40
            return $this->encrypt('RC4', self::ENCPAD, $this->encryptdata['key']);
        }

        if ($this->encryptdata['mode'] < 3) { // RC4-128, AES-128
            $tmp = $this->encrypt('MD5-16', self::ENCPAD . $this->encryptdata['fileid'], 'H*');
            $enc = $this->encrypt('RC4', $tmp, $this->encryptdata['key']);
            $len = \strlen($tmp);
            for ($idx = 1; $idx <= 19; ++$idx) {
                $ekey = '';
                for ($jdx = 0; $jdx < $len; ++$jdx) {
                    $ekey .= \chr((\ord($this->encryptdata['key'][$jdx]) ^ $idx) & 0xFF);
                }

                $enc = $this->encrypt('RC4', $enc, $ekey);
            }

            $enc .= \str_repeat("\x00", 16);
            return \substr($enc, 0, 32);
        }

        // AES-256 ($this->encryptdata['mode'] >= 3)
        return $this->getUValueAes256();
    }

    /**
     * Compute O value
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getOValue(): string
    {
        if ($this->encryptdata['mode'] < 3) { // RC4-40, RC4-128, AES-128
            $tmp = $this->encrypt('MD5-16', $this->encryptdata['owner_password'], 'H*');
            if ($this->encryptdata['mode'] > 0) {
                for ($idx = 0; $idx < 50; ++$idx) {
                    $tmp = $this->encrypt('MD5-16', $tmp, 'H*');
                }
            }

            $owner_key = \substr($tmp, 0, (int) ($this->encryptdata['Length'] / 8));
            $enc = $this->encrypt('RC4', $this->encryptdata['user_password'], $owner_key);
            if ($this->encryptdata['mode'] > 0) {
                $len = \strlen($owner_key);
                for ($idx = 1; $idx <= 19; ++$idx) {
                    $ekey = '';
                    for ($jdx = 0; $jdx < $len; ++$jdx) {
                        $ekey .= \chr((\ord($owner_key[$jdx]) ^ $idx) & 0xFF);
                    }

                    $enc = $this->encrypt('RC4', $enc, $ekey);
                }
            }

            return $enc;
        }

        // AES-256 ($this->encryptdata['mode'] >= 3)
        return $this->getOValueAes256();
    }

    /**
     * Generate a fresh 8-byte validation salt and 8-byte key salt for AES-256
     * (R5/R6) password hashing (ISO 32000 Algorithm 8/9).
     *
     * @return array{0: string, 1: string} [validationSalt, keySalt]
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function generateAes256Salts(): array
    {
        $seed = $this->encrypt('MD5-16', $this->encrypt('seed'), 'H*');
        return [\substr($seed, 0, 8), \substr($seed, 8, 8)];
    }

    /**
     * Compute the U value for AES-256 (R5/R6).
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getUValueAes256(): string
    {
        [$this->encryptdata['UVS'], $this->encryptdata['UKS']] = $this->generateAes256Salts();
        $uHash = $this->encryptdata['mode'] === 4
            ? $this->hash2B($this->encryptdata['user_password'], $this->encryptdata['UVS']) // R6: Algorithm 2.B
            : \hash('sha256', $this->encryptdata['user_password'] . $this->encryptdata['UVS'], true); // R5: SHA-256

        return $uHash . $this->encryptdata['UVS'] . $this->encryptdata['UKS'];
    }

    /**
     * Compute the O value for AES-256 (R5/R6).
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getOValueAes256(): string
    {
        [$this->encryptdata['OVS'], $this->encryptdata['OKS']] = $this->generateAes256Salts();
        $oHash = $this->encryptdata['mode'] === 4
            ? $this->hash2B($this->encryptdata['owner_password'], $this->encryptdata['OVS'], $this->encryptdata['U']) // R6
            : \hash(
                'sha256',
                $this->encryptdata['owner_password'] . $this->encryptdata['OVS'] . $this->encryptdata['U'],
                true,
            ); // R5: SHA-256

        return $oHash . $this->encryptdata['OVS'] . $this->encryptdata['OKS'];
    }

    /**
     * Compute Algorithm 2.B hash (ISO 32000-2 / PDF 2.0) for AES-256 R6 key derivation.
     *
     * Implements the iterative SHA-256/384/512 + AES-128-CBC hash described in
     * ISO 32000-2 section 7.6.4.3.4 (Algorithm 2.B).
     *
     * @param string $password  UTF-8 password (already truncated to ≤ 127 bytes).
     * @param string $salt      8-byte validation or key salt.
     * @param string $userHash  48-byte U value for owner-side calculations; empty for user-side.
     *
     * @throws EncException When the internal AES-128-CBC step fails.
     */
    protected function hash2B(#[\SensitiveParameter] string $password, string $salt, string $userHash = ''): string
    {
        $hashK = \hash('sha256', $password . $salt, true);
        $round = 0;
        do {
            // inputBlock length is always 64 * (len(password) + 32 + len(userHash)).
            // 64 is a multiple of 16, so inputBlock is always AES-block-aligned (no padding needed).
            $inputBlock = \str_repeat($password . $hashK . $userHash, 64);
            $encrypted = \openssl_encrypt(
                $inputBlock,
                'aes-128-cbc',
                \substr($hashK, 0, 16),
                OPENSSL_RAW_DATA | OPENSSL_ZERO_PADDING,
                \substr($hashK, 16, 16),
            );
            if ($encrypted === false) {
                throw new EncException('AES-128-CBC encryption failed in hash2B');
            }

            $sum = 0;
            for ($idx = 0; $idx < 16; ++$idx) {
                $sum += \ord($encrypted[$idx]);
            }

            $algo = match ($sum % 3) {
                0 => 'sha256',
                1 => 'sha384',
                default => 'sha512',
            };
            $hashK = \hash($algo, $encrypted, true);
            ++$round;
            $lastByte = \ord($encrypted[\strlen($encrypted) - 1]);
        } while (!($round >= 64 && $lastByte <= ($round - 32)));

        return \substr($hashK, 0, 32);
    }

    /**
     * Compute encryption key
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function generateEncryptionKey(): void
    {
        if ($this->encryptdata['pubkey']) {
            $this->generatePublicEncryptionKey();
            return;
        }

        // standard mode
        $this->generateStandardEncryptionKey();
    }

    /**
     * Compute standard encryption key
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function generateStandardEncryptionKey(): void
    {
        $keybytelen = $this->encryptdata['Length'] / 8;
        if ($this->encryptdata['mode'] >= 3) { // AES-256 (R5 or R6)
            $this->generateAes256StandardEncryptionKey((int) $keybytelen);

            return;
        }

        $this->generateLegacyStandardEncryptionKey((int) $keybytelen);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function generateAes256StandardEncryptionKey(int $keybytelen): void
    {
        // generate 256 bit random key
        $this->encryptdata['key'] = \substr(\hash('sha256', $this->encrypt('seed'), true), 0, $keybytelen);
        // truncate passwords
        $this->encryptdata['user_password'] = \substr($this->encryptdata['user_password'], 0, 127);
        $this->encryptdata['owner_password'] = \substr($this->encryptdata['owner_password'], 0, 127);
        $this->encryptdata['U'] = $this->getUValue();
        $this->encryptdata['UE'] = $this->getUEValue();
        $this->encryptdata['O'] = $this->getOValue();
        $this->encryptdata['OE'] = $this->getOEValue();
        $this->encryptdata['P'] = $this->encryptdata['protection'];
        // Computing the encryption dictionary's Perms (permissions) value
        $perms = $this->getEncPermissionsString($this->encryptdata['protection']); // bytes 0-3
        $perms .= \chr(255) . \chr(255) . \chr(255) . \chr(255); // bytes 4-7
        $perms .= $this->encryptdata['EncryptMetadata'] ? 'T' : 'F'; // byte 8: EncryptMetadata flag
        $perms .= 'adb'; // bytes 9-11
        $perms .= \openssl_random_pseudo_bytes(4); // bytes 12-15 must be random per PDF spec
        $this->encryptdata['perms'] = $this->encrypt('AESnopad', $perms, $this->encryptdata['key']);
    }

    /**
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function generateLegacyStandardEncryptionKey(int $keybytelen): void
    {
        // RC4-40, RC4-128, AES-128
        // Pad passwords
        $this->encryptdata['user_password'] = \substr($this->encryptdata['user_password'] . self::ENCPAD, 0, 32);
        $this->encryptdata['owner_password'] = \substr($this->encryptdata['owner_password'] . self::ENCPAD, 0, 32);
        $this->encryptdata['O'] = $this->getOValue();
        // get default permissions (reverse byte order)
        $permissions = $this->getEncPermissionsString($this->encryptdata['protection']);
        // Compute encryption key
        $tmp = $this->encrypt(
            'MD5-16',
            $this->encryptdata['user_password'] . $this->encryptdata['O'] . $permissions . $this->encryptdata['fileid'],
            'H*',
        );
        if ($this->encryptdata['mode'] > 0) {
            for ($idx = 0; $idx < 50; ++$idx) {
                $tmp = $this->encrypt('MD5-16', \substr($tmp, 0, $keybytelen), 'H*');
            }
        }

        $this->encryptdata['key'] = \substr($tmp, 0, $keybytelen);
        $this->encryptdata['U'] = $this->getUValue();
        $this->encryptdata['P'] = $this->encryptdata['protection'];
    }

    /**
     * Compute public encryption key
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     * @SuppressWarnings("PHPMD.NPathComplexity")
     * @SuppressWarnings("PHPMD.ExcessiveMethodLength")
     */
    protected function generatePublicEncryptionKey(): void
    {
        $keybytelen = (int) ($this->encryptdata['Length'] / 8);
        // random 20-byte seed
        $seed = \sha1($this->encrypt('seed'), true);
        $recipient_bytes = '';
        $pubkeys = $this->encryptdata['pubkeys'] ?? throw new EncException('Missing pubkeys');
        foreach ($pubkeys as $pubkey) {
            $recipient_bytes .= $this->getEncryptedRecipientBytes($pubkey, $seed);
        }

        // calculate encryption key
        if ($this->encryptdata['mode'] >= 3) { // AES-256 (R5/R6)
            $this->encryptdata['key'] = \substr(\hash('sha256', $seed . $recipient_bytes, true), 0, $keybytelen);
            return;
        }

        // RC4-40, RC4-128, AES-128
        $this->encryptdata['key'] = \substr(\sha1($seed . $recipient_bytes, true), 0, $keybytelen);
    }

    /**
     * @param array{c:string,p:array<string>} $pubkey
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function getEncryptedRecipientBytes(array $pubkey, string $seed): string
    {
        // for each public certificate
        $pkprotection = $this->getUserPermissionCode($pubkey['p'], $this->encryptdata['mode']);

        // get default permissions (reverse byte order)
        $pkpermissions = $this->getEncPermissionsString($pkprotection);
        // envelope data
        $envelope = $seed . $pkpermissions;

        // write the envelope data to a temporary file
        $tempkeyfile = \tempnam(
            \sys_get_temp_dir(),
            '__tcpdf_key_' . \md5($this->encryptdata['fileid'] . $envelope) . '_',
        );
        if ($tempkeyfile === false) {
            throw new EncException('Unable to generate temporary key file name');
        }

        if (\file_put_contents($tempkeyfile, $envelope) === false) {
            \unlink($tempkeyfile);
            throw new EncException('Unable to create temporary key file: ' . $tempkeyfile);
        }

        $tempencfile = \tempnam(
            \sys_get_temp_dir(),
            '__tcpdf_enc_' . \md5($this->encryptdata['fileid'] . $envelope) . '_',
        );
        if ($tempencfile === false) {
            // The key file was already created; remove it before bailing out.
            \unlink($tempkeyfile);
            throw new EncException('Unable to generate temporary key file name');
        }

        try {
            return $this->encryptRecipientEnvelope($pubkey, $tempkeyfile, $tempencfile);
        } finally {
            // Always remove the temporary files: $tempkeyfile holds the plaintext
            // envelope (sensitive key material) and must never be left behind.
            \unlink($tempkeyfile);
            \unlink($tempencfile);
        }
    }

    /**
     * Encrypt the envelope file to the recipient's certificate and extract the
     * resulting PKCS#7 signature bytes.
     *
     * @param array{c:string,p:array<string>} $pubkey       Recipient certificate/permissions.
     * @param string                          $tempkeyfile  Path to the plaintext envelope file.
     * @param string                          $tempencfile  Path to the PKCS#7 output file.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function encryptRecipientEnvelope(array $pubkey, string $tempkeyfile, string $tempencfile): string
    {
        if (!\function_exists('openssl_pkcs7_encrypt')) {
            throw new EncException(
                'Unable to encrypt the file: '
                . $tempkeyfile
                . "\n"
                . 'Public-Key Security requires openssl_pkcs7_encrypt.',
            );
        }

        $pkey = \file_get_contents($pubkey['c']);
        if ($pkey === false) {
            throw new EncException('Unable to read public key file: ' . $pubkey['c']);
        }

        if (!\openssl_pkcs7_encrypt($tempkeyfile, $tempencfile, $pkey, [], PKCS7_BINARY)) {
            throw new EncException(
                'Unable to encrypt the file: '
                . $tempkeyfile
                . "\n"
                . 'OpenSSL error: '
                . (string) \openssl_error_string(),
            );
        }

        // read encryption signature
        $signature = \file_get_contents($tempencfile);
        if ($signature === false) {
            throw new EncException('Unable to read signature file: ' . $tempencfile);
        }

        $sigcontentpos = \strpos($signature, 'Content-Disposition');
        if ($sigcontentpos === false) {
            throw new EncException('Unable to find signature content');
        }

        // extract signature
        $signature = \substr($signature, $sigcontentpos);

        $tmparr = \explode("\n\n", $signature);
        $signature = \trim($tmparr[1] ?? '');
        unset($tmparr);
        // decode signature
        $signature = \base64_decode($signature, strict: true);
        if ($signature === false) {
            throw new EncException('Unable to decode signature: ' . $tempencfile);
        }

        $sigarr = \unpack('H*', $signature);
        if ($sigarr === false) {
            throw new EncException('Unable to unpack signature: ' . $tempencfile);
        }

        // convert signature to hex
        $hexsignature = (string) \current($sigarr);
        if ($hexsignature === '') {
            throw new EncException('Unable to convert signature: ' . $tempencfile);
        }

        // store signature on recipients array
        $this->encryptdata['Recipients'][] = $hexsignature;

        // The bytes of each item in the Recipients array of PKCS#7 objects
        // in the order in which they appear in the array
        return $signature;
    }
}
