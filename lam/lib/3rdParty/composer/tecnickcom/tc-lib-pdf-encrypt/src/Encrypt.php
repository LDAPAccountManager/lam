<?php

declare(strict_types=1);

/**
 * Encrypt.php
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
 * Com\Tecnick\Pdf\Encrypt\Encrypt
 *
 * Encrypts data for PDF documents.
 *
 * @since     2008-01-02
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * @phpstan-import-type TEncryptData from Output
 */
class Encrypt extends \Com\Tecnick\Pdf\Encrypt\Compute
{
    /**
     * Set PDF document protection (permission settings).
     *
     * @param bool   $enabled     False if the encryption is disabled.
     * @param string $file_id     File ID as an even-length hexadecimal string; a random one is
     *                            generated when empty. It is the first element of the trailer /ID
     *                            array, from which revisions 2 to 4 derive the key.
     * @param int    $mode        Encryption strength: 0 = RC4-40 (deprecated); 1 = RC4-128 (deprecated);
     *                            2 = AES-128; 3 = AES-256 R5; 4 = AES-256 R6 (PDF 2.0 / ISO 32000-2).
     * @param array<string> $permissions The set of permissions to block: 'owner' (inverted logic:
     *                            when set, permits change of encryption), 'print', 'modify', 'copy',
     *                            'annot-forms', 'fill-forms', 'extract', 'assemble', 'print-high'.
     * @param string $user_pass   User password.
     * @param string $owner_pass  Owner password. A random value is used when empty.
     * @param ?list<array{'c':string, 'p'?:array<string>}>  $pubkeys
     *                            Recipients, each with a public-key certificate ('c') and the
     *                            permissions to block for it ('p'), for example:
     *                            [['c' => 'file://cert.pem', 'p' => ['print']]].
     * @param bool   $encryptMetadata   When false, adds /EncryptMetadata false to the encryption
     *                            dictionary and leaves metadata streams unencrypted. Requires mode
     *                            2, 3 or 4: for modes 0 and 1 the value is forced back to true with
     *                            an E_USER_WARNING.
     * @param bool   $encryptEmbeddedFiles  Selects the /EFF crypt filter for V 4 and V 5. True points
     *                            /EFF at the same filter as the other streams; false writes
     *                            /EFF /Identity, and the caller must then write embedded file streams
     *                            without calling encryptString() on them.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function __construct(
        bool $enabled = false,
        string $file_id = '',
        int $mode = 0,
        array $permissions = self::DEFAULTPERMS,
        #[\SensitiveParameter]
        string $user_pass = '',
        #[\SensitiveParameter]
        string $owner_pass = '',
        ?array $pubkeys = null,
        bool $encryptMetadata = true,
        bool $encryptEmbeddedFiles = true,
    ) {
        if (!$enabled) {
            return;
        }

        // Set before any seed is drawn: encrypt() returns its input unchanged while this is false.
        $this->encryptdata['encrypted'] = true;

        $this->setupEncryptionFilter($pubkeys, $mode);
        $this->warnAboutIgnoredStandardArguments($permissions, $user_pass, $owner_pass);
        $this->encryptdata['protection'] = $this->getUserPermissionCode($permissions, $mode);

        if ($owner_pass === '') {
            $owner_pass = \bin2hex(\substr($this->randomSeed(), 0, 16));
        }

        $this->encryptdata['user_password'] = $user_pass;
        $this->encryptdata['owner_password'] = $owner_pass;
        // Set before validateAndApplyMode(), which copies the flag into CF.
        $this->encryptdata['EncryptMetadata'] = $this->resolveEncryptMetadata($encryptMetadata, $mode);
        $this->validateAndApplyMode($mode);

        // ISO 32000-1 Table 21 defines EFF for V 4 and V 5 only. A reader that
        // finds no EFF entry falls back to StmF.
        if ($this->encryptdata['V'] >= 4) {
            $this->encryptdata['EFF'] = $encryptEmbeddedFiles ? $this->encryptdata['StmF'] : 'Identity';
        }

        if ($file_id === '') {
            $file_id = \bin2hex(\substr($this->randomSeed(), 0, 16));
        }

        $this->encryptdata['fileid'] = $this->convertHexStringToString($file_id);
        $this->generateEncryptionKey();
    }

    /**
     * Warn when password-mode arguments are supplied alongside recipient certificates.
     *
     * In public-key mode the permission bits travel inside each recipient envelope
     * and no /O, /U or /P entry is written, so these arguments have no effect.
     *
     * @param array<string> $permissions Requested permission set.
     * @param string        $user_pass   Requested user password.
     * @param string        $owner_pass  Requested owner password.
     */
    protected function warnAboutIgnoredStandardArguments(
        array $permissions,
        #[\SensitiveParameter]
        string $user_pass,
        #[\SensitiveParameter]
        string $owner_pass,
    ): void {
        if (!$this->encryptdata['pubkey']) {
            return;
        }

        if ($user_pass !== '' || $owner_pass !== '') {
            \trigger_error(
                'Public-key encryption ignores the user and owner passwords: '
                . 'access is granted by the recipient certificates',
                E_USER_WARNING,
            );
        }

        $requested = $permissions;
        $default = self::DEFAULTPERMS;
        \sort($requested);
        \sort($default);
        if ($requested !== $default) {
            \trigger_error(
                'Public-key encryption ignores the permissions argument: '
                . "set the permissions of each recipient in its own 'p' entry",
                E_USER_WARNING,
            );
        }
    }

    /**
     * Resolve the metadata encryption flag against the resolved mode.
     *
     * ISO 32000-1 Table 21 defines EncryptMetadata for V 4 and V 5 only, which are
     * modes 2 to 4 here. For modes 0 and 1 the flag is forced back to true.
     *
     * @param bool $encryptMetadata Requested value.
     * @param int  $mode            Resolved encryption mode.
     */
    protected function resolveEncryptMetadata(bool $encryptMetadata, int $mode): bool
    {
        if ($encryptMetadata || $mode >= 2) {
            return $encryptMetadata;
        }

        \trigger_error(
            'Unencrypted metadata requires AES (mode 2, 3 or 4); the request is ignored for RC4 modes 0 and 1',
            E_USER_WARNING,
        );

        return true;
    }

    /**
     * Configure the Filter, StmF and StrF entries and handle mode promotion for public-key mode.
     *
     * When $pubkeys are provided and $mode is 0 (RC4-40), the mode is promoted to 1 (RC4-128)
     * with a deprecation notice: public-key security requires at least 128-bit keys.
     *
     * @param ?list<array{'c':string, 'p'?:array<string>}> $pubkeys Recipient public-key certificates.
     * @param int                                          $mode    Encryption mode (modified by reference).
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When a recipient entry is malformed.
     */
    protected function setupEncryptionFilter(?array $pubkeys, int &$mode): void
    {
        if ($pubkeys !== null && $pubkeys !== []) {
            $this->validateRecipients($pubkeys);
            $this->encryptdata['pubkeys'] = $pubkeys;
            if ($mode === 0) {
                \trigger_error(
                    'Public-key encryption requires at least RC4-128; mode upgraded from 0 to 1',
                    E_USER_DEPRECATED,
                );
                $mode = 1;
            }

            // Public-key filter (the handlers are Entrust.PPKEF, Adobe.PPKLite and Adobe.PubSec).
            $this->encryptdata['pubkey'] = true;
            $this->encryptdata['Filter'] = 'Adobe.PubSec';
            $this->encryptdata['StmF'] = 'DefaultCryptFilter';
            $this->encryptdata['StrF'] = 'DefaultCryptFilter';

            return;
        }

        // Standard (password) mode.
        $this->encryptdata['pubkey'] = false;
        $this->encryptdata['Filter'] = 'Standard';
        $this->encryptdata['StmF'] = 'StdCF';
        $this->encryptdata['StrF'] = 'StdCF';
    }

    /**
     * Check the shape of every recipient entry before any of it is used.
     *
     * @param array<mixed> $pubkeys Recipient list as supplied by the caller.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When an entry is malformed.
     */
    protected function validateRecipients(array $pubkeys): void
    {
        $malformed = \array_filter($pubkeys, static fn(mixed $pubkey): bool => self::recipientFault($pubkey) !== '');
        if ($malformed === []) {
            return;
        }

        $idx = (string) \array_key_first($malformed);
        throw new EncException('recipient ' . $idx . ': ' . self::recipientFault(\reset($malformed)));
    }

    /**
     * Describe what is wrong with one recipient entry, or '' when it is valid.
     *
     * @param mixed $pubkey Entry as supplied by the caller.
     */
    protected static function recipientFault(mixed $pubkey): string
    {
        if (!\is_array($pubkey)) {
            return 'each recipient must be an array';
        }

        if (!\array_key_exists('c', $pubkey) || !\is_string($pubkey['c']) || $pubkey['c'] === '') {
            return "the 'c' entry must be a non-empty certificate path";
        }

        if (!\array_key_exists('p', $pubkey)) {
            return '';
        }

        if (!\is_array($pubkey['p'])) {
            return "the 'p' entry must be an array of permission names";
        }

        if (\array_filter($pubkey['p'], static fn(mixed $item): bool => !\is_string($item)) !== []) {
            return "every 'p' entry must be a permission name";
        }

        return '';
    }

    /**
     * Emit deprecation notices for broken modes and throw for invalid ones,
     * then merge ENCRYPT_SETTINGS for the resolved mode.
     *
     * @param int $mode Resolved encryption mode (0 to 4).
     *
     * @throws EncException When mode is outside the 0 to 4 range.
     */
    protected function validateAndApplyMode(int $mode): void
    {
        if ($mode === 0 || $mode === 1) {
            \trigger_error(
                'RC4 encryption (modes 0 and 1) is deprecated and cryptographically broken; use AES (mode 2, 3, or 4)',
                E_USER_DEPRECATED,
            );
        }

        if ($mode < 0 || $mode > 4) {
            throw new EncException('unknown encryption mode: ' . $mode);
        }

        $this->encryptdata['mode'] = $mode;

        $settings = self::ENCRYPT_SETTINGS[$mode] ?? throw new EncException('unknown encryption mode: ' . $mode);
        $this->encryptdata['V'] = $settings['V'];
        $this->encryptdata['R'] = $settings['R'];
        $this->encryptdata['Length'] = $settings['Length'];
        $this->encryptdata['CF'] = [
            'CFM' => $settings['CF']['CFM'],
            'Length' => (int) ($settings['CF']['Length'] ?? 0),
            'AuthEvent' => $settings['CF']['AuthEvent'],
            'EncryptMetadata' => $this->encryptdata['EncryptMetadata'],
        ];
        // SubFilter applies to the public-key handlers only.
        $this->encryptdata['SubFilter'] = $this->encryptdata['pubkey'] ? $settings['SubFilter'] : '';
        $this->encryptdata['Recipients'] = $settings['Recipients'];
    }

    /**
     * Get the encryption data array.
     *
     * @return TEncryptData
     */
    public function getEncryptionData(): array
    {
        return $this->encryptdata;
    }

    /**
     * Return the file ID as a hexadecimal string.
     *
     * The document must carry this value as the first element of the trailer /ID
     * array: revisions 2 to 4 derive the encryption key from it.
     */
    public function getFileId(): string
    {
        return \bin2hex($this->encryptdata['fileid']);
    }

    /**
     * Convert hexadecimal string to string.
     *
     * @param string $bstr Byte-string to convert.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the input is not an even-length hexadecimal string.
     */
    public function convertHexStringToString(string $bstr): string
    {
        if ($bstr !== '' && !\ctype_xdigit($bstr)) {
            throw new EncException('not a hexadecimal string: ' . $bstr);
        }

        if ((\strlen($bstr) % 2) !== 0) {
            throw new EncException('hexadecimal string with an odd number of digits: ' . $bstr);
        }

        $str = \hex2bin($bstr);
        if ($str === false) {
            throw new EncException('unable to decode the hexadecimal string: ' . $bstr);
        }

        return $str;
    }

    /**
     * Convert string to hexadecimal string (byte string).
     *
     * @param string $str String to convert.
     */
    public function convertStringToHexString(string $str): string
    {
        return \bin2hex($str);
    }

    /**
     * Encode a name object.
     *
     * Every character outside [0-9a-zA-Z_=-], including the NUMBER SIGN itself,
     * is written as a two-digit #XX escape (ISO 32000-1 section 7.3.5).
     *
     * @param string $name Name object to encode.
     */
    public function encodeNameObject(string $name): string
    {
        $escname = '';
        $length = \strlen($name);
        for ($idx = 0; $idx < $length; ++$idx) {
            $chr = $name[$idx];
            if (\preg_match('/[0-9a-zA-Z_=-]/', $chr) === 1) {
                $escname .= $chr;
                continue;
            }

            $escname .= \sprintf('#%02X', \ord($chr));
        }

        return $escname;
    }

    /**
     * Encrypt a string.
     *
     * @param string $str    String to encrypt.
     * @param int    $objnum Object ID.
     * @param int    $gennum Object generation number.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function encryptString(string $str, int $objnum = 0, int $gennum = 0): string
    {
        return $this->encrypt($this->encryptdata['mode'], $str, '', $objnum, $gennum);
    }

    /**
     * Format a data string for meta information.
     *
     * @param string $str    Data string to escape.
     * @param int    $objnum Object ID.
     * @param int    $gennum Object generation number.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function escapeDataString(string $str, int $objnum = 0, int $gennum = 0): string
    {
        return '(' . $this->escapeString($this->encryptString($str, $objnum, $gennum)) . ')';
    }

    /**
     * Returns a formatted date-time.
     *
     * The instant is rendered in UTC, not in the ambient date.timezone.
     *
     * @param int $time   UTC time measured in the number of seconds since the Unix Epoch (January 1 1970 00:00:00 GMT).
     * @param int $objnum Object ID.
     * @param int $gennum Object generation number.
     *
     * @return string escaped date string.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function getFormattedDate(?int $time = null, int $objnum = 0, int $gennum = 0): string
    {
        if ($time === null) {
            $time = \time(); // get current UTC time
        }

        $date = (new \DateTimeImmutable('@' . $time))->format('YmdHisO');

        return $this->escapeDataString('D:' . \substr_replace($date, "'", -2, 0) . "'", $objnum, $gennum);
    }
}
