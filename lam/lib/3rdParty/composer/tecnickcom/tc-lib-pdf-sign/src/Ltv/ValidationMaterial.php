<?php

declare(strict_types=1);

/**
 * ValidationMaterial.php
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

namespace Com\Tecnick\Pdf\Sign\Ltv;

use Com\Tecnick\Pdf\Sign\Cms\Asn1;
use Com\Tecnick\Pdf\Sign\Cms\Certificate;
use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Ocsp\Client as OcspClient;
use Com\Tecnick\Pdf\Sign\Timestamp\Config as TimestampConfig;

/**
 * Com\Tecnick\Pdf\Sign\Ltv\ValidationMaterial
 *
 * Collects the long-term validation (LTV) material embedded in a PDF Document
 * Security Store (DSS): the certificate DERs, OCSP responses, and CRLs. URL
 * discovery decodes the certificate AIA and CRL distribution point extensions
 * from their DER rather than from OpenSSL's rendering of them. Network retrieval
 * is delegated to injected transport callables, so this class chooses the URL and
 * the host decides whether to fetch it and carries the SSRF question. The VRI key
 * (SHA-1 of the signature Contents) is not computed here: it belongs to the DSS
 * writer, which holds the final signature bytes.
 *
 * Collection is best-effort: a URL that cannot be reached, or that answers with
 * something the codecs reject, is skipped so the next one can be tried. Every skip
 * is reported to the optional $onSkip observer, with a SkipReason separating a
 * revoked verdict from an unreachable responder.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class ValidationMaterial
{
    /**
     * id-pe-authorityInfoAccess (RFC 5280 section 4.2.2.1).
     */
    private const OID_AUTHORITY_INFO_ACCESS = '1.3.6.1.5.5.7.1.1';

    /**
     * id-ad-ocsp, the accessMethod naming a responder.
     */
    private const OID_AD_OCSP = '1.3.6.1.5.5.7.48.1';

    /**
     * id-ce-cRLDistributionPoints (RFC 5280 section 4.2.1.13).
     */
    private const OID_CRL_DISTRIBUTION_POINTS = '2.5.29.31';

    /**
     * GeneralName ::= CHOICE { ... uniformResourceIdentifier [6] IMPLICIT IA5String ... }.
     */
    private const TAG_URI = 0x86;

    /**
     * Most revocation URLs taken from one certificate extension.
     *
     * Every URL becomes a call to the host's transport. The excess is reported
     * through $onSkip rather than dropped.
     */
    public const MAX_URLS = 8;

    private OcspClient $ocsp;

    private Crl $crl;

    private Certificate $certificate;

    private Asn1 $asn1;

    /**
     * @throws Exception If a default codec cannot be constructed.
     */
    public function __construct(
        ?OcspClient $ocsp = null,
        ?Crl $crl = null,
        ?Certificate $certificate = null,
        ?Asn1 $asn1 = null,
    ) {
        $this->ocsp = $ocsp ?? new OcspClient();
        $this->crl = $crl ?? new Crl();
        $this->asn1 = $asn1 ?? new Asn1();
        $this->certificate = $certificate ?? new Certificate($this->asn1);
    }

    /**
     * Convert a list of PEM certificates to deduplicated DER strings.
     *
     * Each entry is parsed as a certificate, not merely decoded.
     *
     * @param list<string> $certsPem
     *
     * @return list<string>
     *
     * @throws Exception If any entry is not a string, or is not one PEM certificate.
     */
    public function certificates(array $certsPem): array
    {
        $ders = [];
        /** @var mixed $pem */
        foreach ($certsPem as $index => $pem) {
            // As in Signer::collectValidationMaterial(): an entry that is not a string
            // would reach pemToDer() as a TypeError rather than an Exception.
            if (!\is_string($pem)) {
                throw new Exception('Invalid certificate ' . $index);
            }

            $der = Certificate::pemToDer($pem);
            $this->certificate->fields($der);
            $ders[] = $der;
        }

        return Certificate::deduplicate($ders);
    }

    /**
     * Extract the OCSP responder URLs from a certificate's AIA extension.
     *
     * Returns an empty list when the certificate has no AIA extension or cannot be
     * parsed (LTV collection is best-effort; see extensionMembers).
     *
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip Receives
     *                            every URL the caller will not be given: the ones past
     *                            MAX_URLS, and the ones whose scheme no HTTP transport
     *                            can be handed.
     *
     * @return list<string>
     */
    public function certificateOcspUrls(string $certPem, ?callable $onSkip = null): array
    {
        $urls = [];

        // AuthorityInfoAccessSyntax ::= SEQUENCE OF AccessDescription
        // AccessDescription ::= SEQUENCE { accessMethod OBJECT IDENTIFIER,
        //                                  accessLocation GeneralName }
        // RFC 5280 section 4.2.2.1: the responder is the accessLocation of the
        // description whose accessMethod is id-ad-ocsp, and of no other.
        foreach ($this->extensionMembers($certPem, self::OID_AUTHORITY_INFO_ACCESS) as $description) {
            try {
                $offset = 0;
                $method = $this->asn1->readTlv($description, $offset);
                if (
                    $method['tag'] !== 0x06
                    || $this->asn1->decodeObjectIdentifier($method['value']) !== self::OID_AD_OCSP
                ) {
                    continue;
                }

                $location = $this->asn1->readTlv($description, $offset);
                if ($location['tag'] === self::TAG_URI) {
                    $urls[] = $location['value'];
                }
            } catch (Exception) {
                // Best-effort: a member this reader cannot walk names no responder.
                continue;
            }
        }

        return $this->httpUrls('ocsp', $urls, $onSkip);
    }

    /**
     * Extract the CRL distribution point URLs from a certificate.
     *
     * Returns an empty list when the certificate has no CRL distribution point or cannot
     * be parsed (LTV collection is best-effort; see extensionMembers).
     *
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip Receives
     *                            every URL the caller will not be given: the ones past
     *                            MAX_URLS, and the ones whose scheme no HTTP transport
     *                            can be handed.
     *
     * @return list<string>
     */
    public function certificateCrlUrls(string $certPem, ?callable $onSkip = null): array
    {
        $urls = [];

        // CRLDistributionPoints ::= SEQUENCE OF DistributionPoint
        // DistributionPoint ::= SEQUENCE { distributionPoint [0] DistributionPointName
        //                                    OPTIONAL, reasons [1] OPTIONAL,
        //                                  cRLIssuer [2] OPTIONAL }
        // DistributionPointName ::= CHOICE { fullName [0] GeneralNames, ... }
        // RFC 5280 section 4.2.1.13. Only a fullName URI names a list to fetch; a
        // nameRelativeToCRLIssuer and a cRLIssuer name no location at all.
        foreach ($this->extensionMembers($certPem, self::OID_CRL_DISTRIBUTION_POINTS) as $point) {
            try {
                $offset = 0;
                if ($offset >= \strlen($point)) {
                    continue;
                }

                $name = $this->asn1->readTlv($point, $offset);
                if ($name['tag'] !== 0xA0) {
                    continue;
                }

                $nameOffset = 0;
                $fullName = $this->asn1->readTlv($name['value'], $nameOffset);
                if ($fullName['tag'] !== 0xA0) {
                    continue;
                }

                $generalNameOffset = 0;
                while ($generalNameOffset < \strlen($fullName['value'])) {
                    $generalName = $this->asn1->readTlv($fullName['value'], $generalNameOffset);
                    if ($generalName['tag'] === self::TAG_URI) {
                        $urls[] = $generalName['value'];
                    }
                }
            } catch (Exception) {
                // Best-effort: a point this reader cannot walk names no list.
                continue;
            }
        }

        return $this->httpUrls('crl', $urls, $onSkip);
    }

    /**
     * Fetch OCSP responses for a certificate from the given responder URLs.
     *
     * Every response is validated against the request before it is kept. A URL whose
     * response fails validation is skipped, like an unreachable one.
     *
     * @param list<string> $urls
     * @param callable     $transport Receives (url, DER request) and returns the DER response.
     * @param int|null     $now       Unix time the responses are checked against.
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip Receives
     *                                (source, url, reason, code) for every URL whose answer
     *                                was discarded.
     *
     * @return list<string> Deduplicated, validated OCSP response bytes.
     *
     * @throws Exception If the OCSP request cannot be built, or a URL is not a string.
     */
    public function fetchOcsp(
        string $issuerDer,
        string $leafDer,
        array $urls,
        callable $transport,
        ?int $now = null,
        ?callable $onSkip = null,
    ): array {
        if ($urls === []) {
            return [];
        }

        $request = $this->ocsp->build($issuerDer, $leafDer);
        $ocsp = $this->ocsp;

        return $this->fetchDeduplicated(
            'ocsp',
            $urls,
            /** @throws Exception */
            static function (string $url) use ($transport, $request, $ocsp, $now): mixed {
                /** @var mixed $response */
                $response = $transport($url, $request->der);

                return \is_string($response) ? $ocsp->parseResponse($response, $request, $now) : $response;
            },
            $onSkip,
        );
    }

    /**
     * Fetch CRLs from the given distribution point URLs.
     *
     * Every CRL is validated against the certificate that issued it before it is
     * kept, so the issuer is required.
     *
     * @param list<string> $urls
     * @param callable     $transport  Receives (url) and returns the CRL bytes.
     * @param string       $issuerDer  DER of the issuing certificate.
     * @param string|null  $subjectDer DER of the certificate the lists are fetched for, so a
     *                                 list that revokes it is reported rather than embedded,
     *                                 or null to skip the revocation lookup.
     * @param int|null     $now        Unix time the CRLs are checked against.
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip Receives
     *                                 (source, url, reason, code) for every URL whose answer
     *                                 was discarded.
     *
     * @return list<string> Deduplicated, validated CRL bytes.
     *
     * @throws Exception If a URL is not a string.
     */
    public function fetchCrl(
        array $urls,
        callable $transport,
        string $issuerDer,
        ?string $subjectDer,
        ?int $now = null,
        ?callable $onSkip = null,
    ): array {
        $crl = $this->crl;

        return $this->fetchDeduplicated(
            'crl',
            $urls,
            /** @throws Exception */
            static function (string $url) use ($transport, $crl, $issuerDer, $now, $subjectDer): mixed {
                /** @var mixed $data */
                $data = $transport($url);

                return \is_string($data) ? $crl->validate($data, $issuerDer, $subjectDer, $now) : $data;
            },
            $onSkip,
        );
    }

    /**
     * Report URLs that were not tried.
     *
     * @param list<string> $urls
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     *
     * @throws Exception If a URL is not a string.
     */
    public function reportNotAttempted(string $source, array $urls, string $reason, ?callable $onSkip): void
    {
        $this->reportUrls($source, $this->checkedUrls($urls, $source), $reason, $onSkip);
    }

    /**
     * Report a list this class built itself, which needs no member check.
     *
     * @param list<string> $urls
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     */
    private function reportUrls(string $source, array $urls, string $reason, ?callable $onSkip): void
    {
        foreach ($urls as $url) {
            $this->skip($onSkip, $source, $url, $reason, SkipReason::NotAttempted);
        }
    }

    /**
     * Hold a caller-supplied URL list to the shape its parameter declares.
     *
     * A member that is not a string would reach skip() as a TypeError rather than an
     * Exception, and fetchDeduplicated()'s own catch would not turn it back into a
     * skip.
     *
     * @param array<array-key, string> $urls
     *
     * @return list<string>
     *
     * @throws Exception If a member is not a string.
     */
    private function checkedUrls(array $urls, string $source): array
    {
        $checked = [];
        /** @var mixed $url */
        foreach ($urls as $index => $url) {
            if (!\is_string($url)) {
                throw new Exception('Invalid ' . $source . ' URL ' . $index);
            }

            $checked[] = $url;
        }

        return $checked;
    }

    /**
     * Fetch each URL through the callback, skipping failures and duplicates.
     *
     * A revoked verdict discards the material for the whole certificate rather than
     * for the URL that returned it, as Ocsp\Client::parseResponse() does among the
     * entries of one response and Ltv\Crl::validate() among the entries of one list.
     * The URL is still reported through $onSkip and the fetch still continues.
     *
     * @param list<string>             $urls
     * @param callable(string): mixed  $fetch
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     *
     * @return list<string> Empty when any URL answered that the certificate is revoked.
     *
     * @throws Exception If a URL is not a string.
     */
    private function fetchDeduplicated(string $source, array $urls, callable $fetch, ?callable $onSkip): array
    {
        $seen = [];
        $result = [];
        $revoked = false;
        foreach ($this->checkedUrls($urls, $source) as $url) {
            try {
                /** @var mixed $data */
                $data = $fetch($url);
            } catch (\Throwable $e) {
                $code = SkipReason::fromThrowable($e);
                $revoked = $revoked || $code === SkipReason::Revoked;
                $this->skip($onSkip, $source, $url, $e->getMessage(), $code);
                continue;
            }

            if (!\is_string($data) || $data === '') {
                $this->skip($onSkip, $source, $url, 'The transport returned no data', SkipReason::Unreachable);
                continue;
            }

            $fingerprint = \hash('sha256', $data);
            if (isset($seen[$fingerprint])) {
                $this->skip($onSkip, $source, $url, 'Duplicate of an earlier response', SkipReason::Duplicate);
                continue;
            }

            $seen[$fingerprint] = true;
            $result[] = $data;
        }

        return $revoked ? [] : $result;
    }

    /**
     * Report a discarded URL to the caller's observer, when there is one.
     *
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     */
    private function skip(?callable $onSkip, string $source, string $url, string $reason, SkipReason $code): void
    {
        if ($onSkip !== null) {
            $onSkip($source, $url, $reason, $code);
        }
    }

    /**
     * Read the members of a certificate extension that is a SEQUENCE OF.
     *
     * Both extensions read here have that shape, and both are decoded from the
     * certificate's own DER rather than from OpenSSL's rendering of it.
     *
     * Collection is best-effort: a certificate whose extensions cannot be parsed
     * yields no URLs rather than aborting the operation. The certificate itself is
     * still embedded, its DER being obtained separately.
     *
     * @return list<string> Content octets of each member, empty when the extension
     *                      is absent or unreadable.
     */
    private function extensionMembers(string $certPem, string $oid): array
    {
        try {
            $extensions = $this->certificate->extensions(Certificate::pemToDer($certPem));
            $extension = $extensions[$oid] ?? null;
            if ($extension === null) {
                return [];
            }

            $offset = 0;
            $sequence = $this->asn1->readTlv($extension['value'], $offset);
            if ($sequence['tag'] !== 0x30) {
                return [];
            }

            $members = [];
            $inner = 0;
            while ($inner < \strlen($sequence['value'])) {
                $member = $this->asn1->readTlv($sequence['value'], $inner);
                if ($member['tag'] === 0x30) {
                    $members[] = $member['value'];
                }
            }

            return $members;
        } catch (Exception) {
            return [];
        }
    }

    /**
     * Keep the URLs the injected transport can fetch, in first-seen order.
     *
     * Only http and https are returned; a distribution point may name an ldap:// or
     * file:// location, which an HTTP client cannot fetch. The pattern is the one
     * Timestamp\Config holds the TSA URL to, an IA5String admitting control
     * characters.
     *
     * The list is bounded at MAX_URLS. Every URL past the bound, and every URL the
     * transport cannot be handed, is reported through $onSkip rather than dropped.
     *
     * @param list<string> $urls
     * @param (callable(string, string, string, SkipReason): void)|null $onSkip
     *
     * @return list<string>
     */
    private function httpUrls(string $source, array $urls, ?callable $onSkip): array
    {
        $usable = [];
        $unusable = [];
        foreach (\array_values(\array_unique($urls)) as $url) {
            if (\preg_match(TimestampConfig::URL_PATTERN, $url) === 1) {
                $usable[] = $url;
                continue;
            }

            $unusable[] = $url;
        }

        $this->reportUrls(
            $source,
            $unusable,
            'The certificate names a revocation URL the transport cannot be given',
            $onSkip,
        );

        $this->reportUrls(
            $source,
            \array_slice($usable, self::MAX_URLS),
            'The certificate names more than ' . self::MAX_URLS . ' revocation URLs',
            $onSkip,
        );

        return \array_slice($usable, 0, self::MAX_URLS);
    }
}
