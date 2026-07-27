<?php

declare(strict_types=1);

/**
 * Data.php
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

/**
 * Com\Tecnick\Pdf\Encrypt\Data
 *
 * Encrypt common data
 *
 * @since     2008-01-02
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
abstract class Data extends \Com\Tecnick\Pdf\Encrypt\Output
{
    /**
     * Encryption padding string.
     *
     * @var string
     */
    protected const ENCPAD =
        "\x28\xBF\x4E\x5E\x4E\x75\x8A\x41\x64\x00"
            . "\x4E\x56\xFF\xFA\x01\x08\x2E\x2E\x00\xB6"
            . "\xD0\x68\x3E\x80\x2F\x0C\xA9\xFE\x64\x53"
            . "\x69\x7A";

    /**
     * Map permission modes and bits.
     *
     * @var array<string, int>
     */
    protected const PERMBITS = [
        // bit 2 -- inverted logic: cleared by default
        // When set permits change of encryption and enables all other permissions.
        'owner' => 2,

        // bit 3
        // Print the document.
        'print' => 4,

        // bit 4
        // Modify the contents of the document by operations other than those controlled
        // by 'fill-forms', 'extract' and 'assemble'.
        'modify' => 8,

        // bit 5
        // Copy or otherwise extract text and graphics from the document.
        'copy' => 16,

        // bit 6
        // Add or modify text annotations, fill in interactive form fields, and,
        // if 'modify' is also set, create or modify interactive form fields
        // (including signature fields).
        'annot-forms' => 32,

        // bit 9
        // Fill in existing interactive form fields (including signature fields),
        // even if 'annot-forms' is not specified.
        'fill-forms' => 256,

        // bit 10
        // Extract text and graphics (in support of accessibility to users with
        // disabilities or for other purposes).
        'extract' => 512,

        // bit 11
        // Assemble the document (insert, rotate, or delete pages and create bookmarks
        // or thumbnail images), even if 'modify' is not set.
        'assemble' => 1024,

        // bit 12
        // Print the document to a representation from which a faithful digital copy of the
        // PDF content could be generated. When this is not set, printing is limited to a
        // low-level representation of the appearance, possibly of degraded quality.
        'print-high' => 2048,
    ];

    /**
     * Encryption settings.
     *
     * @var array<int, array{
     *          'V': int,
     *          'Length': int,
     *          'CF': array{
     *              'CFM': string,
     *              'Length'?: int,
     *              'AuthEvent': string,
     *          },
     *       'SubFilter': string,
     *       'Recipients': array<string, string>,
     *      }>
     */
    protected const ENCRYPT_SETTINGS = [
        0 => [
            // RC4 40 bit
            'V' => 1,
            'Length' => 40,
            'CF' => [
                'CFM' => 'V2',
                'AuthEvent' => 'DocOpen',
            ],
            'SubFilter' => '',
            'Recipients' => [],
        ],
        1 => [
            // RC4 128 bit
            'V' => 2,
            'Length' => 128,
            'CF' => [
                'CFM' => 'V2',
                'AuthEvent' => 'DocOpen',
            ],
            'SubFilter' => 'adbe.pkcs7.s4',
            'Recipients' => [],
        ],
        2 => [
            // AES 128 bit
            'V' => 4,
            'Length' => 128,
            'CF' => [
                'CFM' => 'AESV2',
                'Length' => 16,
                'AuthEvent' => 'DocOpen',
            ],
            'SubFilter' => 'adbe.pkcs7.s5',
            'Recipients' => [],
        ],
        3 => [
            // AES 256 bit (R5, Acrobat 9 / PDF 1.7 extension)
            'V' => 5,
            'Length' => 256,
            'CF' => [
                'CFM' => 'AESV3',
                'Length' => 32,
                'AuthEvent' => 'DocOpen',
            ],
            'SubFilter' => 'adbe.pkcs7.s5',
            'Recipients' => [],
        ],
        4 => [
            // AES 256 bit R6 (PDF 2.0 / ISO 32000-2)
            'V' => 6,
            'Length' => 256,
            'CF' => [
                'CFM' => 'AESV3',
                'Length' => 32,
                'AuthEvent' => 'DocOpen',
            ],
            'SubFilter' => 'adbe.pkcs7.s5',
            'Recipients' => [],
        ],
    ];

    /**
     * Define a list of available encrypt encoders.
     *
     * @var array<int|string, string>
     */
    protected const ENCMAP = [
        0 => 'RCFourFive', // RC4-40
        1 => 'RCFourSixteen', // RC4-128
        2 => 'AESSixteen', // AES-128
        3 => 'AESThirtytwo', // AES-256 (R5)
        4 => 'AESThirtytwo', // AES-256 (R6)
        'RC4' => 'RCFour', // RC4-40
        'RC4-40' => 'RCFourFive', // RC4-40
        'RC4-128' => 'RCFourSixteen', // RC4-128
        'AES' => 'AES', // AES-256
        'AES-128' => 'AESSixteen', // AES-128
        'AES-256' => 'AESThirtytwo', // AES-256
        'AESnopad' => 'AESnopad', // AES - no padding
        'MD5-16' => 'MDFiveSixteen', // MD5-16
        'seed' => 'Seed', // Random seed string
    ];
}
