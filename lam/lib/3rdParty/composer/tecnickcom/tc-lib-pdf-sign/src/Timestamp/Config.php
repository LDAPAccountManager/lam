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

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
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
     * Derived from DigestAlgorithm, which is the closed set the constructor
     * enforces and which the CMS side shares.
     *
     * @var list<string>
     */
    public const HASH_ALGORITHMS = [
        DigestAlgorithm::Sha256->value,
        DigestAlgorithm::Sha384->value,
        DigestAlgorithm::Sha512->value,
    ];

    /**
     * The shape a URL this library hands to a host transport has to have.
     *
     * Anchored at both ends, so no control character reaches the host's transport.
     * The end anchor is \z, not $, which PCRE matches before a final newline. The
     * authority is held to the same character class as the path.
     *
     * It gates the TSA endpoint a host supplies and the OCSP and CRL URLs
     * Ltv\ValidationMaterial reads out of a certificate extension.
     */
    public const URL_PATTERN = '~^https?://[^\x00-\x20\x7F/?#]+(?:[/?#][^\x00-\x20\x7F]*)?\z~i';

    /**
     * Selected message-imprint digest algorithm (one of the DigestAlgorithm
     * backing values).
     */
    public readonly string $hashAlgorithm;

    /**
     * The codec reads hashAlgorithm, policyOid, and nonceEnabled. The remaining
     * entries describe the transport, which this library does not perform: the
     * host receives them here and has to apply them to its own HTTP client.
     *
     * @param string                 $host          TSA endpoint URL (http or https).
     * @param string|DigestAlgorithm $hashAlgorithm Message-imprint digest name or enum case.
     * @param string                 $policyOid     Optional requested TSA policy OID (dotted form).
     * @param bool                   $nonceEnabled  Add a random nonce to the request.
     * @param int                    $timeout       Transport timeout in seconds (>= 1), for the host.
     * @param bool                   $verifyPeer    Validate the TSA TLS certificate, for the host.
     * @param string                 $username      Optional HTTP basic-auth username, for the host.
     * @param string                 $password      Optional HTTP basic-auth password, for the host.
     * @param string                 $cert          Optional CA bundle path, for the host.
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
        $algorithm = DigestAlgorithm::fromLoose($hashAlgorithm);

        if (\preg_match(self::URL_PATTERN, $host) !== 1) {
            throw new Exception('Invalid TSA host: ' . $host);
        }

        // The three transport entries reach the same host client the endpoint does,
        // so they are refused a control character as URL_PATTERN refuses one. A space
        // and a non-ASCII character are admitted, a password holding either.
        foreach (['username' => $username, 'password' => $password, 'cert' => $cert] as $name => $value) {
            if (\preg_match('/^[^\x00-\x1F\x7F]*\z/', $value) !== 1) {
                throw new Exception('The TSA ' . $name . ' holds a control character');
            }
        }

        // Validated by encoding it, so an arc the encoder rejects is refused here rather
        // than at the first buildRequest() call.
        if ($policyOid !== '') {
            try {
                (new Asn1())->encodeObjectIdentifier($policyOid);
            } catch (Exception $e) {
                throw new Exception('Invalid TSA policy OID: ' . $policyOid, 0, $e);
            }
        }

        if ($timeout < 1) {
            throw new Exception('Invalid TSA timeout: ' . $timeout);
        }

        $this->hashAlgorithm = $algorithm->value;
    }

    /**
     * Keep the basic-auth password out of var_dump() and print_r() output.
     *
     * @return array<string, mixed>
     */
    public function __debugInfo(): array
    {
        return [
            'host' => $this->host,
            'hashAlgorithm' => $this->hashAlgorithm,
            'policyOid' => $this->policyOid,
            'nonceEnabled' => $this->nonceEnabled,
            'timeout' => $this->timeout,
            'verifyPeer' => $this->verifyPeer,
            'username' => $this->username,
            'password' => $this->password === '' ? '' : '***',
            'cert' => $this->cert,
        ];
    }
}
