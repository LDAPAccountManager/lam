<?php

declare(strict_types=1);

/**
 * Seed.php
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

/**
 * Com\Tecnick\Pdf\Encrypt\Type\Seed
 *
 * generate random seed
 *
 * @since     2011-05-23
 * @category  Library
 * @package   PdfEncrypt
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2011-2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class Seed
{
    /**
     * Number of random bytes returned.
     *
     * @var int
     */
    public const SEEDLEN = 64;

    /**
     * Generate a random seed.
     *
     * @param string $data  Additional data appended to the random bytes.
     * @param string $key   Additional data appended to the random bytes.
     * @param string $_mode Unused; retained for signature compatibility.
     *
     * @return string seed
     *
     * @throws \Random\RandomException When no cryptographically secure source is available.
     */
    public function encrypt(string $data = '', string $key = '', string $_mode = ''): string
    {
        return \random_bytes(self::SEEDLEN) . $data . $key;
    }
}
