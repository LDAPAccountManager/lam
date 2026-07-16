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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * This file is part of tc-lib-pdf-encrypt software library.
 */

namespace Com\Tecnick\Pdf\Encrypt;

use Com\Tecnick\Pdf\Encrypt\Exception as EncException;

/**
 * Com\Tecnick\Pdf\Encrypt\Encrypt
 *
 * PHP class for encrypting data for PDF documents
 *
 * @since     2008-01-02
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * @phpstan-import-type TEncryptData from Output
 */
class Encrypt extends \Com\Tecnick\Pdf\Encrypt\Compute
{
    /**
     * Set PDF document protection (permission settings)
     *
     * NOTES: The protection against modification is for people who have the full Acrobat product.
     *        If you don't set any password, the document will open as usual.
     *        If you set a user password, the PDF viewer will ask for it before displaying the document.
     *        The master password, if different from the user one, can be used to get full access.
     *        Protecting a document requires to encrypt it, which requires long processign time and may cause timeouts.
     *
     * @param bool   $enabled     False if the encryption is disabled (i.e. the document is in PDF/A mode)
     * @param string $file_id     File ID
     * @param int    $mode        Encryption strength: 0 = RC4-40 (deprecated); 1 = RC4-128 (deprecated);
     *                            2 = AES-128; 3 = AES-256 R5; 4 = AES-256 R6 (PDF 2.0 / ISO 32000-2)
     * @param array<string>  $permissions The set of permissions (specify the ones you want to block):
     *                'owner' // When set permits change of encryption and enables all other permissions.
     *                // (inverted logic: cleared by default).
     *                'print' // Print the document.
     *                'modify' // Modify the contents of the document by operations other than those controlled
     *                // by 'fill-forms', 'extract' and 'assemble'.
     *                'copy' // Copy or otherwise extract text and graphics from the document.
     *                'annot-forms' // Add or modify text annotations, fill in interactive form fields, and,
     *                // if 'modify' is also set, create or modify interactive form fields
     *                // (including signature fields).
     *                'fill-forms' // Fill in existing interactive form fields (including signature fields),
     *                // even if 'annot-forms' is not specified.
     *                'extract' // Extract text and graphics (in support of accessibility to users with
     *                // disabilities or for other purposes).
     *                'assemble' // Assemble the document (insert, rotate, or delete pages and create bookmarks
     *                // or thumbnail images), even if 'modify' is not set.
     *                'print-high' // Print the document to a representation from which a faithful digital copy of the
     *                // PDF content could be generated. When this is not set, printing is limited to a
     *                // low-level representation of the appearance, possibly of degraded quality.
     *
     * @param string $user_pass         User password. Empty by default.
     * @param string $owner_pass        Owner password. If not specified, a random value is used.
     * @param ?array{array{'c':string, 'p':array<string>}}  $pubkeys
     *               Array of recipients containing public-key certificates ('c') and permissions ('p').
     *               For example:
     *               array(array('c' => 'file://../examples/data/cert/test.crt', 'p' => array('print')))
     *               To create self-signed certificate:
     *               openssl req -x509 -nodes -days 365000 -newkey rsa:1024 -keyout cert.pem -out cert.pem
     *               To export crt to p12: openssl pkcs12 -export -in cert.pem -out cert.p12
     *               To convert pfx certificate to pem: openssl pkcs12 -in cert.pfx -out cert.pem -nodes
     * @param bool   $encryptMetadata   When false, document metadata streams are not encrypted
     *                                  (adds /EncryptMetadata false to the encryption dictionary).
     *                                  Default is true (all streams, including metadata, are encrypted).
     * @param bool   $encryptEmbeddedFiles  When true (default), embedded file streams are encrypted
     *                                      using the same filter as other streams (/EFF entry is added
     *                                      for V >= 4). Set to false to leave embedded files unencrypted.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function __construct(
        bool $enabled = false,
        string $file_id = '',
        int $mode = 0,
        array $permissions = [
            'print',
            'modify',
            'copy',
            'annot-forms',
            'fill-forms',
            'extract',
            'assemble',
            'print-high',
        ],
        string $user_pass = '',
        string $owner_pass = '',
        ?array $pubkeys = null,
        bool $encryptMetadata = true,
        bool $encryptEmbeddedFiles = true,
    ) {
        if (!$enabled) {
            return;
        }

        $this->encryptdata['protection'] = $this->getUserPermissionCode($permissions, $mode);
        $this->setupEncryptionFilter($pubkeys, $mode);

        if ($owner_pass === '') {
            $owner_pass = \md5($this->encrypt('seed'));
        }

        $this->encryptdata['user_password'] = $user_pass;
        $this->encryptdata['owner_password'] = $owner_pass;
        $this->validateAndApplyMode($mode);
        $this->encryptdata['EncryptMetadata'] = $encryptMetadata;

        // Set EFF (embedded file filter) for V >= 4 when embedded file encryption is requested
        if ($encryptEmbeddedFiles && $this->encryptdata['V'] >= 4) {
            $this->encryptdata['EFF'] = $this->encryptdata['StmF'];
        }

        $this->encryptdata['encrypted'] = true;
        $this->encryptdata['fileid'] = $this->convertHexStringToString($file_id);
        $this->generateEncryptionKey();
    }

    /**
     * Configure Filter, StmF, StrF entries and handle mode promotion for public-key mode.
     *
     * When $pubkeys are provided and $mode is 0 (RC4-40), mode is silently promoted to 1 (RC4-128)
     * with a deprecation notice, because public-key security requires at least 128-bit keys.
     *
     * @param ?array{array{'c':string, 'p':array<string>}} $pubkeys Recipient public-key certificates.
     * @param int                                          $mode    Encryption mode (modified by reference).
     */
    protected function setupEncryptionFilter(?array $pubkeys, int &$mode): void
    {
        if ($pubkeys !== null && $pubkeys !== []) {
            $this->encryptdata['pubkeys'] = $pubkeys;
            if ($mode === 0) {
                // public-key Security requires at least 128 bit; upgrade silently but notify the caller
                \trigger_error(
                    'Public-key encryption requires at least RC4-128; mode upgraded from 0 to 1',
                    E_USER_DEPRECATED,
                );
                $mode = 1;
            }

            // Set Public-Key filter (available are: Entrust.PPKEF, Adobe.PPKLite, Adobe.PubSec)
            $this->encryptdata['pubkey'] = true;
            $this->encryptdata['Filter'] = 'Adobe.PubSec';
            $this->encryptdata['StmF'] = 'DefaultCryptFilter';
            $this->encryptdata['StrF'] = 'DefaultCryptFilter';

            return;
        }

        // standard mode (password mode)
        $this->encryptdata['pubkey'] = false;
        $this->encryptdata['Filter'] = 'Standard';
        $this->encryptdata['StmF'] = 'StdCF';
        $this->encryptdata['StrF'] = 'StdCF';
    }

    /**
     * Emit deprecation notices for broken modes and throw for invalid ones,
     * then merge ENCRYPT_SETTINGS for the resolved mode.
     *
     * @param int $mode Resolved encryption mode (0–4).
     *
     * @throws EncException When mode is outside the 0–4 range.
     */
    protected function validateAndApplyMode(int $mode): void
    {
        if ($mode === 0 || $mode === 1) {
            \trigger_error(
                'RC4 encryption (modes 0 and 1) is deprecated and cryptographically broken; use AES (mode 2 or 3)',
                E_USER_DEPRECATED,
            );
        }

        if ($mode < 0 || $mode > 4) {
            throw new EncException('unknown encryption mode: ' . $mode);
        }

        $this->encryptdata['mode'] = $mode;

        $settings = self::ENCRYPT_SETTINGS[$mode] ?? throw new EncException('unknown encryption mode: ' . $mode);
        $this->encryptdata['V'] = $settings['V'];
        $this->encryptdata['Length'] = $settings['Length'];
        $this->encryptdata['CF'] = [
            'CFM' => $settings['CF']['CFM'],
            'Length' => (int) ($settings['CF']['Length'] ?? 0),
            'AuthEvent' => $settings['CF']['AuthEvent'],
            'EncryptMetadata' => $this->encryptdata['EncryptMetadata'],
        ];
        // SubFilter values from ENCRYPT_SETTINGS are for public-key handlers.
        // Standard password encryption must omit SubFilter.
        $this->encryptdata['SubFilter'] = $this->encryptdata['pubkey'] ? $settings['SubFilter'] : '';
        $this->encryptdata['Recipients'] = $settings['Recipients'];

        // Keep SubFilter and Recipients keys set from ENCRYPT_SETTINGS to preserve
        // stable array shape and avoid undefined-key access in output generation.
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
     * Convert hexadecimal string to string.
     *
     * @param string $bstr Byte-string to convert.
     */
    public function convertHexStringToString(string $bstr): string
    {
        $str = ''; // string to be returned
        $bslength = \strlen($bstr);
        if (($bslength % 2) !== 0) {
            // padding
            $bstr .= '0';
            ++$bslength;
        }

        for ($idx = 0; $idx < $bslength; $idx += 2) {
            $str .= \chr((int) \hexdec($bstr[$idx] . $bstr[$idx + 1]) & 0xFF);
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
        $chars = \preg_split('//', $str, -1, PREG_SPLIT_NO_EMPTY);
        if ($chars === false) {
            return '';
        }

        $bstr = '';
        foreach ($chars as $char) {
            $bstr .= \sprintf('%02s', \dechex(\ord($char)));
        }

        return $bstr;
    }

    /**
     * Encode a name object.
     *
     * @param string $name Name object to encode.
     */
    public function encodeNameObject(string $name): string
    {
        $escname = '';
        $length = \strlen($name);
        for ($idx = 0; $idx < $length; ++$idx) {
            $chr = $name[$idx];
            if (\preg_match('/[0-9a-zA-Z#_=-]/', $chr) === 1) {
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
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function encryptString(string $str, int $objnum = 0): string
    {
        return $this->encrypt($this->encryptdata['mode'], $str, '', $objnum);
    }

    /**
     * Format a data string for meta information.
     *
     * @param string $str    Data string to escape.
     * @param int    $objnum Object ID.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function escapeDataString(string $str, int $objnum = 0): string
    {
        return '(' . $this->escapeString($this->encryptString($str, $objnum)) . ')';
    }

    /**
     * Returns a formatted date-time.
     *
     * @param int $time   UTC time measured in the number of seconds since the Unix Epoch (January 1 1970 00:00:00 GMT).
     * @param int $objnum Object ID.
     *
     * @return string escaped date string.
     *
     * @throws \Com\Tecnick\Pdf\Encrypt\Exception
     */
    public function getFormattedDate(?int $time = null, int $objnum = 0): string
    {
        if ($time === null) {
            $time = \time(); // get current UTC time
        }

        return $this->escapeDataString('D:' . \substr_replace(\date('YmdHisO', $time), "'", -2, 0) . "'", $objnum);
    }
}
