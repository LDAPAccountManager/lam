<?php

declare(strict_types=1);

/**
 * Oid.php
 *
 * @since     2026-08-25
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

/**
 * Com\Tecnick\Pdf\Sign\Cms\Oid
 *
 * The object identifiers of the CMS content types and signed attribute types
 * this library emits and reads.
 *
 * Read by the builder, which emits an identifier, by the verifier, which resolves
 * one back, and by SigningRequest, which reserves the types the builder controls.
 *
 * @since     2026-08-25
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Oid
{
    /**
     * id-data, the eContentType of a detached CMS over document bytes.
     */
    public const DATA = '1.2.840.113549.1.7.1';

    /**
     * id-signedData, the ContentInfo type of every message this library handles.
     */
    public const SIGNED_DATA = '1.2.840.113549.1.7.2';

    /**
     * id-ct-TSTInfo, the eContentType of an RFC 3161 timestamp token.
     */
    public const TST_INFO = '1.2.840.113549.1.9.16.1.4';

    /**
     * id-contentType, the signed attribute that repeats the eContentType.
     */
    public const CONTENT_TYPE = '1.2.840.113549.1.9.3';

    /**
     * id-messageDigest, the signed attribute that binds the content.
     */
    public const MESSAGE_DIGEST = '1.2.840.113549.1.9.4';

    /**
     * id-signingTime, the signed attribute the legacy profile carries.
     */
    public const SIGNING_TIME = '1.2.840.113549.1.9.5';

    /**
     * id-aa-signingCertificate, the ESS attribute of RFC 2634, SHA-1 by definition.
     */
    public const SIGNING_CERTIFICATE = '1.2.840.113549.1.9.16.2.12';

    /**
     * id-aa-signingCertificateV2, the ESS attribute of RFC 5035.
     */
    public const SIGNING_CERTIFICATE_V2 = '1.2.840.113549.1.9.16.2.47';

    /**
     * id-aa-signatureTimeStampToken, the CAdES unsigned attribute carrying an
     * RFC 3161 token over the SignerInfo signature.
     */
    public const SIGNATURE_TIMESTAMP = '1.2.840.113549.1.9.16.2.14';

    /**
     * The signed attribute types the CMS builder emits itself.
     *
     * RFC 5652 section 5.3 allows at most one instance of each type in
     * SignedAttributes, so these are the ones a caller may not also supply.
     * signing-time is included, its presence being decided by the profile.
     *
     * @var list<string>
     */
    public const BUILDER_ATTRIBUTES = [
        self::CONTENT_TYPE,
        self::MESSAGE_DIGEST,
        self::SIGNING_TIME,
        self::SIGNING_CERTIFICATE,
        self::SIGNING_CERTIFICATE_V2,
    ];
}
