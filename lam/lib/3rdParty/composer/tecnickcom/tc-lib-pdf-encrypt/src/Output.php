<?php

declare(strict_types=1);

/**
 * Output.php
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

/**
 * Com\Tecnick\Pdf\Encrypt\Output
 *
 * PHP class for output encrypt PDF object
 *
 * @since     2008-01-02
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 *
 * @phpstan-type TEncryptData array{
 *     'CF': array{
 *         'AuthEvent': string,
 *         'CFM': string,
 *         'EncryptMetadata': bool,
 *         'Length': int,
 *     },
 *     'EFF': string,
 *     'EncryptMetadata': bool,
 *     'Filter': string,
 *     'Length': int,
 *     'O': string,
 *     'OE': string,
 *     'OKS': string,
 *     'OVS': string,
 *     'P': int,
 *     'Recipients': array<string>,
 *     'StmF': string,
 *     'StrF': string,
 *     'SubFilter': string,
 *     'U': string,
 *     'UE': string,
 *     'UKS': string,
 *     'UVS': string,
 *     'V': int,
 *     'encrypted': bool,
 *     'fileid': string,
 *     'key': string,
 *     'mode': int,
 *     'objid': int,
 *     'owner_password': string,
 *     'perms': string,
 *     'protection': int,
 *     'pubkey': bool,
 *     'pubkeys'?: array{array{'c':string, 'p':array<string>}},
 *     'user_password': string,
 *     }
 */
abstract class Output
{
    /**
     * Encryption data
     *
     * @var TEncryptData
     */
    protected array $encryptdata = [
        'CF' => [
            'AuthEvent' => '',
            'CFM' => '',
            'EncryptMetadata' => true,
            'Length' => 0,
        ],
        'EFF' => '',
        'EncryptMetadata' => true,
        'Filter' => '',
        'Length' => 0,
        'O' => '',
        'OE' => '',
        'OKS' => '',
        'OVS' => '',
        'P' => 0,
        'Recipients' => [],
        'StmF' => '',
        'StrF' => '',
        'SubFilter' => '',
        'U' => '',
        'UE' => '',
        'UKS' => '',
        'UVS' => '',
        'V' => 0,
        'encrypted' => false,
        'fileid' => '',
        'key' => '',
        'mode' => 0,
        'objid' => 0,
        'owner_password' => '',
        'perms' => '',
        'protection' => 0,
        'pubkey' => false,
        // 'pubkeys' => [],
        'user_password' => '',
    ];

    /**
     * Escape a string: add "\" before "\", "(" and ")".
     *
     * @param string $str String to escape.
     */
    public function escapeString(string $str): string
    {
        return \strtr($str, [
            ')' => '\\)',
            '(' => '\\(',
            '\\' => '\\\\',
            \chr(13) => '\r',
        ]);
    }

    /**
     * Get the PDF encryption block
     *
     * @param int $pon Current PDF object number
     */
    public function getPdfEncryptionObj(int &$pon): string
    {
        $this->setMissingValues();
        $this->encryptdata['objid'] = ++$pon;
        $out =
            $this->encryptdata['objid']
            . ' 0 obj'
            . "\n"
            . '<<'
            . "\n"
            . '/Filter /'
            . $this->encryptdata['Filter']
            . "\n";
        if ($this->encryptdata['SubFilter'] !== '') {
            $out .= '/SubFilter /' . $this->encryptdata['SubFilter'] . "\n";
        }

        // V is a code specifying the algorithm to be used in encrypting and decrypting the document
        $out .= '/V ' . $this->encryptdata['V'] . "\n";
        // The length of the encryption key, in bits. The value shall be a multiple of 8, in the range 40 to 256
        $out .= '/Length ' . $this->encryptdata['Length'] . "\n";
        if ($this->encryptdata['V'] >= 4) {
            $out .= $this->getCryptFilter();
            // The name of the crypt filter that shall be used by default when decrypting streams.
            $out .= '/StmF /' . $this->encryptdata['StmF'] . "\n";
            // The name of the crypt filter that shall be used when decrypting all strings in the document.
            $out .= '/StrF /' . $this->encryptdata['StrF'] . "\n";
            if ($this->encryptdata['EFF'] !== '') {
                // The name of the crypt filter that shall be used when encrypting embedded file streams
                // that do not have their own crypt filter specifier.
                $out .= '/EFF /' . $this->encryptdata['EFF'] . "\n";
            }
        }

        return $out . ($this->getAdditionalEncDic() . '>>' . "\n" . 'endobj' . "\n");
    }

    /**
     * Get Crypt Filter section
     *
     * A dictionary whose keys shall be crypt filter names
     * and whose values shall be the corresponding crypt filter dictionaries.
     */
    protected function getCryptFilter(): string
    {
        $out =
            '/CF <<'
            . "\n"
            . '/'
            . $this->encryptdata['StmF']
            . ' <<'
            . "\n"
            . '/Type /CryptFilter'
            . "\n"
            . '/CFM /'
            . $this->encryptdata['CF']['CFM']
            . "\n"; // The method used
        if ($this->encryptdata['pubkey']) {
            $out .= '/Recipients [';
            foreach ($this->encryptdata['Recipients'] as $rec) {
                $out .= ' <' . $rec . '>';
            }

            $out .=
                ' ]'
                . "\n"
                . '/EncryptMetadata '
                . $this->getBooleanString($this->encryptdata['CF']['EncryptMetadata'])
                . "\n";
        }

        // The event to be used to trigger the authorization
        // that is required to access encryption keys used by this filter.
        $out .= '/AuthEvent /' . $this->encryptdata['CF']['AuthEvent'] . "\n";
        if ($this->encryptdata['CF']['Length'] !== 0) {
            // The bit length of the encryption key.
            $out .= '/Length ' . $this->encryptdata['CF']['Length'] . "\n";
        }

        return $out . ('>>' . "\n" . '>>' . "\n");
    }

    /**
     * get additional encryption dictionary entries for the standard security handler
     */
    protected function getAdditionalEncDic(): string
    {
        $out = '';
        if ($this->encryptdata['pubkey']) {
            if ($this->encryptdata['V'] < 4 && $this->encryptdata['Recipients'] !== []) {
                $out .= ' /Recipients [';
                foreach ($this->encryptdata['Recipients'] as $rec) {
                    $out .= ' <' . $rec . '>';
                }

                $out .= ' ]' . "\n";
            }

            return $out;
        }

        $out .= '/R ';
        if ($this->encryptdata['V'] >= 5) { // AES-256 R5 or R6
            $revision = $this->encryptdata['V'] >= 6 ? '6' : '5';
            $out .=
                $revision
                . "\n"
                . '/OE ('
                . $this->escapeString($this->encryptdata['OE'])
                . ')'
                . "\n"
                . '/UE ('
                . $this->escapeString($this->encryptdata['UE'])
                . ')'
                . "\n"
                . '/Perms ('
                . $this->escapeString($this->encryptdata['perms'])
                . ')'
                . "\n";
        }

        if ($this->encryptdata['V'] === 4) { // AES-128
            $out .= '4' . "\n";
        }

        if ($this->encryptdata['V'] < 2) { // RC-40
            $out .= '2' . "\n";
        }

        if ($this->encryptdata['V'] >= 2 && $this->encryptdata['V'] < 4) { // RC-128
            $out .= '3' . "\n";
        }

        $out .=
            '/O ('
            . $this->escapeString($this->encryptdata['O'])
            . ')'
            . "\n"
            . '/U ('
            . $this->escapeString($this->encryptdata['U'])
            . ')'
            . "\n"
            . '/P '
            . $this->encryptdata['P']
            . "\n"
            . '/EncryptMetadata '
            . $this->getBooleanString($this->encryptdata['EncryptMetadata'])
            . "\n";

        return $out;
    }

    /**
     * Return a string representation of a boolean value
     *
     * @param bool $value Value to convert
     */
    protected function getBooleanString(bool $value): string
    {
        return $value ? 'true' : 'false';
    }

    protected function setMissingValues(): void
    {
        $this->encryptdata['CF']['EncryptMetadata'] = $this->encryptdata['EncryptMetadata'];
    }
}
