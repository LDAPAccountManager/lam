<?php

declare(strict_types=1);

/**
 * Config.php
 *
 * @since     2026-07-15
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

use Com\Tecnick\Pdf\Sign\DigestAlgorithm;
use Com\Tecnick\Pdf\Sign\Exception;

/**
 * Com\Tecnick\Pdf\Sign\Timestamp\Config
 *
 * Immutable RFC 3161 Time Stamping Authority (TSA) configuration. The codec
 * fields (hash algorithm, policy OID, nonce) drive request construction; the
 * transport fields (host, timeout, credentials, CA file, peer verification)
 * are consumed by the caller-provided transport.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Config
{
    /**
     * Supported TSA message-imprint digest algorithms.
     *
     * @var list<string>
     */
    public const HASH_ALGORITHMS = ['sha256', 'sha384', 'sha512'];

    /**
     * Selected message-imprint digest algorithm (one of HASH_ALGORITHMS).
     */
    public readonly string $hashAlgorithm;

    /**
     * @param string                 $host          TSA endpoint URL (https).
     * @param string|DigestAlgorithm $hashAlgorithm Message-imprint digest name or enum case.
     * @param string                 $policyOid     Optional requested TSA policy OID (dotted form).
     * @param bool                   $nonceEnabled  Add a random nonce to the request.
     * @param int                    $timeout       Transport timeout in seconds (>= 1).
     * @param bool                   $verifyPeer    Validate the TSA TLS certificate.
     * @param string                 $username      Optional HTTP basic-auth username.
     * @param string                 $password      Optional HTTP basic-auth password.
     * @param string                 $cert          Optional CA bundle path for the transport.
     *
     * @throws Exception If any option is invalid.
     */
    public function __construct(
        public readonly string $host,
        string|DigestAlgorithm $hashAlgorithm = 'sha256',
        public readonly string $policyOid = '',
        public readonly bool $nonceEnabled = true,
        public readonly int $timeout = 5,
        public readonly bool $verifyPeer = true,
        public readonly string $username = '',
        #[\SensitiveParameter]
        public readonly string $password = '',
        public readonly string $cert = '',
    ) {
        $hashAlgorithm = $hashAlgorithm instanceof DigestAlgorithm ? $hashAlgorithm->value : $hashAlgorithm;
        $this->hashAlgorithm = $hashAlgorithm;

        if ($host === '') {
            throw new Exception('Invalid TSA host');
        }

        if (!\in_array($hashAlgorithm, self::HASH_ALGORITHMS, true)) {
            throw new Exception('Invalid TSA hash algorithm: ' . $hashAlgorithm);
        }

        if ($policyOid !== '' && \preg_match('/^\d+(?:\.\d+)+$/', $policyOid) !== 1) {
            throw new Exception('Invalid TSA policy OID: ' . $policyOid);
        }

        if ($timeout < 1) {
            throw new Exception('Invalid TSA timeout: ' . $timeout);
        }
    }
}
