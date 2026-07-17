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
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 *
 * This file is part of tc-lib-pdf-sign software library.
 */

namespace Com\Tecnick\Pdf\Sign;

/**
 * Com\Tecnick\Pdf\Sign\Config
 *
 * Immutable signature configuration value object. Captures the signing profile,
 * digest algorithm, and certification level, and derives the PDF /SubFilter
 * from the selected profile.
 *
 * @since     2026-07-15
 * @category  Library
 * @package   PdfSign
 * @author    Nicola Asuni <info@tecnick.com>
 * @copyright 2026 Nicola Asuni - Tecnick.com LTD
 * @license   https://www.gnu.org/copyleft/lesser.html GNU-LGPL v3 (see LICENSE.TXT)
 * @link      https://github.com/tecnickcom/tc-lib-pdf-sign
 */
final class Config
{
    /**
     * Legacy ISO 32000-1 signature (/SubFilter /adbe.pkcs7.detached).
     */
    public const PROFILE_LEGACY = 'legacy';

    /**
     * PAdES baseline B-B (CAdES-based, /SubFilter /ETSI.CAdES.detached).
     */
    public const PROFILE_PADES_B_B = 'pades-b-b';

    /**
     * PAdES baseline B-T (B-B plus a signature timestamp).
     */
    public const PROFILE_PADES_B_T = 'pades-b-t';

    /**
     * PAdES baseline B-LT (B-T plus a Document Security Store).
     */
    public const PROFILE_PADES_B_LT = 'pades-b-lt';

    /**
     * PAdES baseline B-LTA (B-LT plus a document timestamp).
     */
    public const PROFILE_PADES_B_LTA = 'pades-b-lta';

    /**
     * Supported signature profiles.
     *
     * @var list<string>
     */
    public const PROFILES = [
        self::PROFILE_LEGACY,
        self::PROFILE_PADES_B_B,
        self::PROFILE_PADES_B_T,
        self::PROFILE_PADES_B_LT,
        self::PROFILE_PADES_B_LTA,
    ];

    /**
     * Supported CMS digest algorithms.
     *
     * @var list<string>
     */
    public const DIGEST_ALGORITHMS = ['sha256', 'sha384', 'sha512'];

    /**
     * @param string $profile         One of the PROFILE_* constants.
     * @param string $digestAlgorithm One of the DIGEST_ALGORITHMS values.
     * @param int    $certType        Certification level (DocMDP P value):
     *                                0 = approval/UR signature,
     *                                1 = no changes permitted,
     *                                2 = form fill-in and signing permitted,
     *                                3 = as 2 plus annotation changes.
     *
     * @throws Exception If any option is invalid.
     */
    public function __construct(
        public readonly string $profile = self::PROFILE_LEGACY,
        public readonly string $digestAlgorithm = 'sha256',
        public readonly int $certType = 2,
    ) {
        if (!\in_array($profile, self::PROFILES, true)) {
            throw new Exception('Invalid signature profile: ' . $profile);
        }

        if (!\in_array($digestAlgorithm, self::DIGEST_ALGORITHMS, true)) {
            throw new Exception('Invalid digest algorithm: ' . $digestAlgorithm);
        }

        if ($certType < 0 || $certType > 3) {
            throw new Exception('Invalid certification level (cert_type): ' . $certType);
        }
    }

    /**
     * True when a PAdES profile is selected.
     */
    public function isPades(): bool
    {
        return $this->profile !== self::PROFILE_LEGACY;
    }

    /**
     * PDF /SubFilter value for the selected profile.
     */
    public function subFilter(): string
    {
        return $this->isPades() ? 'ETSI.CAdES.detached' : 'adbe.pkcs7.detached';
    }

    /**
     * Build a Config from the legacy associative-array shape used by
     * Tcpdf::setSignature(), for backward compatibility.
     *
     * @param array<string, mixed> $data Signature options.
     *
     * @throws Exception If any option is present but of the wrong type or value.
     */
    public static function fromArray(array $data): self
    {
        /** @var mixed $profile */
        $profile = $data['profile'] ?? self::PROFILE_LEGACY;
        if (!\is_string($profile)) {
            throw new Exception('Invalid signature profile');
        }

        /** @var mixed $digest */
        $digest = $data['digest_algorithm'] ?? 'sha256';
        if (!\is_string($digest)) {
            throw new Exception('Invalid digest algorithm');
        }

        /** @var mixed $certType */
        $certType = $data['cert_type'] ?? 2;
        if (!\is_int($certType)) {
            throw new Exception('Invalid certification level (cert_type)');
        }

        return new self($profile, $digest, $certType);
    }
}
