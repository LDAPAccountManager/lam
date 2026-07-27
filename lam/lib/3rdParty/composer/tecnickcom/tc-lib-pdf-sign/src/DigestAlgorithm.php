<?php

declare(strict_types=1);

/**
 * DigestAlgorithm.php
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign;

/**
 * Com\Tecnick\Pdf\Sign\DigestAlgorithm
 *
 * Backed enum for the supported message-digest algorithms. Unifies the two
 * previously identical closed sets: Config::DIGEST_ALGORITHMS (CMS builder) and
 * Timestamp\Config::HASH_ALGORITHMS (RFC 3161 message imprint). The backing
 * value is the lowercase algorithm name accepted by both.
 *
 * @since     2026-07-17
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
enum DigestAlgorithm: string
{
    case Sha256 = 'sha256';

    case Sha384 = 'sha384';

    case Sha512 = 'sha512';

    /**
     * Resolve a loose digest algorithm value to the matching enum case.
     *
     * Accepts the canonical algorithm string (as validated by Config and
     * Timestamp\Config) or an enum instance (returned unchanged). Unknown values
     * throw, matching the closed set enforced by both configs.
     *
     * @param string|self $value Digest algorithm name or enum case.
     *
     * @throws Exception if the value does not match a known digest algorithm.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new Exception('Invalid digest algorithm: ' . $value);
    }
}
