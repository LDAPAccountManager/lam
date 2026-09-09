<?php

declare(strict_types=1);

/**
 * SignatureEncoding.php
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Cms;

use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Cms\SignatureEncoding
 *
 * Backed enum for the encoding of a signature handed to Builder::buildFromSignature().
 * CMS carries an ECDSA signature as the DER SEQUENCE of the two integers, while
 * some hardware tokens and remote signing services return the fixed-width
 * concatenation defined by IEEE P1363. The case selects which one is supplied.
 * An RSA signature is always Der.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
enum SignatureEncoding: string
{
    /**
     * The signature is already encoded as CMS requires it.
     */
    case Der = 'der';

    /**
     * ECDSA signature as the fixed-width concatenation r || s (IEEE P1363).
     */
    case P1363 = 'p1363';

    /**
     * Resolve a loose signature encoding value to the matching enum case.
     *
     * Accepts the canonical encoding string or an enum instance (returned
     * unchanged). Unknown values throw.
     *
     * @param string|self $value Signature encoding name or enum case.
     *
     * @throws Exception if the value does not match a known signature encoding.
     */
    public static function fromLoose(string|self $value): self
    {
        if ($value instanceof self) {
            return $value;
        }

        return self::tryFrom($value) ?? throw new Exception('Invalid signature encoding: ' . $value);
    }
}
