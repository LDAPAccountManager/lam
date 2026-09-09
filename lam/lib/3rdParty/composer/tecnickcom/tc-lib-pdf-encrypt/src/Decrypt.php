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

use Com\Tecnick\Pdf\Encrypt\Exception as EncException;
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
 *       $plaintext = $dec->decryptString($ciphertext, $objnum, $gennum);
 *   }
 *
 * A dictionary read out of an existing document is better passed to
 * fromEncryptionDictionary(), which derives the mode from /V, /R and /CFM for
 * the standard handler, and from /V and /CFM for the public-key one.
 *
 * After successful authentication the derived key is stored internally and:
 *   - decryptString()           decrypts PDF string/stream objects.
 *   - getObjectKey()            returns the per-object key for RC4 and AES-128 streams.
 *   - getDocumentKey()          returns the raw 32-byte (or shorter) file key.
 *   - getAuthenticatedRole()    reports which credential authenticated.
 *   - getRecipientPermissions() returns the permission bits of the matching
 *                               recipient in public-key mode.
 *
 * Limitations:
 *   - Passwords are used as supplied. ISO 32000-2 section 7.6.4.3.3 calls for
 *     SASLprep (RFC 4013) on R5/R6 passwords and ISO 32000-1 expects
 *     PDFDocEncoding for R2 to R4, neither of which is applied here, so
 *     non-ASCII passwords may not match other implementations.
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
 *     'mode': int,
 *     'O': string,
 *     'U': string,
 *     'P': int,
 *     'fileid': string,
 *     'Length'?: int,
 *     'OE'?: string,
 *     'UE'?: string,
 *     'perms'?: string,
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
     * Maximum password length in bytes for R5 and R6 (ISO 32000-2 section 7.6.4.3.3).
     *
     * @var int
     */
    protected const MAXPASSLEN = 127;

    /**
     * /Filter values of the public-key security handlers (ISO 32000-1 section 7.6.5).
     *
     * @var array<string>
     */
    protected const PUBKEYFILTERS = ['Adobe.PubSec', 'Entrust.PPKEF', 'Adobe.PPKLite'];

    /**
     * Role granted by the last successful authenticate() call: 'user', 'owner',
     * 'recipient' or null.
     */
    protected ?string $authrole = null;

    /**
     * Permission bits of the recipient that authenticated in public-key mode.
     */
    protected ?int $recipientPermissions = null;

    /**
     * Initialise the decryptor from an encryption dictionary.
     *
     * Accepts the array returned by Encrypt::getEncryptionData() or any array
     * that satisfies the TDecryptInput shape. Fields not present in the input
     * are filled with the defaults defined in Output::$encryptdata.
     *
     * Every required entry is checked for presence and type before it is used.
     *
     * @param TDecryptInput $input Encryption dictionary fields.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the dictionary is malformed.
     */
    public function __construct(array $input)
    {
        $this->encryptdata['pubkey'] = $this->optionalBool($input, 'pubkey', $this->encryptdata['pubkey']);
        $this->encryptdata['mode'] = $this->requireInt($input, 'mode');
        $this->encryptdata['V'] = $this->requireInt($input, 'V');
        $this->encryptdata['R'] = $this->resolveRevision($input);
        $this->encryptdata['Length'] = $this->resolveKeyLength($input);
        $this->encryptdata['EncryptMetadata'] = $this->optionalBool(
            $input,
            'EncryptMetadata',
            $this->encryptdata['EncryptMetadata'],
        );

        if ($this->encryptdata['pubkey']) {
            // Public-key dictionaries carry Recipients instead of O, U, P and the file ID.
            $this->encryptdata['O'] = $this->optionalString($input, 'O');
            $this->encryptdata['U'] = $this->optionalString($input, 'U');
            $this->encryptdata['fileid'] = $this->optionalString($input, 'fileid');
            $this->encryptdata['P'] = $this->optionalInt($input, 'P');
        } else {
            $this->encryptdata['O'] = $this->requireString($input, 'O');
            $this->encryptdata['U'] = $this->requireString($input, 'U');
            $this->encryptdata['fileid'] = $this->requireString($input, 'fileid');
            $this->encryptdata['P'] = $this->requireInt($input, 'P');
        }

        if (\array_key_exists('Recipients', $input)) {
            $this->encryptdata['Recipients'] = $this->requireHexStringList($input, 'Recipients');
        }

        $this->encryptdata['OE'] = $this->optionalString($input, 'OE');
        $this->encryptdata['UE'] = $this->optionalString($input, 'UE');
        $this->encryptdata['perms'] = $this->optionalString($input, 'perms');

        // Ensure encrypt()-based primitives (RC4, MD5-16) are active for key derivation.
        $this->encryptdata['encrypted'] = true;
        // Clear the key: it must be recovered by a successful authenticate() call.
        $this->encryptdata['key'] = '';

        $this->validateInput();
    }

    /**
     * Build a decryptor from the entries of a PDF encryption dictionary.
     *
     * Derives this library's mode index from /V, /R and the crypt filter method,
     * and accepts /Perms under its PDF name.
     *
     * The public-key handler is recognised from /Filter /Adobe.PubSec or from the
     * presence of /Recipients, and needs no /R: that entry belongs to the
     * standard handler.
     *
     * 'fileid' is not a dictionary entry: it is the first element of the trailer
     * /ID array, from which revisions 2 to 4 derive the key, so the caller has to
     * add it to $dict.
     *
     * @param array<string, mixed> $dict Encryption dictionary entries under their PDF names, plus
     *                                   'fileid'. /R is required for the standard handler; /CFM is
     *                                   required for R 4.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the revision is not supported.
     */
    public static function fromEncryptionDictionary(array $dict): self
    {
        $input = $dict;
        if (\array_key_exists('Perms', $input)) {
            $input['perms'] = $input['Perms'];
        }

        $pubkey = self::isPublicKeyDictionary($dict);
        $input['pubkey'] = $pubkey;
        $input['mode'] = $pubkey ? self::modeFromVersion($dict) : self::modeFromRevision($dict);
        unset($input['CFM'], $input['Perms']);
        if ($pubkey) {
            unset($input['R']);
        }

        /** @var TDecryptInput $input */
        return new self($input);
    }

    /**
     * Report whether the dictionary belongs to the public-key security handler.
     *
     * @param array<string, mixed> $dict Encryption dictionary entries, PDF names.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When /Filter names a handler this library cannot read.
     */
    protected static function isPublicKeyDictionary(array $dict): bool
    {
        if (\array_key_exists('pubkey', $dict)) {
            return $dict['pubkey'] === true;
        }

        $filter = \array_key_exists('Filter', $dict) && \is_string($dict['Filter']) ? $dict['Filter'] : '';
        if ($filter === '') {
            return \array_key_exists('Recipients', $dict);
        }

        if ($filter === 'Standard') {
            return false;
        }

        // Only the standard and the public-key handlers are supported.
        if (!\in_array($filter, self::PUBKEYFILTERS, strict: true)) {
            throw new EncException('unsupported security handler: ' . $filter);
        }

        return true;
    }

    /**
     * Map /V and the crypt filter method to this library's mode index, without /R.
     *
     * Public-key dictionaries carry no revision. V 5 is reported as mode 4
     * (AES-256 R6), which shares its key derivation and its stream cipher with
     * mode 3: the two differ only in the password hash, which the public-key
     * handler does not use.
     *
     * @param array<string, mixed> $dict Encryption dictionary entries, PDF names.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected static function modeFromVersion(array $dict): int
    {
        if (!\array_key_exists('V', $dict) || !\is_int($dict['V'])) {
            throw new EncException('the V entry is required and must be an integer');
        }

        $version = $dict['V'];
        $cfm = \array_key_exists('CFM', $dict) && \is_string($dict['CFM']) ? $dict['CFM'] : '';

        return match (true) {
            $version === 1 => 0,
            $version === 4 && $cfm === 'AESV2' => 2,
            $version === 2, $version === 4 && ($cfm === 'V2' || $cfm === '') => 1,
            $version === 5 => 4,
            default => throw new EncException(
                'unsupported public-key encryption version ' . $version . ' with crypt filter method "' . $cfm . '"',
            ),
        };
    }

    /**
     * Map the /R revision and the crypt filter method to this library's mode index.
     *
     * R 4 is ambiguous on its own: it carries AES-128 with /CFM /AESV2 and
     * RC4-128 with /CFM /V2.
     *
     * @param array<string, mixed> $dict Encryption dictionary entries, PDF names.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected static function modeFromRevision(array $dict): int
    {
        if (!\array_key_exists('R', $dict) || !\is_int($dict['R'])) {
            throw new EncException('the R entry is required and must be an integer');
        }

        $revision = $dict['R'];
        $cfm = \array_key_exists('CFM', $dict) && \is_string($dict['CFM']) ? $dict['CFM'] : '';

        return match (true) {
            $revision === 2 => 0,
            $revision === 4 && $cfm === 'AESV2' => 2,
            $revision === 3, $revision === 4 && ($cfm === 'V2' || $cfm === '') => 1,
            $revision === 5 => 3,
            $revision === 6 => 4,
            default => throw new EncException(
                'unsupported encryption revision ' . $revision . ' with crypt filter method "' . $cfm . '"',
            ),
        };
    }

    /**
     * Read a required integer entry.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is missing or not an integer.
     */
    protected function requireInt(array $input, string $name): int
    {
        if (!\array_key_exists($name, $input) || $input[$name] === null) {
            throw new EncException('missing required entry: ' . $name);
        }

        if (\is_int($input[$name])) {
            return $input[$name];
        }

        if (\is_string($input[$name]) && \preg_match('/^[+-]?\d+$/', $input[$name]) === 1) {
            return (int) $input[$name];
        }

        throw new EncException('the ' . $name . ' entry must be an integer');
    }

    /**
     * Read a required string entry.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is missing or not a string.
     */
    protected function requireString(array $input, string $name): string
    {
        if (!\array_key_exists($name, $input) || $input[$name] === null) {
            throw new EncException('missing required entry: ' . $name);
        }

        if (!\is_string($input[$name])) {
            throw new EncException('the ' . $name . ' entry must be a string');
        }

        return $input[$name];
    }

    /**
     * Read an optional integer entry, defaulting to 0.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is present but not an integer.
     */
    protected function optionalInt(array $input, string $name): int
    {
        return \array_key_exists($name, $input) ? $this->requireInt($input, $name) : 0;
    }

    /**
     * Read an optional string entry, defaulting to the empty string.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is present but not a string.
     */
    protected function optionalString(array $input, string $name): string
    {
        return \array_key_exists($name, $input) ? $this->requireString($input, $name) : '';
    }

    /**
     * Read an optional boolean entry.
     *
     * PDF booleans are keywords, so a parser may hand them over as the strings
     * 'true' and 'false'. Any other value is rejected.
     *
     * @param array<array-key, mixed> $input   Encryption dictionary fields.
     * @param string                  $name    Entry name.
     * @param bool                    $default Value used when the entry is absent or null.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is present but not a boolean.
     */
    protected function optionalBool(array $input, string $name, bool $default): bool
    {
        if (!\array_key_exists($name, $input) || $input[$name] === null) {
            return $default;
        }

        if (\is_bool($input[$name])) {
            return $input[$name];
        }

        if ($input[$name] === 'true' || $input[$name] === 'false') {
            return $input[$name] === 'true';
        }

        throw new EncException('the ' . $name . ' entry must be a boolean');
    }

    /**
     * Read an entry that must be a list of strings.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @return array<string>
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is not a list of strings.
     */
    protected function requireStringList(array $input, string $name): array
    {
        if (!\array_key_exists($name, $input) || !\is_array($input[$name])) {
            throw new EncException('the ' . $name . ' entry must be an array');
        }

        $entries = $input[$name];
        if (\array_filter($entries, static fn(mixed $item): bool => !\is_string($item)) !== []) {
            throw new EncException('every ' . $name . ' entry must be a string');
        }

        /** @var array<string> $entries */
        return \array_values($entries);
    }

    /**
     * Read an entry that must be a list of even-length hexadecimal strings.
     *
     * Algorithm 3 hashes the bytes of every entry, so an entry that cannot be
     * decoded changes the document key.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @return array<string>
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When an entry is not hexadecimal.
     */
    protected function requireHexStringList(array $input, string $name): array
    {
        $entries = $this->requireStringList($input, $name);
        foreach ($entries as $idx => $entry) {
            if ($entry === '' || (\strlen($entry) % 2) !== 0 || !\ctype_xdigit($entry)) {
                throw new EncException(
                    'the ' . $name . ' entry at index ' . $idx . ' is not an even-length hexadecimal string',
                );
            }
        }

        return $entries;
    }

    /**
     * Resolve the security handler revision, as tested by ISO 32000-1
     * Algorithm 2 step (f).
     *
     * Taken from the dictionary when the caller passed it, and derived from the
     * mode otherwise. Mode 1 covers both R3 (V 2) and R4 with /CFM /V2, which V
     * distinguishes.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is present but not an integer.
     */
    protected function resolveRevision(array $input): int
    {
        if (\array_key_exists('R', $input)) {
            return $this->requireInt($input, 'R');
        }

        return match ($this->encryptdata['mode']) {
            0 => 2,
            1 => $this->encryptdata['V'] >= 4 ? 4 : 3,
            2 => 4,
            3 => 5,
            default => 6,
        };
    }

    /**
     * Resolve the key length, applying the default the PDF format leaves implicit.
     *
     * V 5 always means a 256 bit key and V 4 a 128 bit one, so /Length is
     * redundant for both and a producer may omit it. Table 20 gives 40 bits as
     * the default below that.
     *
     * @param array<array-key, mixed> $input Encryption dictionary fields.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the entry is present but not an integer.
     */
    protected function resolveKeyLength(array $input): int
    {
        if (\array_key_exists('Length', $input)) {
            return $this->requireInt($input, 'Length');
        }

        return match (true) {
            $this->encryptdata['V'] >= 5 => 256,
            $this->encryptdata['V'] === 4 => 128,
            default => 40,
        };
    }

    /**
     * Check that the encryption dictionary is internally consistent.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When a required field is missing or malformed.
     */
    protected function validateInput(): void
    {
        $mode = $this->encryptdata['mode'];
        if ($mode < 0 || $mode > 4) {
            throw new EncException('unknown encryption mode: ' . $mode);
        }

        $length = $this->encryptdata['Length'];
        if ($length < 40 || $length > 256 || ($length % 8) !== 0) {
            throw new EncException('invalid key length: ' . $length);
        }

        $this->validateModeCoherence($mode, $length);

        if ($this->encryptdata['pubkey']) {
            // Public-key dictionaries carry Recipients instead of O, U, UE, OE and Perms.
            if ($this->encryptdata['Recipients'] === []) {
                throw new EncException('public-key mode requires a non-empty Recipients array');
            }

            return;
        }

        $minlen = $mode >= 3 ? 48 : 32;
        if (\strlen($this->encryptdata['O']) < $minlen) {
            throw new EncException('the O entry must be at least ' . $minlen . ' bytes');
        }

        if (\strlen($this->encryptdata['U']) < $minlen) {
            throw new EncException('the U entry must be at least ' . $minlen . ' bytes');
        }

        if ($mode < 3) {
            // Revisions 2 to 4 derive the key from the file ID.
            if ($this->encryptdata['fileid'] === '') {
                throw new EncException('the file ID is required for revisions 2 to 4');
            }

            return;
        }

        // R5/R6 password mode: the wrapped keys and the permission block are mandatory.
        if (\strlen($this->encryptdata['UE']) !== 32) {
            throw new EncException('the UE entry must be 32 bytes');
        }

        if (\strlen($this->encryptdata['OE']) !== 32) {
            throw new EncException('the OE entry must be 32 bytes');
        }

        if (\strlen($this->encryptdata['perms']) !== 16) {
            throw new EncException('the Perms entry must be 16 bytes');
        }
    }

    /**
     * Check that V and Length agree with the mode.
     *
     * @param int $mode   Resolved encryption mode.
     * @param int $length Resolved key length in bits.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the combination is impossible.
     */
    protected function validateModeCoherence(int $mode, int $length): void
    {
        $version = $this->encryptdata['V'];

        // Allowed V values, then the inclusive key length range, per mode.
        // Mode 1 covers both the V 2 dictionary of R3 and the V 4 one of R4 with
        // /CFM /V2, and R3 allows any multiple of 8 from 40 to 128 bits.
        [$versions, $minlen, $maxlen] = match ($mode) {
            0 => [[1], 40, 40],
            1 => [[2, 4], 40, 128],
            2 => [[4], 128, 128],
            default => [[5], 256, 256],
        };

        if (!\in_array($version, $versions, strict: true)) {
            throw new EncException(
                'mode ' . $mode . ' requires V ' . \implode(' or ', $versions) . ', got ' . $version,
            );
        }

        if ($length < $minlen || $length > $maxlen) {
            throw new EncException(
                'mode '
                . $mode
                . ' requires a key length between '
                . $minlen
                . ' and '
                . $maxlen
                . ' bits, got '
                . $length,
            );
        }
    }

    // -------------------------------------------------------------------------
    // Public API
    // -------------------------------------------------------------------------

    /**
     * Authenticate using a password and/or private key.
     *
     * Tries the supplied string as the owner password first, then as the user
     * password. For public-key mode, $privkeyPath must be the path to a PEM file
     * containing the recipient's certificate and private key; $password is
     * ignored in that case.
     *
     * On success the derived file-encryption key is stored internally and
     * getAuthenticatedRole() reports which password matched. For R5 and R6 the
     * password is truncated to 127 bytes and the Perms entry is verified against
     * the recovered key, so a document with tampered permission bits is rejected.
     *
     * @param string $password    UTF-8 password to test (ignored for pubkey mode).
     * @param string $privkeyPath Path to PEM file for public-key mode.
     * @param string $passphrase  Passphrase of the PEM private key, when it is encrypted.
     *
     * @return bool True when authentication succeeds.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function authenticate(
        #[\SensitiveParameter]
        string $password,
        string $privkeyPath = '',
        #[\SensitiveParameter]
        string $passphrase = '',
    ): bool {
        $this->authrole = null;
        $this->recipientPermissions = null;
        $this->encryptdata['key'] = '';

        if ($this->encryptdata['pubkey']) {
            return $this->authenticatePublicKey($privkeyPath, $passphrase);
        }

        if ($this->encryptdata['mode'] >= 3) {
            return $this->authenticatePasswordR5R6($password);
        }

        return $this->authenticatePasswordR24($password);
    }

    /**
     * Return the role granted by the last successful authenticate() call.
     *
     * In public-key mode the role is 'recipient': the applicable permissions are
     * the ones getRecipientPermissions() returns.
     *
     * @return string|null 'user', 'owner', 'recipient', or null when not authenticated.
     */
    public function getAuthenticatedRole(): ?string
    {
        return $this->authrole;
    }

    /**
     * Return the permission bits assigned to the recipient that authenticated.
     *
     * Public-key documents carry no /P entry: each recipient's permissions travel
     * inside its own PKCS#7 envelope. Returns null outside public-key mode and
     * before authentication.
     *
     * @return int|null Signed 32-bit permission value, or null when unavailable.
     */
    public function getRecipientPermissions(): ?int
    {
        return $this->recipientPermissions;
    }

    /**
     * Decrypt a PDF string or stream object.
     *
     * Must be called after a successful authenticate() call.
     *
     * For RC4 modes (0, 1) the operation is symmetric: the same method that
     * encrypts also decrypts. For AES modes (2, 3, 4) the first 16 bytes of
     * $data are the random IV and the remainder is the ciphertext; the PKCS#7
     * padding is stripped.
     *
     * @param string $data   Encrypted string/stream data.
     * @param int    $objnum PDF object number (used for per-object key derivation
     *                       in RC4 and AES-128 modes).
     * @param int    $gennum PDF object generation number.
     *
     * @return string Decrypted data.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When no key has been recovered,
     *         when the AES stream is malformed, or when decryption fails.
     */
    public function decryptString(string $data, int $objnum = 0, int $gennum = 0): string
    {
        if ($this->encryptdata['key'] === '') {
            throw new EncException('not authenticated: call authenticate() before decrypting');
        }

        $mode = $this->encryptdata['mode'];

        if ($mode < 2) {
            // RC4 is symmetric: the same encrypt() call decrypts.
            return $this->encrypt($mode, $data, '', $objnum, $gennum);
        }

        return $this->decryptAes($data, $objnum, $gennum);
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

    /**
     * Return the security handler revision the dictionary resolved to.
     *
     * Taken from /R when the dictionary carried it, derived from the mode and /V
     * otherwise.
     */
    public function getRevision(): int
    {
        return $this->encryptdata['R'];
    }

    /**
     * Not available on a decryptor: writing an encryption dictionary is the task
     * of Encrypt.
     *
     * @param int $pon Current PDF object number.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception Always.
     */
    public function getPdfEncryptionObj(int &$pon): string
    {
        throw new EncException('Decrypt cannot write an encryption dictionary: use Encrypt');
    }

    /**
     * Return the per-object key for RC4 and AES-128 streams.
     *
     * Must be called after a successful authenticate() call.
     *
     * @param int $objnum Object number.
     * @param int $gennum Object generation number.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When no key has been recovered,
     *         or when either number is out of range.
     */
    public function getObjectKey(int $objnum, int $gennum = 0): string
    {
        if ($this->encryptdata['key'] === '') {
            throw new EncException('not authenticated: call authenticate() before deriving object keys');
        }

        return parent::getObjectKey($objnum, $gennum);
    }

    // -------------------------------------------------------------------------
    // RC4 / R2 to R4 authentication (modes 0, 1, 2)
    // -------------------------------------------------------------------------

    /**
     * Authenticate a password for R2 to R4 (RC4-40, RC4-128, AES-128).
     *
     * The owner password is tried first: a document may use the same string for both.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticatePasswordR24(#[\SensitiveParameter] string $password): bool
    {
        if ($this->authenticateOwnerR24($password)) {
            $this->authrole = 'owner';
            return true;
        }

        if ($this->authenticateUserR24(\substr($password . self::ENCPAD, 0, 32))) {
            $this->authrole = 'user';
            return true;
        }

        return false;
    }

    /**
     * Authenticate $paddedPass as the user password for R2 to R4.
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
     * Authenticate $password as the owner password for R2 to R4.
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
     * Derive the file encryption key for R2 to R4 from a 32-byte padded user password.
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
            $paddedPass . $this->encryptdata['O'] . $permBytes . $this->encryptdata['fileid']
                . $this->getMetadataKeyExtension($this->encryptdata['R'], $this->encryptdata['EncryptMetadata']),
        );

        if ($this->encryptdata['mode'] > 0) {
            for ($idx = 0; $idx < 50; ++$idx) {
                $tmp = $this->encrypt('MD5-16', \substr($tmp, 0, $keybytelen));
            }
        }

        return \substr($tmp, 0, $keybytelen);
    }

    /**
     * Compare a freshly computed U value against the stored one for R2 to R4.
     *
     * R2 requires an exact 32-byte match; R3/R4 compare only the first 16 bytes.
     */
    protected function compareUserHashR24(string $computedU): bool
    {
        if ($this->encryptdata['mode'] === 0) {
            return \hash_equals($this->encryptdata['U'], $computedU);
        }

        return \hash_equals(\substr($this->encryptdata['U'], 0, 16), \substr($computedU, 0, 16));
    }

    /**
     * Derive the owner key from a 32-byte padded owner password for R2 to R4.
     *
     * Algorithm 3 step (c) re-hashes the full digest, unlike Algorithm 2 step (h)
     * which passes only the first n bytes.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function deriveOwnerKeyR24(string $paddedOwnerPass): string
    {
        $keybytelen = (int) ($this->encryptdata['Length'] / 8);
        $tmp = $this->encrypt('MD5-16', $paddedOwnerPass);

        if ($this->encryptdata['mode'] > 0) {
            for ($idx = 0; $idx < 50; ++$idx) {
                $tmp = $this->encrypt('MD5-16', $tmp);
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
    // AES-256 / R5 to R6 authentication (modes 3, 4)
    // -------------------------------------------------------------------------

    /**
     * Authenticate a password for R5 (mode 3) or R6 (mode 4).
     *
     * The owner password is tried first, as in authenticatePasswordR24().
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function authenticatePasswordR5R6(#[\SensitiveParameter] string $password): bool
    {
        // At most MAXPASSLEN bytes are hashed, as on the encryption side.
        $password = \substr($password, 0, self::MAXPASSLEN);

        if ($this->authenticateOwnerR5R6($password)) {
            $this->recoverKeyFromOwner($password);
            return $this->acceptRole('owner');
        }

        if ($this->authenticateUserR5R6($password)) {
            $this->recoverKeyFromUser($password);
            return $this->acceptRole('user');
        }

        return false;
    }

    /**
     * Accept a recovered key only when the Perms block confirms it.
     *
     * @param string $role Role that authenticated: 'user' or 'owner'.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function acceptRole(string $role): bool
    {
        if (!$this->verifyPerms()) {
            $this->encryptdata['key'] = '';
            return false;
        }

        $this->authrole = $role;
        return true;
    }

    /**
     * Verify the Perms entry against the recovered file key (Algorithm 13).
     *
     * For R5 and R6 the P value is not part of the key derivation: this block is
     * the only integrity protection on the permission bits.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function verifyPerms(): bool
    {
        $aesnopad = new AESnopad();
        // AES-256 ECB with a zero IV: for a single block this equals CBC with a zero IV.
        $plain = $aesnopad->decrypt($this->encryptdata['perms'], $this->encryptdata['key']);

        if (!\hash_equals('adb', \substr($plain, 9, 3))) {
            return false;
        }

        if (!\hash_equals($this->getEncPermissionsString($this->encryptdata['P']), \substr($plain, 0, 4))) {
            return false;
        }

        $metaflag = $plain[8];
        if ($metaflag !== 'T' && $metaflag !== 'F') {
            return false;
        }

        return ($metaflag === 'T') === $this->encryptdata['EncryptMetadata'];
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
        return \hash_equals($expected, $this->hashR5R6($password, $uvs));
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
        return \hash_equals($expected, $this->hashR5R6($password, $ovs, $userHash));
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
        $this->setDocumentKey($aesnopad->decrypt($this->encryptdata['UE'], $hashkey));
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
        $this->setDocumentKey($aesnopad->decrypt($this->encryptdata['OE'], $hashkey));
    }

    /**
     * Discard any pending OpenSSL error, so that a later message reports the
     * operation that failed rather than a stale entry.
     */
    protected function drainOpenSslErrors(): void
    {
        $guard = 0;
        while (\openssl_error_string() !== false && $guard < 64) {
            ++$guard;
        }
    }

    /**
     * Store a recovered AES-256 file key after checking that it is 32 bytes long.
     *
     * @param string $key Unwrapped file encryption key.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception When the key length is wrong.
     */
    protected function setDocumentKey(string $key): void
    {
        if (\strlen($key) !== 32) {
            throw new EncException('the recovered file key must be 32 bytes, got ' . \strlen($key));
        }

        $this->encryptdata['key'] = $key;
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
     * The ciphertext is prefixed with a 16-byte random IV (ISO 32000-1 section
     * 7.6.3). Data that is not one IV plus a whole number of blocks is rejected
     * as malformed.
     *
     * The AESV2 and AESV3 crypt filters apply PKCS#7 block padding, so the
     * decryption runs without OPENSSL_ZERO_PADDING and OpenSSL validates and
     * strips that padding. Invalid padding raises an exception.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    protected function decryptAes(string $data, int $objnum, int $gennum = 0): string
    {
        $ciphertextLen = \strlen($data) - AESnopad::BLOCKSIZE;
        if ($ciphertextLen <= 0 || ($ciphertextLen % AESnopad::BLOCKSIZE) !== 0) {
            throw new EncException('malformed AES stream: ' . \strlen($data) . ' bytes');
        }

        $ivect = \substr($data, 0, AESnopad::BLOCKSIZE);
        $ciphertext = \substr($data, AESnopad::BLOCKSIZE);
        $mode = $this->encryptdata['mode'];
        $key = $mode < 3 ? $this->getObjectKey($objnum, $gennum) : $this->encryptdata['key'];
        $cipher = $mode === 2 ? 'aes-128-cbc' : 'aes-256-cbc';

        $this->drainOpenSslErrors();

        $dec = \openssl_decrypt($ciphertext, $cipher, $key, OPENSSL_RAW_DATA, $ivect);
        if ($dec === false) {
            throw new EncException('decryption failed: ' . (string) \openssl_error_string());
        }

        return $dec;
    }

    // -------------------------------------------------------------------------
    // Public-key mode authentication
    // -------------------------------------------------------------------------

    /**
     * Authenticate using a recipient's PEM private-key file (public-key mode).
     *
     * Iterates over all Recipients entries, tries to decrypt each PKCS#7 envelope
     * with the supplied certificate/key, extracts the seed and the recipient's
     * permission bits, and derives the document encryption key.
     *
     * $privkeyPath is read with file_get_contents() and no scheme restriction:
     * it is trusted caller input, not a value taken from a document.
     *
     * @param string $privkeyPath Path to the recipient's PEM file (cert + key).
     *
     * @return bool True when a matching recipient is found and the key is derived.
     */
    protected function authenticatePublicKey(string $privkeyPath, #[\SensitiveParameter] string $passphrase = ''): bool
    {
        if ($privkeyPath === '') {
            return false;
        }

        // Read the file and let the failure decide, without a separate
        // readability check.
        \set_error_handler(static fn(): bool => true);
        try {
            $certPem = \file_get_contents($privkeyPath);
        } finally {
            \restore_error_handler();
        }

        if ($certPem === false) {
            return false;
        }

        $envelope = $this->findDecryptedRecipientEnvelope($certPem, $passphrase);
        if ($envelope === null) {
            return false;
        }

        // envelope = seed (20 bytes) + permissions (4 bytes, high-order byte first)
        $this->derivePublicKey(\substr($envelope, 0, 20));
        $this->recipientPermissions = $this->decodePubKeyPermissions(\substr($envelope, 20, 4));
        $this->authrole = 'recipient';
        return true;
    }

    /**
     * Decode the four permission bytes of a recipient envelope.
     *
     * They are stored high-order byte first, the opposite of the /P key
     * material, and are returned as the signed 32-bit integer /P uses.
     *
     * @param string $bytes 4 permission bytes from the envelope.
     */
    protected function decodePubKeyPermissions(string $bytes): int
    {
        $unpacked = \unpack('N', $bytes);
        $value = $unpacked === false ? 0 : (int) $unpacked[1];
        return ($value & 0x8000_0000) !== 0 ? $value - 0x1_0000_0000 : $value;
    }

    /**
     * Iterate over all Recipients and return the 24-byte envelope of the first
     * entry that can be decrypted with $certPem, or null on failure.
     *
     * @return string|null 24-byte envelope, or null when no matching recipient found.
     */
    protected function findDecryptedRecipientEnvelope(
        string $certPem,
        #[\SensitiveParameter]
        string $passphrase = '',
    ): ?string {
        foreach ($this->encryptdata['Recipients'] as $hexRecipient) {
            // The constructor guarantees every entry is even-length hexadecimal.
            $derData = (string) \hex2bin($hexRecipient);

            $envelope = $this->tryDecryptRecipient($derData, $certPem, $passphrase);
            if ($envelope !== null && \strlen($envelope) >= 24) {
                return \substr($envelope, 0, 24);
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
    protected function tryDecryptRecipient(
        string $derData,
        string $certPem,
        #[\SensitiveParameter]
        string $passphrase = '',
    ): ?string {
        $smime =
            "MIME-Version: 1.0\r\n"
            . 'Content-Type: application/pkcs7-mime;'
            . " smime-type=enveloped-data; name=\"smime.p7m\"\r\n"
            . "Content-Transfer-Encoding: base64\r\n\r\n"
            . \chunk_split(\base64_encode($derData));

        $tmpIn = \tempnam(\sys_get_temp_dir(), '__tcpdf_dec_in_');
        $tmpOut = \tempnam(\sys_get_temp_dir(), '__tcpdf_dec_out_');

        try {
            if ($tmpIn === false || $tmpOut === false) {
                return null;
            }

            if (\file_put_contents($tmpIn, $smime) === false) {
                return null;
            }

            $privkey = $passphrase === '' ? $certPem : [$certPem, $passphrase];

            \set_error_handler(static fn(): bool => true);
            try {
                $decOk = \openssl_pkcs7_decrypt($tmpIn, $tmpOut, $certPem, $privkey);
            } finally {
                \restore_error_handler();
            }

            $result = $decOk ? \file_get_contents($tmpOut) : null;
            return $result === false ? null : $result;
        } finally {
            // The input holds the envelope and the output the decrypted seed:
            // remove both on every exit path.
            if ($tmpIn !== false) {
                \unlink($tmpIn);
            }

            if ($tmpOut !== false) {
                \unlink($tmpOut);
            }
        }
    }

    /**
     * Derive the document encryption key from the recovered seed.
     *
     * Mirrors generatePublicEncryptionKey(): the key is SHA-256 (modes ≥ 3) or
     * SHA-1 (modes 0 to 2) of the seed concatenated with all Recipients' raw bytes.
     *
     * @param string $seed 20-byte random seed extracted from the recipient envelope.
     */
    protected function derivePublicKey(string $seed): void
    {
        $keybytelen = (int) ($this->encryptdata['Length'] / 8);
        $recipientBytes = '';

        // The constructor guarantees every entry is even-length hexadecimal.
        foreach ($this->encryptdata['Recipients'] as $hexRecipient) {
            $recipientBytes .= (string) \hex2bin($hexRecipient);
        }

        // Algorithm 3 step (a): the four metadata bytes close the hash input.
        $recipientBytes .= $this->getMetadataKeyExtension(
            $this->encryptdata['V'],
            $this->encryptdata['EncryptMetadata'],
        );

        if ($this->encryptdata['mode'] >= 3) {
            $this->encryptdata['key'] = \substr(\hash('sha256', $seed . $recipientBytes, true), 0, $keybytelen);

            return;
        }

        $this->encryptdata['key'] = \substr(\sha1($seed . $recipientBytes, true), 0, $keybytelen);
    }
}
