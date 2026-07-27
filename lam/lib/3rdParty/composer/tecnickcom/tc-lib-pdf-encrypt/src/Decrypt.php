<?php

declare(strict_types=1);

/**
 * Decrypt.php
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

namespace Com\Tecnick\Pdf\Encrypt;

use Com\Tecnick\Pdf\Encrypt\Type\AESnopad;

/**
 * Com\Tecnick\Pdf\Encrypt\Decrypt
 *
 * Authenticates a password (or private key for public-key mode) against a PDF
 * encryption dictionary and recovers the document file-encryption key.
 *
 * Usage:
 *   $dec = new Decrypt($encrypt->getEncryptionData());
 *   if ($dec->authenticate('userpass')) {
 *       $plaintext = $dec->decryptString($ciphertext, $objnum);
 *   }
 *
 * After successful authentication the derived key is stored internally and:
 *   - decryptString() decrypts PDF string/stream objects.
 *   - getObjectKey()   returns the per-object key for AES-128 streams.
 *   - getDocumentKey() returns the raw 32-byte (or shorter) file key.
 *
 * @since     2026-04-30
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * @phpstan-import-type TEncryptData from Output
 *
 * @phpstan-type TDecryptInput TEncryptData|array{
 *     'V': int,
 *     'Length': int,
 *     'O': string,
 *     'U': string,
 *     'P': int,
 *     'fileid': string,
 *     'mode': int,
 *     'OE'?: string,
 *     'UE'?: string,
 *     'EncryptMetadata'?: bool,
 *     'pubkey'?: bool,
 *     'Recipients'?: array<array-key, string>,
 * }
 *
 * @SuppressWarnings("PHPMD.ExcessiveClassComplexity")
 */
class Decrypt extends \Com\Tecnick\Pdf\Encrypt\Compute
{
    /**
     * Initialise the decryptor from an encryption dictionary.
     *
     * Accepts the array returned by Encrypt::getEncryptionData() or any array
     * that satisfies the TDecryptInput shape.  All fields not present in the
     * input are filled with the defaults defined in Output::$encryptdata.
     *
     * @param TDecryptInput $input Encryption dictionary fields.
     */
    public function __construct(array $input)
    {
        $this->encryptdata['V'] = (int) $input['V'];
        $this->encryptdata['Length'] = (int) $input['Length'];
        $this->encryptdata['O'] = $input['O'];
        $this->encryptdata['U'] = $input['U'];
        $this->encryptdata['P'] = (int) $input['P'];
        $this->encryptdata['fileid'] = $input['fileid'];
        $this->encryptdata['mode'] = (int) $input['mode'];
        $this->encryptdata['EncryptMetadata'] = $input['EncryptMetadata'] ?? $this->encryptdata['EncryptMetadata'];
        $this->encryptdata['pubkey'] = $input['pubkey'] ?? $this->encryptdata['pubkey'];
        if (\array_key_exists('Recipients', $input)) {
            $this->encryptdata['Recipients'] = $input['Recipients'];
        }

        if (\array_key_exists('OE', $input)) {
            $this->encryptdata['OE'] = $input['OE'];
        }

        if (\array_key_exists('UE', $input)) {
            $this->encryptdata['UE'] = $input['UE'];
        }

        // Ensure encrypt()-based primitives (RC4, MD5-16) are active for key derivation.
        $this->encryptdata['encrypted'] = true;
        // Clear the key: it must be recovered by a successful authenticate() call.
        $this->encryptdata['key'] = '';
    }

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------

    /**
     * Authenticate using a password and/or private key.
     *
     * Tries the supplied string first as the user password, then as the owner
     * password.  For public-key mode, $privkeyPath must be the path to a PEM
     * file containing the recipient's certificate and private key; $password is
     * ignored in that case.
     *
     * On success the derived file-encryption key is stored internally.
     *
     * @param string $password    UTF-8 password to test (ignored for pubkey mode).
     * @param string $privkeyPath Path to PEM file for public-key mode.
     *
     * @return bool True when authentication succeeds.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function authenticate(#[\SensitiveParameter] string $password, string $privkeyPath = ''): bool
    {
        if ($this->encryptdata['pubkey']) {
            return $this->authenticatePublicKey($privkeyPath);
        }

        if ($this->encryptdata['mode'] >= 3) {
            return $this->authenticatePasswordR5R6($password);
        }

        return $this->authenticatePasswordR24($password);
    }

    /**
     * Decrypt a PDF string or stream object.
     *
     * Must be called after a successful authenticate() call.
     *
     * For RC4 modes (0, 1) the operation is symmetric: the same method that
     * encrypts also decrypts.  For AES modes (2, 3, 4) the first 16 bytes of
     * $data are the random IV; the remainder is the ciphertext.  The PKCS#7
     * padding applied during encryption is stripped, so the exact original
     * plaintext is returned.
     *
     * @param string $data   Encrypted string/stream data.
     * @param int    $objnum PDF object number (used for per-object key derivation
     *                       in RC4 and AES-128 modes).
     *
     * @return string Decrypted data.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function decryptString(string $data, int $objnum = 0): string
    {
        if (!$this->encryptdata['encrypted'] || $this->encryptdata['key'] === '') {
            return $data;
        }

        $mode = $this->encryptdata['mode'];

        if ($mode < 2) {
            // RC4 is symmetric: the same encrypt() call decrypts.
            return $this->encrypt($mode, $data, '', $objnum);
        }

        return $this->decryptAes($data, $objnum);
    }

    /**
     * Return the recovered file-encryption key.
     *
     * @return string Raw binary key (empty string before authenticate() succeeds).
     */
    public function getDocumentKey(): string
    {
        return $this->encryptdata['key'];
    }

    // -------------------------------------------------------------------------
    // RC4 / R2–R4 authentication (modes 0, 1, 2)
    // -------------------------------------------------------------------------

    /**
     * Authenticate a password for R2–R4 (RC4-40, RC4-128, AES-128).
     *
     * First tries the password as the user password; on failure tries it as the
     * owner password.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticatePasswordR24(#[\SensitiveParameter] string $password): bool
    {
        $paddedPass = \substr($password . self::ENCPAD, 0, 32);

        if ($this->authenticateUserR24($paddedPass)) {
            return true;
        }

        return $this->authenticateOwnerR24($password);
    }

    /**
     * Authenticate $paddedPass as the user password for R2–R4.
     *
     * Derives a candidate encryption key from the padded password and verifies
     * it against the stored U value using Algorithm 6 (PDF spec §7.6.3.4).
     * On success the verified key remains stored in $encryptdata['key'].
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticateUserR24(string $paddedPass): bool
    {
        $savedKey = $this->encryptdata['key'];
        $this->encryptdata['key'] = $this->deriveKeyR24($paddedPass);

        $computedU = $this->getUValue();

        if ($this->compareUserHashR24($computedU)) {
            return true;
        }

        $this->encryptdata['key'] = $savedKey;
        return false;
    }

    /**
     * Authenticate $password as the owner password for R2–R4.
     *
     * Derives the owner key, decrypts the O entry to recover the candidate user
     * password, then delegates to authenticateUserR24.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticateOwnerR24(#[\SensitiveParameter] string $password): bool
    {
        $paddedOwner = \substr($password . self::ENCPAD, 0, 32);
        $ownerKey = $this->deriveOwnerKeyR24($paddedOwner);
        $candidateUserPass = $this->decryptOToUserPass($ownerKey);
        return $this->authenticateUserR24($candidateUserPass);
    }

    /**
     * Derive the file encryption key for R2–R4 from a 32-byte padded user password.
     *
     * Implements Algorithm 2 from PDF spec §7.6.3.3.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function deriveKeyR24(string $paddedPass): string
    {
        $keybytelen = (int) ($this->encryptdata['Length'] / 8);
        $permBytes = $this->getEncPermissionsString($this->encryptdata['P']);

        $tmp = $this->encrypt(
            'MD5-16',
            $paddedPass . $this->encryptdata['O'] . $permBytes . $this->encryptdata['fileid'],
        );

        if ($this->encryptdata['mode'] > 0) {
            for ($idx = 0; $idx < 50; ++$idx) {
                $tmp = $this->encrypt('MD5-16', \substr($tmp, 0, $keybytelen));
            }
        }

        return \substr($tmp, 0, $keybytelen);
    }

    /**
     * Compare a freshly computed U value against the stored one for R2–R4.
     *
     * R2 requires an exact 32-byte match; R3/R4 compare only the first 16 bytes.
     */
    protected function compareUserHashR24(string $computedU): bool
    {
        if ($this->encryptdata['mode'] === 0) {
            return $computedU === $this->encryptdata['U'];
        }

        return \substr($computedU, 0, 16) === \substr($this->encryptdata['U'], 0, 16);
    }

    /**
     * Derive the owner key from a 32-byte padded owner password for R2–R4.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function deriveOwnerKeyR24(string $paddedOwnerPass): string
    {
        $keybytelen = (int) ($this->encryptdata['Length'] / 8);
        $tmp = $this->encrypt('MD5-16', $paddedOwnerPass);

        if ($this->encryptdata['mode'] > 0) {
            for ($idx = 0; $idx < 50; ++$idx) {
                $tmp = $this->encrypt('MD5-16', \substr($tmp, 0, $keybytelen));
            }
        }

        return \substr($tmp, 0, $keybytelen);
    }

    /**
     * Decrypt the O entry using the owner key to recover the candidate user password.
     *
     * Reverses the iterative RC4 encryption applied by getOValue() in Compute.
     *
     * @SuppressWarnings("PHPMD.CyclomaticComplexity")
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function decryptOToUserPass(string $ownerKey): string
    {
        if ($this->encryptdata['mode'] === 0) {
            return $this->encrypt('RC4', $this->encryptdata['O'], $ownerKey);
        }

        $keyLen = \strlen($ownerKey);
        $candidate = $this->encryptdata['O'];

        for ($idx = 19; $idx >= 1; --$idx) {
            $xoredKey = '';
            for ($jdx = 0; $jdx < $keyLen; ++$jdx) {
                $xoredKey .= \chr((\ord($ownerKey[$jdx]) ^ $idx) & 0xFF);
            }

            $candidate = $this->encrypt('RC4', $candidate, $xoredKey);
        }

        return $this->encrypt('RC4', $candidate, $ownerKey);
    }

    // -------------------------------------------------------------------------
    // AES-256 / R5–R6 authentication (modes 3, 4)
    // -------------------------------------------------------------------------

    /**
     * Authenticate a password for R5 (mode 3) or R6 (mode 4).
     *
     * Tries the password as the user password, then as the owner password.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticatePasswordR5R6(#[\SensitiveParameter] string $password): bool
    {
        if ($this->authenticateUserR5R6($password)) {
            $this->recoverKeyFromUser($password);
            return true;
        }

        if ($this->authenticateOwnerR5R6($password)) {
            $this->recoverKeyFromOwner($password);
            return true;
        }

        return false;
    }

    /**
     * Verify $password as the user password for R5/R6 (Algorithm 11/13).
     *
     * Computes hash(password ∥ U[32..39]) and compares to U[0..32].
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticateUserR5R6(#[\SensitiveParameter] string $password): bool
    {
        $uvs = \substr($this->encryptdata['U'], 32, 8);
        $expected = \substr($this->encryptdata['U'], 0, 32);
        return $this->hashR5R6($password, $uvs) === $expected;
    }

    /**
     * Verify $password as the owner password for R5/R6 (Algorithm 13/15).
     *
     * Computes hash(password ∥ O[32..39] ∥ U[0..48]) and compares to O[0..32].
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticateOwnerR5R6(#[\SensitiveParameter] string $password): bool
    {
        $ovs = \substr($this->encryptdata['O'], 32, 8);
        $userHash = \substr($this->encryptdata['U'], 0, 48);
        $expected = \substr($this->encryptdata['O'], 0, 32);
        return $this->hashR5R6($password, $ovs, $userHash) === $expected;
    }

    /**
     * Recover the file encryption key using the verified user password (Algorithm 12/14).
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function recoverKeyFromUser(#[\SensitiveParameter] string $password): void
    {
        $uks = \substr($this->encryptdata['U'], 40, 8);
        $hashkey = $this->hashR5R6($password, $uks);
        $aesnopad = new AESnopad();
        $this->encryptdata['key'] = $aesnopad->decrypt($this->encryptdata['UE'], $hashkey);
    }

    /**
     * Recover the file encryption key using the verified owner password (Algorithm 14/16).
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function recoverKeyFromOwner(#[\SensitiveParameter] string $password): void
    {
        $oks = \substr($this->encryptdata['O'], 40, 8);
        $userHash = \substr($this->encryptdata['U'], 0, 48);
        $hashkey = $this->hashR5R6($password, $oks, $userHash);
        $aesnopad = new AESnopad();
        $this->encryptdata['key'] = $aesnopad->decrypt($this->encryptdata['OE'], $hashkey);
    }

    /**
     * Compute the R5/R6 password hash.
     *
     * R5 (mode 3): SHA-256(password ∥ salt ∥ userHash).
     * R6 (mode 4): Algorithm 2.B (ISO 32000-2 §7.6.4.3.4).
     *
     * @param string $password  UTF-8 password (truncated to ≤ 127 bytes by caller).
     * @param string $salt      8-byte validation or key salt.
     * @param string $userHash  48-byte U value for owner-side; empty for user-side.
     *
     * @return string 32-byte binary hash.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function hashR5R6(#[\SensitiveParameter] string $password, string $salt, string $userHash = ''): string
    {
        if ($this->encryptdata['mode'] === 4) {
            return $this->hash2B($password, $salt, $userHash);
        }

        return \hash('sha256', $password . $salt . $userHash, true);
    }

    // -------------------------------------------------------------------------
    // AES stream/string decryption helper
    // -------------------------------------------------------------------------

    /**
     * Decrypt an AES-encrypted PDF string or stream (modes 2, 3, 4).
     *
     * The ciphertext is prefixed with a 16-byte random IV as required by the
     * PDF specification (§7.6.3).  Returns an empty string when the data is
     * shorter than the IV length.
     *
     * The string/stream ciphertext is produced with PKCS#7 block padding
     * (AESV2/AESV3 crypt filters, ISO 32000), so decryption is performed
     * WITHOUT OPENSSL_ZERO_PADDING: OpenSSL validates and strips the PKCS#7
     * padding, recovering the exact original plaintext.  Returns an empty
     * string when the padding is invalid (e.g. wrong key or corrupted data).
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function decryptAes(string $data, int $objnum): string
    {
        if (\strlen($data) <= AESnopad::BLOCKSIZE) {
            return '';
        }

        $ivect = \substr($data, 0, AESnopad::BLOCKSIZE);
        $ciphertext = \substr($data, AESnopad::BLOCKSIZE);
        $mode = $this->encryptdata['mode'];
        $key = $mode < 3 ? $this->getObjectKey($objnum) : $this->encryptdata['key'];
        $cipher = $mode === 2 ? 'aes-128-cbc' : 'aes-256-cbc';

        $dec = \openssl_decrypt($ciphertext, $cipher, $key, OPENSSL_RAW_DATA, $ivect);

        return $dec === false ? '' : $dec;
    }

    // -------------------------------------------------------------------------
    // Public-key mode authentication
    // -------------------------------------------------------------------------

    /**
     * Authenticate using a recipient's PEM private-key file (public-key mode).
     *
     * Iterates over all Recipients entries, tries to decrypt each PKCS#7 envelope
     * with the supplied certificate/key, extracts the seed, and derives the
     * document encryption key.
     *
     * @param string $privkeyPath Path to the recipient's PEM file (cert + key).
     *
     * @return bool True when a matching recipient is found and the key is derived.
     */
    protected function authenticatePublicKey(string $privkeyPath): bool
    {
        if ($privkeyPath === '' || !\is_readable($privkeyPath)) {
            return false;
        }

        $certPem = \file_get_contents($privkeyPath);
        if ($certPem === false) {
            return false;
        }

        $seed = $this->findDecryptedRecipientSeed($certPem);
        if ($seed === null) {
            return false;
        }

        $this->derivePublicKey($seed);
        return true;
    }

    /**
     * Iterate over all Recipients and return the 20-byte seed from the first
     * envelope that can be decrypted with $certPem, or null on failure.
     *
     * @return string|null 20-byte seed, or null when no matching recipient found.
     */
    protected function findDecryptedRecipientSeed(string $certPem): ?string
    {
        foreach ($this->encryptdata['Recipients'] as $hexRecipient) {
            // Silence native warning noise for malformed recipient entries;
            // false is handled explicitly by the guard below.
            \set_error_handler(static fn(): bool => true);
            try {
                $derData = \hex2bin($hexRecipient);
            } finally {
                \restore_error_handler();
            }

            if ($derData === false) {
                continue;
            }

            $envelope = $this->tryDecryptRecipient($derData, $certPem);
            if ($envelope !== null && \strlen($envelope) >= 24) {
                // envelope = seed (20 bytes) + permissions (4 bytes)
                return \substr($envelope, 0, 20);
            }
        }

        return null;
    }

    /**
     * Attempt to decrypt a single DER-encoded PKCS#7 recipient envelope.
     *
     * Reconstructs the S/MIME message (as produced by openssl_pkcs7_encrypt),
     * writes it to a temporary file, calls openssl_pkcs7_decrypt with the
     * supplied certificate/key, and returns the raw decrypted bytes on success.
     *
     * @param string $derData Raw DER binary (one entry from Recipients[]).
     * @param string $certPem PEM certificate + private key of the recipient.
     *
     * @return string|null Decrypted envelope bytes, or null on failure.
     */
    protected function tryDecryptRecipient(string $derData, string $certPem): ?string
    {
        $smime =
            "MIME-Version: 1.0\r\n"
            . 'Content-Type: application/pkcs7-mime;'
            . " smime-type=enveloped-data; name=\"smime.p7m\"\r\n"
            . "Content-Transfer-Encoding: base64\r\n\r\n"
            . \chunk_split(\base64_encode($derData));

        $tmpIn = \tempnam(\sys_get_temp_dir(), '__tcpdf_dec_in_');
        $tmpOut = \tempnam(\sys_get_temp_dir(), '__tcpdf_dec_out_');

        if ($tmpIn === false || $tmpOut === false) {
            return null;
        }

        if (\file_put_contents($tmpIn, $smime) === false) {
            return null;
        }

        \set_error_handler(static fn(): bool => true);
        try {
            $decOk = \openssl_pkcs7_decrypt($tmpIn, $tmpOut, $certPem, $certPem);
        } finally {
            \restore_error_handler();
        }

        $result = $decOk ? \file_get_contents($tmpOut) : null;

        \unlink($tmpIn);
        \unlink($tmpOut);

        return $result === false ? null : $result;
    }

    /**
     * Derive the document encryption key from the recovered seed.
     *
     * Mirrors generatePublicEncryptionKey(): the key is SHA-256 (modes ≥ 3) or
     * SHA-1 (modes 0–2) of the seed concatenated with all Recipients' raw bytes.
     *
     * @param string $seed 20-byte random seed extracted from the recipient envelope.
     */
    protected function derivePublicKey(string $seed): void
    {
        $keybytelen = (int) ($this->encryptdata['Length'] / 8);
        $recipientBytes = '';

        foreach ($this->encryptdata['Recipients'] as $hexRecipient) {
            $binary = \hex2bin($hexRecipient);
            if ($binary !== false) {
                $recipientBytes .= $binary;
            }
        }

        if ($this->encryptdata['mode'] >= 3) {
            $this->encryptdata['key'] = \substr(\hash('sha256', $seed . $recipientBytes, true), 0, $keybytelen);

            return;
        }

        $this->encryptdata['key'] = \substr(\sha1($seed . $recipientBytes, true), 0, $keybytelen);
    }
}
