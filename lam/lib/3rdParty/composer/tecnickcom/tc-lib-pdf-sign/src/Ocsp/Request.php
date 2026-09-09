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

namespace Com\Tecnick\Pdf\Sign\Ocsp;

/**
 * Com\Tecnick\Pdf\Sign\Ocsp\Request
 *
 * An OCSP request together with the CertID it asks about, which RFC 6960 section
 * 3.2 requires the response to be matched against before it is accepted.
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
     * @param string $der       DER-encoded OCSPRequest, as sent to the responder.
     * @param string $certId    DER-encoded CertID the response must quote back.
     * @param string $issuerDer DER of the issuing certificate, which the response is
     *                          verified against: either the responder itself or the
     *                          authority that delegated to it.
     */
    public function __construct(
        public string $der,
        public string $certId,
        public string $issuerDer,
    ) {}
}
