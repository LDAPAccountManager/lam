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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign;

use Com\Tecnick\Pdf\Sign\Cms\Builder;
use Com\Tecnick\Pdf\Sign\Cms\Certificate;
use Com\Tecnick\Pdf\Sign\Cms\SignatureEncoding;
use Com\Tecnick\Pdf\Sign\Cms\SignedDataVerifier;
use Com\Tecnick\Pdf\Sign\Cms\SigningRequest;
use Com\Tecnick\Pdf\Sign\Ltv\SkipReason;
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
 * prepare() and buildFromSignature() are the same call split in two, for a signature made
 * outside this process. They apply the same profile rules as sign().
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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
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

    /**
     * Most certificates accepted in the unauthenticated certificate bag of a
     * timestamp token. The same bound Ocsp\Client applies to a response's certs [0]
     * field, held in Cms\Certificate.
     */
    public const MAX_PATH_CERTIFICATES = Certificate::MAX_EMBEDDED_CERTIFICATES;

    private Builder $builder;

    private ValidationMaterial $validationMaterial;

    private Certificate $certificate;

    private SignedDataVerifier $tokenVerifier;

    /**
     * @param bool $checkSignerCertificate Refuse to sign with a certificate that had
     *                                     expired, was not yet valid, or whose key
     *                                     usage forbids signing. Off by default,
     *                                     since a host may deliberately re-sign
     *                                     historical content.
     * @param SignedDataVerifier|null $tokenVerifier Resolves the signer certificate of a
     *                                     timestamp token passed to
     *                                     collectValidationMaterial(). The default one
     *                                     requires the ESS signing-certificate attribute
     *                                     RFC 3161 section 2.4.2 asks of a token. Pass one
     *                                     constructed with $allowSha1 to accept a token
     *                                     from a responder that signs with nothing else,
     *                                     or without $requireSigningCertificate for a TSA
     *                                     that emits no such attribute.
     *
     * @throws Exception If a default collaborator cannot be constructed.
     */
    public function __construct(
        ?Builder $builder = null,
        ?ValidationMaterial $validationMaterial = null,
        ?Certificate $certificate = null,
        private readonly bool $checkSignerCertificate = false,
        ?SignedDataVerifier $tokenVerifier = null,
    ) {
        $this->builder = $builder ?? new Builder();
        $this->validationMaterial = $validationMaterial ?? new ValidationMaterial();
        $this->certificate = $certificate ?? new Certificate();
        $this->tokenVerifier = $tokenVerifier ?? new SignedDataVerifier(requireSigningCertificate: true);
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
     * @param list<string>         $chainCertsDer      Additional certificates to embed, each as
     *                            PEM or as DER. Every entry is parsed.
     * @param Config               $config             Signature profile and digest configuration.
     * @param int                  $signingTime        Unix timestamp for the signing-time attribute.
     * @param TimestampClient|null  $timestamp          RFC 3161 codec; required for B-T and above.
     * @param (callable(string): string)|null $timestampTransport Maps a DER TimeStampReq to a DER
     *                            TimeStampResp; required for B-T and above.
     * @param array<array-key, string> $extraSignedAttributes Additional signed attributes as
     *                            OID => DER-encoded attribute value, as prepare() accepts.
     * @param int|null             $timestampNow       Unix time the token's genTime is checked
     *                            against; defaults to the current time. It is the moment the
     *                            request is made, not the signing time.
     *
     * @return string DER-encoded CMS ContentInfo ready for /Contents injection.
     *
     * @throws Exception If a timestamp is required but not configured, signing fails, or
     *                   the signing certificate fails the optional checks.
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
        array $extraSignedAttributes = [],
        ?int $timestampNow = null,
    ): string {
        $this->assertSignerCertificate($signerCertDer, $signingTime);

        // PAdES-BASELINE carries the signing time in the /M dictionary entry and forbids
        // the CMS signing-time attribute; only the legacy profile embeds it.
        return $this->builder->sign(
            $content,
            $signerCertDer,
            $privateKey,
            $chainCertsDer,
            $config->digestAlgorithm,
            $signingTime,
            $this->signatureTimestampProvider($config, $timestamp, $timestampTransport, $timestampNow),
            !$config->isPades(),
            $extraSignedAttributes,
        );
    }

    /**
     * Build the request whose bytes an external signer has to sign.
     *
     * The first half of sign(), for a private key this process cannot reach. The
     * digest is of the ByteRange-covered document bytes.
     *
     * Pass the returned request to Cms\Builder::signaturePayload() for the bytes to
     * sign, then back to buildFromSignature() with the signature. Its
     * toArray()/fromArray() pair carries it across a session or a queue.
     *
     * @param string       $messageDigest  Digest of the ByteRange content, raw bytes, computed
     *                            with the profile's digest algorithm.
     * @param string       $signerCertDer  DER of the signing certificate.
     * @param Config       $config         Signature profile and digest configuration.
     * @param int          $signingTime    Unix timestamp for the signing-time attribute.
     * @param array<array-key, string> $extraSignedAttributes Additional signed attributes as
     *                            OID => DER-encoded attribute value.
     *
     * @throws Exception If the digest, the certificate, or an extra attribute is invalid,
     *                   or the signing certificate fails the optional checks.
     */
    public function prepare(
        string $messageDigest,
        string $signerCertDer,
        Config $config,
        int $signingTime,
        array $extraSignedAttributes = [],
    ): SigningRequest {
        $this->assertSignerCertificate($signerCertDer, $signingTime);

        return new SigningRequest(
            $messageDigest,
            $signerCertDer,
            $config->digestAlgorithm,
            $signingTime,
            !$config->isPades(),
            $extraSignedAttributes,
        );
    }

    /**
     * Produce the bytes an external signer has to sign for a prepared request.
     *
     * A passthrough to Cms\Builder::signaturePayload().
     *
     * @param SigningRequest $request The request returned by prepare().
     *
     * @return string DER SET OF signed attributes, ready to be signed.
     *
     * @throws Exception If the digest is unsupported or encoding fails.
     */
    public function signaturePayload(SigningRequest $request): string
    {
        return $this->builder->signaturePayload($request);
    }

    /**
     * Produce the detached CAdES CMS from an externally produced signature.
     *
     * The second half of sign(). As there, a B-T or higher profile requires the
     * timestamp client and transport, and the RFC 3161 token is requested over
     * the raw signature bytes.
     *
     * @param SigningRequest $request       The request returned by prepare().
     * @param string         $signature     Signature over Cms\Builder::signaturePayload($request).
     * @param list<string>   $chainCertsDer Additional certificates to embed, each as PEM or as DER.
     * @param Config         $config        Signature profile and digest configuration.
     * @param TimestampClient|null $timestamp RFC 3161 codec; required for B-T and above.
     * @param (callable(string): string)|null $timestampTransport Maps a DER TimeStampReq to a DER
     *                            TimeStampResp; required for B-T and above.
     * @param string|SignatureEncoding $signatureEncoding Encoding of $signature.
     * @param int|null                 $timestampNow Unix time the token's genTime is checked
     *                            against; defaults to the current time.
     *
     * @return string DER-encoded CMS ContentInfo ready for /Contents injection.
     *
     * @throws Exception If the request was prepared under another configuration, a
     *                   timestamp is required but not configured, the signing
     *                   certificate fails the optional checks, or the signature
     *                   does not verify.
     */
    public function buildFromSignature(
        SigningRequest $request,
        string $signature,
        array $chainCertsDer,
        Config $config,
        ?TimestampClient $timestamp = null,
        ?callable $timestampTransport = null,
        string|SignatureEncoding $signatureEncoding = SignatureEncoding::Der,
        ?int $timestampNow = null,
    ): string {
        $this->assertRequestMatchesConfig($request, $config);

        // Applied here as well as at prepare(): a request may be built by its own
        // constructor and crosses a session or a queue on the way here.
        $this->assertSignerCertificate($request->signerCertDer, $request->signingTime);

        return $this->builder->buildFromSignature(
            $request,
            $signature,
            $chainCertsDer,
            $this->signatureTimestampProvider($config, $timestamp, $timestampTransport, $timestampNow),
            $signatureEncoding,
        );
    }

    /**
     * Check that a request was prepared under the configuration now in hand.
     *
     * The digest algorithm and the PAdES signing-time rule are fixed at prepare()
     * time and carried by the request, so the two halves of a two-phase signature
     * are compared here rather than reapplied.
     *
     * @throws Exception If the request states another digest algorithm or profile.
     */
    private function assertRequestMatchesConfig(SigningRequest $request, Config $config): void
    {
        if ($request->digestAlgorithm !== $config->digestAlgorithm) {
            throw new Exception(
                'The signing request was prepared with '
                . $request->digestAlgorithm
                . ', not '
                . $config->digestAlgorithm,
            );
        }

        if ($request->includeSigningTime === $config->isPades()) {
            throw new Exception('The signing request was prepared for another profile than ' . $config->profile);
        }
    }

    /**
     * Run the optional checks on the signing certificate.
     *
     * The validity period and key usage checks run only when the constructor
     * enabled them. Whether a certificate is trusted stays the host's question.
     *
     * @throws Exception If the certificate cannot be parsed, did not cover
     *                   $signingTime, or may not be used for signing.
     */
    private function assertSignerCertificate(string $signerCertDer, int $signingTime): void
    {
        if (!$this->checkSignerCertificate) {
            return;
        }

        $this->certificate->assertValidAt($signerCertDer, $signingTime);
        $this->certificate->assertUsableForSigning($signerCertDer);
    }

    /**
     * Resolve the signature timestamp provider required by the profile.
     *
     * @param (callable(string): string)|null $timestampTransport Maps a DER TimeStampReq to a DER
     *                            TimeStampResp.
     * @param int|null $timestampNow Unix time the token's genTime is checked against.
     *
     * @return (callable(string): string)|null Null when the profile embeds no signature timestamp.
     *
     * @throws Exception If the profile requires a timestamp but none is configured.
     */
    private function signatureTimestampProvider(
        Config $config,
        ?TimestampClient $timestamp,
        ?callable $timestampTransport,
        ?int $timestampNow = null,
    ): ?callable {
        if (!\in_array($config->profile, self::TIMESTAMPED_PROFILES, true)) {
            return null;
        }

        if ($timestamp === null || $timestampTransport === null) {
            throw new Exception('Profile ' . $config->profile . ' requires a timestamp client and transport');
        }

        return /** @throws Exception */ static fn(string $signature): string => $timestamp->requestToken(
            $signature,
            $timestampTransport,
            $timestampNow,
        );
    }

    /**
     * Read back the signature timestamp tokens a CMS carries.
     *
     * Returns the tokens sign() and buildFromSignature() embed as the
     * id-aa-signatureTimeStampToken unsigned attribute, which is what
     * collectValidationMaterial() takes as its fourth argument.
     *
     * @param string $cmsDer The CMS returned by sign() or buildFromSignature().
     *
     * @return list<string> DER tokens, empty for a B-B or legacy signature.
     *
     * @throws Exception If the CMS cannot be parsed.
     */
    public function signatureTimestampTokens(string $cmsDer): array
    {
        return $this->certificate->signatureTimestampTokens($cmsDer);
    }

    /**
     * Collect the long-term validation material for an ordered certificate chain.
     *
     * The chain must be ordered leaf-first, each entry followed by its issuer, and
     * the ordering is verified by signature. For every certificate that has an
     * issuer in the chain, OCSP and CRL lookups are attempted against the responder
     * URLs in its AIA extension and the distribution points it names. A certificate
     * with no issuer in the chain gets neither, and every URL skipped for that
     * reason is reported through $onSkip with the NotAttempted code; a self-signed
     * root is not reported. A null transport skips that revocation source. Responses
     * are deduplicated across the whole chain.
     *
     * A certificate either source reported revoked contributes no material at all,
     * not even the other source's. The certificate itself stays in the result, and
     * the verdict reaches the caller through $onSkip with the Revoked code.
     *
     * Each token in $timestampTokens is verified, its embedded TSA certificates are
     * ordered into a path starting at the certificate that signed it, and that path
     * is collected alongside the signer's chain with the same lookups, as ETSI EN
     * 319 142-1 requires of a B-LT Document Security Store. A bag member outside
     * that path is embedded but not looked up, and its URLs are reported through
     * $onSkip.
     *
     * @param list<string>  $chainPem      Certificates leaf first up to the root, each
     *                            as PEM or as DER.
     * @param (callable(string, string): (string|false))|null $ocspTransport Maps (url, DER request) to
     *                            the DER response, or null to skip OCSP.
     * @param (callable(string): (string|false))|null $crlTransport Maps a url to the CRL bytes, or null
     *                            to skip CRLs.
     * @param list<string>  $timestampTokens DER timestamp tokens whose embedded certificates
     *                            are added to the material, along with revocation data for the
     *                            paths they carry.
     * @param int|null      $now           Unix time the responses are checked against; defaults to
     *                            the current time. Pass the signing time so a retried or queued
     *                            signature collects against the same instant it signs for.
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip Receives
     *                            (source, url, reason, code) for every revocation URL whose
     *                            answer was discarded or never fetched. The code separates a
     *                            revoked verdict from an unreachable responder.
     *
     * @return array{certs: list<string>, ocsp: list<string>, crls: list<string>} DSS-ready material.
     *
     * @throws Exception If a certificate or a token is not a string, a certificate
     *                   cannot be parsed, the chain is not ordered leaf-first with each
     *                   entry followed by its issuer, a token does not verify against a
     *                   certificate it embeds, or a token embeds more than
     *                   MAX_PATH_CERTIFICATES certificates.
     */
    public function collectValidationMaterial(
        array $chainPem,
        ?callable $ocspTransport = null,
        ?callable $crlTransport = null,
        array $timestampTokens = [],
        ?int $now = null,
        ?callable $onSkip = null,
    ): array {
        $certs = [];
        /** @var mixed $entry */
        foreach ($chainPem as $index => $entry) {
            // As in Cms\Builder::certificateSet(): an entry that is not a string
            // would reach toDer() as a TypeError rather than an Exception.
            if (!\is_string($entry)) {
                throw new Exception('Invalid chain certificate ' . $index);
            }

            $der = Certificate::toDer($entry);
            $certs[] = ['pem' => Certificate::derToPem($der), 'der' => $der];
        }

        $this->assertOrderedChain($certs);

        $material = $this->pathMaterial($certs, $ocspTransport, $crlTransport, $now, $onSkip);
        $allCerts = $material['certs'];
        $allOcsp = $material['ocsp'];
        $allCrls = $material['crls'];

        // ETSI EN 319 142-1: a B-LT DSS covers the signature timestamp's validation
        // path too, so the TSA chain gets the same treatment as the signer's.
        /** @var mixed $token */
        foreach ($timestampTokens as $index => $token) {
            // As with a chain entry above: a member that is not a string would reach
            // boundedTokenCertificates() as a TypeError rather than an Exception.
            if (!\is_string($token)) {
                throw new Exception('Invalid timestamp token ' . $index);
            }

            // Bounded before the token is verified, since resolving the signer walks
            // the same bag twice more.
            $bag = $this->boundedTokenCertificates($token);

            [$path, $unchained] = $this->orderedPath($bag, $this->tokenVerifier->verify($token));

            $timestampMaterial = $this->pathMaterial($path, $ocspTransport, $crlTransport, $now, $onSkip);

            // A bag member outside the TSA's path is still embedded, but no revocation
            // lookup can be built for it, so its URLs are reported instead.
            foreach ($unchained as $cert) {
                $this->reportUnchained($cert, $ocspTransport, $crlTransport, $onSkip);
            }

            // Appended rather than re-spread, which would copy the signer's own
            // material once per token.
            \array_push(
                $allCerts,
                ...$timestampMaterial['certs'],
                ...\array_map(static fn(array $cert): string => $cert['der'], $unchained),
            );
            \array_push($allOcsp, ...$timestampMaterial['ocsp']);
            \array_push($allCrls, ...$timestampMaterial['crls']);
        }

        return [
            'certs' => Certificate::deduplicate($allCerts),
            'ocsp' => Certificate::deduplicate($allOcsp),
            'crls' => Certificate::deduplicate($allCrls),
        ];
    }

    /**
     * Collect the material for one ordered certification path.
     *
     * @param list<array{pem: string, der: string}> $certs Leaf first, each followed by its issuer.
     * @param (callable(string, string): (string|false))|null $ocspTransport
     * @param (callable(string): (string|false))|null $crlTransport
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     *
     * @return array{certs: list<string>, ocsp: list<string>, crls: list<string>}
     *
     * @throws Exception If a certificate cannot be parsed.
     */
    private function pathMaterial(
        array $certs,
        ?callable $ocspTransport,
        ?callable $crlTransport,
        ?int $now,
        ?callable $onSkip,
    ): array {
        $ocsp = [];
        $crls = [];
        foreach ($certs as $idx => $cert) {
            $issuer = $certs[$idx + 1] ?? null;

            // A revoked verdict belongs to the certificate rather than to the source
            // that returned it, so a verdict from either source discards both.
            $revoked = false;
            $watch = static function (string $source, string $url, string $reason, SkipReason $code) use (
                $onSkip,
                &$revoked,
            ): void {
                if ($code === SkipReason::Revoked) {
                    $revoked = true;
                }

                if ($onSkip !== null) {
                    $onSkip($source, $url, $reason, $code);
                }
            };

            $certOcsp = [];
            if ($ocspTransport !== null) {
                $urls = $this->validationMaterial->certificateOcspUrls($cert['pem'], $watch);
                $certOcsp = $issuer === null
                    ? $this->notAttempted('ocsp', $urls, $cert, $watch)
                    : $this->validationMaterial->fetchOcsp(
                        $issuer['der'],
                        $cert['der'],
                        $urls,
                        $ocspTransport,
                        $now,
                        $watch,
                    );
            }

            $certCrls = [];
            if ($crlTransport !== null) {
                $urls = $this->validationMaterial->certificateCrlUrls($cert['pem'], $watch);
                $certCrls = $issuer === null
                    ? $this->notAttempted('crl', $urls, $cert, $watch)
                    : $this->validationMaterial->fetchCrl(
                        $urls,
                        $crlTransport,
                        $issuer['der'],
                        $cert['der'],
                        $now,
                        $watch,
                    );
            }

            // Both sources are consulted and every skip reported before the verdict
            // is read.
            if ($revoked) {
                continue;
            }

            $ocsp = [...$ocsp, ...$certOcsp];
            $crls = [...$crls, ...$certCrls];
        }

        return [
            'certs' => \array_map(static fn(array $cert): string => $cert['der'], $certs),
            'ocsp' => $ocsp,
            'crls' => $crls,
        ];
    }

    /**
     * Report the URLs of a certificate with no issuer in the path, and collect nothing.
     *
     * An OCSP request needs the issuer to build the CertID, and a CRL needs it to
     * check the signature. A self-signed root is the trust anchor and is not
     * reported.
     *
     * @param list<string>                  $urls
     * @param array{pem: string, der: string} $cert
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     *
     * @return list<string> Always empty; the shape lets the caller spread it.
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    private function notAttempted(string $source, array $urls, array $cert, ?callable $onSkip): array
    {
        if ($urls !== [] && !$this->isSelfSigned($cert['der'])) {
            $this->validationMaterial->reportNotAttempted(
                $source,
                $urls,
                'No issuer certificate in the chain to check the answer against',
                $onSkip,
            );
        }

        return [];
    }

    /**
     * Read the certificates a timestamp token embeds, deduplicated and bounded.
     *
     * Ordering a bag of n members costs n^2 signature checks, so a bag larger than
     * MAX_PATH_CERTIFICATES is refused before that work. The field sits outside
     * signedAttrs and is covered by no signature.
     *
     * @return list<string> DER certificates, in the order the bag carries them.
     *
     * @throws Exception If the token cannot be parsed, or the bag is too large to be
     *                   a certification path.
     */
    private function boundedTokenCertificates(string $tokenDer): array
    {
        $certs = Certificate::deduplicate($this->certificate->fromSignedData($tokenDer));
        if (\count($certs) > self::MAX_PATH_CERTIFICATES) {
            throw new Exception(
                'The timestamp token embeds more than ' . self::MAX_PATH_CERTIFICATES . ' certificates: '
                    . \count($certs),
            );
        }

        return $certs;
    }

    /**
     * Order an unordered certificate set into a leaf-first path from a known anchor.
     *
     * A CMS CertificateSet has no order, so the certificates a timestamp token
     * embeds arrive as a bag. The path starts at $anchorDer, and each next entry is
     * the one that issued the last, established by signature as assertOrderedChain()
     * does for the signer's own chain.
     *
     * Members that do not chain to the anchor are returned separately: they are
     * still collected as certificates, but no revocation lookup can be built for
     * them.
     *
     * @param list<string> $remaining Deduplicated bag members, as
     *                                boundedTokenCertificates() returns them.
     * @param string       $anchorDer DER of the certificate the token was verified against.
     *
     * @return array{list<array{pem: string, der: string}>, list<array{pem: string, der: string}>}
     *         [ordered path, unchained members]
     *
     * @throws Exception If a certificate cannot be parsed.
     */
    private function orderedPath(array $remaining, string $anchorDer): array
    {
        $path = [];
        $current = $anchorDer;
        while ($current !== null) {
            $path[] = ['pem' => Certificate::derToPem($current), 'der' => $current];

            $subject = $current;
            $remaining = \array_values(\array_filter($remaining, static fn(string $c): bool => $c !== $subject));
            $next = \array_values(\array_filter(
                $remaining,
                /** @throws Exception */
                fn(string $c): bool => $this->issued($c, $subject),
            ));

            $current = $next[0] ?? null;
        }

        $unchained = \array_map(static fn(string $cert): array => [
            'pem' => Certificate::derToPem($cert),
            'der' => $cert,
        ], $remaining);

        return [$path, $unchained];
    }

    /**
     * Report the revocation URLs of a bag member that chains to nothing.
     *
     * A source with no transport is not reported, nothing being fetched for it
     * either way.
     *
     * @param array{pem: string, der: string} $cert
     * @param (callable(string, string): (string|false))|null $ocspTransport
     * @param (callable(string): (string|false))|null $crlTransport
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    private function reportUnchained(
        array $cert,
        ?callable $ocspTransport,
        ?callable $crlTransport,
        ?callable $onSkip,
    ): void {
        if ($ocspTransport !== null) {
            $this->notAttempted(
                'ocsp',
                $this->validationMaterial->certificateOcspUrls($cert['pem'], $onSkip),
                $cert,
                $onSkip,
            );
        }

        if ($crlTransport !== null) {
            $this->notAttempted(
                'crl',
                $this->validationMaterial->certificateCrlUrls($cert['pem'], $onSkip),
                $cert,
                $onSkip,
            );
        }
    }

    /**
     * Verify that each chain entry is followed by the certificate that issued it.
     *
     * The issuer link is checked by signature rather than by Name.
     *
     * @param list<array{pem: string, der: string}> $certs
     *
     * @throws Exception If an entry is not issued by the one after it.
     */
    private function assertOrderedChain(array $certs): void
    {
        $previous = null;
        foreach ($certs as $idx => $cert) {
            if ($previous !== null && !$this->issued($cert['der'], $previous)) {
                throw new Exception(
                    'The certificate chain is not ordered leaf-first: entry '
                    . $idx
                    . ' did not issue entry '
                    . ($idx - 1),
                );
            }

            $previous = $cert['der'];
        }
    }

    /**
     * True when a certificate signed another.
     *
     * The signature is the whole of the test; the two Names are not compared, so an
     * authority that re-issued its own certificate with a Name of the same value in
     * another string type is still recognised.
     *
     * @throws Exception If either certificate cannot be parsed.
     */
    private function issued(string $issuerDer, string $subjectDer): bool
    {
        $issuerKey = \openssl_pkey_get_public(Certificate::derToPem($issuerDer));
        $issued = $issuerKey !== false && \openssl_x509_verify(Certificate::derToPem($subjectDer), $issuerKey) === 1;

        // Most calls here are expected to fail, this being asked of every pair in a
        // bag, so their queue entries are discarded.
        Certificate::clearOpenSslErrors();

        return $issued;
    }

    /**
     * True when a certificate signed itself, which is what a trust anchor is.
     *
     * Established by signature, as assertOrderedChain() establishes the issuer link.
     * RFC 5280 separates a self-issued certificate, whose subject and issuer Names
     * are equal, from a self-signed one, which its own key also signed; a CA key
     * rollover certificate and a cross-certificate are the first without being the
     * second.
     *
     * @throws Exception If the certificate cannot be parsed.
     */
    private function isSelfSigned(string $certDer): bool
    {
        return $this->issued($certDer, $certDer);
    }
}
