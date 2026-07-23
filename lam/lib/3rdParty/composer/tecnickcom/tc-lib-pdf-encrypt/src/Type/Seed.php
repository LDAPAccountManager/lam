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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-encrypt
 */
class Seed
{
    /**
     * Encrypt the data
     *
     * @param string $data Random seed data
     * @param string $key  Random seed data
     * @param string $mode Default mode (openssl or raw)
     *
     * @return string seed
     *
     * @throws \Random\RandomException
     */
    public function encrypt(string $data = '', string $key = '', string $mode = 'openssl'): string
    {
        $rnd = \uniqid(\random_int(0, \mt_getrandmax()) . microtime(true), true);

        if (\function_exists('posix_getpid')) {
            $rnd .= \posix_getpid();
        }

        if (
            $mode === 'openssl'
            && \function_exists('openssl_random_pseudo_bytes')
            && \strtoupper(\substr(PHP_OS, 0, 3)) !== 'WIN'
        ) {
            // this is not used on windows systems because it is very slow for a know bug
            $rnd .= \openssl_random_pseudo_bytes(512);

            return $rnd . $data . __DIR__ . __FILE__ . $key . \serialize($_SERVER) . microtime(true);
        }

        for ($idx = 0; $idx < 23; ++$idx) {
            $rnd .= \uniqid('', true);
        }

        return $rnd . $data . __DIR__ . __FILE__ . $key . \serialize($_SERVER) . microtime(true);
    }
}
