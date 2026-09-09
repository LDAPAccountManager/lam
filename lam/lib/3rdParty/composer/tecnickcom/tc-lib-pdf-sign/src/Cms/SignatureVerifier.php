<?php

declare(strict_types=1);

/**
 * SignatureVerifier.php
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
 * Com\Tecnick\Pdf\Sign\Cms\SignatureVerifier
 *
 * Verifies the signature of a DER structure that follows the X.509 shape of a
 * signed body, an AlgorithmIdentifier, and a signature BIT STRING. An OCSP
 * BasicOCSPResponse and a CRL CertificateList are both built that way, and both
 * are accepted as validation material only once the signature over them checks
 * out against the certificate that produced it.
 *
 * The accepted algorithms are SHA-256 and above. SHA-1 is refused unless the
 * caller passes $allowSha1.
 *
 * @since     2026-08-24
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class SignatureVerifier
{
    /**
     * rsaEncryption, the PKCS #1 v1.5 signature value identifier that names no digest.
     *
     * RFC 3370 section 3.2, repeated by RFC 5754 section 3.2: in CMS an RSA
     * signature value is identified by rsaEncryption whatever the digest, which the
     * structure carries in a field of its own and the caller passes to verify(). It
     * is what Builder emits. RFC 5280 section 4.1.1.2 requires the sha*With* form of
     * a certificate, a CRL, or an OCSP response instead.
     */
    public const OID_RSA_ENCRYPTION = '1.2.840.113549.1.1.1';

    /**
     * Digest name to the openssl constant, for a signature identifier naming none.
     *
     * @var array<string, int>
     */
    private const DIGEST_CONSTANTS = [
        'sha256' => OPENSSL_ALGO_SHA256,
        'sha384' => OPENSSL_ALGO_SHA384,
        'sha512' => OPENSSL_ALGO_SHA512,
        'sha1' => OPENSSL_ALGO_SHA1,
    ];

    /**
     * Signature AlgorithmIdentifier OID to the openssl digest constant.
     *
     * RSASSA-PSS (1.2.840.113549.1.1.10) is absent because openssl_verify()
     * cannot express its parameters, so a structure signed with it is reported as
     * unsupported rather than accepted unchecked.
     *
     * @var array<string, int>
     */
    public const ALGORITHMS = [
        '1.2.840.113549.1.1.11' => OPENSSL_ALGO_SHA256, // sha256WithRSAEncryption
        '1.2.840.113549.1.1.12' => OPENSSL_ALGO_SHA384, // sha384WithRSAEncryption
        '1.2.840.113549.1.1.13' => OPENSSL_ALGO_SHA512, // sha512WithRSAEncryption
        '1.2.840.10045.4.3.2' => OPENSSL_ALGO_SHA256, // ecdsa-with-SHA256
        '1.2.840.10045.4.3.3' => OPENSSL_ALGO_SHA384, // ecdsa-with-SHA384
        '1.2.840.10045.4.3.4' => OPENSSL_ALGO_SHA512, // ecdsa-with-SHA512
    ];

    /**
     * SHA-1 signature algorithms, accepted only when the caller opts in.
     *
     * Reachable for a legacy responder or CRL distribution point that emits nothing
     * else.
     *
     * @var array<string, int>
     */
    public const LEGACY_ALGORITHMS = [
        '1.2.840.113549.1.1.5' => OPENSSL_ALGO_SHA1, // sha1WithRSAEncryption
        '1.2.840.10045.4.1' => OPENSSL_ALGO_SHA1, // ecdsa-with-SHA1
    ];

    private Asn1 $asn1;

    /**
     * @param bool $allowSha1 Accept the SHA-1 signature algorithms as well.
     */
    public function __construct(
        ?Asn1 $asn1 = null,
        private readonly bool $allowSha1 = false,
    ) {
        $this->asn1 = $asn1 ?? new Asn1();
    }

    /**
     * Verify a signature against the certificate that is said to have produced it.
     *
     * @param string $signedDer      Complete DER of the signed body, as the signature covers it.
     * @param string $algorithmIdDer Complete DER of the signature AlgorithmIdentifier.
     * @param string $signature      Signature octets, without the BIT STRING unused-bits count.
     * @param string $signerCertDer  DER of the certificate holding the verifying public key.
     * @param string|null $digestName Digest the structure names in a field of its own, for a
     *                               signature identifier that names none. Required for
     *                               rsaEncryption and ignored otherwise, since every other
     *                               identifier here implies its digest.
     *
     * @throws Exception If the algorithm is unsupported, the certificate is unreadable, or the
     *                   signature does not verify.
     */
    public function verify(
        string $signedDer,
        string $algorithmIdDer,
        string $signature,
        string $signerCertDer,
        ?string $digestName = null,
    ): void {
        [$algorithm, $digest] = $this->algorithm($algorithmIdDer, $digestName);

        $publicKey = \openssl_pkey_get_public(Certificate::derToPem($signerCertDer));
        if ($publicKey === false) {
            Certificate::clearOpenSslErrors();
            throw new Exception('Unreadable signing certificate');
        }

        $this->checkKeyType($algorithm, $publicKey);

        $verified = \openssl_verify($signedDer, $signature, $publicKey, $digest);
        Certificate::clearOpenSslErrors();
        if ($verified !== 1) {
            throw new Exception('The signature does not verify against the signing certificate');
        }
    }

    /**
     * Check that the signature algorithm is one the certificate's key can produce.
     *
     * openssl_verify() takes a digest and reads the algorithm from the key, so the
     * OID selects nothing but the digest: without this an RSA signature labelled
     * ecdsa-with-SHA256 would verify against an RSA certificate.
     *
     * @param string $algorithm Signature algorithm OID.
     *
     * @throws Exception If the key type cannot be read or does not match the OID.
     */
    private function checkKeyType(string $algorithm, \OpenSSLAsymmetricKey $publicKey): void
    {
        $details = \openssl_pkey_get_details($publicKey);
        $type = $details !== false ? $details['type'] ?? -1 : -1;
        Certificate::clearOpenSslErrors();

        $expected = \str_starts_with($algorithm, '1.2.840.10045.') ? OPENSSL_KEYTYPE_EC : OPENSSL_KEYTYPE_RSA;
        if ($type !== $expected) {
            throw new Exception('The signature algorithm ' . $algorithm . ' does not match the certificate key');
        }
    }

    /**
     * Resolve the OID and openssl digest constant of a signature AlgorithmIdentifier.
     *
     * Asn1::decodeAlgorithmIdentifier() owns the walk, which bounds both layers of
     * the field.
     *
     * @param string|null $digestName Digest supplied by the caller, for an identifier
     *                                that names none.
     *
     * @return array{string, int} [signature algorithm OID, openssl digest constant]
     *
     * @throws Exception If the identifier is malformed or names an unsupported algorithm.
     */
    private function algorithm(string $algorithmIdDer, ?string $digestName): array
    {
        $algorithm = $this->asn1->decodeAlgorithmIdentifier($algorithmIdDer, 'signature');

        if ($algorithm === self::OID_RSA_ENCRYPTION) {
            return [$algorithm, $this->suppliedDigest($digestName)];
        }

        if (isset(self::ALGORITHMS[$algorithm])) {
            return [$algorithm, self::ALGORITHMS[$algorithm]];
        }

        if (isset(self::LEGACY_ALGORITHMS[$algorithm])) {
            if (!$this->allowSha1) {
                throw new Exception('Refusing the SHA-1 signature algorithm: ' . $algorithm);
            }

            return [$algorithm, self::LEGACY_ALGORITHMS[$algorithm]];
        }

        throw new Exception('Unsupported signature algorithm: ' . $algorithm);
    }

    /**
     * Resolve the openssl constant of a digest the caller read elsewhere.
     *
     * Held to the same SHA-1 rule as an identifier that names its own digest.
     *
     * @throws Exception If no digest was supplied, or it is unsupported or refused.
     */
    private function suppliedDigest(?string $digestName): int
    {
        if ($digestName === null) {
            throw new Exception('The rsaEncryption signature algorithm names no digest and none was supplied');
        }

        if (!isset(self::DIGEST_CONSTANTS[$digestName])) {
            throw new Exception('Unsupported digest algorithm: ' . $digestName);
        }

        if ($digestName === 'sha1' && !$this->allowSha1) {
            throw new Exception('Refusing the SHA-1 digest algorithm: ' . $digestName);
        }

        return self::DIGEST_CONSTANTS[$digestName];
    }
}
