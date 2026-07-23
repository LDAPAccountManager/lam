<?php

declare(strict_types=1);

/**
 * Signer.php
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign;

use Com\Tecnick\Pdf\Sign\Cms\Builder;
use Com\Tecnick\Pdf\Sign\Ltv\ValidationMaterial;
use Com\Tecnick\Pdf\Sign\Timestamp\Client as TimestampClient;
use OpenSSLAsymmetricKey;

/**
 * Com\Tecnick\Pdf\Sign\Signer
 *
 * Package-internal orchestration entry point that ties the CMS builder, the RFC
 * 3161 timestamp codec, and the LTV material collector together behind two
 * host-facing calls. It stays transport-injected and free of file and network
 * access: the host loads keys and owns HTTP (and SSRF protection).
 *
 * sign() produces the detached CAdES CMS for a document's ByteRange bytes. For a
 * legacy or PAdES B-B profile that is the plain CMS; for B-T and above it also
 * requests an RFC 3161 signature timestamp and embeds it as the SignerInfo
 * id-aa-signatureTimeStampToken unsigned attribute.
 *
 * collectValidationMaterial() gathers the certificates, OCSP responses, and CRLs
 * a B-LT or B-LTA document needs, shaped for the DSS emitter. The VRI key is not
 * computed here: it depends on the final signature Contents and belongs to the
 * DSS writer.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Signer
{
    /**
     * Profiles that require an embedded signature timestamp (B-T and above).
     *
     * @var list<string>
     */
    private const TIMESTAMPED_PROFILES = [
        Config::PROFILE_PADES_B_T,
        Config::PROFILE_PADES_B_LT,
        Config::PROFILE_PADES_B_LTA,
    ];

    private Builder $builder;

    private ValidationMaterial $validationMaterial;

    public function __construct(?Builder $builder = null, ?ValidationMaterial $validationMaterial = null)
    {
        $this->builder = $builder ?? new Builder();
        $this->validationMaterial = $validationMaterial ?? new ValidationMaterial();
    }

    /**
     * Produce the detached CAdES CMS for a document's ByteRange content.
     *
     * When the profile is B-T or above, the timestamp client and transport are
     * required: the RFC 3161 token is requested over the raw signature bytes and
     * embedded as the id-aa-signatureTimeStampToken unsigned attribute.
     *
     * @param string               $content            ByteRange-covered document bytes to sign.
     * @param string               $signerCertDer      DER of the signing certificate.
     * @param OpenSSLAsymmetricKey  $privateKey         Signing private key (RSA or EC).
     * @param list<string>         $chainCertsDer      Additional certificates (DER) to embed.
     * @param Config               $config             Signature profile and digest configuration.
     * @param int                  $signingTime        Unix timestamp for the signing-time attribute.
     * @param TimestampClient|null  $timestamp          RFC 3161 codec; required for B-T and above.
     * @param (callable(string): string)|null $timestampTransport Maps a DER TimeStampReq to a DER
     *                            TimeStampResp; required for B-T and above.
     *
     * @return string DER-encoded CMS ContentInfo ready for /Contents injection.
     *
     * @throws Exception If a timestamp is required but not configured, or signing fails.
     */
    public function sign(
        string $content,
        string $signerCertDer,
        OpenSSLAsymmetricKey $privateKey,
        array $chainCertsDer,
        Config $config,
        int $signingTime,
        ?TimestampClient $timestamp = null,
        ?callable $timestampTransport = null,
    ): string {
        $signatureTimestamp = null;
        if (\in_array($config->profile, self::TIMESTAMPED_PROFILES, true)) {
            if ($timestamp === null || $timestampTransport === null) {
                throw new Exception('Profile ' . $config->profile . ' requires a timestamp client and transport');
            }

            $signatureTimestamp =
                /** @throws Exception */
                static fn(string $signature): string => $timestamp->requestToken($signature, $timestampTransport);
        }

        // PAdES-BASELINE carries the signing time in the /M dictionary entry and forbids
        // the CMS signing-time attribute; only the legacy profile embeds it.
        return $this->builder->sign(
            $content,
            $signerCertDer,
            $privateKey,
            $chainCertsDer,
            $config->digestAlgorithm,
            $signingTime,
            $signatureTimestamp,
            !$config->isPades(),
        );
    }

    /**
     * Collect the long-term validation material for an ordered certificate chain.
     *
     * The chain must be ordered leaf-first, each entry followed by its issuer.
     * For every certificate that has an issuer in the chain, OCSP is attempted
     * against the responder URLs found in its AIA extension; CRLs are attempted
     * against every certificate's CRL distribution points. A null transport skips
     * that revocation source. Responses are deduplicated across the whole chain.
     *
     * @param list<string>  $chainPem      Certificates in PEM, leaf first up to the root.
     * @param (callable(string, string): (string|false))|null $ocspTransport Maps (url, DER request) to
     *                            the DER response, or null to skip OCSP.
     * @param (callable(string): (string|false))|null $crlTransport Maps a url to the CRL bytes, or null
     *                            to skip CRLs.
     *
     * @return array{certs: list<string>, ocsp: list<string>, crls: list<string>} DSS-ready material.
     *
     * @throws Exception If a certificate cannot be parsed or converted.
     */
    public function collectValidationMaterial(
        array $chainPem,
        ?callable $ocspTransport = null,
        ?callable $crlTransport = null,
    ): array {
        $certs = [];
        foreach ($chainPem as $pem) {
            $certs[] = ['pem' => $pem, 'der' => $this->pemToDer($pem)];
        }

        $ocsp = [];
        $crls = [];
        foreach ($certs as $idx => $cert) {
            $issuer = $certs[$idx + 1] ?? null;
            if ($ocspTransport !== null && $issuer !== null) {
                $urls = $this->validationMaterial->certificateOcspUrls($cert['pem']);
                $ocsp = [
                    ...$ocsp,
                    ...$this->validationMaterial->fetchOcsp($issuer['der'], $cert['der'], $urls, $ocspTransport),
                ];
            }

            if ($crlTransport !== null) {
                $urls = $this->validationMaterial->certificateCrlUrls($cert['pem']);
                $crls = [...$crls, ...$this->validationMaterial->fetchCrl($urls, $crlTransport)];
            }
        }

        $certDers = \array_map(static fn(array $cert): string => $cert['der'], $certs);

        return [
            'certs' => $this->deduplicate($certDers),
            'ocsp' => $this->deduplicate($ocsp),
            'crls' => $this->deduplicate($crls),
        ];
    }

    /**
     * Deduplicate a list of binary blobs by content, preserving first-seen order.
     *
     * @param list<string> $items
     *
     * @return list<string>
     */
    private function deduplicate(array $items): array
    {
        $seen = [];
        $result = [];
        foreach ($items as $item) {
            $fingerprint = \hash('sha256', $item);
            if (isset($seen[$fingerprint])) {
                continue;
            }

            $seen[$fingerprint] = true;
            $result[] = $item;
        }

        return $result;
    }

    /**
     * Decode a PEM certificate to DER.
     *
     * @throws Exception If the PEM cannot be decoded.
     */
    private function pemToDer(string $pem): string
    {
        $stripped = (string) \preg_replace('/-----[^-]+-----|\s+/', '', $pem);
        $der = \base64_decode($stripped, true);
        if ($der === false) {
            throw new Exception('Invalid PEM certificate');
        }

        return $der;
    }
}
