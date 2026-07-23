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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign\Ltv;

use Com\Tecnick\Pdf\Sign\Exception;
use Com\Tecnick\Pdf\Sign\Ocsp\Client as OcspClient;

/**
 * Com\Tecnick\Pdf\Sign\Ltv\ValidationMaterial
 *
 * Collects the long-term validation (LTV) material embedded in a PDF Document
 * Security Store (DSS): the certificate DERs, OCSP responses, and CRLs. URL
 * discovery uses the certificate AIA and CRL distribution point extensions;
 * network retrieval is delegated to injected transport callables so this class
 * stays testable and free of SSRF concerns. The VRI key (SHA-1 of the signature
 * Contents) is intentionally not computed here: it belongs to the DSS writer,
 * which holds the final signature bytes.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class ValidationMaterial
{
    private OcspClient $ocsp;

    public function __construct(?OcspClient $ocsp = null)
    {
        $this->ocsp = $ocsp ?? new OcspClient();
    }

    /**
     * Convert a list of PEM certificates to deduplicated DER strings.
     *
     * @param list<string> $certsPem
     *
     * @return list<string>
     *
     * @throws Exception If any certificate is not valid PEM.
     */
    public function certificates(array $certsPem): array
    {
        $seen = [];
        $result = [];
        foreach ($certsPem as $pem) {
            $der = $this->pemToDer($pem);
            $fingerprint = \hash('sha256', $der);
            if (isset($seen[$fingerprint])) {
                continue;
            }

            $seen[$fingerprint] = true;
            $result[] = $der;
        }

        return $result;
    }

    /**
     * Extract the OCSP responder URLs from a certificate's AIA extension.
     *
     * Returns an empty list when the certificate has no AIA extension or cannot be
     * parsed (LTV collection is best-effort; see extensionText).
     *
     * @return list<string>
     */
    public function certificateOcspUrls(string $certPem): array
    {
        return $this->extractUris($this->extensionText($certPem, 'authorityInfoAccess'), '~OCSP\s*-\s*URI:(\S+)~i');
    }

    /**
     * Extract the CRL distribution point URLs from a certificate.
     *
     * Returns an empty list when the certificate has no CRL distribution point or cannot
     * be parsed (LTV collection is best-effort; see extensionText).
     *
     * @return list<string>
     */
    public function certificateCrlUrls(string $certPem): array
    {
        return $this->extractUris($this->extensionText($certPem, 'crlDistributionPoints'), '~URI:(\S+)~');
    }

    /**
     * Fetch OCSP responses for a certificate from the given responder URLs.
     *
     * @param list<string> $urls
     * @param callable     $transport Receives (url, DER request) and returns the DER response.
     *
     * @return list<string> Deduplicated OCSP response bytes.
     *
     * @throws Exception If the OCSP request cannot be built.
     */
    public function fetchOcsp(string $issuerDer, string $leafDer, array $urls, callable $transport): array
    {
        if ($urls === []) {
            return [];
        }

        $request = $this->ocsp->build($issuerDer, $leafDer);

        return $this->fetchDeduplicated($urls, static fn(string $url): mixed => $transport($url, $request));
    }

    /**
     * Fetch CRLs from the given distribution point URLs.
     *
     * @param list<string> $urls
     * @param callable     $transport Receives (url) and returns the CRL bytes.
     *
     * @return list<string> Deduplicated CRL bytes.
     */
    public function fetchCrl(array $urls, callable $transport): array
    {
        return $this->fetchDeduplicated($urls, static fn(string $url): mixed => $transport($url));
    }

    /**
     * Fetch each URL through the callback, skipping failures and duplicates.
     *
     * @param list<string>             $urls
     * @param callable(string): mixed  $fetch
     *
     * @return list<string>
     */
    private function fetchDeduplicated(array $urls, callable $fetch): array
    {
        $seen = [];
        $result = [];
        foreach ($urls as $url) {
            try {
                /** @var mixed $data */
                $data = $fetch($url);
            } catch (\Throwable) {
                continue;
            }

            if (!\is_string($data) || $data === '') {
                continue;
            }

            $fingerprint = \hash('sha256', $data);
            if (isset($seen[$fingerprint])) {
                continue;
            }

            $seen[$fingerprint] = true;
            $result[] = $data;
        }

        return $result;
    }

    /**
     * Return the human-readable text of a named certificate extension.
     *
     * LTV material collection is best-effort: a certificate whose extensions cannot be
     * parsed (for example a legacy certificate with a negative serial that a strict
     * OpenSSL build rejects) yields no extension text, so no OCSP/CRL URLs are derived
     * from it, rather than aborting the whole signing operation. The certificate itself
     * is still embedded, since its DER bytes are obtained separately (see pemToDer).
     */
    private function extensionText(string $certPem, string $name): string
    {
        $info = \openssl_x509_parse($certPem);
        if (!\is_array($info)) {
            return '';
        }

        $extensions = $info['extensions'];
        /** @var mixed $value */
        $value = $extensions[$name] ?? '';

        return \is_string($value) ? $value : '';
    }

    /**
     * Extract and deduplicate the capture group 1 matches of a pattern.
     *
     * @return list<string>
     */
    private function extractUris(string $text, string $pattern): array
    {
        $matches = [];
        \preg_match_all($pattern, $text, $matches);

        return \array_values(\array_unique($matches[1] ?? []));
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
