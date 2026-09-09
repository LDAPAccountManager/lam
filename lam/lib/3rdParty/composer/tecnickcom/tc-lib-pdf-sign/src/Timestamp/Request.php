<?php

declare(strict_types=1);

/**
 * Request.php
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

namespace Com\Tecnick\Pdf\Sign\Timestamp;

/**
 * Com\Tecnick\Pdf\Sign\Timestamp\Request
 *
 * A TimeStampReq together with the message imprint and nonce it carries, which
 * RFC 3161 section 2.4.2 requires the requester to check against the returned
 * token.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final readonly class Request
{
    /**
     * @param string $der       DER-encoded TimeStampReq, as sent to the TSA.
     * @param string $imprint   Raw digest of the timestamped bytes.
     * @param string $hashOid   OID of the digest algorithm of $imprint.
     * @param string $nonce     DER INTEGER of the nonce, or '' when none was sent.
     * @param string $policyOid Requested TSA policy OID, or '' when none was requested.
     *                          RFC 3161 section 2.4.2 requires a token to be issued under
     *                          the policy the request named.
     */
    public function __construct(
        public string $der,
        public string $imprint,
        public string $hashOid,
        public string $nonce = '',
        public string $policyOid = '',
    ) {}
}
